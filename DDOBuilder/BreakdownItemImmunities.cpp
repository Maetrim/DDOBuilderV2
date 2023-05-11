// BreakdownItemImmunities.cpp
//
#include "stdafx.h"
#include "BreakdownItemImmunities.h"
#include "BreakdownsPane.h"

BreakdownItemImmunities::BreakdownItemImmunities(
        CBreakdownsPane* pPane,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_Immunity, this);
}

BreakdownItemImmunities::~BreakdownItemImmunities()
{
}

// required overrides
CString BreakdownItemImmunities::Title() const
{
    return "Immunities";
}

CString BreakdownItemImmunities::Value() const
{
    // value is the concatenation of all the individual effects
    CString value;
    // just append all the items together, each should be an effect
    // which as a dice item
    std::list<Effect>::const_iterator it = m_otherEffects.begin();
    while (it != m_otherEffects.end())
    {
        AddEffectToString(&value, (*it));
        ++it;
    }
    it = m_effects.begin();
    while (it != m_effects.end())
    {
        AddEffectToString(&value, (*it));
        ++it;
    }
    it = m_itemEffects.begin();
    while (it != m_itemEffects.end())
    {
        AddEffectToString(&value, (*it));
        ++it;
    }
    return value;
}

void BreakdownItemImmunities::CreateOtherEffects()
{
    // no other effects for this simple item
    m_otherEffects.clear();
}

bool BreakdownItemImmunities::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemImmunities::AddEffectToString(
        CString * value,
        const Effect & effect) const
{
    UNREFERENCED_PARAMETER(effect);
    if ((*value) != "")
    {
        (*value) += ", ";
    }
    (*value) += "NYI";
    //(*value) += effect.Description().c_str();
}
