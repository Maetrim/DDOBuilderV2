// FavorTypes.h
//
#pragma once

#include "XmlLib\SaxContentElement.h"

// Special note - The  "Unknown" entries of each enum must be entry 0 in the
// enum map as this is used by the SAX loaders to check for badly loaded values
// these enumerations are used across multiple object types
enum FavorType
{
    Favor_Unknown = 0,
    Favor_None,
    Favor_Solo,
    Favor_Casual,
    Favor_Normal,
    Favor_Hard,
    Favor_Elite,
    Favor_Reaper1,
    Favor_Reaper2,
    Favor_Reaper3,
    Favor_Reaper4,
    Favor_Reaper5,
    Favor_Reaper6,
    Favor_Reaper7,
    Favor_Reaper8,
    Favor_Reaper9,
    Favor_Reaper10,

    Favor_Count
};

const XmlLib::enumMapEntry<FavorType> favorTypeMap[] =
{
    {Favor_Unknown,     L"Unknown"},
    {Favor_None,        L"None"},
    {Favor_Solo,        L"Solo"},
    {Favor_Casual,      L"Casual"},
    {Favor_Normal,      L"Normal"},
    {Favor_Hard,        L"Hard"},
    {Favor_Elite,       L"Elite"},
    {Favor_Reaper1,     L"Reaper 1"},
    {Favor_Reaper2,     L"Reaper 2"},
    {Favor_Reaper3,     L"Reaper 3"},
    {Favor_Reaper4,     L"Reaper 4"},
    {Favor_Reaper5,     L"Reaper 5"},
    {Favor_Reaper6,     L"Reaper 6"},
    {Favor_Reaper7,     L"Reaper 7"},
    {Favor_Reaper8,     L"Reaper 8"},
    {Favor_Reaper9,     L"Reaper 9"},
    {Favor_Reaper10,    L"Reaper 10"},
    {FavorType(0),      NULL}
};

