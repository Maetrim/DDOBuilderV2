// BreakdownItemSkill.cpp
//
#include "stdafx.h"
#include "BreakdownItemSkill.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"

BreakdownItemSkill::BreakdownItemSkill(
        CBreakdownsPane* pPane,
        SkillType skill,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        BreakdownItem * pAbility) :
    BreakdownItem(type, treeList, hItem),
    m_skill(skill),
    m_pAbility(pAbility)
{
    m_pAbility->AttachObserver(this);   // we update if this changes
    pPane->RegisterBuildCallbackEffect(Effect_SkillBonus, this);
    pPane->RegisterBuildCallbackEffect(Effect_SkillBonusAbility, this);
}

BreakdownItemSkill::~BreakdownItemSkill()
{
}

// required overrides
CString BreakdownItemSkill::Title() const
{
    // skills use their name as the title
    CString text;
    text = EnumEntryText(m_skill, skillTypeMap);
    AbilityType at = StatFromSkill(m_skill);
    text += "    (";
    text += EnumEntryText(at, abilityShortTypeMap);
    text += ")";
    return text;
}

CString BreakdownItemSkill::Value() const
{
    // just return the total formatted as a double or "N/A" if not active for this class combination
    CString value;
    value = "N/A";          // assume not active
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // some skills must have trained ranks else always shown as N/A
            double maxSkill = pBuild->MaxSkillForLevel(
                    m_skill,
                    min(MAX_CLASS_LEVEL, pBuild->Level())-1);   // only check heroic level spend
            if (maxSkill > 0)
            {
                value.Format(
                        "%5.1f",            // possible to have half ranks
                        Total());
                if (m_skill == Skill_Jump)
                {
                    // include the game cap in description
                    value += " (Caps at 40)";
                }
            }
        }
    }
    return value;
}

void BreakdownItemSkill::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // all skills have the amount trained first
            double amount = pBuild->SkillAtLevel(
                    m_skill,
                    pBuild->Level()-1, // 0 based
                    false);     // skill tome not included
            if (amount > 0)
            {
                Effect amountTrained(
                        Effect_SkillBonus,
                        "Trained ranks",
                        "Inherent",
                        amount);
                AddOtherEffect(amountTrained);
            }

            // all skills have an ability bonus, we need to use the total after all modifiers applied
            // for the base ability
            AbilityType at = StatFromSkill(m_skill);
            double ability = m_pAbility->Total();
            amount = BaseStatToBonus(ability);
            if (amount != 0)
            {
                // example label is "Str Modifier"
                std::string abilityName = EnumEntryText(at, abilityTypeMap);
                abilityName.resize(3);          // cut down to 1st 3 characters, e.g. Strength becomes Str
                abilityName += " Modifier";
                Effect abilityMod(
                        Effect_SkillBonus,
                        abilityName,
                        "Ability",
                        amount);
                AddOtherEffect(abilityMod);
            }
            // skills can have a tome for them
            amount = pBuild->SkillTomeValue(m_skill, pBuild->Level());
            if (amount > 0)
            {
                Effect tome(
                        Effect_SkillBonus,
                        "Skill tome",
                        "Inherent",
                        amount);
                AddOtherEffect(tome);
            }
            // armor check penalty
            // armor check penalties may or may not exist for this skill
            int multiplier = ArmorCheckPenalty_Multiplier(m_skill);
            if (multiplier != 0)
            {
                BreakdownItem * pBI = FindBreakdown(Breakdown_ArmorCheckPenalty);
                ASSERT(pBI != NULL);
                pBI->AttachObserver(this);  // need to know about changes to this effect
                double total = min(0, pBI->Total()); // ACP cannot be a bonus!
                Effect acp(
                        Effect_SkillBonus,
                        "Armor check penalty",
                        "Penalty",
                        total * multiplier);        // its a penalty
                AddOtherEffect(acp);
            }
            // skill penalty due to negative levels
            BreakdownItem* pNL = FindBreakdown(Breakdown_NegativeLevels);
            if (pNL != NULL)
            {
                pNL->AttachObserver(this); // need to know about changes
                int negLevels = static_cast<int>(pNL->Total());
                if (negLevels != 0)
                {
                    Effect negLevelsEffect(
                        Effect_Unknown,
                        "Negative Levels",
                        "Negative Levels",
                        static_cast<double>(-1 * negLevels));          // -1 per neg level
                    AddOtherEffect(negLevelsEffect);
                }
            }
        }
    }
}

void BreakdownItemSkill::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // do base class stuff also
    BreakdownItem::UpdateTotalChanged(item, type);
    // ability stat for this skill has changed, update our skill total
    CreateOtherEffects();
    Populate();
}

void BreakdownItemSkill::SkillSpendChanged(
        Build*,
        size_t,
        SkillType skill)
{
    if (skill == m_skill)
    {
        // our skill has changed, update
        CreateOtherEffects();
        Populate();
    }
}

void BreakdownItemSkill::SkillTomeChanged(
        Life*,
        SkillType skill)
{
    if (skill == m_skill)
    {
        // our skill has changed, update
        CreateOtherEffects();
        Populate();
    }
}

bool BreakdownItemSkill::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    // see if this effect applies to us
    AbilityType rat = StatFromSkill(m_skill);
    for (auto&& iit : effect.Item())
    {
        SkillType st = TextToEnumEntry(iit, skillTypeMap, false);
        if (st == Skill_All
            || st == m_skill)
        {
            isUs |= true;
        }
        AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
        isUs |= (rat == at);
    }
    return isUs;
}

SkillType BreakdownItemSkill::Skill() const
{
    return m_skill;
}

