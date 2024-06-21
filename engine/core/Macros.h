#pragma once

#if defined(__APPLE__)

#include <TargetConditionals.h>

#if TARGET_OS_MAC
#define VISION_MACOS
#endif

#endif // __APPLE__

#if defined(_WIN32)
#define VISION_WINDOWS
#endif // _WIN32
