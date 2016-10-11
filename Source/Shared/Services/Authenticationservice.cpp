/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-10
    Notes:
*/

#include "../../STDInclude.h"
#include "../Database.h"
#include <Nyffenegger/base64.h>
#include <nlohmann/json.hpp>
#include <fossa/fossa.h>

struct Authenticationservice
{
    // Serverside
    static bool Authenticate(void *State, std::string Data)
    {
        auto Localstate = (Connection_t *)State;
        auto Connection = (ns_connection *)Localstate->Socket;

        char IPAddress[16]{};
        ns_sock_addr_to_str(&Connection->sa, IPAddress, 16, NS_SOCK_STRINGIFY_IP);
        Localstate->IPAddress = IPAddress;

        // Formulate a reply.
        nlohmann::json Response;
        Response["Service"] = "Authenticate_reply";

        try
        {
            auto Request = nlohmann::json::parse(Data.c_str());

            // Client authentication.
            if (Request["Version"].is_null())
            {
                // Get the 'plain' input.
                auto Email = base64_decode(Request["Email"]);
                auto Password = base64_decode(Request["Password"]);

                // Hash them to match the DB.
                auto Emailhash = COAL::SMS3::Hash(Email);
                auto Passwordhash = COAL::SMS3::Hash(Password);

				// Obtain userdata from SQL
				std::vector < Database::QueryResult > results = Database::Select("users",
					{
						{ "email", Email },
						{ "password", Password }
					}
				);

				// Check if result size is 1 (2 or more shouldn't be possible)
                if (results.size() == 1)
                {
                    Ticket_t Userticket;
                    Userticket.UserID = results[0]["uid"].ToUint32();
                    Userticket.Username = results[0]["username"].ToString();
                    Userticket.Expiration = time(NULL) + 120;
                    Userticket.IPAddress = Localstate->IPAddress;

                    // Format the ticket for the lobby server.
                    auto Serialized = Userticket::Serialize(Userticket);
                    auto Encrypted = Userticket::Encrypt(Serialized);

                    Response["Lobby"] = Auth::Findserver(Localstate)->IPAddress;
                    Response["Ticket"] = base64_encode((uint8_t *)Encrypted.data(), Encrypted.size());
                }
                else
                {
                    Response["Error"] = "Credentials were not found in the DB";
                }
            }
            else
            {
                // Lobby authentication.
                uint64_t Version = Request["Version"];

                // If the client is on the same version, upgrade.
                if (Version == Userticket::Version())
                {
                    Lobby_t *Newlobby = new Lobby_t();
                    Newlobby->IPAddress = Localstate->IPAddress;
                    Newlobby->Socket = Localstate->Socket;
                    Newlobby->Version = Version;

                    Newlobby->CPULoad = Request["CPULoad"];
                    Newlobby->RAMLoad = Request["RAMLoad"];
                    Newlobby->NETLoad = Request["NETLoad"];
                    Newlobby->GEOInfo = Request["GEOInfo"];
                    Newlobby->ServerID = Localstate->Socket;

                    Auth::Upgradesocket(Localstate->Socket, Newlobby);
                    Response["ServerID"] = Newlobby->ServerID;
                }
                else
                {
                    Response["Error"] = "Invalid version";
                }
            }

            Networking::Publish(Localstate->Socket, Response.dump());
            return true;
        }
        catch (std::exception &e) 
        {
            VAPrint("%s error: %s", __FUNCTION__, e.what()); 
        }

        return false;
    }
    static bool Remauth(void *State, std::string Data)
    {
        try
        {
            auto Request = nlohmann::json::parse(Data.c_str());
            auto Decrypted = Userticket::Decrypt(Request["Ticket"].get<std::string>());
            auto Ticket = Userticket::Deserialize(Decrypted);

            // Verify the expirationdate.
            if (!Ticket.Expiration || Ticket.Expiration < time(NULL))
                return false;

            // Verify the IP to avoid MITM.
            char IPAddress[16]{};
            auto Client = (Lobbyclient_t *)State;
            auto Connection = (ns_connection *)Client->Socket;
            ns_sock_addr_to_str(&Connection->sa, IPAddress, 16, NS_SOCK_STRINGIFY_IP);
            if (std::strcmp(IPAddress, Ticket.IPAddress.c_str())) 
                return false;

            // Set the state.
            Client->Authenticated = true;
            Client->Accountinfo = Ticket;

            // Up the expiration date if needed.
            if (Ticket.Expiration && Ticket.Expiration - time(NULL) < 600)
                Client->Accountinfo.Expiration = time(NULL) + 600;

            // Return a new ticket if needed.
            nlohmann::json Response;
            Response["Service"] = "Remauth_reply";
            if (Ticket.Expiration != Client->Accountinfo.Expiration)
            {
                auto Encryptedticket = Userticket::Encrypt(Userticket::Serialize(Client->Accountinfo));
                Response["Ticket"] = Encryptedticket;
            }

            Networking::Publish(Client->Socket, Response.dump());
            return true;
        }
        catch (std::exception &e) 
        {
            VAPrint("%s error: %s", __FUNCTION__, e.what()); 
        }

        return false;
    }

    // Clientside
    static bool Authenticate_reply(void *State, std::string Data)
    {
        try
        {
            auto Request = nlohmann::json::parse(Data.c_str());

            // Authentication failed.
            if (!Request["Error"].is_null())
            {
                /*
                    TODO(Convery):
                    Handle this error in some way.
                    Maybe something like Messages::Popup(error);
                */

                VAPrint("%s error: %s", __FUNCTION__, Request["Error"].get<std::string>().c_str()); 
                return false;
            }

            // Lobby authentication.
            if (Request["ServerID"])
            {
                auto Lobby = (Lobby_t *)State;
                Lobby->ServerID = Request["ServerID"];
            }
            else
            {
                // Client authentication.
                auto Client = (Client_t *)State;
                Client->Userticket = Request["Ticket"].get<std::string>();
                Client->Lobbyaddress = Request["Lobby"].get<std::string>();
            }

            return true;
        }
        catch (std::exception &e) 
        {
            VAPrint("%s error: %s", __FUNCTION__, e.what()); 
        }

        return false;
    }
    static bool Remauth_reply(void *State, std::string Data)
    {
        try
        {
            auto Request = nlohmann::json::parse(Data.c_str());
            if (!Request["Ticket"].is_null())
            {
                auto Client = (Client_t *)State;
                Client->Userticket = Request["Ticket"].get<std::string>();
            }

            return true;
        }
        catch (std::exception &e) 
        {
            VAPrint("%s error: %s", __FUNCTION__, e.what()); 
        }

        return false;
    }

    Authenticationservice()
    {
#ifdef COAL_LOBBY
        Services::Register("Remauth", Remauth);
#endif

#ifdef COAL_AUTH
        Services::Register("Authenticate", Authenticate);
#else
        Services::Register("Authenticate_reply", Authenticate_reply);
        Services::Register("Remauth_reply", Remauth_reply);
#endif
    }
};
static Authenticationservice Serviceloader;
