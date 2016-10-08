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
    std::string Resultstring;
    std::string Connectionstring;
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
            else
            {
                if(Activerequests[Connection].RequestData)
                    ns_printf(Connection, "GET %s HTTP/1.0\r\n\r\n", Activerequests[Connection].Connectionstring.c_str());
                else
                    ns_printf(Connection, "POST %s HTTP/1.0\r\nContent-Length: %i\r\n\r\n%s", Activerequests[Connection].Connectionstring.c_str(), 
                        Activerequests[Connection].RequestData->size(), Activerequests[Connection].RequestData->c_str());
            }
            break;
        }

        case NS_HTTP_REPLY:
        {
            Connection->flags |= NSF_SEND_AND_CLOSE;

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
static std::string InternalRequest(Connectioninfo Info, std::string Hostname)
{
    ns_mgr Manager;
    ns_mgr_init(&Manager, NULL);

    auto Connection = ns_connect(&Manager, Hostname.c_str(), InternalEventhandler);
    ns_set_protocol_http_websocket(Connection);
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

std::string HTTPClient::RequestSync(std::string Hostname, std::string Connectionstring, bool Includeheaders, std::string *PostData)
{
    Connectioninfo Info;
    Info.RequestData = PostData;
    Info.Includeheaders = Includeheaders;
    Info.Connectionstring = Connectionstring;    
    
    return InternalRequest(Info, Hostname);
}
void HTTPClient::RequestAsync(std::string Hostname, std::string Connectionstring, HTTPCallback *CB, bool Includeheaders, std::string *PostData)
{
    Connectioninfo Info;
    Info.CB = CB;
    Info.RequestData = PostData;
    Info.Includeheaders = Includeheaders;
    Info.Connectionstring = Connectionstring; 

    std::thread(InternalRequest, Info, Hostname).detach();
}
