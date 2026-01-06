// BreakdownItemTurnUndeadHitDice.cpp
//
#include "stdafx.h"
#include "BreakdownItemTurnUndeadHitDice.h"
#include "Class.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemTurnUndeadHitDice::BreakdownItemTurnUndeadHitDice(
        CBreakdownsPane* pPane,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_TurnDiceBonus, this);
}

BreakdownItemTurnUndeadHitDice::~BreakdownItemTurnUndeadHitDice()
{
}

// required overrides
CString BreakdownItemTurnUndeadHitDice::Title() const
{
    CString text("Turn Undead Hit Dice");
    return text;
}

CString BreakdownItemTurnUndeadHitDice::Value() const
{
    CString value;

    value.Format(
            "2d6 + %d",
            (int)Total());
    return value;
}

void BreakdownItemTurnUndeadHitDice::CreateOtherEffects()
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
            BreakdownType dabt = static_cast<BreakdownType>(Breakdown_CasterLevel_First + FindClass("Dark Apostate").Index());
            BreakdownType pbt = static_cast<BreakdownType>(Breakdown_CasterLevel_First + FindClass("Paladin").Index());
            BreakdownItem * pCB = FindBreakdown(cbt);
            pCB->AttachObserver(this);
            BreakdownItem * pDAB = FindBreakdown(dabt);
            pDAB->AttachObserver(this);
            BreakdownItem * pPB = FindBreakdown(pbt);
            pPB->AttachObserver(this);
            double clericLevels = pCB->Total();
            double darkApostateLevels = pDAB->Total();
            clericLevels = max(clericLevels, darkApostateLevels);
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

            // also includes your charisma modifier
            BreakdownItem * pBI = FindBreakdown(StatToBreakdown(Ability_Charisma));
            ASSERT(pBI != NULL);
            pBI->AttachObserver(this);  // need to know about changes
            int bonus = BaseStatToBonus(pBI->Total());
            if (bonus != 0) // only add to list if non zero
            {
                // should now have the best option
                Effect feat(
                        Effect_Unknown,
                        "Ability bonus (Charisma)",
                        "Ability bonus (Charisma)",
                        bonus);
                AddOtherEffect(feat);
            }
        }
    }
}

bool BreakdownItemTurnUndeadHitDice::AffectsUs(const Effect &) const
{
    return true;
}

// BreakdownObserver overrides
void BreakdownItemTurnUndeadHitDice::UpdateTotalChanged(BreakdownItem * item, BreakdownType type)
{
    // and also call the base class
    BreakdownItem::UpdateTotalChanged(item, type);
    // a class level that a base effect depends on has changed
    CreateOtherEffects();
    Populate();
}

void BreakdownItemTurnUndeadHitDice::ClassChanged(
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

//void BreakdownItemTurnUndeadHitDice::UpdateSpellTrained(
//        Character * charData,
//        const TrainedSpell & spell)
//{
//    BreakdownItem::UpdateSpellTrained(charData, spell);
//    // need to re-create other effects list
//    CreateOtherEffects();
//    Populate();
//}
//
//void BreakdownItemTurnUndeadHitDice::UpdateSpellRevoked(
//        Character * charData,
//        const TrainedSpell & spell)
//{
//    BreakdownItem::UpdateSpellRevoked(charData, spell);
//    // need to re-create other effects list
//    CreateOtherEffects();
//    Populate();
//}
