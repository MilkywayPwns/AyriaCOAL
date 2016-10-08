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
#include <functional>

namespace HTTPClient
{
    namespace Sync
    {
        // Returns the body, with or without the result code.
        std::string GET(std::string Hostname, std::string Resource);
        uint32_t GET(std::string Hostname, std::string Resource, std::string *Body);
        std::string POST(std::string Hostname, std::string Resource, std::string Data);
        uint32_t POST(std::string Hostname, std::string Resource, std::string Data, std::string *Body);
    }

    namespace Async
    {
        using HTTPCallback = std::function<void(uint32_t Resultcode, std::string Resultbody, std::string Additionaldata)>;

        // Calls the callback when finished.
        void GET(std::string Hostname, std::string Resource, HTTPCallback Callback);
        void POST(std::string Hostname, std::string Resource, HTTPCallback Callback, std::string Data);
    }
}
