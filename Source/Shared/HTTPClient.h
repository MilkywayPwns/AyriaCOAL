/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-8
    Notes:
        Connect to a server and send a message.
*/

#pragma once
#include <cstdint>
#include <string>

namespace HTTPClient
{
    using HTTPCallback = void(__cdecl *)(std::string Result);

    // PostData ? POST : GET
    std::string RequestSync(std::string URL, bool Includeheaders = true, std::string *PostData = nullptr);
    void RequestAsync(std::string URL, HTTPCallback *CB, bool Includeheaders = true, std::string *PostData = nullptr);
}
