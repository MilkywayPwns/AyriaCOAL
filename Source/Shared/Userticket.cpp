/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-8
    Notes:
        This encrypted ticket verifies the clients identity.
        It also contains some information for the servers.
*/

#include <Nyffenegger/base64.h>
#include <nlohmann/json.hpp>
#include "../STDInclude.h"
#include "Userticket.h"

// JSON representation of the ticket to allow for modification between versions.
std::string Userticket::Serialize(Ticket_t &Ticket)
{
    nlohmann::json Object;
    Object["userid"] = Ticket.UserID;
    Object["username"] = Ticket.Username;
    Object["ipaddress"] = Ticket.IPAddress;
    Object["expiration"] = Ticket.Expiration;
    
    return Object.dump();
}
Ticket_t Userticket::Deserialize(std::string &Blob)
{
    nlohmann::json Object = nlohmann::json::parse(Blob.c_str());
    Ticket_t Ticket;

    Ticket.UserID = Object["userid"];
    Ticket.Username = Object["username"].get<std::string>();
    Ticket.IPAddress = Object["ipaddress"].get<std::string>();
    Ticket.Expiration = Object["expiration"];

    return Ticket;
}

// Encryption/decryption on base64 strings.
std::string Encryptionkey;
std::string EncryptionIV;
std::string Userticket::Encrypt(std::string &Plain)
{
    auto Encrypted = COAL::SMS4::Encrypt(Encryptionkey, EncryptionIV, Plain);
    return base64_encode((uint8_t *)Encrypted.data(), Encrypted.size());
}
std::string Userticket::Decrypt(std::string &b64)
{
    auto Decoded = base64_decode(b64);
    return COAL::SMS4::Decrypt(Encryptionkey, EncryptionIV, Decoded);
}

// Identification for servers.
const uint64_t Userticket::Version()
{
    return *(uint64_t *)&EncryptionIV[20];
}

// Initialize the encryption on startup.
struct InitializeCrypto
{
    InitializeCrypto()
    {
        Encryptionkey = COAL::SMS3::Hash(CRYPTOSEED);
        EncryptionIV = COAL::SMS3::Hash(Encryptionkey);
    }
};
static InitializeCrypto Cryptoloader;
