/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: MIT
    Started: 2016-10-8
    Notes:
        This is a wrapper around goalboars implementation.
*/

#pragma once
#include <cstdint>
#include <string>

namespace COAL
{
    namespace SMS3
    {
        std::string Hash(std::string Input);
        std::string Hash(const void *Data, size_t Length);
    }
}
