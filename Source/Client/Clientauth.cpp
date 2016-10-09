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
#include <thread>

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
    auto Resource = "/auth?b64email=" + b64Email + "&b64pwhash" + b64Password;
    auto Hostname = std::string("http://") + "coalauth.ayria.se";

    /*
        TODO(Convery):
        Override Hostname through a config when implemented.
    */

    // This is a bit of a hack as we need the additional async data.
    {
        bool Authenticated = false;
        bool Callbackhit = false;

        // Perform the HTTP request to the auth server.
        auto Callback = [&](uint32_t Resultcode, std::string Resultbody, std::string Additionaldata)
        {
            // Incorrect details.
            if (Resultcode == 403) VAPrint("%s: Invalid credentials", "AuthenticateAuthserver");

            // Set the lobby even if the data is null.
            State.Lobbyaddress = Additionaldata;

            // Decode the ticket if we got one.
            if(Resultbody.size())
                State.Userticket = base64_decode(Resultbody);

            // Set the result and return.
            Authenticated = Resultcode && Resultcode != 403;
            Callbackhit = true;
        };
        HTTPClient::Async::GET(Hostname, Resource, Callback);

        // Wait for the callback.
        while (!Callbackhit) std::this_thread::sleep_for(std::chrono::milliseconds(10));

        return Authenticated;
    }
}
bool AuthenticateLobbyserver(Client_t &State)
{
    auto Connectionstring = "GET /auth?b64ticket" + base64_encode((uint8_t *)State.Userticket.data(), State.Userticket.size());
    auto Hostname = "ws://" + State.Lobbyaddress;

    State.Socket = Websocket::Connect(Hostname);
    if (!State.Socket) return false;

    /*
        TODO(Convery):
        Connect and upgrade to websocket.
        if 200 -> set State.Socket;
        if dropped -> cry.
    */

    return false;
}
