#include "log/logmanager.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

// 静态成员初始化
std::unordered_map<std::string, LogManager::LoggerConfig> LogManager::configs_;
std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> LogManager::loggers_;
std::mutex LogManager::mutex_;
std::string LogManager::config_path_;

void LogManager::Initialize(const std::string& config_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_path_ = config_path;
    LoadConfig(config_path);
}

void LogManager::LoadConfig(const std::string& config_path) {
    try {
        YAML::Node config = YAML::LoadFile(config_path);
        std::string base_dir = config["base_dir"].as<std::string>("/root/app/logs");
        spdlog::set_pattern("[%H:%M:%S] [%n] [%l] %v");  // 设置日志格式
        spdlog::flush_every(std::chrono::seconds(3)); //每3s写入一次日志        
        for (const auto& node : config["loggers"]) {
            std::string name = node.first.as<std::string>();
            LoggerConfig cfg;
            
            // 构建完整路径
            std::string rel_path = node.second["file"].as<std::string>();
            cfg.path = (fs::path(base_dir) / rel_path).string();
            
            // 创建目录
            fs::create_directories(fs::path(cfg.path).parent_path());
            
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
            
            configs_[name] = cfg;
        }

    } catch (const std::exception& e) {
        spdlog::error("Failed to load log config: {}", e.what());
        throw;
    }
}

std::shared_ptr<spdlog::logger> LogManager::CreateLogger(
    const std::string& name, 
    const LoggerConfig& config) 
{
    try {
        std::shared_ptr<spdlog::logger> logger;
        
        switch (config.rotation) {
            case RotationMode::SIZE:
                logger = spdlog::rotating_logger_mt(
                    name, config.path, config.max_size, 3);
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
        
        logger->set_level(config.level);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%n][%l] %v");
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
    
    // 检查配置是否存在
    if (configs_.find(name) == configs_.end()) {
        throw std::invalid_argument("Logger config not found: " + name);
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