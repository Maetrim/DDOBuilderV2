// BreakdownItemMaximumKi.cpp
//
#include "stdafx.h"
#include "BreakdownItemMaximumKi.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"

BreakdownItemMaximumKi::BreakdownItemMaximumKi(
        CBreakdownsPane * pPane,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(Breakdown_KiMaximum, treeList, hItem)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_KiMaximum, this);
}

BreakdownItemMaximumKi::~BreakdownItemMaximumKi()
{
}

// required overrides
CString BreakdownItemMaximumKi::Title() const
{
    CString text;
    text = "Maximum Ki";
    return text;
}

CString BreakdownItemMaximumKi::Value() const
{
    CString value;

    value.Format(
            "%4d",
            (int)Total());
    return value;
}

void BreakdownItemMaximumKi::CreateOtherEffects()
{
    // add class hit points
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        BreakdownItem * pWisdom = FindBreakdown(Breakdown_Wisdom);
        if (pWisdom != NULL)
        {
            pWisdom->AttachObserver(this);      // need to track changes
            int total = static_cast<int>(pWisdom->Total());
            int wisStatBonus =  BaseStatToBonus(total);
            Effect wisBonus(
                    Effect_KiMaximum,
                    "Wisdom bonus x5",
                    "Ability",
                    wisStatBonus * 5.0);
            AddOtherEffect(wisBonus);
        }
        Effect baseBonus(
                Effect_KiMaximum,
                "Standard Max Ki",
                "Base",
                40);
        AddOtherEffect(baseBonus);
    }
}

bool BreakdownItemMaximumKi::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemMaximumKi::ClassChanged(
        Build * pBuild,
        const std::string& classFrom,
        const std::string& classTo,
        size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    // need to re-create other effects list (class hp)
    CreateOtherEffects();
    Populate();
}

void BreakdownItemMaximumKi::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // total constitution has changed, update
    CreateOtherEffects();
    // do base class stuff also
    BreakdownItem::UpdateTotalChanged(item, type);
}
