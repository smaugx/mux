#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#define MUX_DEBUG(fmt, ...)    spdlog::debug(fmt,     ## __VA_ARGS__);
#define MUX_INFO(fmt, ...)     spdlog::info(fmt,      ## __VA_ARGS__);
#define MUX_WARN(fmt, ...)     spdlog::warn(fmt,      ## __VA_ARGS__);
#define MUX_ERROR(fmt, ...)    spdlog::error(fmt,     ## __VA_ARGS__);
#define MUX_CRI(fmt, ...)      spdlog::critical(fmt,  ## __VA_ARGS__);
