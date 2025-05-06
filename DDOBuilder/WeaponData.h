// WeaponData.h
//
// Tracks the damage setup for the given attack in the AttackChain
// not saved as generated on the fly
#pragma once

#include "BasicDice.h"
#include "BreakdownTypes.h"
#include "InventorySlotTypes.h"

class WeaponData
{
    public:
        WeaponData(InventorySlotType slot);
        ~WeaponData();

        void Initialise();

    private:
        double GetBreakdownValue(BreakdownType type);

        // weapon specific
        BasicDice m_weaponDice;
        double m_weaponW;
        double m_damageBonus;
        double m_criticalDamageBonus;
        double m_criticalThreatRange;
        double m_criticalMultiplier;
        double m_criticalMultiplier19to20;
        double m_vorpalRange;
        double m_weaponAlacrity;
};
