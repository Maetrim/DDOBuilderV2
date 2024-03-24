// BreakdownItemOffhandDoublestrike.cpp
//
#include "stdafx.h"
#include "BreakdownItemOffhandDoublestrike.h"
#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

const std::string c_PTWF = "Perfect Two Weapon Fighting";

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
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        double mainhandDoublestrike = pBI->Total();
        if (mainhandDoublestrike > 0)
        {
            std::string name;
            if (pBuild->IsFeatTrained(c_PTWF))
            {
                // offhand doublestrike procs at half the rate of the main hand +15% due to PWTF
                mainhandDoublestrike *= 0.65;
                name = "65% of main hand doublestrike";
            }
            else
            {
                // offhand doublestrike procs at half the rate of the main hand
                mainhandDoublestrike /= 2;
                name = "50% of main hand doublestrike";
            }
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

void BreakdownItemOffhandDoublestrike::FeatTrained(
    Build* pBuild,
    const std::string& featName)
{
    UNREFERENCED_PARAMETER(pBuild);
    // Perfect Two Weapon Fighting affects max doublestrike chance
    if (featName == c_PTWF)
    {
        CreateOtherEffects();
    }
}

void BreakdownItemOffhandDoublestrike::FeatRevoked(
    Build* pBuild,
    const std::string& featName)
{
    UNREFERENCED_PARAMETER(pBuild);
    // Perfect Two Weapon Fighting affects max doublestrike chance
    if (featName == c_PTWF)
    {
        CreateOtherEffects();
    }
}

