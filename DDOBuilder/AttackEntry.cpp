// AttackEntry.cpp
//
#include "StdAfx.h"
#include "AttackEntry.h"
#include "GlobalSupportFunctions.h"
#include "BreakdownItem.h"

AttackEntry::AttackEntry() :
    //m_weaponDice(0),
    m_weaponW(0),
    m_damageBonus(0),
    m_criticalDamageBonus(0),
    m_criticalThreatRange(0),
    m_criticalMultiplier(0),
    m_criticalMultiplier19to20(0),
    m_vorpalRange(0),
    m_weaponAlacrity(0),
    m_meleePower(0),
    m_doublestrikeChance(0),
    m_mainhandAbilityMulitplier(0),
    m_helplessDamageBonus(0),
    m_rangedPower(0),
    m_doubleshotChance(0),
    m_misslesPerShot(0),
    m_strikethroughChance(0),
    //m_offhandWeaponDice(0),
    m_offhandWeaponW(0),
    m_offhandAttackChance(0),
    m_offhandDoublestrikeChance(0),
    m_offhandAbilityMulitplier(0),
    m_offhandDamageBonus(0),
    m_offhandCriticalDamageBonus(0),
    m_offhandCriticalMultiplier(0),
    m_offhandCriticalMultiplier19to20(0),
    m_offhandVorpalRange(0),
    m_offhandWeaponAlacrity(0)
{
}

AttackEntry::~AttackEntry()
{
}

void AttackEntry::Initialise()
{
    // get all the current breakdown values for standard parameters used to calculate Attack Chain data

    // melee specific
    m_meleePower = GetBreakdownValue(Breakdown_MeleePower);
    m_doublestrikeChance = GetBreakdownValue(Breakdown_DoubleStrike);
    m_mainhandAbilityMulitplier = GetBreakdownValue(Breakdown_DamageAbilityMultiplier);
    m_helplessDamageBonus = GetBreakdownValue(Breakdown_HelplessDamage);
    // ranged specific
    m_rangedPower = GetBreakdownValue(Breakdown_RangedPower);
    m_doubleshotChance = GetBreakdownValue(Breakdown_DoubleShot);
    //m_misslesPerShot = GetBreakdownValue();
    // THF specific
    m_strikethroughChance = GetBreakdownValue(Breakdown_Strikethrough);
    // TWF specific
    //Dice m_offhandWeaponDice = GetBreakdownValue();
    ///m_offhandWeaponW = GetBreakdownValue();
    //m_offhandAttackChance = GetBreakdownValue();
    m_offhandDoublestrikeChance = GetBreakdownValue(Breakdown_DoublestrikeOffhand);
    m_offhandAbilityMulitplier = GetBreakdownValue(Breakdown_DamageAbilityMultiplierOffhand);
    //m_offhandDamageBonus = GetBreakdownValue();
    //m_offhandCriticalDamageBonus = GetBreakdownValue();
    //m_offhandCriticalMultiplier = GetBreakdownValue();
    //m_offhandCriticalMultiplier19to20 = GetBreakdownValue();
    //m_offhandVorpalRange = GetBreakdownValue();
    //m_offhandWeaponAlacrity = GetBreakdownValue();
/*    // main hand weapon
    Dice m_weaponDice;
    double m_weaponW;
    double m_damageBonus;
    double m_criticalDamageBonus;
    double m_criticalThreatRange;
    double m_criticalMultiplier;
    double m_criticalMultiplier19to20;
    double m_vorpalRange;
    double m_weaponAlacrity;*/
}

double AttackEntry::GetBreakdownValue(BreakdownType type)
{
    BreakdownItem* pBD = FindBreakdown(type);
    double value = pBD->Total();
    return value;
}
