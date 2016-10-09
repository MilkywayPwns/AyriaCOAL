/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-9
    Notes:
        Websocket over Fossa using callbacks.
*/

#pragma once
#include <functional>
#include <cstdint>
#include <string>

namespace Networking
{
    enum class NetEvent
    {
        DISCONNECT,
        CONNECT,
        DATA,
    };

    using NetCallback = std::function<void(size_t Socket, NetEvent Event, std::string Data)>;

    void Subscribe(size_t Socket, NetCallback Callback);
    void Publish(size_t Socket, std::string Data);

    size_t Connect(std::string Hostname);
    void onConnect(NetCallback Callback);
}
