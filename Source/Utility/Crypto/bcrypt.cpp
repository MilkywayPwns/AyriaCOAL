/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-7
    Notes:
        Simply wraps OpenWalls implementation.
*/

#include "bcrypt.h"
#include <algorithm>
#include <openwall/ow-crypt.h>
#include <goldboar/sm3_cl.hpp>

#define BCRYPT_HASHSIZE 64

std::string COAL::bcrypt::CreateHash(std::string &Header, std::string Input)
{
    char Hash[BCRYPT_HASHSIZE];
    auto Result = crypt_rn(Input.c_str(), Header.c_str(), Hash, BCRYPT_HASHSIZE);

    if (Result) return std::string(Result);
    else return std::string("");
}
std::string COAL::bcrypt::CreateSalt(size_t Workfactor, std::string Input)
{
    // Verify that workfactor is in range.
    Workfactor = std::min(size_t(31), Workfactor);
    Workfactor = std::max(size_t(4), Workfactor);

    // Generate a nice hash of the input.
    uint8_t Localhash[32];
    SM3 Hasher((uint8_t *)Input.data(), Input.size());
    Hasher.digest(Localhash);

    // Create the actual hash.
    char Salt[BCRYPT_HASHSIZE];
    auto Result = crypt_gensalt_rn("$2a$", Workfactor, (char *)Localhash, 16, Salt, BCRYPT_HASHSIZE);

    if (Result) return std::string(Result);
    else return std::string("");
}
bool COAL::bcrypt::Verify(std::string &Hash, std::string Plain)
{
    std::string Localhash = CreateHash(Hash, Plain);
    return 0 == std::strcmp(Hash.c_str(), Localhash.c_str());
}
