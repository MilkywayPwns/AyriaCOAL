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

struct Connection_t
{
    size_t Socket;
    uint64_t Version;
    std::string IPAddress;
};
struct Lobby_t : public Connection_t
{
    uint8_t CPULoad;
    uint8_t RAMLoad;
    uint8_t NETLoad;
    uint8_t GEOInfo;

    uint64_t ServerID;
};

namespace Auth
{
    // Move the socket into the permanent list.
    void Upgradesocket(size_t Socket, Lobby_t *State);

    // Find a lobby server for the client.
    Lobby_t *Findserver(Connection_t *Client);
}
