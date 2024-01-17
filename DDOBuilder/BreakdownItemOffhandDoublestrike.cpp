// BreakdownItemOffhandDoublestrike.cpp
//
#include "stdafx.h"
#include "BreakdownItemOffhandDoublestrike.h"
#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemOffhandDoublestrike::BreakdownItemOffhandDoublestrike(
        CBreakdownsPane* pPane,
        BreakdownType type,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_title(title)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_DoublestrikeOffhand, this);
    BreakdownItem* pBI = FindBreakdown(Breakdown_DoubleStrike);
    pBI->AttachObserver(this);
}

BreakdownItemOffhandDoublestrike::~BreakdownItemOffhandDoublestrike()
{
}

// required overrides
CString BreakdownItemOffhandDoublestrike::Title() const
{
    return m_title;
}

CString BreakdownItemOffhandDoublestrike::Value() const
{
    CString value;

    value.Format(
        "%4d",
        (int)Total());
    return value;
}

void BreakdownItemOffhandDoublestrike::CreateOtherEffects()
{
    m_otherEffects.clear();
    
    // we need to know when these breakdown values changes
    BreakdownItem * pBI = FindBreakdown(Breakdown_DoubleStrike);
    if (m_pCharacter->ActiveBuild() != NULL)
    {
        double mainhandDoublestrike = pBI->Total();
        if (mainhandDoublestrike > 0)
        {
            // offhand doublestrike procs at half the rate of the main hand
            mainhandDoublestrike /= 2;
            Effect mainhandBonus(
                    Effect_DoublestrikeOffhand,
                    "Half main hand doublestrike",
                    "Base",
                    mainhandDoublestrike);
            AddOtherEffect(mainhandBonus);
        }
    }
}

bool BreakdownItemOffhandDoublestrike::AffectsUs(const Effect &) const
{
    return true;
}

// BreakdownObserver overrides
void BreakdownItemOffhandDoublestrike::UpdateTotalChanged(
    BreakdownItem* item,
    BreakdownType type)
{
    // Base doublestrike has changed
    CreateOtherEffects();
    // and finally call the base class
    BreakdownItem::UpdateTotalChanged(item, type);
    Populate();
}
