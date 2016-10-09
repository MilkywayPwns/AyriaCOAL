/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-9
    Notes:
        Performs operations on the clients ticket when requested.
*/

#include "../../STDInclude.h"
#include <nlohmann/json.hpp>
#include "../Clientstate.h"

namespace Client
{
    struct Ticketservice
    {
        static bool Updateticket(void *State, std::string Data)
        {
            PrintFunction();
            Client_t *Localstate = (Client_t *)State;

            try
            {
                auto Request = nlohmann::json::parse(Data.c_str());

                if (Request["Ticket"].is_null()) return false;
                Localstate->Userticket = Request["Ticket"].get<std::string>();

                return true;;
            }
            catch (...) {}

            return false;
        }
        static bool Fetchticket(void *State, std::string Data)
        {
            PrintFunction();
            Client_t *Localstate = (Client_t *)State;

            nlohmann::json Response;
            Response["Service"] = "Fetchticket_response";
            Response["Ticket"] = Localstate->Userticket;

            Networking::Publish(Localstate->Socket, Response.dump());
            return true;
        }

        Ticketservice()
        {
            Services::Register("Updateticket", Updateticket);
            Services::Register("Fetchticket", Fetchticket);
        }
    };
    static Ticketservice Serviceloader;
}
