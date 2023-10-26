#pragma once
#include <loguru.hpp>

#include "Exception.h"

#include <vector>
#include <string>
#include <iostream>
#if defined(LOGGING) || defined(_DEBUG)
	#define ENABLE_LOGGING 1
#else
	#define ENABLE_LOGGING 0
#endif

#if ENABLE_LOGGING
	#define InfoLog(...)		LOG_F(INFO, __VA_ARGS__); \
                                std::cout << loguru::textprintf(__VA_ARGS__).c_str() << std::endl
	#define Warning(...)		LOG_F(WARNING, __VA_ARGS__); \
                                std::cout << loguru::textprintf(__VA_ARGS__).c_str() << std::endl
	#define ErrorNoThrow(...)	LOG_F(ERROR, __VA_ARGS__);; \
                                std::cout << loguru::textprintf(__VA_ARGS__).c_str() << std::endl
	#define Error(...)			LOG_F(ERROR, __VA_ARGS__); \
								throw EXCEPTION(loguru::textprintf(__VA_ARGS__).c_str()); \
								std::cout << loguru::textprintf(__VA_ARGS__).c_str() << std::endl
#else
	#define InfoLog(...)
	#define Warning(...)
	#define ErrorNoThrow(...)	
	#define Error(...)			throw EXCEPTION(loguru::textprintf(__VA_ARGS__).c_str())
#endif

#if !defined(NDEBUG)
	#define CheckVK(exp)		if ((exp) != VK_SUCCESS) Error("Vulkan error occurred.")
#else
	#define CheckVK(exp)		exp
#endif

/*
 * Attributes
 */

#define UNUSED(x) (void)(x)

// Set loguru configurations
void LogsInit();

// Read binary file
void ReadFile(const std::string& filename, std::vector<char>& buffer);

