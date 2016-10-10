/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-10
    Notes:
        Listens on new connections for messages.
*/

#include "Clientheader.h"
#include "../STDInclude.h"
#include <nlohmann/json.hpp>

// Connections that we manage.
static std::unordered_map<size_t, Client_t *> Connectedsockets;

// Get the default Client_t struct.
Client_t *Client::GetClient()
{
    static Client_t Default;
    return &Default;
}

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
        Connectedsockets[Socket] = Client::GetClient();
        Connectedsockets[Socket]->Socket = Socket;
        return;
    }

    if (Event == Networking::NetEvent::DISCONNECT)
    {
        Connectedsockets[Socket]->Socket = NULL;
        Connectedsockets.erase(Socket);
        return;
    }
}

// On a new connection, install the listener.
static void Connectionlistener(size_t Socket, Networking::NetEvent Event, std::string Data)
{
    Networking::Subscribe(Socket, Socketlistener);
}

#ifdef COAL_CLIENT
struct Clientloader
{
    Clientloader()
    {
        Networking::onConnect(Connectionlistener);
    }
};
static Clientloader Loader;
#endif
