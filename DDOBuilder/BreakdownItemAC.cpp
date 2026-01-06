// BreakdownItemAC.cpp
//
#include "stdafx.h"
#include "BreakdownItemAC.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"
#include "BreakdownItemWeaponEffects.h"

BreakdownItemAC::BreakdownItemAC(
        CBreakdownsPane* pPane,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_bCreatingOtherEffects(false)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_ACBonus, this);
    pPane->RegisterBuildCallbackEffect(Effect_EnchantArmor, this);
    // note that shield enchantment is handled by the weapons breakdowns
    // and total values are then read from them later
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
                double valueArmor = GetEffectValue("Armor", false);
                double valueEnhancement = GetEffectValue("Armor Enhancement", false);
                int amount = static_cast<int>(((valueArmor + valueEnhancement) * percentBonus) / 100.0);
                std::stringstream ss;
                ss << "Armor " << percentBonus << "% Bonus of " << (valueArmor + valueEnhancement) << " (Armor(" << valueArmor << ") + Enhancement(" << valueEnhancement << "))";
                    // now add a percentage of that
                Effect feat(
                        Effect_Unknown,
                        ss.str(),
                        "Stacking",
                        amount);
                AddOtherEffect(feat);
            }

            // add any shield enhancement (must have a shield equipped)
            if (pBuild->IsStanceActive("Shield"))
            {
                pBI = FindBreakdown(Breakdown_BonusShieldAC);
                if (pBI != NULL)
                {
                    pBI->AttachObserver(this);      // need to know about changes
                    double percentBonus = pBI->Total();
                    if (percentBonus > 0)
                    {
                        double valueShield = GetEffectValue("Shield", false);
                        int amount = static_cast<int>((valueShield * percentBonus) / 100.0);
                        std::stringstream ss;
                        ss << "Shield " << percentBonus << "% Bonus of " << valueShield;
                        // now add a percentage of that
                        Effect feat(
                                Effect_Unknown,
                                ss.str(),
                                "Stacking",
                                amount);
                        AddOtherEffect(feat);
                    }
                }
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

void BreakdownItemAC::LinkUp()
{
    BreakdownItem* pBI = FindBreakdown(Breakdown_WeaponEffectHolder);
    BreakdownItemWeaponEffects* pBIW = dynamic_cast<BreakdownItemWeaponEffects*>(pBI);
    if (pBIW != NULL)
    {
        // AC needs to know when Shield enhancement bonus changes
        BreakdownItem* pBWE = pBIW->GetWeaponBreakdown(false, Breakdown_WeaponEnchantment);
        if (pBWE != NULL)
        {
            pBWE->AttachObserver(this);  // need to know about changes to this effect
        }
        CreateOtherEffects();
    }
}

void BreakdownItemAC::EffectAdded()
{
    if (!m_bCreatingOtherEffects)
    {
        m_bCreatingOtherEffects = true; // avoid recursion
        CreateOtherEffects();
        m_bCreatingOtherEffects = false;
    }
}

void BreakdownItemAC::StanceActivated(Build* pBuild, const std::string& stanceName)
{
    if (stanceName == "Shield")
    {
        CreateOtherEffects();
    }
    BreakdownItem::StanceActivated(pBuild, stanceName);
}

void BreakdownItemAC::StanceDeactivated(Build* pBuild, const std::string& stanceName)
{
    if (stanceName == "Shield")
    {
        CreateOtherEffects();
    }
    BreakdownItem::StanceDeactivated(pBuild, stanceName);
}
