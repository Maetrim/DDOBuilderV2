// WeaponData.cpp
//
#include "StdAfx.h"
#include "WeaponData.h"
#include "GlobalSupportFunctions.h"
#include "BreakdownItem.h"
#include "BreakdownItemWeaponEffects.h"

WeaponData::WeaponData(InventorySlotType it) :
    m_weaponDice(L""),
    m_weaponW(0),
    m_damageBonus(0),
    m_criticalDamageBonus(0),
    m_criticalThreatRange(0),
    m_criticalMultiplier(0),
    m_criticalMultiplier19to20(0),
    m_vorpalRange(0),
    m_weaponAlacrity(0)
{
    bool bMainHand = (it == Inventory_Weapon1);
    BreakdownItem* pBI = FindBreakdown(Breakdown_WeaponEffectHolder);
    BreakdownItemWeaponEffects* pBIW = dynamic_cast<BreakdownItemWeaponEffects*>(pBI);
    if (pBIW != NULL)
    {
        m_weaponDice = pBIW->WeaponDice(bMainHand);
        pBI = pBIW->GetWeaponBreakdown(bMainHand, Breakdown_WeaponBaseDamage);
        if (NULL != pBI)
        {
            m_weaponW = pBI->Total();
        }
        pBI = pBIW->GetWeaponBreakdown(bMainHand, Breakdown_WeaponDamageBonus);
        if (NULL != pBI)
        {
            m_damageBonus = pBI->Total();
        }
        pBI = pBIW->GetWeaponBreakdown(bMainHand, Breakdown_WeaponCriticalDamageBonus);
        if (NULL != pBI)
        {
            m_criticalDamageBonus = pBI->Total();
        }
        pBI = pBIW->GetWeaponBreakdown(bMainHand, Breakdown_WeaponCriticalThreatRange);
        if (NULL != pBI)
        {
            m_criticalThreatRange = pBI->Total();
        }
        pBI = pBIW->GetWeaponBreakdown(bMainHand, Breakdown_WeaponCriticalMultiplier);
        if (NULL != pBI)
        {
            m_criticalMultiplier = pBI->Total();
        }
        pBI = pBIW->GetWeaponBreakdown(bMainHand, Breakdown_WeaponCriticalMultiplier19To20);
        if (NULL != pBI)
        {
            m_criticalMultiplier19to20 = pBI->Total();
        }
        pBI = pBIW->GetWeaponBreakdown(bMainHand, Breakdown_WeaponVorpalRange);
        if (NULL != pBI)
        {
            m_vorpalRange = pBI->Total();
        }
        pBI = pBIW->GetWeaponBreakdown(bMainHand, Breakdown_WeaponAttackSpeed);
        if (NULL != pBI)
        {
            m_weaponAlacrity = pBI->Total();
        }
    }
}

WeaponData::~WeaponData()
{
}

void WeaponData::Initialise()
{
    // get all the current breakdown values for standard parameters used to calculate Attack Chain data
}

double WeaponData::GetBreakdownValue(BreakdownType type)
{
    BreakdownItem* pBD = FindBreakdown(type);
    double value = pBD->Total();
    return value;
}
