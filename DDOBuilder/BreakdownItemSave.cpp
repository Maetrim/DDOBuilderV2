// BreakdownItemSave.cpp
//
#include "stdafx.h"
#include "BreakdownItemSave.h"

#include "BreakdownsPane.h"
#include "Class.h"
#include "GlobalSupportFunctions.h"

BreakdownItemSave::BreakdownItemSave(
        CBreakdownsPane* pPane,
        BreakdownType type,
        SaveType st, 
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        AbilityType ability,
        BreakdownItemSave * pBaseItem,
        bool bCreateNoFail) :
    BreakdownItem(type, treeList, hItem),
    m_saveType(st),
    m_ability(ability),
    m_pBaseBreakdown(pBaseItem),
    m_pNoFailOnOne(NULL)
{
    if (m_pBaseBreakdown != NULL)
    {
            // need to know changes in our base save type
        m_pBaseBreakdown->AttachObserver(this);
    }
    if (bCreateNoFail)
    {
        // need to know when base total value changes
        if (ability != Ability_Unknown)
        {
            AddAbility(ability);
        }
        m_pNoFailOnOne = new BreakdownItemSave(pPane, type, st, NULL, hItem, ability, this, false);
        m_pNoFailOnOne->AttachObserver(this);
        pPane->RegisterBuildCallbackEffect(Effect_SaveBonus, this);
        pPane->RegisterBuildCallbackEffect(Effect_DivineGrace, this);
        pPane->RegisterBuildCallbackEffect(Effect_SaveBonusAbility, this);
    }
    else
    {
        // we are the no fail tracking variant
        pPane->RegisterBuildCallbackEffect(Effect_SaveNoFailOn1, this);
    }
    m_baseAbility = m_mainAbility;
}

BreakdownItemSave::~BreakdownItemSave()
{
    delete m_pNoFailOnOne;      // ok if NULL
    m_pNoFailOnOne = NULL;
}

void BreakdownItemSave::BuildChanged(Character* charData)
{
    // pass through to the base class
    BreakdownItem::BuildChanged(charData);
    // and set on our owned element
    if (m_pNoFailOnOne != NULL)
    {
        m_pNoFailOnOne->BuildChanged(charData);
    }
}

// required overrides
CString BreakdownItemSave::Title() const
{
    CString text;
    text = EnumEntryText(m_saveType, saveTypeMap);
    if (m_saveType != Save_Fortitude
            && m_saveType != Save_Reflex
            && m_saveType != Save_Will)
    {
        text = "vs " + text;
    }
    return text;
}

CString BreakdownItemSave::Value() const
{
    CString value;

    value.Format(
            "%4d",
            (int)Total());
    if (HasNoFailOn1())
    {
        value += " (No fail on 1)";
    }
    return value;
}

void BreakdownItemSave::CreateOtherEffects()
{
    // add class/ability saves for the generic saves types of Fortitude, Reflex and Will
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            m_otherEffects.clear();
            if (m_pNoFailOnOne != NULL)
            {
                if (m_saveType == Save_Fortitude
                        || m_saveType == Save_Reflex
                        || m_saveType == Save_Will)
                {
                    AddClassSaves();
                    if (!AddDivineGrace())
                    {
                        AddHalfElfDivineGrace();
                    }
                    // save penalty due to negative levels
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
                if (m_ability != Ability_Unknown)
                {
                    // Base ability bonus to save
                    AbilityType ability = LargestStatBonus();
                    BreakdownItem * pBI = FindBreakdown(StatToBreakdown(ability));
                    ASSERT(pBI != NULL);
                    int bonus = BaseStatToBonus(pBI->Total());
                    if (bonus != 0) // only add to list if non zero
                    {
                        // should now have the best option
                        std::string bonusName = "Ability bonus (" + EnumEntryText(ability, abilityTypeMap) + ")";
                        Effect feat(
                                Effect_AbilityBonus,
                                bonusName,
                                "Ability",
                                bonus);
                        AddOtherEffect(feat);
                    }
                }
                if (m_pBaseBreakdown != NULL)
                {
                    // show the base save value used in our total
                    double bonus = m_pBaseBreakdown->Total();
                    Effect statBonus(
                            Effect_SaveBonus,
                            (LPCTSTR)(m_pBaseBreakdown->Title() + " Save"),
                            "Base",
                            bonus);
                    AddOtherEffect(statBonus);
                }
            }
        }
    }
}

bool BreakdownItemSave::AffectsUs(const Effect & effect) const
{
    // return true if the effect applies to this save
    // note that effect that apply to "All" only apply to Fortitude, Reflex and Will
    // as the sub-save types use the total from the main category as a part of their total
    // so we do not want to count the bonus twice
    bool isUs = false;
    if (m_saveType == Save_Fortitude
            || m_saveType == Save_Reflex
            || m_saveType == Save_Will)
    {
        // all or specific save type
        for (auto&& iit : effect.Item())
        {
            SaveType st = TextToEnumEntry(iit, saveTypeMap, false);
            if (st == Save_All
                    || st == m_saveType)
            {
                isUs |= true;
            }
        }
    }
    else
    {
        // for a sub-type, it must be the specific save type only
        for (auto&& iit : effect.Item())
        {
            SaveType st = TextToEnumEntry(iit, saveTypeMap, false);
            if (st == m_saveType)
            {
                isUs |= true;
            }
        }
    }
    if (effect.IsType(Effect_DivineGrace))
    {
        // divine grace status has changed, just update
        isUs = true;
    }
    return isUs;
}

void BreakdownItemSave::BuildLevelChanged(Build* charData)
{
    BreakdownItem::BuildLevelChanged(charData);
    // need to re-create other effects list
    CreateOtherEffects();
    Populate();
}

void BreakdownItemSave::ClassChanged(
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

void BreakdownItemSave::FeatEffectApplied(Build* pBuild, const Effect& effect)
{
    if (AffectsUs(effect))
    {
        if (effect.IsType(Effect_SaveBonusAbility))
        {
            for (auto iit: effect.Item())
            {
                AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
                if (at != Ability_Unknown)
                {
                    AddAbility(at);
                }
            }
        }
        else
        {
            BreakdownItem::FeatEffectApplied(pBuild, effect);
        }
    }
}

void BreakdownItemSave::FeatEffectRevoked(Build* pBuild, const Effect& effect)
{
    if (AffectsUs(effect))
    {
            if (effect.IsType(Effect_SaveBonusAbility))
        {
            for (auto iit : effect.Item())
            {
                AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
                if (at != Ability_Unknown)
                {
                    RemoveFirstAbility(at);
                }
            }
        }
        else
        {
            BreakdownItem::FeatEffectRevoked(pBuild, effect);
        }
    }
}

void BreakdownItemSave::ItemEffectApplied(Build* pBuild, const Effect& effect)
{
    if (AffectsUs(effect))
    {
        if (effect.IsType(Effect_SaveBonusAbility))
        {
            for (auto iit : effect.Item())
            {
                AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
                if (at != Ability_Unknown)
                {
                    AddAbility(at);
                }
            }
        }
        else
        {
            BreakdownItem::ItemEffectApplied(pBuild, effect);
        }
    }
}

void BreakdownItemSave::ItemEffectRevoked(Build* pBuild, const Effect& effect)
{
    if (AffectsUs(effect))
    {
        if (effect.IsType(Effect_SaveBonusAbility))
        {
            for (auto iit : effect.Item())
            {
                AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
                if (at != Ability_Unknown)
                {
                    RemoveFirstAbility(at);
                }
            }
        }
        else
        {
            BreakdownItem::ItemEffectRevoked(pBuild, effect);
        }
    }
}

void BreakdownItemSave::ItemEffectApplied(Build* pBuild, const Effect& effect, WeaponType wt)
{
    if (AffectsUs(effect))
    {
        if (effect.IsType(Effect_SaveBonusAbility))
        {
            for (auto iit : effect.Item())
            {
                AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
                if (at != Ability_Unknown)
                {
                    AddAbility(at);
                }
            }
        }
        else
        {
            BreakdownItem::ItemEffectApplied(pBuild, effect, wt);
        }
    }
}

void BreakdownItemSave::ItemEffectRevoked(Build* pBuild, const Effect& effect, WeaponType wt)
{
    if (AffectsUs(effect))
    {
        if (effect.IsType(Effect_SaveBonusAbility))
        {
            for (auto iit : effect.Item())
            {
                AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
                if (at != Ability_Unknown)
                {
                    RemoveFirstAbility(at);
                }
            }
        }
        else
        {
            BreakdownItem::ItemEffectRevoked(pBuild, effect, wt);
        }
    }
}

void BreakdownItemSave::EnhancementEffectApplied(Build* pBuild, const Effect& effect)
{
    if (AffectsUs(effect))
    {
        if (effect.IsType(Effect_SaveBonusAbility))
        {
            for (auto iit : effect.Item())
            {
                AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
                if (at != Ability_Unknown)
                {
                    AddAbility(at);
                }
            }
        }
        else
        {
            BreakdownItem::EnhancementEffectApplied(pBuild, effect);
        }
    }
}

void BreakdownItemSave::EnhancementEffectRevoked(Build* pBuild, const Effect& effect)
{
    if (AffectsUs(effect))
    {
        if (effect.IsType(Effect_SaveBonusAbility))
        {
            for (auto iit : effect.Item())
            {
                AbilityType at = TextToEnumEntry(iit, abilityTypeMap, false);
                if (at != Ability_Unknown)
                {
                    RemoveFirstAbility(at);
                }
            }
        }
        else
        {
            BreakdownItem::EnhancementEffectRevoked(pBuild, effect);
        }
    }
}

void BreakdownItemSave::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // Stat bonus has changed
    CreateOtherEffects();
    // do base class stuff also
    BreakdownItem::UpdateTotalChanged(item, type);
    if (m_pNoFailOnOne != NULL)
    {
        // a no fail on a 1 may have changed, ensure updates
        Populate();
    }
}

bool BreakdownItemSave::HasNoFailOn1() const
{
    if (m_pBaseBreakdown != NULL)
    {
        // saves which are based on a main save type (Will, Reflex and Fortitude)
        // will have a no fail on a 1 in their base save also has a no fail on a 1
        BreakdownItemSave * pBB = dynamic_cast<BreakdownItemSave*>(m_pBaseBreakdown);
        if (pBB->HasNoFailOn1())
        {
            return true;
        }
    }
    return (m_pNoFailOnOne->Total() > 0);
}

void BreakdownItemSave::AppendItems(CListCtrl * pControl)
{
    if (m_pNoFailOnOne != NULL
            && m_pNoFailOnOne->Total() != 0)
    {
        size_t start = pControl->GetItemCount();
        pControl->InsertItem(start, "");
        pControl->InsertItem(start + 1, "No Fail on a 1");
        m_pNoFailOnOne->AddItems(pControl);
    }
}

void BreakdownItemSave::AddClassSaves()
{
    Build* pBuild = m_pCharacter->ActiveBuild();
    size_t ci = 0;
    std::vector<size_t> classLevels = pBuild->ClassLevels(pBuild->Level() - 1);
    for (auto&& cii : classLevels)
    {
        if (cii > 0)
        {
            const ::Class& c = ClassFromIndex(ci);
            std::stringstream ss;
            ss << cii << " " << c.Name() << " Level"; // "e.g. "7 Fighter Level"
            if (cii > 1) ss << "s";                     // plural if required
            Effect classBonus(
                    Effect_SaveBonus,
                    ss.str(),
                    "Class",
                    c.ClassSave(m_saveType, cii));
            AddOtherEffect(classBonus);
        }
        ci++;
    }
}

bool BreakdownItemSave::AddDivineGrace()
{
    bool bAdded = false;
    Build* pBuild = m_pCharacter->ActiveBuild();
    size_t count = TrainedCount(pBuild->CurrentFeats(pBuild->Level()), "Divine Grace");
    if (count > 0)
    {
        // character has divine grace feat trained. Add charisma bonus for this save
        // Divine Grace is capped at 2+(3 x paladin level).
        // For multi-classing, this means 2 levels of Paladin would at best
        // grant a max of +8 to saves (2 + {3x2}). 
        int maxBonusPaladin = pBuild->ClassLevels("Paladin", pBuild->Level() - 1) * 3 + 2;
        int maxBonusSacredFist = pBuild->ClassLevels("Sacred Fist", pBuild->Level()-1) * 3 + 2;
        BreakdownItem* pBI = FindBreakdown(StatToBreakdown(Ability_Charisma));
        ASSERT(pBI != NULL);
        pBI->AttachObserver(this); // watch for any changes
        int bonus = BaseStatToBonus(pBI->Total());
        bonus = min(bonus, max(maxBonusPaladin, maxBonusSacredFist));
        if (bonus > 0) // only add to list if not negative
        {
            // should now have the best option
            CString text;
            text.Format(
                    "Divine Grace (Charisma) (Capped @ %d)",
                    max(maxBonusPaladin, maxBonusSacredFist));
            Effect feat(
                    Effect_SaveBonus,
                    (LPCTSTR)text,
                    "Divine",
                    bonus);
            AddOtherEffect(feat);
        }
        bAdded = true;
    }
    return bAdded;
}

void BreakdownItemSave::AddHalfElfDivineGrace()
{
    // now check for the half elf: Paladin Dilettante feat
    Build* pBuild = m_pCharacter->ActiveBuild();
    size_t count = TrainedCount(pBuild->CurrentFeats(pBuild->Level()), "Half-Elf Dilettante: Paladin");
    if (count > 0)
    {
        // yes, they have it. Work out what the max capped charisma bonus to saves is
        // its 2 + any enhancement upgrades
        int maxBonus = 2; // default max bonus is 2
        if (pBuild->IsEnhancementTrained(
                "HalfElfImprovedDilettanteI",
                "Improved Dilettante: Paladin",
                TT_enhancement))
        {
            maxBonus++;
        }
        if (pBuild->IsEnhancementTrained(
                "HalfElfImprovedDilettanteII",
                "Improved Dilettante: Paladin",
                TT_enhancement))
        {
            maxBonus++;
        }
        if (pBuild->IsEnhancementTrained(
                "HalfElfImprovedDilettanteIII",
                "Improved Dilettante: Paladin",
                TT_enhancement))
        {
            maxBonus++;
        }
        BreakdownItem* pBI = FindBreakdown(StatToBreakdown(Ability_Charisma));
        ASSERT(pBI != NULL);
        pBI->AttachObserver(this); // watch for any changes
        int bonus = BaseStatToBonus(pBI->Total());
        // cap it if required
        bonus = min(bonus, maxBonus);
        if (bonus != 0) // only add to list if non zero
        {
            CString text;
            text.Format(
                    "Lesser Divine Grace (Charisma) (Capped @ %d)",
                    maxBonus);
            // should now have the best option
            Effect feat(
                    Effect_SaveBonus,
                    (LPCTSTR)text,
                    "Divine",
                    bonus);
            AddOtherEffect(feat);
        }
    }
}
