/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: MIT
    Started: 2016-10-8
    Notes:
        This is a wrapper around goalboars implementation.
*/

#include "SMS3.h"
#include <memory>
#include <goldboar/sm3.h>

std::string COAL::SMS3::Hash(std::string Input)
{
    return Hash(Input.data(), Input.size());
}
std::string COAL::SMS3::Hash(const void *Data, size_t Length)
{
    auto Buffer = std::make_unique<uint8_t[]>(32);

    sm3((uint8_t *)Data, Length, Buffer.get());

    return std::string((char *)Buffer.get(), 32);
}
