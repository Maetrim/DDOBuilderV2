// BreakdownItemCasterLevel.cpp
//
#include "stdafx.h"
#include "BreakdownItemSchoolCasterLevel.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"

BreakdownItemSchoolCasterLevel::BreakdownItemSchoolCasterLevel(
        CBreakdownsPane* pPane,
        EffectType et,
        SpellSchoolType schoolType,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_school(schoolType)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(et, this);
}

BreakdownItemSchoolCasterLevel::~BreakdownItemSchoolCasterLevel()
{
}

// required overrides
CString BreakdownItemSchoolCasterLevel::Title() const
{
    return "";
}

CString BreakdownItemSchoolCasterLevel::Value() const
{
    CString value;
    value.Format(
            "%+3d",          // Caster level values are always whole numbers
            (int)Total());
    return value;
}

void BreakdownItemSchoolCasterLevel::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
    }
}

bool BreakdownItemSchoolCasterLevel::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    // see if this effect applies to us
    if (effect.HasSpellSchool(m_school))
    {
        // it is a caster level school bonus
        isUs = true;
    }
    return isUs;
}
