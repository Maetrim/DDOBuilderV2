// BreakdownItemWeaponVorpalRange.cpp
//
#include "stdafx.h"
#include "BreakdownItemWeaponVorpalRange.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemWeaponVorpalRange::BreakdownItemWeaponVorpalRange(
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem)
{
}

BreakdownItemWeaponVorpalRange::~BreakdownItemWeaponVorpalRange()
{
}

// required overrides
CString BreakdownItemWeaponVorpalRange::Title() const
{
    return "Vorpal Range";
}

CString BreakdownItemWeaponVorpalRange::Value() const
{
    CString value;
    int total = (int)Total();
    if (total > 1)
    {
        value.Format("%d-20", 21 - total);  // e.g. 19-20
    }
    else
    {
        value = "20";
    }
    return value;
}

void BreakdownItemWeaponVorpalRange::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
    }
}

bool BreakdownItemWeaponVorpalRange::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    if (effect.IsType(Effect_Weapon_VorpalRange))
    {
        // if its the right effect its for us as our holder class determines whether
        // it is the right weapon target type
        isUs = true;
    }
    return isUs;
}
