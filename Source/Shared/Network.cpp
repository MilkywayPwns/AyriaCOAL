/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-9
    Notes:
        Websocket over Fossa using callbacks.
*/

#include <vector>
#include <thread>
#include "Network.h"
#include <fossa/fossa.h>
#include <unordered_map>
#include "../STDInclude.h"

// Callback buffers.
static std::vector<Networking::NetCallback> Connectcallbacks;
static std::unordered_map<size_t, std::vector<Networking::NetCallback>> Socketcallbacks;

// Internal functionality.
static ns_mgr Manager;
static void InternalEventhandler(ns_connection *Connection, int EventID, void *Eventdata)
{
    websocket_message *Message = (struct websocket_message *)Eventdata;
    size_t Socket = size_t(Connection);

    switch (EventID)
    {
        case NS_WEBSOCKET_FRAME:
        {
            // Post this to all subscribers.
            for (auto &S : Socketcallbacks[Socket])
            {
                // Create the callback in a new thread.
                std::thread(S, Socket, Networking::NetEvent::DATA, std::string((char *)Message->data, Message->size)).detach();
            }
            break;
        }
        case NS_WEBSOCKET_HANDSHAKE_DONE:
        {
            // Post this to all subscribers.
            for (auto &S : Connectcallbacks)
            {
                // Create the callback in a new thread.
                std::thread(S, Socket, Networking::NetEvent::CONNECT, "").detach();
            }
            break;
        }
        case NS_CLOSE:
        {
            // Post this to all subscribers.
            for (auto &S : Socketcallbacks[Socket])
            {
                // Create the callback in a new thread.
                std::thread(S, Socket, Networking::NetEvent::DISCONNECT, "").detach();
            }

            // Remove all subscribers.
            Socketcallbacks[Socket].clear();
            Socketcallbacks.erase(Socket);
            break;
        }
    }
}

// Initialize the manager on startup.
struct Networkinitializer
{
    Networkinitializer()
    {
        ns_mgr_init(&Manager, NULL);

#ifndef COAL_CLIENT
        auto Connection = ns_bind(&Manager, COAL_PORT, InternalEventhandler);
        ns_set_protocol_http_websocket(Connection);
#endif

		// Database loading
#ifdef COAL_CLIENT
		Database::Load("client.db");
#elif defined COAL_AUTH
		Database::Load("auth.db");
#elif defined COAL_LOBBY
		Database::Load("lobby.db");
#endif

        std::thread([]() { while (true) ns_mgr_poll(&Manager, 100); }).detach();
    }
};
static Networkinitializer Networkloader;

void Networking::Subscribe(size_t Socket, NetCallback Callback)
{
    if(Socket != NULL)
        Socketcallbacks[Socket].push_back(Callback);
}
void Networking::Publish(size_t Socket, std::string Data)
{
    if(Socket != NULL)
        ns_send_websocket_frame((ns_connection *)Socket, WEBSOCKET_OP_TEXT, Data.data(), Data.size());
}

size_t Networking::Connect(std::string Hostname)
{
    auto Connection = ns_connect(&Manager, Hostname.c_str(), InternalEventhandler);
    ns_set_protocol_http_websocket(Connection);
    return size_t(Connection);
}
void Networking::onConnect(NetCallback Callback)
{
#ifndef COAL_CLIENT
    Connectcallbacks.push_back(Callback);
#endif
}
