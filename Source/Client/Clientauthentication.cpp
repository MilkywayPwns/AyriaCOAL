/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-10
    Notes:
        Authenticate to the auth and lobby systems.
*/

#include <thread>
#include "Clientheader.h"
#include "../STDInclude.h"
#include <nlohmann/json.hpp>
#include <Nyffenegger/base64.h>

// Authenticate the client to the network.
bool Client::Authenticate(std::string Password, Client_t *State)
{
    nlohmann::json Request;

    // Create the JSON formated request.
    Request["Service"] = "Authenticate";
    Request["Email"] = base64_encode((uint8_t *)State->Email.data(), State->Email.size());
    Request["Password"] = [&]()
    {
        auto Salt = COAL::bcrypt::CreateSalt(14, State->Email + "AYRIA_PASSWORD");
        auto Hash = COAL::bcrypt::CreateHash(Salt, Password);
        return base64_encode((uint8_t *)Hash.data(), Hash.size());
    }();

    // Set defaults for the state so we can verify.
    State->Authenticated = false;
    State->Offline = false;

    /*
        TODO(Convery):
        When we have created some configuration manager,
        we should override the hostname through it.
    */
    std::string Hostname = "coalauth.ayria.se";

    // Publish to the auth server.
    auto Socket = Networking::Connect(Hostname + ":" + COAL_PORT);
    Networking::Publish(Socket, Request.dump());

    // Wait for auth to finish.
    while (!State->Offline && !State->Authenticated)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return State->Authenticated;
}

// (re-)Authenticate the client to the lobby system.
void Client::Remauth(Client_t *State)
{
    nlohmann::json Request;

    // Create the JSON formated request.
    Request["Service"] = "Remauth";
    Request["Ticket"] = State->Userticket;

    // Publish to the lobby server.
    auto Socket = Networking::Connect(State->Lobbyaddress + ":" + COAL_PORT);
    Networking::Publish(Socket, Request.dump());
}
