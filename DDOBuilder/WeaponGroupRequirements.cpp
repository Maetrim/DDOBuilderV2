// WeaponGroupRequirements.cpp
//
#include "stdafx.h"
#include "WeaponGroupRequirements.h"
#include "Build.h"

WeaponGroupRequirements::WeaponGroupRequirements(
        WeaponType wt,
        const Requirements& requirements) :
    m_weaponType(wt),
    m_requirements(requirements)
{
}

WeaponType WeaponGroupRequirements::Type() const
{
    return m_weaponType;
}

bool WeaponGroupRequirements::RequirementsMet(const Build& build) const
{
    return m_requirements.Met(build, build.Level() - 1, true, Inventory_Unknown, m_weaponType, Weapon_Unknown);
}

bool WeaponGroupRequirements::operator==(const WeaponGroupRequirements& other) const
{
    return (m_weaponType == other.m_weaponType)
            && (m_requirements == other.m_requirements);
}
