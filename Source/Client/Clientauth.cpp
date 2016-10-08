/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-7
    Notes:
        Authenticates the client to the servers.
*/

#include <Nyffenegger/base64.h>
#include "../STDInclude.h"
#include "Clientstate.h"

// Base64 encoded hashes used for auth.
std::string CreatePasswordhash(std::string &Password, Client_t &State)
{
    auto Salt = COAL::bcrypt::CreateSalt(14, State.Email + "AYRIA_PASSWORD");
    auto Hash = COAL::bcrypt::CreateHash(Salt, Password);

    return base64_encode((uint8_t *)Hash.data(), Hash.size());
}
std::string CreateEmailhash(Client_t &State)
{
    auto Localhash = COAL::SMS3::Hash(State.Email);
    return base64_encode((uint8_t *)Localhash.data(), Localhash.size());
}

// Authenticate to the different servers.
bool AuthenticateAuthserver(std::string &Password, Client_t &State)
{
    auto b64Email = CreateEmailhash(State);
    auto b64Password = CreatePasswordhash(Password, State);
    auto Connectionstring = "GET /auth?email=" + b64Email + "&b64pwhash" + b64Password;
    auto Hostname = std::string("http://") + "coalauth.ayria.se";

    /*
        TODO(Convery):
        Override Hostname through a config when implemented.
    */

    /*
        TODO(Convery):
        Create a HTTP request.
        if 404 -> return false;
        if 308 -> set State.Lobbyaddress
        && set State.Userticket;
    */

    return false;
}
bool AuthenticateLobbyserver(Client_t &State)
{
    auto Connectionstring = "GET /auth?b64ticket" + base64_encode((uint8_t *)State.Userticket.data(), State.Userticket.size());
    auto Hostname = "ws://" + State.Lobbyaddress;

    /*
        TODO(Convery):
        Connect and upgrade to websocket.
        if 200 -> set State.Socket;
        if dropped -> cry.
    */

    return false;
}
