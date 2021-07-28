#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#define VK_LOG_CRITICAL(...) vk_engine::logger::getCoreLogger()->critical(__VA_ARGS__);
#define VK_LOG_ERROR(...)    vk_engine::logger::getCoreLogger()->error(__VA_ARGS__);
#define VK_LOG_INFO(...)     vk_engine::logger::getCoreLogger()->info(__VA_ARGS__);
#define VK_LOG_WARN(...)     vk_engine::logger::getCoreLogger()->warn(__VA_ARGS__);

namespace VkEngine
{

    class Logger
    {
    public:

        static void init()
        {
            _corelogger = spdlog::stdout_color_mt("core");
            // _clientlogger = spdlog::rotating_logger_mt("client", "logs", 1048576 * 5, 3);
        };

        static std::shared_ptr<spdlog::logger> getCoreLogger() { return _coreLogger; };
        static std::shared_ptr<spdlog::logger> getClientLogger() { return _clientLogger; };

    private:

        static std::shared_ptr<spdlog::logger> _coreLogger;
        static std::shared_ptr<spdlog::logger> _clientLogger;
    };
}