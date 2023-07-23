// BreakdownItemMDB.cpp
//
#include "stdafx.h"
#include "BreakdownItemMDB.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"

BreakdownItemMDB::BreakdownItemMDB(
        CBreakdownsPane* pPane,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_bNoLimit(false)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_MaxDexBonus, this);
}

BreakdownItemMDB::~BreakdownItemMDB()
{
}

// required overrides
CString BreakdownItemMDB::Title() const
{
    CString text = "Max Dex Bonus";
    return text;
}

CString BreakdownItemMDB::Value() const
{
    CString value;
    if (m_bNoLimit)
    {
        value = "No limit";
    }
    else
    {
        value.Format(
                "%3d",
                (int)Total());
    }
    return value;
}

void BreakdownItemMDB::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            m_bNoLimit = pBuild->IsStanceActive("Cloth Armor");
        }
    }
}

bool BreakdownItemMDB::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemMDB::StanceActivated(
    Build* pBuild,
    const std::string& stanceName)
{
    CreateOtherEffects();
    BreakdownItem::StanceActivated(pBuild, stanceName);
}

void BreakdownItemMDB::StanceDeactivated(
    Build* pBuild,
    const std::string& stanceName)
{
    CreateOtherEffects();
    BreakdownItem::StanceDeactivated(pBuild, stanceName);
}
