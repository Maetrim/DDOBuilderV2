// PatronTypes.h
//
#pragma once

#include "XmlLib\SaxContentElement.h"

// Special note - The  "Unknown" entries of each enum must be entry 0 in the
// enum map as this is used by the SAX loaders to check for badly loaded values
// these enumerations are used across multiple object types
enum PatronType
{
    Patron_AgentsOfArgonnessen = 0,
    Patron_HouseCannith,
    Patron_TheCoinLords,
    Patron_HouseDeneith,
    Patron_TheFreeAgents,
    Patron_TheGatekeepers,
    Patron_TheHarpers,
    Patron_HouseJorasco,
    Patron_KeepersOfTheFeather,
    Patron_HouseKundarak,
    Patron_MorgraveUniversity,
    Patron_HousePhiarlan,
    Patron_PurpleDragonKnights,
    Patron_SharnCityCouncil,
    Patron_TheSilverFlame,
    Patron_TheSummerCourt,
    Patron_TheTwelve,
    Patron_TheYugoloth,
    Patron_CormanthorElves,
    Patron_TotalFavor,

    Patron_Count,
    Patron_Unknown,
    Patron_None
};

const XmlLib::enumMapEntry<PatronType> patronTypeMap[] =
{
    {Patron_Unknown,                L"Unknown"},
    {Patron_AgentsOfArgonnessen,    L"Agents of Argonnessen"},
    {Patron_HouseCannith,           L"House Cannith"},
    {Patron_TheCoinLords,           L"The Coin Lords"},
    {Patron_HouseDeneith,           L"House Deneith"},
    {Patron_TheFreeAgents,          L"The Free Agents"},
    {Patron_TheGatekeepers,         L"The Gatekeepers"},
    {Patron_TheHarpers,             L"The Harpers"},
    {Patron_HouseJorasco,           L"House Jorasco"},
    {Patron_KeepersOfTheFeather,    L"Keepers of the Feather"},
    {Patron_HouseKundarak,          L"House Kundarak"},
    {Patron_MorgraveUniversity,     L"Morgrave University"},
    {Patron_HousePhiarlan,          L"House Phiarlan"},
    {Patron_PurpleDragonKnights,    L"Purple Dragon Knights"},
    {Patron_SharnCityCouncil,       L"Sharn City Council"},
    {Patron_TheSilverFlame,         L"The Silver Flame"},
    {Patron_TheSummerCourt,         L"The Summer Court"},
    {Patron_TheTwelve,              L"The Twelve"},
    {Patron_TheYugoloth,            L"The Yugoloth"},
    {Patron_CormanthorElves,        L"Cormanthor Elves"},
    {Patron_TotalFavor,             L"Total Favor"},
    {Patron_None,                   L"None"},
    {PatronType(0),                 NULL}
};

