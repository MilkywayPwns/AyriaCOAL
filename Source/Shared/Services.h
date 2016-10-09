/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-9
    Notes:
        Routes messages based on an identifier.
*/

#pragma once
#include <string>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include "../STDInclude.h"

using ServiceCallback = std::function<bool(void *State, std::string Data)>;
static std::unordered_map<uint32_t, ServiceCallback> Servicemap;

namespace Services
{
    inline void Register(std::string Name, ServiceCallback Service)
    {
        Servicemap[COAL::FNV1::Runtime::FNV1_32(Name)] = Service;
    }
    inline void Remove(std::string Name)
    {
        Servicemap.erase(COAL::FNV1::Runtime::FNV1_32(Name));
    }
    inline bool Call(std::string Name, void *State, std::string Data)
    {
        auto Result = Servicemap.find(COAL::FNV1::Runtime::FNV1_32(Name));
        if (Result == Servicemap.end()) return false;
        else return Result->second(State, Data);
    }
}
