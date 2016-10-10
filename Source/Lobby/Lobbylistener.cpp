/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-10
    Notes:
        Listens on new connections for messages.
*/

#include "Lobbyheader.h"
#include "../STDInclude.h"
#include <nlohmann/json.hpp>

// Connected clients that we manage.
static std::unordered_map<size_t, Lobbyclient_t> Lobbysockets;

// Callback on network events.
static void Socketlistener(size_t Socket, Networking::NetEvent Event, std::string Data)
{
    if (Event == Networking::NetEvent::DATA)
    {
        try
        {
            auto Response = nlohmann::json::parse(Data.c_str());
            Services::Call(Response["Service"], &Lobbysockets[Socket], Data);
        }
        catch (std::exception &e) 
        {
            VAPrint("%s error: %s", __FUNCTION__, e.what()); 
        }

        return;
    }

    if (Event == Networking::NetEvent::CONNECT)
    {
        Lobbysockets[Socket].Socket = Socket;
        Lobbysockets[Socket].Authenticated = false;
        return;
    }

    if (Event == Networking::NetEvent::DISCONNECT)
    {
        Lobbysockets.erase(Socket);
        return;
    }
}

// On a new connection, install the listener.
static void Connectionlistener(size_t Socket, Networking::NetEvent Event, std::string Data)
{
    Networking::Subscribe(Socket, Socketlistener);
}

struct Lobbyloader
{
    Lobbyloader()
    {
        Networking::onConnect(Connectionlistener);
    }
};
static Lobbyloader Loader;
