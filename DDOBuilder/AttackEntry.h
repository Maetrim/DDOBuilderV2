// AttackEntry.h
//
// Tracks the damage setup for the given attack in the AttackChain
// not saved as generated on the fly
#pragma once
#include "Dice.h"

class AttackEntry
{
    public:
        AttackEntry(void);
        ~AttackEntry();

    private:
        // weapon specific
        Dice m_weaponDice;
        double m_weaponW;
        double m_damageBonus;
        double m_criticalDamageBonus;
        double m_criticalThreatRange;
        double m_criticalMultiplier;
        double m_criticalMultiplier19to20;
        double m_vorpalRange;
        double m_weaponAlacrity;
        // melee specific
        double m_meleePower;
        double m_doublestrikeChance;
        double m_mainhandAbilityMulitplier;
        double m_helplessDamageBonus;
        // ranged specific
        double m_rangedPower;
        double m_doubleshotChance;
        double m_misslesPerShot;
        // THF specific
        double m_strikethroughChance;
        // TWF specific
        Dice m_offhandWeaponDice;
        double m_offhandWeaponW;
        double m_offhandAttackChance;
        double m_offhandDoublestrikeChance;
        double m_offhandAbilityMulitplier;
        double m_offhandDamageBonus;
        double m_offhandCriticalDamageBonus;
        double m_offhandCriticalMultiplier;
        double m_offhandCriticalMultiplier19to20;
        double m_offhandVorpalRange;
        double m_offhandWeaponAlacrity;
};
