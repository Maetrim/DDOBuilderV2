// BreakdownItemSpellPower.cpp
//
#include "stdafx.h"
#include "BreakdownItemSpellPower.h"
#include "BreakdownItemSkill.h"
#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
//#include "BreakdownItemWeaponEffects.h"

BreakdownItemSpellPower::BreakdownItemSpellPower(
        CBreakdownsPane * pPane,
        BreakdownType type,
        EffectType effect,
        SpellPowerType spType,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect),
    m_spellPowerType(spType),
    m_replacementSpellPower(spType)
{
    pPane->RegisterBuildCallbackEffect(effect, this);
    if (effect == Effect_SpellPower)
    {
        // also need to track whether our spell power is replaced by another which
        // is used if its total is higher
        pPane->RegisterBuildCallbackEffect(Effect_SpellPowerReplacement, this);
    }
}

BreakdownItemSpellPower::~BreakdownItemSpellPower()
{
}

// required overrides
CString BreakdownItemSpellPower::Title() const
{
    return m_title;
}

CString BreakdownItemSpellPower::Value() const
{
    CString text;
    double value = Total();
    double replacementValue = ReplacementTotal();
    if (value != replacementValue)
    {
        CString altSpellPower = EnumEntryText(m_replacementSpellPower, spellPowerTypeMap);
        text.Format(
            "%.2f (%s) from %.2f",  // spell power can have fractional values to 2 dp
            replacementValue,
            (LPCSTR)altSpellPower,
            value);
    }
    else
    {
        text.Format(
            "%.2f",             // spell power can have fractional values to 2 dp
            value);
    }

    return text;
}

double BreakdownItemSpellPower::ReplacementTotal() const
{
    // by default we always return the true value from our breakdown
    m_replacementSpellPower = m_spellPowerType;  // assume our own one
    double value = Total();
    // now iterate the list of effects and look for "replacement" spell powers we may have
    // and use that value if the total is greater than our own
    value = IterateList(m_otherEffects, value, &m_replacementSpellPower);
    value = IterateList(m_effects, value, &m_replacementSpellPower);
    value = IterateList(m_itemEffects, value, &m_replacementSpellPower);
    return value;
}

BreakdownType BreakdownItemSpellPower::SpellPowerBreakdown() const
{
    // from the wiki:
    // Skill bonuses to spell power
    // Heal increases your Positive and Negative spell power 
    // Perform increases your Sonic spell power 
    // Repair increases your Repair and Rust spell power 
    // Spellcraft affects everything else 

    // assume spellcraft
    BreakdownType bt = Breakdown_SkillSpellCraft;
    switch (m_spellPowerType)
    {
    case SpellPower_Positive:
    case SpellPower_Negative:
        bt = Breakdown_SkillHeal;
        break;
    case SpellPower_Sonic:
        bt = Breakdown_SkillPerform;
        break;
    case SpellPower_Repair:
    case SpellPower_Rust:
        bt = Breakdown_SkillRepair;
        break;
    default:
        // all the rest are spell craft
        break;
    }
    return bt;
}

void BreakdownItemSpellPower::CreateOtherEffects()
{
    m_otherEffects.clear();
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // these bonus's only applies for spell powers, not critical chances etc
            if (m_effect == Effect_SpellPower)
            {
                // specific skill only affects spell power (used for critical and multiplier also)
                BreakdownType bt = SpellPowerBreakdown();
                BreakdownItemSkill * pBIS = dynamic_cast<BreakdownItemSkill*>(FindBreakdown(bt));
                CString text;
                text.Format("%s skill bonus",
                        (LPCTSTR)EnumEntryText(pBIS->Skill(), skillTypeMap));
                pBIS->AttachObserver(this);      // need to observe changes
                Effect levels(
                        Effect_SkillBonus,
                        (LPCTSTR)text,
                        "Skill Bonus",
                        pBIS->Total()); 
                AddOtherEffect(levels);

                // add the universal spell power in
                BreakdownItem* pBI = FindBreakdown(Breakdown_SpellPowerUniversal);
                if (pBI != NULL)
                {
                    pBI->AttachObserver(this);      // need to observe changes
                    Effect usp(
                        Effect_SkillBonus,
                        "Universal Spell Power",
                        "Universal Spell Power",
                        pBI->Total());
                    AddOtherEffect(usp);
                }
            }

            if (m_effect == Effect_SpellLore)
            {
                // add the universal spell power in
                BreakdownItem* pBI = FindBreakdown(Breakdown_SpellCriticalChanceUniversal);
                if (pBI != NULL)
                {
                    pBI->AttachObserver(this);      // need to observe changes
                    Effect usp(
                        Effect_SkillBonus,
                        "Universal Spell Lore",
                        "Universal Spell Lore",
                        pBI->Total());
                    AddOtherEffect(usp);
                }
            }

            if (m_effect == Effect_SpellCriticalDamage)
            {
                // add the universal spell power in
                BreakdownItem* pBI = FindBreakdown(Breakdown_SpellCriticalMultiplierUniversal);
                if (pBI != NULL)
                {
                    pBI->AttachObserver(this);      // need to observe changes
                    Effect usp(
                        Effect_SkillBonus,
                        "Universal Spell Multiplier",
                        "Universal Spell Multiplier",
                        pBI->Total());
                    AddOtherEffect(usp);
                }
            }
        }
    }
}

bool BreakdownItemSpellPower::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    if (effect.Type().front() == Effect_SpellPowerReplacement)
    {
        // we only track this if the first item is our spell power
        SpellPowerType sp = TextToEnumEntry(effect.Item().front(), spellPowerTypeMap);
        if (sp == m_spellPowerType)
        {
            isUs = true;
        }
    }
    else if (effect.HasSpellPower(m_spellPowerType))
    {
        isUs = true;
    }
    return isUs;
}

void BreakdownItemSpellPower::AppendItems(CListCtrl* pControl)
{
    std::list<SpellPowerType> overrideList;
    AppendList(m_otherEffects, &overrideList);
    AppendList(m_effects, &overrideList);
    AppendList(m_itemEffects, &overrideList);
    if (overrideList.size() > 0)
    {
        size_t start = pControl->GetItemCount();
        pControl->InsertItem(start++, "");
        // add entries to show which spell powers apply an override
        // for this spell power
        for (auto&& it : overrideList)
        {
            CString altSpellPower = EnumEntryText(it, spellPowerTypeMap);
            CString text;
            text.Format("Replacement spell power type %s", (LPCTSTR)altSpellPower);
            pControl->InsertItem(start, text);
        }
    }
}

void BreakdownItemSpellPower::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // call base class also
    BreakdownItem::UpdateTotalChanged(item, type);
    CreateOtherEffects();
}

//void BreakdownItemSpellPower::UpdateEnhancementTrained(
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
//    // check for "Battle Engineer: Master Engineer"  being trained specifically
//    if (enhancementName == "BECore6")
//    {
//        // need to re-create other effects list
//        CreateOtherEffects();
//        Populate();
//    }
//}
//
//void BreakdownItemSpellPower::UpdateEnhancementRevoked(
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
//    // check for "Battle Engineer: Master Engineer"  being revoked specifically
//    if (enhancementName == "BECore6"
//            || enhancementName == "BomCore2")
//    {
//        // need to re-create other effects list
//        CreateOtherEffects();
//        Populate();
//    }
//}
//
//void BreakdownItemSpellPower::UpdateGearChanged(Character * charData, InventorySlotType slot)
//{
//    BreakdownItem::UpdateGearChanged(
//            charData,
//            slot);
//    if (slot == Inventory_Weapon1)
//    {
//        // if "Battle Engineer: Master Engineer" is trained, the implement bonus may have changed
//        if (m_pCharacter->IsEnhancementTrained("BECore6", "", TT_enhancement)
//                || m_pCharacter->IsEnhancementTrained("BomCore2", "", TT_enhancement))
//        {
//            // need to re-create other effects list
//            CreateOtherEffects();
//            Populate();
//        }
//    }
//}
//

double BreakdownItemSpellPower::IterateList(
        const std::list<Effect>& effects,
        double value,
        SpellPowerType* usedSpt) const
{
    auto it = effects.begin();
    while (it != effects.end())
    {
        if ((*it).IsType(Effect_SpellPowerReplacement))
        {
            // effects requirements must be met
            if ((*it).IsActive(*m_pCharacter, Weapon_Unknown, Weapon_Unknown))
            {
                // look through the list of overrides in this
                auto eiit = (*it).Item().begin();
                ++eiit;               // skip the first entry which is our own spell power
                while (eiit != (*it).Item().end())
                {
                    SpellPowerType spt = TextToEnumEntry((*eiit), spellPowerTypeMap, false);
                    BreakdownType bt = SpellPowerToBreakdown(spt);
                    BreakdownItem* pBreakdown = FindBreakdown(bt);
                    if (pBreakdown != NULL)
                    {
                        double alternateTotal = pBreakdown->Total();
                        if (alternateTotal > value)
                        {
                            value = alternateTotal;
                            *usedSpt = spt;
                        }
                    }
                    ++eiit;
                }
            }
        }
        ++it;
    }
    return value;
}

void BreakdownItemSpellPower::AppendList(
        const std::list<Effect>& effects,
        std::list<SpellPowerType>* pSPL) const
{
    auto it = effects.begin();
    while (it != effects.end())
    {
        if ((*it).IsType(Effect_SpellPowerReplacement))
        {
            // effects requirements must be met
            if ((*it).IsActive(*m_pCharacter, Weapon_Unknown, Weapon_Unknown))
            {
                // look through the list of overrides in this
                auto eiit = (*it).Item().begin();
                ++eiit;               // skip the first entry which is our own spell power
                while (eiit != (*it).Item().end())
                {
                    SpellPowerType spt = TextToEnumEntry((*eiit), spellPowerTypeMap, false);
                    pSPL->push_back(spt);
                    ++eiit;
                }
            }
        }
        ++it;
    }
}
