// BreakdownItemDestinyAps.cpp
//
#include "stdafx.h"
#include "BreakdownItemDestinyAps.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"

BreakdownItemDestinyAps::BreakdownItemDestinyAps(
        CBreakdownsPane * pPane,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(Breakdown_DestinyPoints, treeList, hItem)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_DestinyAPBonus, this);
    // need to know when total fate points changes
    FindBreakdown(Breakdown_FatePoints)->AttachObserver(this);
}

BreakdownItemDestinyAps::~BreakdownItemDestinyAps()
{
}

// required overrides
CString BreakdownItemDestinyAps::Title() const
{
    CString text;
    text = "Destiny APs";
    return text;
}

CString BreakdownItemDestinyAps::Value() const
{
    CString value;

    value.Format(
            "%4d",
            (int)Total());
    return value;
}

void BreakdownItemDestinyAps::CreateOtherEffects()
{
    // add class hit points
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();

        // Fate point bonus to Destiny APs
        double fatePoints = FindBreakdown(Breakdown_FatePoints)->Total();
        double bonus = static_cast<int>(fatePoints / 3.0);  // drop fractions
        if (bonus != 0)
        {
            Effect fpBonus(
                    Effect_DestinyAPBonus,
                    "Fate Points / 3",
                    "Inherent",
                    bonus);
            AddOtherEffect(fpBonus);
        }
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            size_t level = pBuild->Level();
            if (level > MAX_CLASS_LEVEL)
            {
                level -= MAX_CLASS_LEVEL;
                size_t epicLevels = min(level, 10);
                size_t legendarylevels = level - epicLevels;
                if (epicLevels > 0)
                {
                    Effect elBonus(
                        Effect_DestinyAPBonus,
                        "Epic Levels",
                        "Class",
                        epicLevels * 4);
                    AddOtherEffect(elBonus);
                }
                if (legendarylevels > 0)
                {
                    Effect llBonus(
                        Effect_DestinyAPBonus,
                        "Legendary Levels",
                        "Class",
                        legendarylevels * 4);
                    AddOtherEffect(llBonus);
                }
            }
        }
    }
}

bool BreakdownItemDestinyAps::AffectsUs(const Effect&) const
{
    return true;
}

void BreakdownItemDestinyAps::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // total fate points has changed, update
    CreateOtherEffects();
    // do base class stuff also
    BreakdownItem::UpdateTotalChanged(item, type);
}

void BreakdownItemDestinyAps::BuildLevelChanged(Build* pBuild)
{
    CreateOtherEffects();
    // do base class stuff also
    BreakdownItem::BuildLevelChanged(pBuild);
}

