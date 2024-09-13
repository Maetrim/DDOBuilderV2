// BreakdownItemUniversalSpellPower.cpp
//
#include "stdafx.h"
#include "BreakdownItemUniversalSpellPower.h"
#include "BreakdownItemSkill.h"
#include "GlobalSupportFunctions.h"
#include "Character.h"
#include "BreakdownItemWeaponEffects.h"

BreakdownItemUniversalSpellPower::BreakdownItemUniversalSpellPower(
        CBreakdownsPane* pPane,
        BreakdownType type,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItemSimple(pPane, type, Effect_UniversalSpellPower, title, treeList, hItem)
{
}

BreakdownItemUniversalSpellPower::~BreakdownItemUniversalSpellPower()
{
}

void BreakdownItemUniversalSpellPower::CreateOtherEffects()
{
    m_otherEffects.clear();
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        BreakdownItem* pBIIYH = FindBreakdown(Breakdown_ImplementInYourHands);
        if (pBIIYH != NULL && pBIIYH->Total() > 0)
        {
            pBIIYH->AttachObserver(this);   // we need to know about changes
            // Do not add the bonus if we already have an Implement bonus present
            if (!IsBonusTypePresent(m_itemEffects, "Implement"))
            {
                // we do need to list an implement bonus for our main hands weapon
                // based on its weapon plus
                // get the main hand weapon breakdown
                BreakdownItem* pBI = FindBreakdown(Breakdown_WeaponEffectHolder);
                BreakdownItemWeaponEffects* pBIW = dynamic_cast<BreakdownItemWeaponEffects*>(pBI);
                if (pBIW != NULL)
                {
                    pBI = pBIW->GetWeaponBreakdown(true, Breakdown_WeaponEnchantment);
                    if (pBI != NULL)
                    {
                        int weaponPlus = static_cast<int>(pBI->Total());
                        Effect implementBonusEffect(
                            Effect_Weapon_Enchantment,
                            "Implement in your hands",
                            "Implement",
                            weaponPlus);    // not 3*, see https://ddowiki.com/page/Implement_bonus
                        AddOtherEffect(implementBonusEffect);
                    }
                }
            }
        }
        if (pBuild->IsEnhancementTrained("U51DraconicIncarnationConduit", "", TT_epicDestiny)
                && pBuild->IsStanceActive("Quarterstaff"))
        {
            BreakdownItem* pBI = FindBreakdown(Breakdown_WeaponEffectHolder);
            BreakdownItemWeaponEffects* pBIW = dynamic_cast<BreakdownItemWeaponEffects*>(pBI);
            if (pBIW != NULL)
            {
                pBI = pBIW->GetWeaponBreakdown(true, Breakdown_WeaponEnchantment);
                if (pBI != NULL)
                {
                    pBI->AttachObserver(this);
                    int weaponPlus = static_cast<int>(pBI->Total());
                    if (weaponPlus > 0)
                    {
                        Effect implementBonusDoubling(
                            Effect_Weapon_Enchantment,
                            "Conduit + Quarterstaff",
                            "Destiny",
                            weaponPlus * 3);
                        AddOtherEffect(implementBonusDoubling);
                    }
                }
            }
        }
    }
}

void BreakdownItemUniversalSpellPower::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // call base class also
    BreakdownItemSimple::UpdateTotalChanged(item, type);
    CreateOtherEffects();
}

void BreakdownItemUniversalSpellPower::EnhancementTrained(
    Build* pBuild,
    const EnhancementItemParams& item)
{
    CreateOtherEffects();
    BreakdownItemSimple::EnhancementTrained(pBuild, item);
}

void BreakdownItemUniversalSpellPower::EnhancementRevoked(
    Build* pBuild,
    const EnhancementItemParams& item)
{
    CreateOtherEffects();
    BreakdownItemSimple::EnhancementRevoked(pBuild, item);
}

void BreakdownItemUniversalSpellPower::StanceActivated(
    Build* pBuild,
    const std::string& stanceName)
{
    CreateOtherEffects();
    BreakdownItemSimple::StanceActivated(pBuild, stanceName);
}

void BreakdownItemUniversalSpellPower::StanceDeactivated(
    Build* pBuild,
    const std::string& stanceName)
{
    CreateOtherEffects();
    BreakdownItemSimple::StanceDeactivated(pBuild, stanceName);
}

void BreakdownItemUniversalSpellPower::GearChanged(
    Build* pBuild,
    InventorySlotType slot)
{
    CreateOtherEffects();
    BreakdownItemSimple::GearChanged(pBuild, slot);
}
