// BreakdownItemAC.cpp
//
#include "stdafx.h"
#include "BreakdownItemAC.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"

BreakdownItemAC::BreakdownItemAC(
        CBreakdownsPane* pPane,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_ACBonus, this);
}

BreakdownItemAC::~BreakdownItemAC()
{
}

// required overrides
CString BreakdownItemAC::Title() const
{
    CString text = "AC";
    return text;
}

CString BreakdownItemAC::Value() const
{
    CString value;
    value.Format(
            "%3d",
            (int)Total());
    return value;
}

void BreakdownItemAC::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        Life* pLife = m_pCharacter->ActiveLife();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pLife != NULL
            && pBuild != NULL)
        {
            // need to know about Max Dex bonus, Dexterity, Max Dex Bonus shields
            BreakdownItem * pBIDex = FindBreakdown(StatToBreakdown(Ability_Dexterity));
            pBIDex->AttachObserver(this);  // need to know about changes
            int dexBonus = BaseStatToBonus(pBIDex->Total());
            bool bCappedByMDB = false;
            bool bCappedByMDBShields = false;

            // this value is limited by MDB
            BreakdownItem * pBIMDB = FindBreakdown(Breakdown_MaxDexBonus);
            pBIMDB->AttachObserver(this);
            int mdb = (int)pBIMDB->Total();
            if (dexBonus > mdb)
            {
                bCappedByMDB = true;
                dexBonus = mdb;
            }
            // MDB Shields affects max if a tower shield is in use
            BreakdownItem * pBIMDBShields = FindBreakdown(Breakdown_MaxDexBonusShields);
            pBIMDBShields->AttachObserver(this);
            if (pBuild->IsStanceActive("Tower Shield"))
            {
                int mdbShields = (int)pBIMDBShields->Total();
                if (dexBonus > mdbShields)
                {
                    bCappedByMDBShields = true;
                    dexBonus = mdbShields;
                }
            }
            // should now have the best option
            if (!bCappedByMDB && !bCappedByMDBShields)
            {
                Effect feat(
                        Effect_AbilityBonus,
                        "Dex Bonus",
                        "Dex Bonus",
                        dexBonus);
                AddOtherEffect(feat);
            }
            else if (bCappedByMDBShields)
            {
                // capped by shields max dex bonus
                Effect feat(
                        Effect_AbilityBonus,
                        "Dex Bonus (MDB Tower Shield Capped)",
                        "Dex Bonus (MDB Tower Shield Capped)",
                        dexBonus);
                AddOtherEffect(feat);
            }
            else
            {
                // capped by armor max dex bonus
                Effect feat(
                        Effect_AbilityBonus,
                        "Dex Bonus (MDB Capped)",
                        "Dex Bonus (MDB Capped)",
                        dexBonus);
                AddOtherEffect(feat);
            }
            // some enhancements provide a bonus to existing totals
            // of either Armor and Shields
            BreakdownItem * pBI = FindBreakdown(Breakdown_BonusArmorAC);
            if (pBI != NULL)
            {
                pBI->AttachObserver(this);      // need to know about changes
                double percentBonus = pBI->Total();
                double value = GetEffectValue("Armor");
                int amount = (int)((value * percentBonus) / 100.0);
                std::stringstream ss;
                ss << "Armor " << percentBonus << "% Bonus";
                // now add a percentage of that
                Effect feat(
                    Effect_Unknown,
                        ss.str(),
                        ss.str(),
                        amount);
                AddOtherEffect(feat);
            }
            pBI = FindBreakdown(Breakdown_BonusShieldAC);
            if (pBI != NULL)
            {
                pBI->AttachObserver(this);      // need to know about changes
                double percentBonus = pBI->Total();
                double value = GetEffectValue("Shield");
                int amount = (int)((value * percentBonus) / 100.0);
                std::stringstream ss;
                ss << "Shield " << percentBonus << "% Bonus";
                // now add a percentage of that
                Effect feat(
                        Effect_Unknown,
                        ss.str(),
                        ss.str(),
                        amount);
                AddOtherEffect(feat);
            }
            pBI = FindBreakdown(Breakdown_NaturalArmor);
            if (pBI != NULL)
            {
                pBI->AttachObserver(this);      // need to know about changes
                double naturalArmor = pBI->Total();
                Effect na(
                        Effect_Unknown,
                        "Natural Armor",
                        "Natural Armor",
                        naturalArmor);
                AddOtherEffect(na);
            }
        }
    }
}

bool BreakdownItemAC::AffectsUs(const Effect &) const
{
    return true;    // its us if we got this far
}

void BreakdownItemAC::UpdateTotalChanged(BreakdownItem * item, BreakdownType type)
{
    // and also call the base class
    BreakdownItem::UpdateTotalChanged(item, type);
    // our Dex bonus to AC may have changed. Update
    CreateOtherEffects();
}
