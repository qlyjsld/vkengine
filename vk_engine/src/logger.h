#pragma once
#include "spdlog/spdlog.h"

#define SPDLOG_CRITICAL(message) spdlog::critical(message);
#define SPDLOG_ERROR(message) spdlog::error(message);
#define SPDLOG_INFO(message) spdlog::info(message);
#define SPDLOG_WARN(message) spdlog::warn(message);
