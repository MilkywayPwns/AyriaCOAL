/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-9
    Notes:
        Unlike the HTTP implementation, WS needs a socket var.
*/

#include "Websocket.h"
#include <unordered_map>
#include <fossa/fossa.h>
#include <thread>
#include <mutex>
#include <queue>

// Per socket dataqueues anf guards.
std::unordered_map<size_t /* Socket */, std::vector<Websocket::WSCallback>> Subscribers;
std::unordered_map<size_t /* Socket */, std::queue<std::string>> Incomingbuffer;
std::unordered_map<size_t /* Socket */, std::queue<std::string>> Outgoingbuffer;
std::unordered_map<size_t /* Socket */, std::mutex> Bufferguard;

// Internal functions.
static ns_mgr Manager;
static void InitializeWS()
{
    static bool Initialized = false;
    if (Initialized) return;
    Initialized = true;

    ns_mgr_init(&Manager, NULL);
    std::thread([]() { while (true) ns_mgr_poll(&Manager, 100); }).detach();
}
static void InternalEventhandler(ns_connection *Connection, int EventID, void *Eventdata)
{
    websocket_message *Message = (struct websocket_message *)Eventdata;
    size_t Socket = size_t(Connection);

    switch (EventID)
    {
        case NS_WEBSOCKET_FRAME:
        {
            // If there's a subscriber, send the data to it.
            if (!Subscribers[Socket].empty())
            {
                for (auto &S : Subscribers[Socket])
                {
                    S(Socket, EventID, std::string((char *)Message->data, Message->size));
                }

                break;
            }

            // Else we enqueue the frame.
            Bufferguard[Socket].lock();
            {
                Incomingbuffer[Socket].push(std::string((char *)Message->data, Message->size));
            }
            Bufferguard[Socket].unlock();
            break;
        }
        case NS_HTTP_REQUEST:
        {
            // We don't handle HTTP.
            Websocket::Disconnect(Socket);
            break;
        }
        case NS_WEBSOCKET_HANDSHAKE_DONE:
        {
            for (auto &S : Subscribers[Socket])
            {
                S(Socket, EventID, "");
            }
            break;
        }
        case NS_CLOSE:
        {
            for (auto &S : Subscribers[Socket])
            {
                S(Socket, EventID, "");
            }
            break;
        }
        case NS_POLL:
        {
            // Send a frame.
            Bufferguard[Socket].lock();
            {
                if (!Incomingbuffer[Socket].empty())
                {
                    std::string Frame = Outgoingbuffer[Socket].front();
                    Outgoingbuffer[Socket].pop();
                    ns_send_websocket_frame(Connection, WEBSOCKET_OP_BINARY, Frame.data(), Frame.size());            
                }
            }
            Bufferguard[Socket].unlock();
        }
    }    
}

// Initialize the connection.
void Websocket::Disconnect(size_t Socket)
{
    auto Connection = (ns_connection *)Socket;

    Connection->flags |= NSF_SEND_AND_CLOSE;
    while (!Incomingbuffer[Socket].empty())
        Incomingbuffer[Socket].pop();
    Incomingbuffer.erase(Socket);
}
size_t Websocket::Connect(std::string Hostname)
{
    InitializeWS();

    auto Connection = ns_connect(&Manager, Hostname.c_str(), NULL);
    return size_t(Connection);
}

// Enqueue or dequeue a frame.
std::string Websocket::Read(size_t Socket)
{
    std::string Result;

    Bufferguard[Socket].lock();
    {
        if (!Incomingbuffer[Socket].empty())
        {
            Result = Incomingbuffer[Socket].front();
            Incomingbuffer[Socket].pop();
        }
    }
    Bufferguard[Socket].unlock();

    return Result;
}
void Websocket::Write(std::string Data, size_t Socket)
{
    Bufferguard[Socket].lock();
    {
        Outgoingbuffer[Socket].push(Data);
    }
    Bufferguard[Socket].unlock();
}

// Callback on incoming data..
void Websocket::Subscribe(size_t Socket, WSCallback Callback)
{
    Bufferguard[Socket].lock();
    {
        Subscribers[Socket].push_back(Callback);
    }
    Bufferguard[Socket].unlock();
}
