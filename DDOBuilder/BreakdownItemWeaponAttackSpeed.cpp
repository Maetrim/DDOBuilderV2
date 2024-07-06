// BreakdownItemWeaponAttackSpeed.cpp
//
#include "stdafx.h"
#include "BreakdownItemWeaponAttackSpeed.h"

#include "GlobalSupportFunctions.h"
#include "Character.h"
#include "BreakdownItemWeaponEffects.h"

BreakdownItemWeaponAttackSpeed::BreakdownItemWeaponAttackSpeed(
        CBreakdownsPane* pPane,
        BreakdownType type,
        EffectType effect,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItemSimple(pPane, type, effect, title, treeList, hItem, false),
    m_title(title),
    m_effect(effect)
{
}

BreakdownItemWeaponAttackSpeed::~BreakdownItemWeaponAttackSpeed()
{
}

// required overrides
CString BreakdownItemWeaponAttackSpeed::Title() const
{
    return m_title;
}

CString BreakdownItemWeaponAttackSpeed::Value() const
{
    CString value;
    value.Format(
            "%+3d%%",
            (int)Total());
    return value;
}

void BreakdownItemWeaponAttackSpeed::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
    }
}

bool BreakdownItemWeaponAttackSpeed::AffectsUs(const Effect & effect) const
{
    // note that we only get here if the effect is for this weapon type
    // we just need to filter on the effects we are interested in
    bool isUs = false;
    if (effect.IsType(m_effect))
    {
        // if its the right effect its for us as our holder class determines whether
        // it is the right weapon target type
        isUs = true;
    }
    if (effect.IsType(Effect_WeaponAlacrityClass))
    {
        isUs = true;
    }
    return isUs;
}
