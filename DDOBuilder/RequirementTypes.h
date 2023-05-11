// RequirementTypes.h
//
#pragma once

#include "XmlLib\SaxContentElement.h"

// Special note - The  "Unknown" entries of each enum must be entry 0 in the
// enum map as this is used by the SAX loaders to check for badly loaded values
// these enumerations are used across multiple object types
enum RequirementType
{
    Requirement_Unknown = 0,
    // Actual requirement types supported
    Requirement_Ability,                    // must have at least this ability value
    Requirement_AbilityGreaterCondition,    // the first ability value must be greater than the second
    Requirement_Alignment,                  // must have one of the listed  alignments
    Requirement_BAB,                        // must have at least the listed BAB
    Requirement_BaseClass,                  // must have at least 1 level of this base class
    Requirement_BaseClassAtLevel,           // must have a specific base class level
    Requirement_BaseClassMinLevel,          // must have at least this level in base class
    Requirement_Class,                      // must have at least 1 level of this class
    Requirement_ClassAtLevel,               // must have a specific class level
    Requirement_ClassMinLevel,              // must have at least this level in class
    Requirement_EnemyType,                  // specific enemy type
    Requirement_Enhancement,                // must have the listed enhancement/selection/ranks
    Requirement_Exclusive,                  // must be the named exclusive item
    Requirement_Feat,                       // must have this feat trained (not granted)
    Requirement_FeatAnySource,              // must have this feat from any source (except equipment)
    Requirement_GroupMember,                // weapon has to be a member of this named group type (group does not have to exist)
    Requirement_GroupMember2,               // off hand weapon has to be a member of this named group type (group does not have to exist)
    Requirement_ItemTypeInSlot,             // specific item type needs to be in a specified slot
    Requirement_Level,                      // must be at least this level
    Requirement_Race,                       // must have at leats one of listed Races
    Requirement_Skill,                      // must have at least the listed skill points
    Requirement_SpecificLevel,              // must be this level
    Requirement_Stance,                     // the specified stance must be active
    Requirement_WeaponTypesEquipped,        // the specified weapon types must be equipped in weapon1/2
    Requirement_WeaponClassMainHand,        // Main hand weapon is a "type" e.g. Light weapon
    Requirement_WeaponClassOffHand,         // off hand weapon is a "type" e.g. Heavy weapon
};
const XmlLib::enumMapEntry<RequirementType> requirementTypeMap[] =
{
    {Requirement_Unknown,                   L"Unknown"},
    {Requirement_Ability,                   L"Ability"},
    {Requirement_AbilityGreaterCondition,   L"AbilityGreaterCondition"},
    {Requirement_Alignment,                 L"Alignment"},
    {Requirement_BAB,                       L"BAB"},
    {Requirement_BaseClass,                 L"BaseClass"},
    {Requirement_BaseClassAtLevel,          L"BaseClassAtLevel"},
    {Requirement_BaseClassMinLevel,         L"BaseClassMinLevel"},
    {Requirement_Class,                     L"Class"},
    {Requirement_ClassAtLevel,              L"ClassAtLevel"},
    {Requirement_ClassMinLevel,             L"ClassMinLevel"},
    {Requirement_EnemyType,                 L"EnemyType"},
    {Requirement_Enhancement,               L"Enhancement"},
    {Requirement_Exclusive,                 L"Exclusive"},
    {Requirement_Feat,                      L"Feat"},
    {Requirement_FeatAnySource,             L"FeatAnySource"},
    {Requirement_GroupMember,               L"GroupMember"},
    {Requirement_GroupMember2,              L"GroupMember2"},
    {Requirement_ItemTypeInSlot,            L"ItemTypeInSlot"},
    {Requirement_Level,                     L"Level"},
    {Requirement_Race,                      L"Race"},
    {Requirement_Skill,                     L"Skill"},
    {Requirement_SpecificLevel,             L"SpecificLevel"},
    {Requirement_Stance,                    L"Stance"},
    {Requirement_WeaponTypesEquipped,       L"WeaponTypesEquipped"},
    {Requirement_WeaponClassMainHand,       L"WeaponClassMainHand"},
    {Requirement_WeaponClassOffHand,        L"WeaponClassOffHand"},
    {RequirementType(0), NULL}
};
