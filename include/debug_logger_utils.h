#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "glm/gtx/string_cast.hpp"

enum IVRDebugLevel {
	Success,
	Info,
	Warning,
	Error
};

//uses termcolor to do some color coded logging
class IVRDebugLogger {

private:
	static std::shared_ptr<spdlog::logger> Logger_;

public:
	static void InitLogger()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		Logger_ = spdlog::stdout_color_mt("IVR");
	}

	static std::shared_ptr<spdlog::logger>& GetLogger() { return Logger_; }

};

#define IVR_LOG_ERROR(...) IVRDebugLogger::GetLogger()->error(__VA_ARGS__)
#define IVR_LOG_WARNING(...) IVRDebugLogger::GetLogger()->warn(__VA_ARGS__)
#define IVR_LOG_INFO(...) IVRDebugLogger::GetLogger()->info(__VA_ARGS__)
