// BreakdownItemWeaponCriticalMultiplier.cpp
//
#include "stdafx.h"
#include "BreakdownItemWeaponCriticalMultiplier.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemWeaponCriticalMultiplier::BreakdownItemWeaponCriticalMultiplier(
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        BreakdownItem * pBaseTotal) :
    BreakdownItem(type, treeList, hItem),
    m_pBaseTotal(pBaseTotal)
{
    if (m_pBaseTotal != NULL)
    {
        m_pBaseTotal->AttachObserver(this);
    }
}

BreakdownItemWeaponCriticalMultiplier::~BreakdownItemWeaponCriticalMultiplier()
{
}

// required overrides
CString BreakdownItemWeaponCriticalMultiplier::Title() const
{
    if (Type() == Breakdown_WeaponCriticalMultiplier)
    {
        return "Critical Multiplier";
    }
    else
    {
        return "Critical Multiplier (19-20)";
    }
}

CString BreakdownItemWeaponCriticalMultiplier::Value() const
{
    CString value;
    value.Format("%d", (int)Total());
    return value;
}

void BreakdownItemWeaponCriticalMultiplier::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        if (Type() == Breakdown_WeaponCriticalMultiplier19To20)
        {
            // we need this weapons standard multiplier as our base
            ASSERT(m_pBaseTotal != NULL);
            double base = m_pBaseTotal->Total();
            if (base != 0)
            {
                Effect keenEffect(
                        Effect_Weapon_CriticalMultiplier,
                        "Standard Multiplier",
                        "Base",
                        base);
                AddOtherEffect(keenEffect);
            }
        }
    }
}

bool BreakdownItemWeaponCriticalMultiplier::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    if (effect.IsType(Effect_Weapon_CriticalMultiplier)
            && Type() == Breakdown_WeaponCriticalMultiplier)
    {
        // if its the right effect its for us as our holder class determines whether
        // it is the right weapon target type
        isUs = true;
    }
    if (effect.IsType(Effect_Weapon_CriticalMultiplier19To20)
            && Type() == Breakdown_WeaponCriticalMultiplier19To20)
    {
        // if its the right effect its for us as our holder class determines whether
        // it is the right weapon target type
        isUs = true;
    }
    if (effect.IsType(Effect_WeaponCriticalMultiplierClass)
        && Type() == Breakdown_WeaponCriticalMultiplier)
    {
        // if its the right effect its for us as our holder class determines whether
        // it is the right weapon target type
        isUs = true;
    }
    return isUs;
}

void BreakdownItemWeaponCriticalMultiplier::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    BreakdownItem::UpdateTotalChanged(item, type);
    // our base multiplier value has changed
    CreateOtherEffects();
    Populate();
}

