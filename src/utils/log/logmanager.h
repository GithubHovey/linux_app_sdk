#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <unordered_map>
#include <mutex>

enum class RotationMode {
    SIZE,       // 按大小分割
    DAILY,      // 每日分割
    SESSION,    // 每次启动新文件
    NONE        // 不分割
};

class LogManager {
public:
    // 初始化全局配置
    static void Initialize(const std::string& config_path);
    
    // 获取logger实例
    static std::shared_ptr<spdlog::logger> GetLogger(const std::string& name);
    
    // 重新加载配置
    static void Reload();
    
    // 关闭所有logger
    static void Shutdown();

private:
    struct LoggerConfig {
        std::string path;
        size_t max_size = 0;
        int max_days = 0;
        spdlog::level::level_enum level = spdlog::level::info;
        RotationMode rotation = RotationMode::SIZE;
    };

    static void LoadConfig(const std::string& config_path);
    static std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name, const LoggerConfig& config);

    static std::unordered_map<std::string, LoggerConfig> configs_;
    static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
    static std::mutex mutex_;
    static std::string config_path_;
};