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
    std::string Method;
    std::string Hostname;
    std::string Resource;
    std::string Requestdata;
    std::string Responsebody;
    std::string Responseadditional;
    uint32_t Responsecode;
    HTTPClient::Async::HTTPCallback CB;
};
std::unordered_map<void *, Connectioninfo> Activerequests;

// Called on each poll, usually every 100ms when a request is active.
static void InternalEventhandler(ns_connection *Connection, int EventID, void *Eventdata)
{
    http_message *Message = (struct http_message *)Eventdata;

    switch (EventID)
    {
        case NS_CONNECT:
        {
            auto Connectionerror = *(int *)Eventdata;

            // On error:
            if (Connectionerror != 0)
            {
                VAPrint("HTTPClient::Connect(\"%s\") failed with: \"%s\"", Activerequests[Connection].Hostname.c_str(), strerror(*(int *)Eventdata));
                Activerequests[Connection].Done = true;
                break;
            }

            std::string Request =
            {
                Activerequests[Connection].Method + " " +
                Activerequests[Connection].Resource + " " +
                "HTTP/1.0\r\nHost: " +
                Activerequests[Connection].Hostname + "\r\n"
            };
            if (std::strstr(Activerequests[Connection].Method.c_str(), "GET"))
            {
                Request.append(COAL::va_small("Content-Length: %i\r\n\r\n", Activerequests[Connection].Requestdata.size()));
                Request.append(Activerequests[Connection].Requestdata);
            }
            else Request.append("\r\n");

            ns_send(Connection, Request.data(), Request.size());
            break;
        }
        case NS_HTTP_REPLY:
        {
            // This is HTTP so we just close the connection.
            Connection->flags |= NSF_SEND_AND_CLOSE;

            // Set the response code.
            Activerequests[Connection].Responsecode = Message->resp_code;

            // If we get a 'moved' response, copy the location header.
            if (Message->resp_code > 300 && Message->resp_code < 400)
            {
                for (int i = 0; Message->header_names[i].p && i < 40; ++i)
                {
                    if (std::strstr(Message->header_names[i].p, "Location"))
                    {
                        Activerequests[Connection].Responseadditional = Message->header_values[i].p;
                        break;
                    }
                }
            }
        
            Activerequests[Connection].Done = true;
            break;
        }
        default:
        {
            break;
        }
    }
}
static void InternalRequest(Connectioninfo Info)
{
    ns_mgr Manager;
    ns_mgr_init(&Manager, NULL);

    auto Connection = ns_connect(&Manager, Info.Hostname.c_str(), InternalEventhandler);
    ns_set_protocol_http_websocket(Connection);
    Activerequests[Connection] = Info;

    // Poll for < 5 seconds.
    auto Timestamp = std::chrono::system_clock::now();
    do
    {
        ns_mgr_poll(&Manager, 100);
    } while (!Activerequests[Connection].Done && 
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - Timestamp).count() < 5);

    // If timed out, return 522 as CF would for other connections.
    if (Activerequests[Connection].Done) Activerequests[Connection].CB(522, "", "");
    else Activerequests[Connection].CB(Activerequests[Connection].Responsecode, 
        Activerequests[Connection].Responsebody, Activerequests[Connection].Responseadditional);

    // Remove the information.
    Activerequests.erase(Connection);
}

// Returns the body, with or without the result code.
std::string HTTPClient::Sync::GET(std::string Hostname, std::string Resource)
{
    std::string Result;

    auto Callback = [&](uint32_t Resultcode, std::string Resultbody, std::string Additionaldata)
    {
        Result = Resultbody;
    };

    Connectioninfo Info;
    Info.CB = Callback;
    Info.Method = "GET";
    Info.Hostname = Hostname;    
    Info.Resource = Resource;

    InternalRequest(Info);
    return Result;
}
uint32_t HTTPClient::Sync::GET(std::string Hostname, std::string Resource, std::string *Body)
{
    uint32_t Result;

    auto Callback = [&](uint32_t Resultcode, std::string Resultbody, std::string Additionaldata)
    {
        Result = Resultcode;
        *Body = Resultbody;
    };

    Connectioninfo Info;
    Info.CB = Callback;
    Info.Method = "GET";
    Info.Hostname = Hostname;    
    Info.Resource = Resource;

    InternalRequest(Info);
    return Result;
}
std::string HTTPClient::Sync::POST(std::string Hostname, std::string Resource, std::string Data)
{
    std::string Result;

    auto Callback = [&](uint32_t Resultcode, std::string Resultbody, std::string Additionaldata)
    {
        Result = Resultbody;
    };

    Connectioninfo Info;
    Info.CB = Callback;
    Info.Method = "POST";
    Info.Hostname = Hostname;    
    Info.Resource = Resource;
    Info.Requestdata = Data;

    InternalRequest(Info);
    return Result;
}
uint32_t HTTPClient::Sync::POST(std::string Hostname, std::string Resource, std::string Data, std::string *Body)
{
    uint32_t Result;

    auto Callback = [&](uint32_t Resultcode, std::string Resultbody, std::string Additionaldata)
    {
        Result = Resultcode;
        *Body = Resultbody;
    };

    Connectioninfo Info;
    Info.CB = Callback;
    Info.Method = "POST";
    Info.Hostname = Hostname;    
    Info.Resource = Resource;

    InternalRequest(Info);
    return Result;
}

// Calls the callback when finished.
void HTTPClient::Async::GET(std::string Hostname, std::string Resource, HTTPCallback Callback)
{
    Connectioninfo Info;
    Info.CB = Callback;
    Info.Method = "GET";
    Info.Hostname = Hostname;    
    Info.Resource = Resource;

    std::thread(InternalRequest, Info).detach();
}
void HTTPClient::Async::POST(std::string Hostname, std::string Resource, HTTPCallback Callback, std::string Data)
{
    Connectioninfo Info;
    Info.CB = Callback;
    Info.Method = "POST";
    Info.Hostname = Hostname;    
    Info.Resource = Resource;

    std::thread(InternalRequest, Info).detach();
}
