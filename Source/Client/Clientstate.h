/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-7
    Notes:
        Information about the client which is modified elsewhere.
*/

#pragma once
#include <cstdint>
#include <string>

struct Client_t
{
    bool Authed;
    bool Offline;

    size_t Socket;

    uint32_t AppID;
    uint32_t UserID;
    uint32_t Expiration;

    std::string Email;
    std::string Username;
    std::string Userticket;
    std::string Lobbyaddress;
};

namespace Client
{
    // Authentication to the auth server.
    bool Authenticate(std::string &Password, Client_t &State);
    bool isAuthenticated(Client_t &State);

    // Remain authenticated via the lobby server.
    void Lobbycallback(size_t Socket, Networking::NetEvent Event, std::string Data);
    bool Remauth(Client_t &State);
}
