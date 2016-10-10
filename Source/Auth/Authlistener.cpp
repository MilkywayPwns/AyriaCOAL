/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-10
    Notes:
        Listens on new connections for messages.
*/

#include "Authheader.h"
#include <fossa/fossa.h>
#include "../STDInclude.h"
#include <nlohmann/json.hpp>

// Connections that we manage.
static std::unordered_map<size_t, Connection_t *> Connectedsockets;

// Callback on network events.
static void Socketlistener(size_t Socket, Networking::NetEvent Event, std::string Data)
{
    if (Event == Networking::NetEvent::DATA)
    {
        try
        {
            auto Response = nlohmann::json::parse(Data.c_str());
            Services::Call(Response["Service"], Connectedsockets[Socket], Data);
        }
        catch (std::exception &e) 
        {
            VAPrint("%s error: %s", __FUNCTION__, e.what()); 
        }

        return;
    }

    if (Event == Networking::NetEvent::CONNECT)
    {
        Connectedsockets[Socket] = new Connection_t();
        Connectedsockets[Socket]->Socket = Socket;
        return;
    }

    if (Event == Networking::NetEvent::DISCONNECT)
    {
        Connectedsockets.erase(Socket);
        return;
    }
}

// On a new connection, install the listener.
static void Connectionlistener(size_t Socket, Networking::NetEvent Event, std::string Data)
{
    Networking::Subscribe(Socket, Socketlistener);
}

// Move the socket into the permanent list.
void Auth::Upgradesocket(size_t Socket, Lobby_t *State)
{
    delete Connectedsockets[Socket];
    Connectedsockets[Socket] = State;
}

// Find a lobby server for the client.
Lobby_t *Auth::Findserver(Connection_t Client)
{
    for (auto &C : Connectedsockets)
    {
        if (C.second->Version == 0) continue;

        auto Lobby = (Lobby_t *)C.second;
        if (Lobby->CPULoad > 80) continue;
        if (Lobby->RAMLoad > 80) continue;
        if (Lobby->NETLoad > 80) continue;
        
        return Lobby;
    }

    return nullptr;
}

#ifdef COAL_AUTH
struct Authloader
{
    Authloader()
    {
        Networking::onConnect(Connectionlistener);
    }
};
static Authloader Loader;
#endif
