#pragma once

#include <cstdint>
#include <iostream>

#ifndef ABY_DBG_BREAK
#	ifdef _MSC_VER
#		define ABY_DBG_BREAK() __debugbreak()
#	elif defined(__has_builtin)
#		if __has_builtin(__builtin_debugtrap)
#			define ABY_DBG_BREAK() __builtin_debugtrap()
#		elif __has_builtin(__builtin_trap)
#			define ABY_DBG_BREAK() __builtin_trap()
#		endif
#	elif defined(POSIX)
extern "C" int raise(int sig);
#		define ABY_DBG_BREAK() raise(SIGTRAP)
#	elif defined(_WIN32)
extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#		define ABY_DBG_BREAK() DebugBreak()
#	else
#		define ABY_DBG_BREAK()   \
			std::system("pause"); \
			std::abort(-1);
#	endif
#endif

#define FT_ASSERT(cond, fmt, ...)                                                                         \
	do {                                                                                                  \
		if (!(cond)) {                                                                                    \
			std::cerr << std::format("[freetype] [assert] " fmt __VA_OPT__(, ) __VA_ARGS__) << std::endl; \
			ABY_DBG_BREAK();                                                                              \
		}                                                                                                 \
	} while (0);
#define FT_CHECK(err)                                                        \
	do {                                                                     \
		FT_ASSERT(err == FT_Error(0), "({}): {}", err, FT_Error_String(err)) \
	} while (0);
#define FT_ERROR(fmt, ...)                                                              \
	do {                                                                                \
		std::cerr << std::format("[freetype] [error] " fmt __VA_OPT__(, ) __VA_ARGS__) << std::endl; \
	} while (0);
#define FT_WARN(fmt, ...)                                                              \
	do {                                                                               \
		std::cerr << std::format("[freetype] [warn] " fmt __VA_OPT__(, ) __VA_ARGS__) << std::endl; \
	} while (0);
#define FT_STATUS(fmt, ...)                                                              \
	do {                                                                                 \
		std::cout << std::format("[freetype] [status] " fmt __VA_OPT__(, ) __VA_ARGS__) << std::endl; \
	} while (0);

namespace aby::ft {

	using u32 = std::uint32_t;
	using i64 = std::int64_t;

} // namespace aby::ft
