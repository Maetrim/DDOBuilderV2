// BreakdownItemSneakAttackDice.cpp
//
#include "stdafx.h"
#include "BreakdownItemSneakAttackDice.h"
#include "BreakdownsPane.h"

BreakdownItemSneakAttackDice::BreakdownItemSneakAttackDice(
        CBreakdownsPane* pPane,
        BreakdownType type,
        EffectType effect,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(effect, this);
}

BreakdownItemSneakAttackDice::~BreakdownItemSneakAttackDice()
{
}

// required overrides
CString BreakdownItemSneakAttackDice::Title() const
{
    return m_title;
}

CString BreakdownItemSneakAttackDice::Value() const
{
    CString value;
    value.Format(
            "%dD6",
            (int)Total());
    return value;
}

void BreakdownItemSneakAttackDice::CreateOtherEffects()
{
    // no other effects for this simple item
    m_otherEffects.clear();
}

bool BreakdownItemSneakAttackDice::AffectsUs(const Effect &) const
{
    return true;
}
