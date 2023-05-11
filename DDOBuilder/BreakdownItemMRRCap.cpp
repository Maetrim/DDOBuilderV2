// BreakdownItemMRRCap.cpp
//
#include "stdafx.h"
#include "BreakdownItemMRRCap.h"
#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemMRRCap::BreakdownItemMRRCap(
        CBreakdownsPane* pPane,
        BreakdownType type,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_title(title)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_MRRCap, this);
}

BreakdownItemMRRCap::~BreakdownItemMRRCap()
{
}

// required overrides
CString BreakdownItemMRRCap::Title() const
{
    return m_title;
}

CString BreakdownItemMRRCap::Value() const
{
    // value shown if a cap applies else shows "None"
    CString value;
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            if (pBuild->IsStanceActive("Cloth Armor")
                || pBuild->IsStanceActive("Light Armor"))
            {
                value.Format(
                    "%3d",
                    (int)Total());
            }
            else
            {
                value = "None";
            }
        }
    }
    return value;
}

void BreakdownItemMRRCap::CreateOtherEffects()
{
    m_otherEffects.clear();
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            if (pBuild->IsStanceActive("Cloth Armor"))
            {
                // cap of 50 for no or cloth armor
                Effect cloth(
                    Effect_Unknown,
                    "No/Cloth Armor Cap",
                    "Armor",
                    50);
                AddOtherEffect(cloth);
            }
            if (pBuild->IsStanceActive("Light Armor"))
            {
                // cap of 100 for light armor
                Effect lightArmor(
                    Effect_Unknown,
                    "Light Armor Cap",
                    "Armor",
                    100);
                AddOtherEffect(lightArmor);
            }
            // no cap for medium armor
            // no cap for heavy armor
        }
    }
}

bool BreakdownItemMRRCap::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemMRRCap::StanceActivated(
        Build*,
        const std::string&)
{
    CreateOtherEffects();
    Populate();
}

void BreakdownItemMRRCap::StanceDeactivated(
    Build*,
    const std::string&)
{
    CreateOtherEffects();
    Populate();
}

void BreakdownItemMRRCap::GearChanged(Build* pBuild, InventorySlotType slot)
{
    BreakdownItem::GearChanged(pBuild, slot);
    if (slot == Inventory_Armor)
    {
        CreateOtherEffects();
        Populate();
    }
}
