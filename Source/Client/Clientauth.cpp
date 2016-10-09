/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-7
    Notes:
        Authenticates the client to the servers.
*/

#include <Nyffenegger/base64.h>
#include <nlohmann/json.hpp>
#include "../STDInclude.h"
#include <unordered_map>
#include "Clientstate.h"
#include <thread>

// Authentication to the auth server.
bool Client::Authenticate(std::string &Password, Client_t &State)
{
    nlohmann::json Request;

    // Create the JSON formated request.
    Request["Service"] = "Authentication";
    Request["Email"] = base64_encode((uint8_t *)State.Email.data(), State.Email.size());
    Request["Password"] = [&]()
    {
        auto Salt = COAL::bcrypt::CreateSalt(14, State.Email + "AYRIA_PASSWORD");
        auto Hash = COAL::bcrypt::CreateHash(Salt, Password);
        return base64_encode((uint8_t *)Hash.data(), Hash.size());
    }();

    /*
        TODO(Convery):
        When we have created some configuration manager,
        we should override the hostname through it.
    */
    std::string Hostname = "coalauth.ayria.se";
    auto Callback = [&](size_t Socket, Networking::NetEvent Event, std::string Data)
    {
        // Ignore dropped connections, they are a failed auth.
        if (Event != Networking::NetEvent::DATA) return;

        try
        {
            auto Response = nlohmann::json::parse(Data.c_str());
            if (std::strstr(Response["Result"].get<std::string>().c_str(), "Success"))
            {
                State.Userticket = Response["Ticket"].get<std::string>();
                State.Lobbyaddress = Response["Lobby"].get<std::string>();
            }
        }
        catch (std::exception &e) 
        {
            VAPrint("Client::Authenticate::Callback error: %s", e.what()); 
        }
    };

    // Connect to the auth server.
    State.Socket = Networking::Connect(Hostname + ":" + COAL_PORT);
    Networking::Subscribe(State.Socket, Callback);
    Networking::Publish(State.Socket, Request.dump());

    return State.Socket != NULL;
}
bool Client::isAuthenticated(Client_t &State)
{
    return State.Userticket.size() > 0;
}

// Remain authenticated via the lobby server.
static std::unordered_map<size_t, Client_t> Clientsockets;
void Client::Lobbycallback(size_t Socket, Networking::NetEvent Event, std::string Data)
{
    try
    {
        auto Response = nlohmann::json::parse(Data.c_str());
        Services::Call(Response["Service"], &Clientsockets[Socket], Data);
    }
    catch (std::exception &e) 
    {
        VAPrint("%s error: %s", __FUNCTION__, e.what()); 
    }
}
bool Client::Remauth(Client_t &State)
{
    nlohmann::json Request;

    // Create the JSON formated request.
    Request["Service"] = "Remauth";
    Request["Ticket"] = State.Userticket;

    // Connect to the lobby server.
    State.Socket = Networking::Connect(State.Lobbyaddress + ":" + COAL_PORT);
    Networking::Subscribe(State.Socket, Lobbycallback);
    Networking::Publish(State.Socket, Request.dump());

    return State.Socket != NULL;
}
