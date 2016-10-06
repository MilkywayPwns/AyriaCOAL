/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: Apache 2.0
    Started: 2016-10-6
    Notes:
        A simple system to log messages to disk and stdout.
        The logfile is stored in LOGFILEDIR\\MODULENAME.log.
*/

#pragma once
#include <cstdint>

namespace COAL
{
    void DeleteLogfile();
    void AppendLogfileTimestamp(const char *Message);
    void AppendLogfile(const char *Message, const char *Prefix = nullptr);    
}
