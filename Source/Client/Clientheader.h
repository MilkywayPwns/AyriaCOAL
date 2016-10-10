/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-10
    Notes:
        Contains all information other modules need.
*/

#pragma once
#include <cstdint>
#include <string>

struct Client_t
{
    bool Offline;
    size_t Socket;
    uint32_t UserID;
    bool Authenticated;
    uint32_t ApplicationID;
    uint32_t Ticketexpiration;

    std::string Email;
    std::string Username;
    std::string Userticket;
    std::string Lobbyaddress;
};

namespace Client
{
    // Get the default Client_t struct.
    Client_t *GetClient();

    // Authenticate the client to the network.
    bool Authenticate(std::string Password, Client_t *State);

    // (re-)Authenticate the client to the lobby system.
    void Remauth(Client_t *State);
}
