// BreakdownItemTurnUndeadLevel.cpp
//
#include "stdafx.h"
#include "BreakdownItemTurnUndeadLevel.h"
#include "Class.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemTurnUndeadLevel::BreakdownItemTurnUndeadLevel(
        CBreakdownsPane* pPane,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_TurnLevelBonus, this);
}

BreakdownItemTurnUndeadLevel::~BreakdownItemTurnUndeadLevel()
{
}

// required overrides
CString BreakdownItemTurnUndeadLevel::Title() const
{
    CString text("Turn Undead Level");
    return text;
}

CString BreakdownItemTurnUndeadLevel::Value() const
{
    CString value;

    value.Format(
            "%d",
            (int)Total());
    return value;
}

void BreakdownItemTurnUndeadLevel::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();

        Life* pLife = m_pCharacter->ActiveLife();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pLife != NULL
            && pBuild != NULL)
        {
            // your base level is the higher off:
            // effective cleric level or effective paladin level - 3
            // get the caster level breakdowns for cleric and paladin
            BreakdownType cbt = static_cast<BreakdownType>(Breakdown_CasterLevel_First + FindClass("Cleric").Index());
            BreakdownType pbt = static_cast<BreakdownType>(Breakdown_CasterLevel_First + FindClass("Paladin").Index());
            BreakdownItem * pCB = FindBreakdown(cbt);
            pCB->AttachObserver(this);
            BreakdownItem * pPB = FindBreakdown(pbt);
            pPB->AttachObserver(this);
            double clericLevels = pCB->Total();
            double paladinLevels = pPB->Total() - 3;
            if (clericLevels >= paladinLevels)
            {
                Effect cl(
                        Effect_Unknown,
                        "Cleric Levels",
                        "Cleric Levels",
                        clericLevels);
                AddOtherEffect(cl);
            }
            else
            {
                Effect pl(
                        Effect_Unknown,
                        "Effective Cleric Levels (Paladin-3)",
                        "Effective Cleric Levels (Paladin-3)",
                        paladinLevels);
                AddOtherEffect(pl);
            }
        }
    }
}

bool BreakdownItemTurnUndeadLevel::AffectsUs(const Effect &) const
{
    return true;
}

// BreakdownObserver overrides
void BreakdownItemTurnUndeadLevel::UpdateTotalChanged(BreakdownItem * item, BreakdownType type)
{
    // and also call the base class
    BreakdownItem::UpdateTotalChanged(item, type);
    // a class level that a base effect depends on has changed
    CreateOtherEffects();
    Populate();
}

void BreakdownItemTurnUndeadLevel::ClassChanged(
        Build* pBuild,
        const std::string& classFrom,
    const std::string& classTo,
        size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    // need to re-create other effects list
    CreateOtherEffects();
    Populate();
}
