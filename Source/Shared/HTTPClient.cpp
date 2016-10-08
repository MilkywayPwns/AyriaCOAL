/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-8
    Notes:
        Connect to a server and send a message.
*/

#include "../STDInclude.h"
#include "HTTPClient.h"
#include <fossa/fossa.h>
#include <unordered_map>
#include <thread>

struct Connectioninfo
{
    bool Done{ false };
    bool Includeheaders;
    std::string RequestURL;    
    std::string Resultstring;
    std::string *RequestData{ nullptr };
    HTTPClient::HTTPCallback *CB{ nullptr };
};
std::unordered_map<void *, Connectioninfo> Activerequests;

static void InternalEventhandler(ns_connection *Connection, int EventID, void *Eventdata)
{
    http_message *Message = (struct http_message *)Eventdata;

    switch (EventID)
    {
        case NS_CONNECT:
        {
            if (*(int *)Eventdata != 0)
            {
                VAPrint("HTTPClient::Connect() failed with: \"%s\"", strerror(*(int *)Eventdata));
                Activerequests[Connection].Done = true;
            }
            break;
        }

        case NS_HTTP_REPLY:
        {
            Connection->flags |= NSF_CLOSE_IMMEDIATELY;

            if (Activerequests[Connection].Includeheaders)
                Activerequests[Connection].Resultstring.append(Message->message.p);
            else
                Activerequests[Connection].Resultstring.append(Message->body.p);

            Activerequests[Connection].Done = true;
            break;
        }
        default:
        {
            break;
        }
    }
}
static std::string InternalRequest(Connectioninfo Info)
{
    ns_mgr Manager;
    ns_mgr_init(&Manager, NULL);

    auto Connection = ns_connect_http(&Manager, InternalEventhandler, Info.RequestURL.c_str(), NULL, Info.RequestData ? Info.RequestData->data() : NULL);
    Activerequests[Connection] = Info;

    auto Timestamp = std::chrono::system_clock::now();
    do
    {
        ns_mgr_poll(&Manager, 100);
    } while (!Activerequests[Connection].Done && 
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - Timestamp).count() < 5);

    std::string Result = [&]() { if (Activerequests[Connection].Done) return Activerequests[Connection].Resultstring; else return std::string(""); }();
    if (Activerequests[Connection].CB) 
        (*Activerequests[Connection].CB)(Result);
    Activerequests.erase(Connection);

    return Result;
}

std::string HTTPClient::RequestSync(std::string URL, bool Includeheaders, std::string *PostData)
{
    Connectioninfo Info;
    Info.Includeheaders = Includeheaders;
    Info.RequestURL = URL;
    Info.RequestData = PostData;
    
    return InternalRequest(Info);
}
void HTTPClient::RequestAsync(std::string URL, HTTPCallback *CB, bool Includeheaders, std::string *PostData)
{
    Connectioninfo Info;
    Info.Includeheaders = Includeheaders;
    Info.RequestURL = URL;
    Info.RequestData = PostData;
    Info.CB = CB;

    std::thread(InternalRequest, Info).detach();
}
