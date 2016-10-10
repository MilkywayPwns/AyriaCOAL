/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-10
    Notes:
        Listens on new connections for messages.
*/

#include "../STDInclude.h"
#include <nlohmann/json.hpp>

struct Lobbyclient_t
{
    bool Verified;
    size_t Socket;
    Ticket_t Userticket;
};

// Listen on incoming packets.
static std::unordered_map<size_t, Lobbyclient_t> Lobbysockets;
static void Socketlistener(size_t Socket, Networking::NetEvent Event, std::string Data)
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
