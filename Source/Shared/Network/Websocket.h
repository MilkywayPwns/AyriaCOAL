/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-9
    Notes:
        Unlike the HTTP implementation, WS needs a socket var.
*/

#pragma once
#include <functional>
#include <cstdint>
#include <string>

// Manager for all WS connections.
namespace Websocket
{
    // Initialize the connection.
    void Disconnect(size_t Socket);
    size_t Connect(std::string Hostname);

    // Enqueue or dequeue a frame.
    std::string Read(size_t Socket);
    void Write(std::string Data, size_t Socket);

    // Callback on incoming data..
    using WSCallback = std::function<void(size_t Socket, int EventID, std::string Data)>;
    void Subscribe(size_t Socket, WSCallback Callback);
}
