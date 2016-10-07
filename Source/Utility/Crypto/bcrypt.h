/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-7
    Notes:
        Simply wraps OpenWalls implementation.
*/

#pragma once

#include <string>
#include <cstdint>

namespace COAL
{
    namespace bcrypt
    {
        std::string CreateHash(std::string &Header, std::string Input);
        std::string CreateSalt(size_t Workfactor, std::string Input);
        bool Verify(std::string &Hash, std::string Plain);
    }
}
