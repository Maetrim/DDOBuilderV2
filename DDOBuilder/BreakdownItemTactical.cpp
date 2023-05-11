// BreakdownItemTactical.cpp
//
#include "stdafx.h"
#include "BreakdownItemTactical.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemTactical::BreakdownItemTactical(
        CBreakdownsPane* pPane,
        BreakdownType type,
        TacticalType tactical,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_tacticalType(tactical)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_TacticalDC, this);

    BreakdownItem * pBIStr = FindBreakdown(StatToBreakdown(Ability_Strength));
    pBIStr->AttachObserver(this);  // need to know about changes
    BreakdownItem * pBICha = FindBreakdown(StatToBreakdown(Ability_Charisma));
    pBICha->AttachObserver(this);  // need to know about changes
}

BreakdownItemTactical::~BreakdownItemTactical()
{
}

// required overrides
CString BreakdownItemTactical::Title() const
{
    CString text;
    text = EnumEntryText(m_tacticalType, tacticalTypeMap);
    return text;
}

CString BreakdownItemTactical::Value() const
{
    CString value;

    value.Format(
            "%4d",
            (int)Total());
    return value;
}

void BreakdownItemTactical::CreateOtherEffects()
{
    // add standard tactical ability based on a stat
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
    }
}

bool BreakdownItemTactical::AffectsUs(const Effect & effect) const
{
    // return true if the effect applies to this tactical
    bool isUs = false;
    if (effect.HasTacticalType(m_tacticalType))
    {
        isUs = true;
    }
    return isUs;
}

// BreakdownObserver overrides
void BreakdownItemTactical::UpdateTotalChanged(BreakdownItem * item, BreakdownType type)
{
    BreakdownItem::UpdateTotalChanged(item, type);  // call base class
    // a stat ability this breakdown uses as a component has changed
    CreateOtherEffects();
    Populate();
}

//void BreakdownItemTactical::UpdateEnhancementTrained(
//        Character * charData,
//        const std::string& enhancementName,
//        const std::string& selection,
//        bool isTier5)
//{
//    BreakdownItem::UpdateEnhancementTrained(
//            charData,
//            enhancementName,
//            selection,
//            isTier5);
//}
//
//void BreakdownItemTactical::UpdateEnhancementRevoked(
//        Character * charData,
//        const std::string& enhancementName,
//        const std::string& selection,
//        bool isTier5)
//{
//    BreakdownItem::UpdateEnhancementRevoked(
//            charData,
//            enhancementName,
//            selection,
//            isTier5);
//}
