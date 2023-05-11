// DamageTypes.h
//
#pragma once

#include "XmlLib\SaxContentElement.h"

// Special note - The  "Unknown" entries of each enum must be entry 0 in the
// enum map as this is used by the SAX loaders to check for badly loaded values
// these enumerations are used across multiple object types

// damage types
enum DamageType
{
    Damage_Unknown = 0,
    Damage_Acid,
    Damage_All,
    Damage_Adamantine,
    Damage_Bleeding,
    Damage_Bludgeon,
    Damage_Cold,
    Damage_Electric,
    Damage_Evil,
    Damage_Fire,
    Damage_Force,
    Damage_Good,
    Damage_Holy,
    Damage_Light,
    Damage_Negative,
    Damage_Pierce,
    Damage_Poison,
    Damage_Positive,
    Damage_Repair,
    Damage_Rust,
    Damage_Slash,
    Damage_Sonic,
    Damage_Untyped,
    Damage_Bane,
    Damage_Chaos,
    Damage_Lawful,
    Damage_Strength,
    Damage_Intelligence,
    Damage_Wisdom,
    Damage_Dexterity,
    Damage_Constitution,
    Damage_Charisma,
    Damage_Ceruleite,
    Damage_Crimsonite,
    Damage_Gildleaf,
};
const XmlLib::enumMapEntry<DamageType> damageTypeMap[] =
{
    {Damage_Unknown, L"Unknown"},
    {Damage_Acid, L"Acid"},
    {Damage_All, L"All"},
    {Damage_Adamantine, L"Adamantine"},
    {Damage_Bleeding, L"Bleeding"},
    {Damage_Bludgeon, L"Bludgeon"},
    {Damage_Cold, L"Cold"},
    {Damage_Electric, L"Electric"},
    {Damage_Evil, L"Evil"},
    {Damage_Fire, L"Fire"},
    {Damage_Force, L"Force"},
    {Damage_Good, L"Good"},
    {Damage_Holy, L"Holy"},
    {Damage_Light, L"Light"},
    {Damage_Negative, L"Negative"},
    {Damage_Pierce, L"Pierce"},
    {Damage_Poison, L"Poison"},
    {Damage_Positive, L"Positive"},
    {Damage_Repair, L"Repair"},
    {Damage_Rust, L"Rust"},
    {Damage_Slash, L"Slash"},
    {Damage_Sonic, L"Sonic"},
    {Damage_Untyped, L"Untyped"},
    {Damage_Bane, L"Bane"},
    {Damage_Chaos, L"Chaos"},
    {Damage_Lawful, L"Lawful"},
    {Damage_Strength, L"Strength"},
    {Damage_Intelligence, L"Intelligence"},
    {Damage_Wisdom, L"Wisdom"},
    {Damage_Dexterity, L"Dexterity"},
    {Damage_Constitution, L"Constitution"},
    {Damage_Charisma, L"Charisma"},
    {Damage_Ceruleite, L"Ceruleite"},
    {Damage_Crimsonite, L"Crimsonite"},
    {Damage_Gildleaf, L"Gildleaf"},
    {DamageType(0), NULL}
};
