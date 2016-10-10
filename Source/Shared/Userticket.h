/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-7
    Notes:
        This encrypted ticket verifies the clients identity.
        It also contains some information for the servers.
*/

#pragma once
#include <cstdint>
#include <string>

struct Ticket_t
{
    std::string IPAddress;
    std::string Username;
    uint32_t Expiration;
    uint32_t UserID;
};

// JSON representation of the ticket to allow for modification between versions.
std::string SerializeUserticket(Ticket_t &Ticket);
Ticket_t DeserializeUserticket(std::string &Blob);

// Encryption/decryption on base64 strings.
std::string EncryptUserticket(std::string &Plain);
std::string DecryptUserticket(std::string &b64);

// Identification for servers.
const uint32_t FetchTicketversion();
