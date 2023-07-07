// StartingWorldTypes.h
//
#pragma once

#include "XmlLib\SaxContentElement.h"

// Special note - The  "Unknown" entries of each enum must be entry 0 in the
// enum map as this is used by the SAX loaders to check for badly loaded values
// these enumerations are used across multiple object types
enum StartingWorldType
{
    StartingWorld_Unknown = 0,
    // these are not alphabetical, but in the order they appear in the UI
    StartingWorld_Eberron,
    StartingWorld_ForgottenRealms,

    StartingWorld_Count
};
const XmlLib::enumMapEntry<StartingWorldType> startingWorldTypeMap[] =
{
    {StartingWorld_Unknown, L"Unknown"},
    {StartingWorld_Eberron, L"Eberron"},
    {StartingWorld_ForgottenRealms, L"Forgotten Realms"},
    {StartingWorldType(0), NULL}
};
