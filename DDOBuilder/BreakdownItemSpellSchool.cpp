// BreakdownItemSpellSchool.cpp
//
#include "stdafx.h"
#include "BreakdownItemSpellSchool.h"
#include "BreakdownsPane.h"

BreakdownItemSpellSchool::BreakdownItemSpellSchool(
        CBreakdownsPane* pPane,
        BreakdownType type,
        EffectType effect,
        SpellSchoolType ssType,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        bool bSpecificDCOnly) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect),
    m_spellSchoolType(ssType),
    m_bSpecificDCOnly(bSpecificDCOnly)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(effect, this);
}

BreakdownItemSpellSchool::~BreakdownItemSpellSchool()
{
}

// required overrides
CString BreakdownItemSpellSchool::Title() const
{
    return m_title;
}

CString BreakdownItemSpellSchool::Value() const
{
    CString value;
    value.Format(
            "%3d",
            (int)Total());
    return value;
}

void BreakdownItemSpellSchool::CreateOtherEffects()
{
    m_otherEffects.clear();
}

bool BreakdownItemSpellSchool::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    if (effect.HasSpellSchool(m_spellSchoolType, m_bSpecificDCOnly))
    {
        isUs = true;
    }
    return isUs;
}
