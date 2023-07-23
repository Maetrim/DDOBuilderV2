// BreakdownItemDR.cpp
//
#include "stdafx.h"
#include "BreakdownItemDR.h"
#include "BreakdownsPane.h"

BreakdownItemDR::BreakdownItemDR(
        CBreakdownsPane* pPane,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_DR, this);
}

BreakdownItemDR::~BreakdownItemDR()
{
}

// required overrides
CString BreakdownItemDR::Title() const
{
    return "Damage Reduction";
}

CString BreakdownItemDR::Value() const
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

void BreakdownItemDR::CreateOtherEffects()
{
    // no other effects for this simple item
    m_otherEffects.clear();
}

bool BreakdownItemDR::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemDR::AddEffectToString(
        CString * value,
        const Effect & effect) const
{
    UNREFERENCED_PARAMETER(effect);
    if ((*value) != "")
    {
        (*value) += "   ";
    }
    int amount = static_cast<int>(effect.TotalAmount(true));
    CString txt;
    txt.Format("%d/%s", amount, effect.Item().front().c_str());
    (*value) += txt;
}
