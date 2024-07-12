// WeaponClassTypes.h
//
#pragma once

#include "XmlLib\SaxContentElement.h"

// Special note - The  "Unknown" entries of each enum must be entry 0 in the
// enum map as this is used by the SAX loaders to check for badly loaded values
// these enumerations are used across multiple object types

// weapon class types
enum WeaponClassType
{
    WeaponClass_Unknown = 0,
    WeaponClass_Thrown,
    WeaponClass_Unarmed,
    WeaponClass_OneHanded,
    WeaponClass_Ranged,
    WeaponClass_TwoHanded,
    WeaponClass_Axe,
    WeaponClass_Bows,
    WeaponClass_Crossbows,
    WeaponClass_ReapeatingCrossbows,
    WeaponClass_Melee,
    WeaponClass_Light,
    WeaponClass_Martial,
    WeaponClass_Simple,
    WeaponClass_Exotic,
    WeaponClass_Finesseable,
    WeaponClass_FocusGroup,
    WeaponClass_Shield,
};
const XmlLib::enumMapEntry<WeaponClassType> weaponClassTypeMap[] =
{
    {WeaponClass_Unknown, L"Unknown"},
    {WeaponClass_Thrown, L"Thrown"},
    {WeaponClass_Unarmed, L"Unarmed"},
    {WeaponClass_OneHanded, L"One Handed"},
    {WeaponClass_Ranged, L"Ranged"},
    {WeaponClass_TwoHanded, L"Two Handed"},
    {WeaponClass_Axe, L"Axe"},
    {WeaponClass_Bows, L"Bows"},
    {WeaponClass_Crossbows, L"Crossbows"},
    {WeaponClass_ReapeatingCrossbows, L"RepeatingCrossbows"},
    {WeaponClass_Melee, L"Melee"},
    {WeaponClass_Light, L"Light"},
    {WeaponClass_Martial, L"Martial"},
    {WeaponClass_Simple, L"Simple"},
    {WeaponClass_Exotic, L"Exotic"},
    {WeaponClass_Finesseable, L"Finesseable"},
    {WeaponClass_FocusGroup, L"FocusGroup"},
    {WeaponClass_Shield, L"Shield"},
    {WeaponClassType(0), NULL}
};
