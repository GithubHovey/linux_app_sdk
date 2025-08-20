#include "log/logmanager.h"
#include <iostream>
#include <algorithm>
#include <sys/stat.h> // 用于mkdir
#include <unistd.h>   // 用于access
#include <string.h>   // 用于strerror
#include <dirent.h>   // 用于目录操作

// 静态成员初始化
std::unordered_map<std::string, LogManager::LoggerConfig> LogManager::configs_;
std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> LogManager::loggers_;
std::mutex LogManager::mutex_;
std::string LogManager::config_path_;

// 辅助函数：创建目录
static bool create_directories(const std::string& path) {
    size_t pos = 0;
    std::string dir;
    int mdret;
    
    if(path[path.size()-1] != '/') {
        dir = path + "/";
    } else {
        dir = path;
    }
    
    while((pos = dir.find_first_of('/', pos)) != std::string::npos) {
        std::string subdir = dir.substr(0, pos++);
        if(subdir.empty()) continue; // 忽略开头的/
        
        if(access(subdir.c_str(), F_OK) != 0) {
            mdret = mkdir(subdir.c_str(), 0755);
            if(mdret != 0) {
                spdlog::error("Failed to create directory {}: {}", subdir, strerror(errno));
                return false;
            }
        }
    }
    return true;
}

// 辅助函数：获取父目录
static std::string parent_path(const std::string& path) {
    size_t pos = path.find_last_of('/');
    if(pos == std::string::npos) return "";
    return path.substr(0, pos);
}

void LogManager::Initialize(const std::string& config_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_path_ = config_path;
    std::cout << "log_path:" << config_path << std::endl;
    LoadConfig(config_path);
}

void LogManager::LoadConfig(const std::string& config_path) {
    try {
        YAML::Node config = YAML::LoadFile(config_path);
        std::string base_dir = config["base_dir"].as<std::string>("/userdata/app/logs");
        // spdlog::set_pattern("[%n] [%l] %v");
        // spdlog::flush_every(std::chrono::seconds(1));
        
        for (const auto& node : config["loggers"]) {
            std::string name = node.first.as<std::string>();
            LoggerConfig cfg;
            
            // 构建完整路径
            std::string rel_path = node.second["file"].as<std::string>();
            if(!base_dir.empty() && base_dir.back() != '/') {
                base_dir += '/';
            }
            cfg.path = base_dir + rel_path;
            
            // 创建目录
            std::string dir_path = parent_path(cfg.path);
            if (!dir_path.empty() && access(dir_path.c_str(), F_OK) != 0) {
                if(!create_directories(dir_path)) {
                    throw std::runtime_error("Failed to create log directory: " + dir_path);
                }
            }
            
            // 解析大小 (支持KB/MB/GB)
            if (node.second["max_size"]) {
                std::string size_str = node.second["max_size"].as<std::string>();
                size_t multiplier = 1;
                if (size_str.find("KB") != std::string::npos) {
                    multiplier = 1024;
                } else if (size_str.find("MB") != std::string::npos) {
                    multiplier = 1024 * 1024;
                } else if (size_str.find("GB") != std::string::npos) {
                    multiplier = 1024 * 1024 * 1024;
                }
                cfg.max_size = std::stoul(size_str) * multiplier;
                if (cfg.max_size == 0) {
                    spdlog::error("Invalid max_size (0) for logger: {}", name);
                    cfg.max_size = 10 * 1024 * 1024; // 默认10MB
                }
            }
            
            // 解析其他参数
            if (node.second["max_days"]) {
                cfg.max_days = node.second["max_days"].as<int>();
            }
            
            if (node.second["level"]) {
                std::string level = node.second["level"].as<std::string>();
                std::transform(level.begin(), level.end(), level.begin(), ::tolower);
                if (level == "trace") cfg.level = spdlog::level::trace;
                else if (level == "debug") cfg.level = spdlog::level::debug;
                else if (level == "info") cfg.level = spdlog::level::info;
                else if (level == "warn") cfg.level = spdlog::level::warn;
                else if (level == "error") cfg.level = spdlog::level::err;
                else if (level == "critical") cfg.level = spdlog::level::critical;
            }
            
            // 解析分割模式
            if (node.second["rotation"]) {
                std::string mode = node.second["rotation"].as<std::string>();
                if (mode == "size") cfg.rotation = RotationMode::SIZE;
                else if (mode == "daily") cfg.rotation = RotationMode::DAILY;
                else if (mode == "session") cfg.rotation = RotationMode::SESSION;
                else if (mode == "none") cfg.rotation = RotationMode::NONE;
            }
            
            if (node.second["console"]) {
                cfg.console = node.second["console"].as<bool>();
            }
            configs_[name] = cfg;
        }
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to load log config: {}", e.what());
        throw;
    }
}

// 其余函数保持不变...
std::shared_ptr<spdlog::logger> LogManager::CreateLogger(
    const std::string& name, 
    const LoggerConfig& config) 
{
    try {
        std::shared_ptr<spdlog::logger> logger;
        std::vector<spdlog::sink_ptr> sinks;

        switch (config.rotation) {
            case RotationMode::SIZE:
                if (config.max_size == 0) {
                    spdlog::error("Invalid max_size (0) for rotating logger: {}", name);
                    logger = spdlog::basic_logger_mt(name, config.path); // 降级为普通logger
                } else {
                    logger = spdlog::rotating_logger_mt(
                        name, config.path, config.max_size, 3);
                }
                if (config.max_size > 0) {
                    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                        config.path, config.max_size, 3));
                }
                break;
                
            case RotationMode::DAILY:
                logger = spdlog::daily_logger_mt(
                    name, config.path, 0, 0, false, config.max_days);
                break;
                
            case RotationMode::SESSION: {
                auto now = std::chrono::system_clock::now();
                auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count();
                std::string session_path = fmt::format("{}.{}", config.path, timestamp);
                logger = spdlog::basic_logger_mt(name, session_path);
                break;
            }
                
            case RotationMode::NONE:
            default:
                logger = spdlog::basic_logger_mt(name, config.path);
                break;
        }
        
        if (config.console) {
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }

        if (sinks.empty()) {
            throw spdlog::spdlog_ex("No valid sinks configured");
        }
        logger = std::make_shared<spdlog::logger>(name, begin(sinks), end(sinks));
        logger->set_level(config.level);
        logger->set_pattern("[%H:%M:%S.%e][%n][%l] %v");
        logger->flush_on(spdlog::level::info);  
        return logger;
        
    } catch (const spdlog::spdlog_ex& ex) {
        spdlog::error("Logger creation failed: {}", ex.what());
        throw;
    }
}

std::shared_ptr<spdlog::logger> LogManager::GetLogger(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 已存在则直接返回
    if (loggers_.find(name) != loggers_.end()) {
        return loggers_[name];
    }
    
    // 检查配置是否存在，如果不存在则直接退出程序
    if (configs_.find(name) == configs_.end()) {
        std::cerr << "FATAL: Logger config not found: " << name << std::endl;
        std::exit(EXIT_FAILURE);  // 或 std::abort();
    }
    
    // 创建新logger
    auto logger = CreateLogger(name, configs_[name]);
    loggers_[name] = logger;
    return logger;
}

void LogManager::Reload() {
    std::lock_guard<std::mutex> lock(mutex_);
    loggers_.clear();  // 清除现有logger
    LoadConfig(config_path_);
}

void LogManager::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::drop_all();
    loggers_.clear();
}