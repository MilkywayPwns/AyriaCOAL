/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: Apache 2.0
    Started: 2016-10-6
    Notes:
        C style preprocessor macros for common functions.
*/

#pragma once
#include "Defines.h"
#include "../Utility/Strings/Debugstring.h"
#include "../Utility/Strings/Variadicstring.h"

// Debug information.
#ifdef NDEBUG
#define PrintFunction()
#define DebugPrint(string)
#define InfoPrint(string)
#define VAPrint(format, ...)
#else
#define PrintFunction() COAL::AppendLogfile(__FUNCTION__, "Call to")
#define DebugPrint(string) COAL::AppendLogfile(string, "Debug")
#define InfoPrint(string) COAL::AppendLogfile(string, "Info")
#define VAPrint(format, ...) COAL::AppendLogfileTimestamp(COAL::va(format, __VA_ARGS__))
#endif
