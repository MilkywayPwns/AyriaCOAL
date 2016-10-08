/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: MIT
    Started: 2016-10-8
    Notes:
        This is a wrapper around Jamis Hoo's brilliant implementation.
        See his work at: https://github.com/JamisHoo
*/

#pragma once
#include <cstdint>
#include <string>

namespace COAL
{
    namespace SMS4
    {
        std::string Encrypt(std::string &Key, std::string &IV, std::string Plaintext);
        std::string Decrypt(std::string &Key, std::string &IV, std::string Ciphertext);
    }
}
