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

    std::string Email;
    std::string Username;
    std::string Userticket;
    std::string Lobbyaddress;
};

// Base64 encoded hashes used for auth.
std::string CreatePasswordhash(std::string &Password, Client_t &State);
std::string CreateEmailhash(Client_t &State);

// Authenticate to the different servers.
bool AuthenticateAuthserver(std::string &Password, Client_t &State);
bool AuthenticateLobbyserver(Client_t &State);
