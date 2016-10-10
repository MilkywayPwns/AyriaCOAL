/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-10
    Notes:
        Compares the clients credentials to the database.
*/

#include "../../../STDInclude.h"
#include <Nyffenegger/base64.h>
#include <nlohmann/json.hpp>
#include <fossa/fossa.h>

struct Authenticationservice
{
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

                /*
                    TODO(Convery):
                    Compare against the SQLite database.
                    Return Username and UserID.
                */
                if (true)
                {
                    Ticket_t Userticket;
                    Userticket.UserID = 0;
                    Userticket.Username = "TODO";
                    Userticket.Expiration = time(NULL) + 30000;
                    Userticket.IPAddress = Localstate->IPAddress;

                    // Format the ticket for the lobby server.
                    auto Serialized = Userticket::Serialize(Userticket);
                    auto Encrypted = Userticket::Encrypt(Serialized);

                    Response["Lobby"] = Auth::Findserver(Localstate);
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

    Authenticationservice()
    {
#ifdef COAL_AUTH
        Services::Register("Authenticate", Authenticate);
#endif
    }
};
static Authenticationservice Serviceloader;
