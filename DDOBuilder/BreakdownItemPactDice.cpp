// BreakdownItemPactDice.cpp
//
#include "stdafx.h"
#include "BreakdownItemPactDice.h"
#include "BreakdownsPane.h"

BreakdownItemPactDice::BreakdownItemPactDice(
        CBreakdownsPane* pPane,
        BreakdownType type,
        EffectType effect,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        size_t diceSize) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect),
    m_diceSize(diceSize)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(effect, this);
}

BreakdownItemPactDice::~BreakdownItemPactDice()
{
}

// required overrides
CString BreakdownItemPactDice::Title() const
{
    return m_title;
}

CString BreakdownItemPactDice::Value() const
{
    CString value;
    int totalDice = static_cast<int>(Total());
    value.Format("%dd%d", totalDice, m_diceSize);
    return value;
}

void BreakdownItemPactDice::CreateOtherEffects()
{
    // no other effects for this simple item
    m_otherEffects.clear();
}

bool BreakdownItemPactDice::AffectsUs(const Effect &) const
{
    return true;
}
