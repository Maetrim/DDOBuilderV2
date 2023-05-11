// WeaponGroupRequirements.h
//
// holds a weapon tyep and the requirements for it to be active in the weapon grouping
#pragma once
#include "WeaponTypes.h"
#include "Requirements.h"

class WeaponGroupRequirements
{
    public:
        WeaponGroupRequirements(WeaponType wt, const Requirements& requirements);

        WeaponType Type() const;
        bool RequirementsMet(const Build& build) const;

        bool operator==(const WeaponGroupRequirements& other) const;

    private:
        WeaponType m_weaponType;
        Requirements m_requirements;
};
