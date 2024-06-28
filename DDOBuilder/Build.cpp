// Build.cpp
//
#include "StdAfx.h"
#include "Build.h"

#include "Augment.h"
#include "AutomaticAcquisition.h"
#include "Class.h"
#include "EnhancementTree.h"
#include "GlobalSupportFunctions.h"
#include "Feat.h"
#include "Filigree.h"
#include "GuildBuff.h"
#include "Life.h"
#include "Race.h"
#include "TrainedSpell.h"
#include "XmlLib\SaxWriter.h"
#include "LogPane.h"
#include "GrantedFeatsPane.h"
#include "SetBonus.h"
#include "StancesPane.h"
#include "OptionalBuff.h"
#include <algorithm>
#include "MainFrm.h"
#include "BreakdownItem.h"
#include "LegacyEnhancementSelectedTrees.h"
#include "LegacyDestinySelectedTrees.h"

#define DL_ELEMENT Build

namespace
{
    const wchar_t f_saxElementName[] = L"Build";
    DL_DEFINE_NAMES(Build_PROPERTIES)
    const unsigned f_verCurrent = 1;

    struct ClassEntry
    {
        ClassEntry() : levels(0) {};
        std::string type;
        size_t levels;
    };
    bool SortClassEntry(const ClassEntry& a, const ClassEntry& b)
    {
        if (a.levels == b.levels)
        {
            // same level, sort on class name
            return (a.type < b.type);
        }
        else
        {
            // sort on number of class levels
            return (a.levels > b.levels);
        }
    }
}

Build::Build(Life * pParentLife) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_FavorFeats(L"FavorFeats"),
    m_pLife(pParentLife),
    m_racialTreeSpend(0),
    m_universalTreeSpend(0),
    m_classTreeSpend(0),
    m_destinyTreeSpend(0),
    m_previousGuildLevel(0),
    m_bSwitchingBuildsOrGear(false)
{
    DL_INIT(Build_PROPERTIES)
    // make sure we have Level() default LevelTraining objects in the list
    size_t count = m_Levels.size();
    for (size_t index = count; index < Level(); ++index)
    {
        LevelTraining lt;
        if (index >= MAX_CLASS_LEVEL && index < MAX_CLASS_LEVEL + MAX_EPIC_LEVEL)
        {
            // all levels above level MAX_CLASS_LEVELS are epic levels by default
            lt.Set_Class(Class_Epic);
        }
        else if (index >= MAX_CLASS_LEVEL + MAX_EPIC_LEVEL)
        {
            // all levels above level MAX_CLASS_LEVEL + MAX_EPIC_LEVEL are legendary
            // levels by default
            lt.Set_Class(Class_Legendary);
        }
        m_Levels.push_back(lt);
    }
    m_Class1 = Class_Unknown;
    m_Class2 = Class_Unknown;
    m_Class3 = Class_Unknown;
    UpdateFeats();
    UpdateSkillPoints();
    // by default they start with 1 gear layout called "Standard"
    EquippedGear gear("Standard");  // with no items yet selected
    m_GearSetups.push_back(gear);
    m_ActiveGear = "Standard";
    SetupDefaultWeaponGroups();
    UpdateCachedClassLevels();
}

DL_DEFINE_ACCESS(Build_PROPERTIES)

XmlLib::SaxContentElementInterface * Build::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Build_PROPERTIES)

    return subHandler;
}

void Build::EndElement()
{
    m_hasNotes = true;
    SaxContentElement::EndElement();
    DL_END(Build_PROPERTIES)
    // as a build has a default number of LevelTraining objects setup in the constructor
    // we need to make sure at the end of the load procedure that only Level()
    // number of them still exists as the loaded ones were appended to the original
    // list of MAX_CLASS_LEVEL items
    while (m_Levels.size() > Level())
    {
        m_Levels.pop_front();   // oldest are the ones to throw away
    }
    // also remove the default "Standard" gear layout which we get by default
    m_GearSetups.pop_front();
    m_hasActiveGear = true;
    // notes text is not saved as \r\n's replace all text
    m_Notes = ReplaceAll(m_Notes, "\n", "\r\n");
    UpdateCachedClassLevels();
}

void Build::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName(), VersionAttributes());
    DL_WRITE(Build_PROPERTIES)
    writer->EndElement();
}

void Build::SetLifePointer(Life* pLife)
{
    m_pLife = pLife;
}

CString Build::UIDescription(size_t buildIndex) const
{
    // Ui description is based on class levels
    std::string complexDescription = ComplexUIDescription();
    CString strDescription;
    strDescription.Format("Build %d: Level %d - %s", buildIndex + 1, m_Level, complexDescription.c_str());
    return strDescription;
}

std::string Build::ComplexUIDescription() const
{
    // determine how many levels of each class have been trained
    std::vector<size_t> classLevels = ClassLevels(Level()-1);

    // levels in each non zero class listed, sorted by count then name
    // unknown listed also as shows how many need to be trained
    std::vector<ClassEntry> classes;
    size_t ci = 0;
    for (auto&& cit: classLevels)
    {
        if (cit > 0)
        {
            const ::Class & c = ClassFromIndex(ci);
            // we have levels trained in this class, add it
            ClassEntry data;
            data.type = c.Name();
            data.levels = cit;
            classes.push_back(data);
        }
        ci++;
    }
    // now that we have the full list, sort them into increasing order
    // now create the text to display
    std::sort(classes.begin(), classes.end(), SortClassEntry);
    std::stringstream ss;
    ci = 0;
    for (auto&& cit: classes)
    {
        if (ci > 0)
        {
            // 2nd or following items separated by a ","
            ss << ", ";
        }
        ss << cit.levels
            << " "
            << cit.type;
        ci++;
    }
    // e.g. "10 Epic, 8 Fighter, 6 Monk, 6 Ranger"
    return ss.str();
}

void Build::BuildNowActive()
{
    m_bSwitchingBuildsOrGear = true; // stop snapshots being corrupted
    // ensure all gear sets have images setup
    for (auto&& git : m_GearSetups)
    {
        git.UpdateImages();
    }
    SetupDefaultWeaponGroups();
    VerifySpecialFeats();

    // make sure we are observing the ability breakdowns for snapshot values to be updated
    for (size_t i = Breakdown_Strength; i <= Breakdown_Charisma; ++i)
    {
        BreakdownItem* pBI = FindBreakdown(static_cast<BreakdownType>(i));
        if (pBI != NULL)
        {
            pBI->AttachObserver(this);
        }
    }
    // make sure we are correctly tracking how many APs have been spent
    // in each tree type
    m_racialTreeSpend = 0;
    m_universalTreeSpend = 0;
    m_classTreeSpend = 0;
    m_destinyTreeSpend = 0;
    m_exclusiveEnhancements.clear();
    m_setBonusStacks.clear();
    for (auto&& tit : m_EnhancementTreeSpend)
    {
        const EnhancementTree& eTree = EnhancementTree::GetTree(tit.TreeName());
        if (eTree.HasIsRacialTree())
        {
            // it's points spent in the racial tree
            m_racialTreeSpend += tit.Spent();
        }
        else if (eTree.HasIsUniversalTree())
        {
            // it's points spent in a universal tree
            m_universalTreeSpend += tit.Spent();
        }
        else
        {
            // can only be a class tree then
            m_classTreeSpend += tit.Spent();
        }
    }
    for (auto&& tit : m_DestinyTreeSpend)
    {
        //const EnhancementTree& eTree = EnhancementTree::GetTree(tit.TreeName());
        m_destinyTreeSpend += tit.Spent();
    }
    // needs to be before Notify to avoid multiple feat notifications
    UpdateFeats();
    m_pLife->NotifyActiveBuildChanged();
    // first notify all the feat effects for this builds level (trained and automatic)
    std::list<TrainedFeat> feats = CurrentFeats(Level());
    for (auto&& fit: feats)
    {
        // get the list of effects this feat has
        const Feat & feat = FindFeat(fit.FeatName());
        ApplyFeatEffects(feat);
    }
    // notify all enhancement effects from all trees and update trained element costs
    for (auto&& ets : m_EnhancementTreeSpend)
    {
        for (auto&& tit: ets.Enhancements())
        {
            for (size_t rank = 0; rank < tit.Ranks(); ++rank)
            {
                ApplyEnhancementEffects(
                        ets.TreeName(),
                        tit.EnhancementName(),
                        tit.HasSelection() ? tit.Selection() : "",
                        rank+1);
            }
        }
    }
    for (auto&& rts : m_ReaperTreeSpend)
    {
        for (auto&& tit: rts.Enhancements())
        {
            for (size_t rank = 0; rank < tit.Ranks(); ++rank)
            {
                ApplyEnhancementEffects(
                        rts.TreeName(),
                        tit.EnhancementName(),
                        tit.HasSelection() ? tit.Selection() : "",
                        rank+1);
            }
        }
    }
    for (auto&& dts : m_DestinyTreeSpend)
    {
        for (auto&& tit: dts.Enhancements())
        {
            for (size_t rank = 0; rank < tit.Ranks(); ++rank)
            {
                ApplyEnhancementEffects(
                        dts.TreeName(),
                        tit.EnhancementName(),
                        tit.HasSelection() ? tit.Selection() : "",
                        rank+1);
            }
        }
    }
    UpdateGearToLatestVersions();
    ApplySpellEffects();
    ApplyGearEffects();     // apply effects from equipped gear
    ApplySelfAndPartyBuffs();
    m_previousGuildLevel = 0;   // ensure all effects apply
    ApplyGuildBuffs(m_pLife->ApplyGuildBuffs());
    //UpdateGreensteelStances();
    //NotifyAllSelfAndPartyBuffs();
    NotifyGearChanged(Inventory_Weapon1);   // updates both in breakdowns
    m_bSwitchingBuildsOrGear = false;
    UpdateTotalChanged(NULL, Breakdown_Strength);       // ensure snapshots are current
    UpdateTotalChanged(NULL, Breakdown_Intelligence);
    UpdateTotalChanged(NULL, Breakdown_Wisdom);
    UpdateTotalChanged(NULL, Breakdown_Dexterity);
    UpdateTotalChanged(NULL, Breakdown_Constitution);
    UpdateTotalChanged(NULL, Breakdown_Charisma);
}

void Build::SetLevel(size_t level)
{
    if (level < m_Level)
    {
        RevokeLostLevelFeats(level, m_Level);
    }
    m_Level = level;
    // throw away any extra levels we now may have
    while (m_Levels.size() > level)
    {
        m_Levels.pop_back();
    }
    // now add any new levels that are required
    // assume an unknown class if we have multiple classes
    // already selected. Else default to the single class
    std::string defaultClass = Class_Unknown;
    if (Class2() == Class_Unknown
            && Class3() == Class_Unknown)
    {
        defaultClass = Class1();
    }
    while (m_Levels.size() < level)
    {
        LevelTraining lt;
        if (m_Levels.size() < MAX_CLASS_LEVEL)
        {
            lt.Set_Class(defaultClass);
        }
        else if (m_Levels.size() < MAX_CLASS_LEVEL + MAX_EPIC_LEVEL)
        {
            // need to add epic
            lt.Set_Class(Class_Epic);
        }
        else
        {
            // need to add legendary
            lt.Set_Class(Class_Legendary);
        }
        m_Levels.push_back(lt);
    }
    UpdateFeats();
    UpdateSkillPoints();
    UpdateSpells();
    VerifyTrainedFeats();
    AutoTrainSingleSelectionFeats();
    VerifyGear();
    NotifyBuildLevelChanged();
}

void Build::SetName(const std::string& name)
{
    m_pLife->SetName(name);
}

const std::string& Build::Name() const
{
    return m_pLife->Name();
}

const std::string& Build::Race() const
{
    // race belongs to our parent life, ask it
    return m_pLife->Race();
}

void Build::SetRace(const std::string& race)
{
    if (race != m_pLife->Race())
    {
        // add the log entry
        std::stringstream ss;
        ss << "Race changed from \"" << m_pLife->Race() << "\" to \"" << race << "\"";
        GetLog().AddLogEntry(ss.str().c_str());
        m_pLife->SetRace(race);
        VerifyTrainedFeats();
        VerifyGear();
    }
}

AlignmentType Build::Alignment() const
{
    // alignment belongs to our parent life, ask it
    return m_pLife->Alignment();
}

void Build::SetAlignment(AlignmentType alignment)
{
    if (alignment != m_pLife->Alignment())
    {
        // add the log entry
        std::stringstream ss;
        ss << "Alignment changed from \""
                << EnumEntryText(Alignment(), alignmentTypeMap)
                << "\" to \""
                << EnumEntryText(alignment,  alignmentTypeMap) << "\"";
        GetLog().AddLogEntry(ss.str().c_str());
        m_pLife->SetAlignment(alignment);
    }
}

void Build::NotifyBuildLevelChanged()
{
    NotifyAll(&BuildObserver::UpdateBuildLevelChanged, this);
}

void Build::NotifyClassChoiceChanged()
{
    NotifyAll(&BuildObserver::UpdateClassChoiceChanged, this);
}

void Build::NotifyClassChanged(
        const std::string& classFrom,
        const std::string& classTo,
        size_t level)
{
    NotifyAll(&BuildObserver::UpdateClassChanged, this, classFrom, classTo, level);
}

void Build::NotifyFeatTrained(const std::string& featName)
{
    NotifyAll(&BuildObserver::UpdateFeatTrained, this, featName);
}

void Build::NotifyFeatRevoked(const std::string& featName)
{
    NotifyAll(&BuildObserver::UpdateFeatRevoked, this, featName);
}

void Build::NotifyFeatEffectApplied(
    const Effect& effect)
{
    if (effect.IsType(Effect_AddGroupWeapon))
    {
        AddWeaponToGroup(effect);
    }
    if (effect.IsType(Effect_MergeGroups))
    {
        MergeGroups(effect);
    }
    NotifyAll(&BuildObserver::UpdateFeatEffectApplied, this, effect);
}

void Build::NotifyFeatEffectRevoked(
    const Effect& effect)
{
    if (effect.IsType(Effect_AddGroupWeapon))
    {
        RemoveFromWeaponGroup(effect);
    }
    if (effect.IsType(Effect_MergeGroups))
    {
        RemoveMergeGroup(effect);
    }
    NotifyAll(&BuildObserver::UpdateFeatEffectRevoked, this, effect);
}

void Build::NotifyItemEffect(const std::string& itemName, Effect effect, InventorySlotType ist)
{
    if (!effect.HasDisplayName())
    {
        if (ist == Inventory_Ring2)
        {
            // stop duplicate effects from 2 of the same ring stacking
            effect.SetDisplayName(itemName + " ");
        }
        else
        {
            effect.SetDisplayName(itemName);
        }
    }
    if (effect.IsType(Effect_AddGroupWeapon))
    {
        AddWeaponToGroup(effect);
    }
    if (effect.IsType(Effect_MergeGroups))
    {
        MergeGroups(effect);
    }
    NotifyAll(&BuildObserver::UpdateItemEffectApplied, this, effect);
}

void Build::NotifyItemEffectRevoked(const std::string& itemName, Effect effect, InventorySlotType ist)
{
    if (!effect.HasDisplayName())
    {
        if (ist == Inventory_Ring2)
        {
            // stop duplicate effects from 2 of the same ring stacking
            effect.SetDisplayName(itemName + " ");
        }
        else
        {
            effect.SetDisplayName(itemName);
        }
    }
    if (effect.IsType(Effect_AddGroupWeapon))
    {
        RemoveFromWeaponGroup(effect);
    }
    if (effect.IsType(Effect_MergeGroups))
    {
        RemoveMergeGroup(effect);
    }
    NotifyAll(&BuildObserver::UpdateItemEffectRevoked, this, effect);
}

void Build::NotifyItemWeaponEffect(
        const std::string& itemName,
        Effect effect,
        WeaponType wt,
        InventorySlotType ist)
{
    if (!effect.HasDisplayName())
    {
        effect.SetDisplayName(itemName);
    }
    if (effect.IsType(Effect_AddGroupWeapon))
    {
        AddWeaponToGroup(effect);
    }
    if (effect.IsType(Effect_MergeGroups))
    {
        MergeGroups(effect);
    }
    NotifyAll(&BuildObserver::UpdateItemWeaponEffectApplied, this, effect, wt, ist);
}

void Build::NotifyItemWeaponEffectRevoked(
        const std::string& itemName,
        Effect effect,
        WeaponType wt,
        InventorySlotType ist)
{
    if (!effect.HasDisplayName())
    {
        effect.SetDisplayName(itemName);
    }
    if (effect.IsType(Effect_AddGroupWeapon))
    {
        RemoveFromWeaponGroup(effect);
    }
    if (effect.IsType(Effect_MergeGroups))
    {
        RemoveMergeGroup(effect);
    }
    NotifyAll(&BuildObserver::UpdateItemWeaponEffectRevoked, this, effect, wt, ist);
}

void Build::NotifyGearChanged(InventorySlotType slot)
{
    NotifyAll(&BuildObserver::UpdateGearChanged, this, slot);
}

void Build::NotifyNewSetBonusStack(const SetBonus& setBonus)
{
    NotifyAll(&BuildObserver::UpdateNewSetBonusStack, this, setBonus);
}

void Build::NotifyRevokeSetBonusStack(const SetBonus& setBonus)
{
    NotifyAll(&BuildObserver::UpdateRevokeSetBonusStack, this, setBonus);
}

void Build::SetClass1(const std::string& ct)
{
    std::string classFrom = Class1();
    if (Class1() != Class_Unknown)
    {
        if (RevokeClass(Class1()))
        {
            // if a class got revoked keep the class list up to date
            NotifyClassChanged(classFrom, ct, static_cast<size_t>(-1));
        }
    }
    Set_Class1(ct);
    if (ct == Class_Unknown)
    {
        // may need to bump classes down one if set to Unknown
        Set_Class1(Class2());
        Set_Class2(Class3());
        Set_Class3(Class_Unknown);
    }
    // now set all levels that were classFrom to current Class1
    for (size_t level = 0; level < m_Level; ++level)
    {
        if (!LevelData(level).HasClass()
                || LevelData(level).Class() == classFrom
                || LevelData(level).Class() == Class_Unknown)
        {
            std::list<LevelTraining>::iterator it = m_Levels.begin();
            std::advance(it, level);
            (*it).Set_Class(Class1());
            UpdateSkillPoints(level);
            UpdateCachedClassLevels();
            NotifyClassChanged(classFrom, Class1(), level);    // must be done before feat updates to keep spell lists kosher
        }
    }
    UpdateSpells();
    UpdateFeats();
    VerifyTrainedFeats();
    AutoTrainSingleSelectionFeats();
    VerifyGear();
    NotifyClassChoiceChanged();
    SetModifiedFlag(TRUE);
    NotifyStanceDeactivated("Force correct update of effects");
    // add the log entry
    std::stringstream ss;
    ss << "Class 1 changed from \""
        << classFrom
        << "\" to \""
        << ct << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
}

void Build::SetClass2(const std::string& ct)
{
    std::string classFrom = Class2();
    if (Class2() != Class_Unknown)
    {
        if (RevokeClass(Class2()))
        {
            // if a class got revoked keep the class list up to date
            NotifyClassChanged(classFrom, ct, static_cast<size_t>(-1));
        }
    }
    Set_Class2(ct);
    if (ct == Class_Unknown)
    {
        // may need to bump classes down one if set to Unknown
        Set_Class2(Class3());
        Set_Class3(Class_Unknown);
    }
    // now set all levels that were classFrom to current Class1
    for (size_t level = 0; level < m_Level; ++level)
    {
        if (!LevelData(level).HasClass()
                || LevelData(level).Class() == classFrom
                || LevelData(level).Class() == Class_Unknown)
        {
            std::list<LevelTraining>::iterator it = m_Levels.begin();
            std::advance(it, level);
            (*it).Set_Class(Class2());
            UpdateSkillPoints(level);
            UpdateCachedClassLevels();
            NotifyClassChanged(classFrom, Class2(), level);    // must be done before feat updates to keep spell lists kosher
        }
    }
    UpdateSpells();
    UpdateFeats();
    VerifyTrainedFeats();
    AutoTrainSingleSelectionFeats();
    VerifyGear();
    NotifyClassChoiceChanged();
    SetModifiedFlag(TRUE);
    NotifyStanceDeactivated("Force correct update of effects");
    // add the log entry
    std::stringstream ss;
    ss << "Class 2 changed from \""
        << classFrom
        << "\" to \""
        << ct << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
}

void Build::SetClass3(const std::string& ct)
{
    std::string classFrom = Class3();
    if (Class3() != Class_Unknown)
    {
        if (RevokeClass(Class3()))
        {
            // if a class got revoked keep the class list up to date
            NotifyClassChanged(classFrom, ct, static_cast<size_t>(-1));
        }
    }
    Set_Class3(ct);
    // now set all levels that were classFrom to current Class1
    for (size_t level = 0; level < m_Level; ++level)
    {
        if (!LevelData(level).HasClass()
                || LevelData(level).Class() == classFrom
                || LevelData(level).Class() == Class_Unknown)
        {
            std::list<LevelTraining>::iterator it = m_Levels.begin();
            std::advance(it, level);
            (*it).Set_Class(Class3());
            UpdateSkillPoints(level);
            UpdateCachedClassLevels();
            NotifyClassChanged(classFrom, Class3(), level);    // must be done before feat updates to keep spell lists kosher
        }
    }
    UpdateSpells();
    UpdateFeats();
    VerifyTrainedFeats();
    AutoTrainSingleSelectionFeats();
    VerifyGear();
    NotifyClassChoiceChanged();
    SetModifiedFlag(TRUE);
    NotifyStanceDeactivated("Force correct update of effects");
    // add the log entry
    std::stringstream ss;
    ss << "Class 3 changed from \""
        << classFrom
        << "\" to \""
        << ct << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
}

void Build::SetClass(size_t level, const std::string& ct)
{
    std::string classFrom = Class_Unknown;
    if (level < m_Level)    // 0 based
    {
        std::list<LevelTraining>::iterator it = m_Levels.begin();
        std::advance(it, level);
        classFrom = (*it).HasClass() ? (*it).Class() : Class_Unknown;
        (*it).Set_Class(ct);
        // add the log entry
        std::stringstream ss;
        if (classFrom != Class_Unknown)
        {
            ss << "Changed the class at level " << level + 1
                    << " from \"" << classFrom.c_str() << "\" to \""
                    << ct.c_str() << "\"";
        }
        else
        {
            ss << "Set the class at level " << level + 1
                    << " to \""
                    << ct.c_str() << "\"";
        }
        GetLog().AddLogEntry(ss.str().c_str()); // and finally log it
        UpdateCachedClassLevels();
        UpdateSkillPoints(level);
        SetModifiedFlag(TRUE);
    }
    UpdateSpells();
    UpdateFeats();
    VerifyTrainedFeats();
    AutoTrainSingleSelectionFeats();
    VerifyGear();
    NotifyClassChanged(classFrom, ct, level); //?? check spells not messed up
}

bool Build::RevokeClass(const std::string& ct)
{
    bool hadRevoke = false;
    size_t level = 0;
    std::list<LevelTraining>::iterator it = m_Levels.begin();
    while (it != m_Levels.end())
    {
        if ((*it).HasClass()
                && (*it).Class() == ct)
        {
            // class selection no longer available
            (*it).Set_Class(Class_Unknown);
            UpdateSkillPoints(level);
            hadRevoke = true;
        }
        ++it;
        ++level;
    }
    if (hadRevoke)
    {
        UpdateFeats();
        VerifyTrainedFeats();
        VerifyGear();
    }
    return hadRevoke;
}

const std::string& Build::Class(size_t index) const
{
    switch (index)
    {
    case 0: return Class1();
    case 1: return Class2();
    case 2: return Class3();
    }
    return Class_Unknown;
}

std::vector<FeatSlot> Build::TrainableFeatTypeAtLevel(size_t level) const
{
    // determine what feat types can be trained at this level up
    // it should never be more than 3 possible feat types
    std::vector<FeatSlot> tft;

    // add any racial feat slots
    const ::Race & race = FindRace(Build::Race());
    race.AddFeatSlots(level, &tft);

    // see if we are at a level we can train a general standard feat choice
    if (level == 0
            || ((level + 1) % 3) == 0)
    {
        // we can train a standard feat selection at this level
        if (level < MAX_CLASS_LEVEL)
        {
            FeatSlot standardFeat(level, "Standard", false, false);
            tft.push_back(standardFeat);
        }
    }

    // all other bonus feat types depend on class level
    // determine which class this level is
    std::list<LevelTraining>::const_iterator clit = m_Levels.begin();
    std::advance(clit, level);
    std::string ctName = (*clit).HasClass() ? (*clit).Class() : Class_Unknown;
    // now count how many previous levels were also this class, so we can evaluate
    // the feats at the right class level
    size_t classLevel = ClassLevels(ctName, level);
    // now look at what class is being trained and see if any bonus feats
    // apply at this class level
    FindClass(ctName).AddFeatSlots(*this, classLevel, &tft, level);
    return tft;
}

const LevelTraining & Build::LevelData(size_t level) const
{
    ASSERT(level < m_Levels.size());
    std::list<LevelTraining>::const_iterator it = m_Levels.begin();
    std::advance(it, level);  // point to level of interest
    const LevelTraining & data = (*it);
    return data;
}

size_t Build::BaseAttackBonus(size_t level) const
{
    size_t bab = 0;
    const std::list<::Class> & classes = Classes();
    std::list<::Class>::const_iterator cci = classes.begin();
    while (cci != classes.end())
    {
        size_t classLevels = ClassLevels((*cci).Name(), level);
        double classBab = 0.0;
        if (classLevels > 0)
        {
            classBab = (*cci).BAB()[classLevels];
        }
        bab += (size_t)(classBab); // fractions dropped
        ++cci;
    }
    return bab;
}

size_t Build::AbilityAtLevel(
        AbilityType ability,
        size_t level,
        bool includeTomes) const  // level is 0 based
{
    int abilityValue = 8
            + FindRace(Race()).RacialModifier(ability)
            + m_BuildPoints.GetAbilitySpend(ability);

    abilityValue += LevelUpsAtLevel(ability, level);

    // Update 37 Patch 1 changed the way tomes apply
    // they now apply at:
    // +1-+2 Level 1
    // +3 Level 3
    // +4 Level 7
    // +5 Level 11
    // +6 Level 15
    // +7 Level 19
    // +8 Level 22
    if (includeTomes || level > 0) // level is 0 based
    {
        abilityValue += TomeAtLevel(ability, level);
    }

    return (size_t)abilityValue;
}

std::vector<size_t> Build::BaseClassLevels(size_t level) const
{
    const std::list<::Class>& classes = Classes();
    std::vector<size_t> levels(classes.size(), 0);
    std::list<::Class>::const_iterator cci = classes.begin();
    while (cci != classes.end())
    {
        std::string baseClass = FindClass((*cci).Name()).GetBaseClass();
        size_t classLevels = ClassLevels((*cci).Name(), level);
        size_t baseClassIndex = ClassIndex(baseClass);
        levels[baseClassIndex] = classLevels;
        ++cci;
    }
    return levels;
}

size_t Build::BaseClassLevels(
    const std::string& ct,
    size_t level) const
{
    // return the number of levels trained as the given class up to the specified level
    size_t baseClassLevels = 0;
    std::list<LevelTraining>::const_iterator clit = m_Levels.begin();
    for (size_t i = 0; i <= level && i < m_Levels.size(); ++i)
    {
        std::string levelClass = (*clit).HasClass() ? (*clit).Class() : Class_Unknown;
        std::string baseClass = FindClass(levelClass).GetBaseClass();
        if (baseClass == ct
                || levelClass == ct)
        {
            ++baseClassLevels;
        }
        ++clit;
    }
    return baseClassLevels;
}

std::vector<size_t> Build::ClassLevels(size_t level) const
{
    const std::list<::Class>& classes = Classes();
    std::vector<size_t> levels(classes.size(), 0);
    for (auto&& it: m_cachedClassLevels[level])
    {
        const std::string& c = it.first;
        const ::Class& cl = FindClass(c);
        levels[cl.Index()] = it.second;
    }
    return levels;
}

size_t Build::ClassLevels(
        const std::string& ct,
        size_t level) const
{
    // return the number of levels trained as the given class up to the specified level
    size_t classLevels = 0;
    if (ct == "All")
    {
        // just return the total class level for "All"
        classLevels = level + 1;
    }
    else
    {
        auto it = m_cachedClassLevels[level].find(ct);
        if (it != m_cachedClassLevels[level].end())
        {
            // class is present, get the cached value
            classLevels = it->second;
        }
    }
    return classLevels;
}

std::string Build::ClassAtLevel(size_t level) const
{
    std::string c = Class_Unknown;
    if (level < m_Levels.size())
    {
        std::list<LevelTraining>::const_iterator clit = m_Levels.begin();
        std::advance(clit, level);
        c = (*clit).HasClass() ? (*clit).Class() : Class_Unknown;
    }
    return c;
}

std::string Build::BaseClassAtLevel(size_t level) const
{
    std::string c = Class_Unknown;
    if (level < m_Levels.size())
    {
        std::list<LevelTraining>::const_iterator clit = m_Levels.begin();
        std::advance(clit, level);
        if ((*clit).HasClass())
        {
            c = (*clit).Class();
            const ::Class& cl = FindClass(c);
            if (cl.HasBaseClass())
            {
                c = cl.BaseClass();
            }
        }
    }
    return c;
}

int Build::LevelUpsAtLevel(AbilityType ability, size_t level) const
{
    return m_pLife->LevelUpsAtLevel(ability, level);
}

void Build::RevokeLostLevelFeats(size_t targetLevel, size_t currentLevel)
{
    std::list<TrainedFeat> allLostFeats;
    std::list<LevelTraining>::iterator it = m_Levels.begin();
    std::advance(it, targetLevel);
    for (size_t i = targetLevel; i < currentLevel; ++i)
    {
        const std::list<TrainedFeat>& automaticFeats = (*it).AutomaticFeats();
        const std::list<TrainedFeat>& trainedFeats = (*it).TrainedFeats();
        allLostFeats.insert(allLostFeats.end(), trainedFeats.begin(), trainedFeats.end());
        allLostFeats.insert(allLostFeats.end(), automaticFeats.begin(), automaticFeats.end());
        std::advance(it, 1);
    }

    // now revoke the effects of every lost feat
    for (auto lfit : allLostFeats)
    {
        const Feat& lfeat = FindFeat(lfit.FeatName());
        RevokeFeatEffects(lfeat);
    }
}

bool Build::IsFeatTrained(
        const std::string& featName) const
{
    // return true if the given feat is trained
    bool bTrained = false;
    std::list<TrainedFeat> currentFeats = CurrentFeats(Level());

    std::list<TrainedFeat>::const_iterator it = currentFeats.begin();
    while (it != currentFeats.end())
    {
        if ((*it).FeatName() == featName)
        {
            bTrained = true;
            break;
        }
        ++it;
    }
    return bTrained;
}

bool Build::IsGrantedFeat(const std::string& featName) const
{
    bool isGranted = false;
    CWnd* pWnd = AfxGetMainWnd();
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    const CGrantedFeatsPane* pGrantedFeatsPane =
            dynamic_cast<const CGrantedFeatsPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CGrantedFeatsPane)));
    if (pGrantedFeatsPane != NULL)
    {
        isGranted = pGrantedFeatsPane->IsGrantedFeat(featName);
    }
    return isGranted;
}

size_t Build::FeatTrainedCount(const std::string& featName) const
{
    std::list<TrainedFeat> currentFeats = CurrentFeats(Level());
    size_t count = TrainedCount(
            currentFeats,
            featName);
    return count;
}

TrainedFeat Build::GetTrainedFeat(
        size_t level,
        const std::string& type) const
{
    // find the specific trained feat at level by type
    TrainedFeat feat;       // blank if no feat found

    const LevelTraining & lt = LevelData(level);
    const std::list<TrainedFeat> & tfs = lt.TrainedFeats();
    std::list<TrainedFeat>::const_iterator it = tfs.begin();
    bool found = false;
    while (!found && it != tfs.end())
    {
        if ((*it).Type() == type)
        {
            feat = (*it);
            found = true;
        }
        ++it;
    }
    return feat;
}

std::vector<Feat> Build::TrainableFeats(
        const std::string& type,
        size_t level,
        const std::string& includeThisFeat) const
{
    std::vector<Feat> trainable;
    // need to know which feats have already been trained by this point
    // include any feats also trained at the current level
    std::list<TrainedFeat> currentFeats = CurrentFeats(level);

    // iterate the list of all feats and see if the character qualifies
    // to train it
    const std::map<std::string, Feat>& allFeats = StandardFeats();
    for (auto&& fit: allFeats)
    {
        if (IsFeatTrainable(level, type, fit.second, true)
                ||  fit.first == includeThisFeat)
        {
            // they can select this one, add it to the available list
            // unless it is in the ignore list
            if (fit.first == includeThisFeat
                || g_bShowIgnoredItems
                || !IsInIgnoreList(fit.first))
            {
                trainable.push_back(fit.second);
            }
        }
    }

    return trainable;
}

std::list<TrainedFeat> Build::CurrentFeats(size_t level) const
{
    // return a list of all the feats trained at the current level
    std::list<TrainedFeat> currentFeats;

    // first add any special feats (Past lives etc)
    const std::list<TrainedFeat> & specialFeats = Build::SpecialFeats();
    currentFeats.insert(currentFeats.end(), specialFeats.begin(), specialFeats.end());

    // now add the automatic and the trained feats at each level up to the level wanted
    size_t currentLevel = 0;
    std::list<LevelTraining>::const_iterator ldit = m_Levels.begin();
    while (currentLevel <= level && ldit != m_Levels.end())
    {
        const LevelTraining & levelData = (*ldit);
        // add the automatic feats for this level
        const std::list<TrainedFeat> & autoFeats = levelData.AutomaticFeats();
        currentFeats.insert(currentFeats.end(), autoFeats.begin(), autoFeats.end());

        // add the trained feats for this level
        const std::list<TrainedFeat> & trainedFeats = levelData.TrainedFeats();
        currentFeats.insert(currentFeats.end(), trainedFeats.begin(), trainedFeats.end());

        ++currentLevel;
        ++ldit;
    }
    return currentFeats;
}

bool Build::IsFeatTrainable(
        size_t level,
        const std::string& type,
        const Feat & feat,
        bool includeTomes,
        bool alreadyTrained,
        bool bIgnoreEpicOnly) const
{
    bool bShowEpicOnly = m_pLife->m_pCharacter->ShowEpicOnly();
    bool bShowUnavailable = m_pLife->m_pCharacter->ShowUnavailable();
    // function returns true if the given feat can be trained at this level
    // i.e. all required prerequisite feats/skills/abilities are met

    // need to know how many levels and of what classes they have trained
    std::vector<size_t> classLevels = ClassLevels(level);
    // need to know which feats have already been trained by this point
    // include any feats also trained at the current level
    std::list<TrainedFeat> currentFeats = CurrentFeats(level);

    // must be a trainable feat first
    bool bCanTrain = (feat.Acquire() == FeatAcquisition_Train);
    if (bCanTrain)
    {
        bool bHasGroupStandard = false;
        // must be a member of this group to be listed
        bCanTrain = false;
        for (auto&& git: feat.Group())
        {
            bCanTrain |= (git == type);
            bHasGroupStandard |= (git == "Standard");
        }
        if (!bCanTrain && feat.HasConditionalGroups())
        {
            if (feat.ConditionalGroups().RequirementsToUse().Met(*this, level, includeTomes, Weapon_Unknown, Weapon_Unknown))
            {
                for (auto&& git : feat.ConditionalGroups().Group())
                {
                    bCanTrain |= (git == type);
                    bHasGroupStandard |= (git == "Standard");
                }
            }
        }
        if (!bShowUnavailable)
        {
            if (type == "Epic Feat"
                && bShowEpicOnly
                && bHasGroupStandard
                && !bIgnoreEpicOnly)
            {
                // user only wants to show epic feats
                bCanTrain = false;
            }
        }
        else
        {
            if (!bShowEpicOnly
                && type == "Epic Feat")
            {
                bCanTrain |= bHasGroupStandard;
            }
        }
    }
    if (bCanTrain && !bShowUnavailable)
    {
        // do we meet the requirements to train this feat?
        if (feat.HasRequirementsToTrain())
        {
            bCanTrain = feat.RequirementsToTrain().Met(
                    *this,
                    //classLevels,
                    level,
                    //currentFeats,
                    includeTomes,
                    Weapon_Unknown,
                    Weapon_Unknown);
        }
    }
    if (bCanTrain)
    {
        // may have trained this feat the max times already
        size_t trainedCount = TrainedCount(currentFeats, feat.Name());
        if (alreadyTrained)
        {
            // were checking if an already trained feat is still trainable
            --trainedCount;
        }
        bCanTrain = (trainedCount < feat.MaxTimesAcquire());
    }
    // has to be one of the trainable feat types
    if (bCanTrain)
    {
        std::vector<FeatSlot> tfts = TrainableFeatTypeAtLevel(level);
        bool found = false;
        for (size_t i = 0; i < tfts.size(); ++i)
        {
            if (tfts[i].FeatType() == type)
            {
                found = true;
            }
        }
        bCanTrain = found;
    }
    return bCanTrain;
}

std::list<TrainedFeat> Build::SpecialFeats() const
{
    std::list<TrainedFeat> allSpecialFeats = m_pLife->SpecialFeats().Feats();
    std::list<TrainedFeat> allFavorFeats = FavorFeats().Feats();
    allSpecialFeats.insert(allSpecialFeats.end(), allFavorFeats.begin(), allFavorFeats.end());
    return allSpecialFeats;
}

void Build::TrainFeat(
        const std::string& featName,
        const std::string& type,
        size_t level,
        bool autoTrained)
{
    const Feat & feat = FindFeat(featName);

    bool featSwapWarning = false;
    if (level == 0)
    {
        // we evaluate the feat requirements again but without tomes being
        // applied to the characters abilities. This allows us to determine
        // whether the feat can be selected before entering the world or
        // after via a feat swap with Fred the Mind Flayer.
        if (!IsFeatTrainable(level, type, feat, false)
                && featName != " No Selection")
        {
            // this feat although trainable with Fred is not trainable
            // during character creation.
            featSwapWarning = true;
        }
    }
    // ensure re-selection of same feat in same slot does not change anything
    // as this can cause enhancements and feats to be revoked.
    std::list<LevelTraining>::iterator it = m_Levels.begin();
    std::advance(it, level);
    if (featName != (*it).FeatName(type) // is it the same feat that was previously selected?
            && !(featName == " No Selection" && (*it).FeatName(type) == ""))
    {
        // first revoke any previous trained feat in this slot at level
        std::string lostFeat = (*it).RevokeFeat(type);
        if (!lostFeat.empty())
        {
            const Feat & lfeat = FindFeat(lostFeat);
            RevokeFeatEffects(lfeat);
        }
        // create the log entry
        std::stringstream ss;
        if (featName != " No Selection")
        {
            // train new
            (*it).TrainFeat(featName, type, level, featSwapWarning);
            ApplyFeatEffects(feat);
            if (lostFeat != "")
            {
                ss << "Replaced the feat \"" << lostFeat.c_str()
                        << "\" with the feat \"" << featName.c_str()
                        << "\" in slot \"" << type.c_str()
                        << "\" at level  " << level + 1;
            }
            else
            {
                ss << "Trained the feat \"" << feat.Name().c_str()
                        << "\" in slot \"" << type.c_str()
                        << "\" at level  " << level + 1;
            }
        }
        else
        {
            ss << "Cleared the feat \"" << lostFeat.c_str()
                    << "\" in slot \"" << type.c_str()
                    << "\" at level  " << level + 1;
        }

        NotifyFeatTrained(featName);
        // some automatic feats may have changed due to the trained feat
        UpdateFeats();
        // a feat change can invalidate a feat selection at a later level
        VerifyTrainedFeats();
        SetModifiedFlag(TRUE);
        if (!autoTrained)
        {
            AutoTrainSingleSelectionFeats();
        }
        // Warforged feats can affect active stances
        if (!autoTrained)
        {
            GetLog().AddLogEntry(ss.str().c_str()); // and finally log it
        }
    }
}

void Build::Enhancement_SwapTrees(
        const std::string& tree1,
        const std::string& tree2)
{
    m_EnhancementSelectedTrees.SwapTrees(tree1, tree2);
    NotifyEnhancementTreeOrderChanged();
}

void Build::Destiny_SwapTrees(
        const std::string& tree1,
        const std::string& tree2)
{
    m_DestinySelectedTrees.SwapTrees(tree1, tree2);
    NotifyEnhancementTreeOrderChanged();
}

void Build::Reaper_SwapTrees(
        const std::string& tree1,
        const std::string& tree2)
{
    m_ReaperSelectedTrees.SwapTrees(tree1, tree2);
    NotifyEnhancementTreeOrderChanged();
}

int Build::AvailableActionPoints(
        size_t level,
        TreeType type) const
{
    // limit APs to heroic levels only
    level = min(MAX_CLASS_LEVEL, level);

    int levelAps = level * 4;
    // for actions points we need to track:
    // how many bonus action points the character has (m_bonusActionPoints)
    // how many action points have been spent in the racial tree (m_racialTreeSpend)
    // how many action points have been spent in the universal tree(s) (m_universalTreeSpend)
    // how many action points have been spent in other trees (m_classTreeSpend)
    int aps = 0;
    switch (type)
    {
    case TT_racial:
        aps = levelAps
                + m_pLife->BonusRacialActionPoints() - m_racialTreeSpend
                - max(m_universalTreeSpend - m_pLife->BonusUniversalActionPoints(), 0)
                - m_classTreeSpend;
        break;
    case TT_enhancement:
        aps = levelAps
                - max(m_racialTreeSpend - m_pLife->BonusRacialActionPoints(), 0)
                - max(m_universalTreeSpend - m_pLife->BonusUniversalActionPoints(), 0)
                - m_classTreeSpend;
        break;
    case TT_universal:
        aps = levelAps
                - max(m_racialTreeSpend - m_pLife->BonusRacialActionPoints(), 0)
                + m_pLife->BonusUniversalActionPoints() - m_universalTreeSpend
                - m_classTreeSpend;
        break;
    case TT_reaper:
        aps = 1000;  // no upper limit on reaper APs (i.e. you can buy all)
        break;
    case TT_epicDestiny:
        if (Level() >= MAX_CLASS_LEVEL)
        {
            aps = static_cast<int>(FindBreakdown(Breakdown_DestinyPoints)->Total())
                - m_destinyTreeSpend;
        }
        break;
    case TT_allEnhancementMinusSpent:
        aps -= m_racialTreeSpend;
        aps -= m_universalTreeSpend;
        aps -= m_classTreeSpend;
        // fall through
    case TT_allEnhancement:
        aps += levelAps
            + m_pLife->BonusRacialActionPoints()
            + m_pLife->BonusUniversalActionPoints();
        break;
    }
    return aps;
}

size_t Build::BonusRacialActionPoints() const
{
    return m_pLife->BonusRacialActionPoints();
}

size_t Build::BonusUniversalActionPoints() const
{
    return m_pLife->BonusUniversalActionPoints();
}

int Build::APSpentInTree(const std::string& treeName) const
{
    int spent = 0;
    const SpendInTree * pItem = FindSpendInTree(treeName);
    if (pItem != NULL)  // NULL if no items spent in tree or reaper/destiny tree
    {
        spent = pItem->Spent();
    }
    return spent;
}

const TrainedEnhancement * Build::IsTrained(
        const std::string& enhancementName,
        const std::string& selection) const
{
    // return NULL if item is not in the list
    const TrainedEnhancement * pItem = NULL;
    // iterate the list to see if its present
    std::list<EnhancementSpendInTree>::const_iterator eit = m_EnhancementTreeSpend.begin();
    while (pItem == NULL
            && eit != m_EnhancementTreeSpend.end())
    {
        const std::list<TrainedEnhancement> & te = (*eit).Enhancements();
        std::list<TrainedEnhancement>::const_iterator teit = te.begin();
        while (pItem == NULL
                && teit != te.end())
        {
            if ((*teit).EnhancementName() == enhancementName)
            {
                // may have a required sub-selection for the enhancement
                if (selection.empty()
                        || selection == (*teit).Selection())
                {
                    pItem = &(*teit);
                    break;
                }
            }
            ++teit;
        }
        ++eit;
    }
    if (pItem == NULL)
    {
        // iterate the reaper list to see if its present
        std::list<ReaperSpendInTree>::const_iterator rit = m_ReaperTreeSpend.begin();
        while (pItem == NULL
                && rit != m_ReaperTreeSpend.end())
        {
            const std::list<TrainedEnhancement> & te = (*rit).Enhancements();
            std::list<TrainedEnhancement>::const_iterator teit = te.begin();
            while (pItem == NULL
                    && teit != te.end())
            {
                if ((*teit).EnhancementName() == enhancementName)
                {
                    // may have a required sub-selection for the enhancement
                    if (selection.empty()
                            || selection == (*teit).Selection())
                    {
                        pItem = &(*teit);
                        break;
                    }
                }
                ++teit;
            }
            ++rit;
        }
    }
    if (pItem == NULL)
    {
        // iterate the epic destiny list to see if its present
        std::list<DestinySpendInTree>::const_iterator dit = m_DestinyTreeSpend.begin();
        while (pItem == NULL
                && dit != m_DestinyTreeSpend.end())
        {
            const std::list<TrainedEnhancement> & te = (*dit).Enhancements();
            std::list<TrainedEnhancement>::const_iterator teit = te.begin();
            while (pItem == NULL
                    && teit != te.end())
            {
                if ((*teit).EnhancementName() == enhancementName)
                {
                    // may have a required sub-selection for the enhancement
                    if (selection.empty()
                            || selection == (*teit).Selection())
                    {
                        pItem = &(*teit);
                        break;
                    }
                }
                ++teit;
            }
            ++dit;
        }
    }
    return pItem;
}

bool Build::IsClassSkill(
        SkillType skill,
        size_t level) const     // 0 based
{
    bool isClassSkill = false;  // assume not a class skill
    // check the class at each previous trained level to see if its a class skill
    std::list<LevelTraining>::const_iterator it = m_Levels.begin();
    for (size_t li = 0; li < m_Levels.size() && li <= level; ++li)
    {
        if ((*it).HasClass())
        {
            const ::Class & ct = FindClass((*it).Class());
            if (ct.IsClassSkill(skill))
            {
                // ok, it is a class skill
                isClassSkill = true;
                break;  // no need to check the rest
            }
        }
        ++it;
    }
    return isClassSkill;
}

void Build::SpendSkillPoint(
        size_t level,
        SkillType skill,
        bool suppressUpdate)
{
    // add the log entry
    std::stringstream ss;
    ss << "Trained skill \""
            << EnumEntryText(skill, skillTypeMap)
            << "\" at level " << level + 1;
    GetLog().AddLogEntry(ss.str().c_str());
    // update the skill point spend for the correct level data
    std::list<LevelTraining>::iterator it = m_Levels.begin();
    std::advance(it, level);
    (*it).TrainSkill(skill);
    if (!suppressUpdate)
    {
        NotifySkillSpendChanged(level, skill);
    }
    SetModifiedFlag(TRUE);
}

void Build::RevokeSkillPoint(
        size_t level,
        SkillType skill,
        bool suppressUpdate)
{
    // add the log entry
    std::stringstream ss;
    ss << "Revoked skill \""
            << EnumEntryText(skill, skillTypeMap)
            << "\" at level " << level + 1;
    GetLog().AddLogEntry(ss.str().c_str());
    std::list<LevelTraining>::iterator it = m_Levels.begin();
    std::advance(it, level);
    (*it).RevokeSkill(skill);
    if (!suppressUpdate)
    {
        NotifySkillSpendChanged(level, skill);
        // revoking a skill point in theory can invalidate a feat selection
        VerifyTrainedFeats();
        SetModifiedFlag(TRUE);
    }
}

double Build::MaxSkillForLevel(
        SkillType skill,
        size_t level) const       // 0 based
{
    double maxSkill = 0;
    // max skill depends on whether this skill is or has been a previous level
    // class skill.
    bool isClassSkill = IsClassSkill(
            skill,
            level);

    ++level;        // 1 based for calculation
    level = min(MAX_CLASS_LEVEL, level); // only valid for heroic levels
    if (isClassSkill)
    {
        // max skill is 3 + character level
        maxSkill = 3.0 + level;
    }
    else
    {
        // some skills must be a class skill to be trained
        if (skill == Skill_DisableDevice
                || skill == Skill_OpenLock)
        {
            // this skill cannot be trained unless its a class skill for
            // any of the classes in a multi-class character
            maxSkill = 0;
        }
        else
        {
            // max skill for cross class is (3 + level) / 2
            maxSkill = (3.0 + level) / 2.0;
        }
    }
    return maxSkill;
}

size_t Build::SpentAtLevel(
        SkillType skill,
        size_t level) const
{
    size_t spent = 0;
    const LevelTraining & levelData = LevelData(level);
    // count points spent in this skill at this level
    const std::list<TrainedSkill> & trainedSkills = levelData.TrainedSkills();
    std::list<TrainedSkill>::const_iterator tsit = trainedSkills.begin();
    while (tsit != trainedSkills.end())
    {
        if ((*tsit).Skill() == skill)
        {
            ++spent;
        }
        ++tsit;
    }
    return spent;
}

double Build::SkillAtLevel(
        SkillType skill,
        size_t level,
        bool includeTome) const
{
    double skillLevel = 0;  // assume untrained

    std::list<LevelTraining>::const_iterator it = m_Levels.begin();
    for (size_t li = 0; li <= level; ++li)
    {
        // full point per spend if its a class skill at this level
        // half point per spend if its a cross class skill at this level
        const LevelTraining & ld = (*it);
        const ::Class & ct = FindClass(ld.HasClass() ? ld.Class() : Class_Unknown);
        bool isClassSkill = ct.IsClassSkill(skill);
        size_t spent = SpentAtLevel(skill, li);
        if (isClassSkill)
        {
            // full ranks for each train
            skillLevel += spent;
        }
        else
        {
            // half rank for each train
            skillLevel += (spent / 2.0);
        }
        ++it;
    }
    // add in any tome bonus if required
    if (includeTome)
    {
        size_t tomeValue = SkillTomeValue(skill, Level());
        skillLevel += tomeValue;
    }
    return skillLevel;
}

void Build::SkillsUpdated()
{
    // once for every skill at level 20
    for (size_t skill = Skill_Unknown + 1; skill < Skill_Count; ++skill)
    {
        NotifySkillSpendChanged(MAX_CLASS_LEVEL, (SkillType)skill);
    }
    VerifyTrainedFeats();
    SetModifiedFlag(TRUE);
}

void Build::UpdateSkillPoints()
{
    // update the skill points for all levels
    std::list<LevelTraining>::iterator it = m_Levels.begin();
    for (size_t level = 0; level < MAX_CLASS_LEVEL && level < Level(); ++level)
    {
        std::string ct = (*it).HasClass() ? (*it).Class() : Class_Unknown;
        const ::Class & c = FindClass(ct);
        size_t available = c.SkillPoints(
                Race(),
                AbilityAtLevel(Ability_Intelligence, level, (level != 0)),  // include tomes for level 2 onward
                level);
        (*it).Set_SkillPointsAvailable(available);
        ++it;
    }
}

void Build::UpdateSkillPoints(size_t level)
{
    UNREFERENCED_PARAMETER(level);
    // update the skill points for this specific level
    if (level < MAX_CLASS_LEVEL)
    {
        // only have skill points for heroic levels
        std::list<LevelTraining>::iterator it = m_Levels.begin();
        std::advance(it, level);
        std::string ct = (*it).HasClass() ? (*it).Class() : Class_Unknown;
        const ::Class & c = FindClass(ct);
        size_t available = c.SkillPoints(
                Race(),
                AbilityAtLevel(Ability_Intelligence, level, (level != 0)),  // include tomes for level 2 onward
                level);
        (*it).Set_SkillPointsAvailable(available);
    }
}

void Build::VerifyTrainedFeats()
{
    // user has taken an action that could have invalidated a current feat selection
    // check them all to see if they are still valid, revoke those which are not
    // but also keep a list of them. This allows us to try and re-place them in other
    // feats slots if we can
    FeatsListObject revokedFeats(L"Revoked Feats");

    std::list<LevelTraining>::iterator it = m_Levels.begin();
    size_t level = 0;
    // we have to repeat a levels revokes as we cannot guarantee the order that
    // the feats will be checked in. e.g. we check Mobility and Dodge in that order
    // The prerequisite for mobility is met as dodge is trained at the time its checked
    // but dodge is removed due to its requirements. We then have an orphaned mobility feat
    // who's requirements are no longer met. So we repeat the checks on the feats at this level
    // until such time as no feat revocations occur.
    while (it != m_Levels.end())
    {
        bool redoLevel = false;     // true if have to check feats at this level again
        const std::list<TrainedFeat> & feats = (*it).TrainedFeats();
        std::list<TrainedFeat>::const_iterator fit = feats.begin();
        while (!redoLevel && fit != feats.end())
        {
            // is this feat trainable at this level?
            const Feat & feat = FindFeat((*fit).FeatName());
            if (!IsFeatTrainable(level, (*fit).Type(), feat, true, true, true))
            {
                // no longer trainable, remove it from the list
                revokedFeats.Add(*fit);
                // revoke from level training in this slot at level
                (*it).RevokeFeat((*fit).Type());
                // effects and feat revoked if it cannot be moved elsewhere later
                redoLevel = true;   // check the other feats at this level again
            }
            else
            {
                ++fit;
            }
        }
        if (!redoLevel)
        {
            // only increment these if we are not checking the feats at this level again
            ++it;
            ++level;
        }
    }
    // if we have any revoked feats, try and re-train them in matching empty
    // feat slots which qualify to hold those feats
    std::list<TrainedFeat> feats = revokedFeats.Feats();
    if (feats.size() > 0)
    {
        it = m_Levels.begin();
        level = 0;
        while (it != m_Levels.end())
        {
            // check each revoked feat to see if its trainable at this level in
            // an empty feat slot. If it is, train it and remove from the list
            std::list<TrainedFeat>::const_iterator fit = feats.begin();
            while (fit != feats.end())
            {
                // is this feat trainable at this level?
                const Feat & feat = FindFeat((*fit).FeatName());
                if (IsFeatTrainable(level, (*fit).Type(), feat, true, false))
                {
                    // slot must be empty
                    TrainedFeat tf = GetTrainedFeat(
                            level,
                            (*fit).Type());
                    // name is blank if no feat currently trained
                    if (tf.FeatName() == "")
                    {
                        // it is, train it
                        (*it).TrainFeat((*fit).FeatName(), (*fit).Type(), level, false);
                        // add log entry
                        std::stringstream ss;
                        ss << "Moved the feat \"" << (*fit).FeatName().c_str()
                                << "\" from level " << (*fit).LevelTrainedAt() + 1
                                << " to level " << level + 1;
                        GetLog().AddLogEntry(ss.str().c_str());
                        fit = feats.erase(fit); // remove and go to next
                    }
                    else
                    {
                        ++fit;  // check the next
                    }
                }
                else
                {
                    ++fit;      // check the next
                }
            }
            ++it;
            ++level;
        }
    }
    // now generate a log entry for the revoked feats that could not be re-trained
    // in slots elsewhere
    std::list<TrainedFeat>::const_iterator fit = feats.begin();
    while (fit != feats.end())
    {
        const Feat & feat = FindFeat((*fit).FeatName());
        RevokeFeatEffects(feat);
        NotifyFeatRevoked(feat.Name());
        // add log entry
        std::stringstream ss;
        ss << "Revoked the feat \"" << feat.Name().c_str()
                << "\" at level  " << (*fit).LevelTrainedAt() + 1
                << " as the requirements no longer met.";
        GetLog().AddLogEntry(ss.str().c_str());
        ++fit;
    }
}

void Build::UpdateSpells()
{
    // check that we do not have more spells trained at a given spell
    // level than the current class assignments allow. If we do,
    // revoke the spells over the maximum size
    std::vector<size_t> classLevels = ClassLevels(Level()-1);
    const std::list<::Class> & classes = Classes();
    for (auto&& ci : classes)
    {
        // get the number of spells available for this class at this level
        std::vector<size_t> spellSlots = ci.SpellSlotsAtLevel(classLevels[ClassIndex(ci.Name())]);
        for (int spellLevel = 0; spellLevel < (int)spellSlots.size(); ++spellLevel)
        {
            // get the current trained spells at this level
            std::list<TrainedSpell> trainedSpells = TrainedSpells(ci.Name(), spellLevel + 1); // 1 based
            while (trainedSpells.size() > spellSlots[spellLevel])
            {
                TrainedSpell ts = trainedSpells.back();
                trainedSpells.pop_back(); // remove oldest
                // need to revoke this spell
                RevokeSpell(ci.Name(), ts.Level(), ts.SpellName(), true);
                std::stringstream ss;
                ss << "Revoked the spell \"" << ts.SpellName()
                    << "\" at spell level  " << spellLevel + 1;
                GetLog().AddLogEntry(ss.str().c_str());
            }
        }
    }
}

std::list<TrainedSpell> Build::TrainedSpells(
        const std::string& classType,
        size_t level) const
{
    // return the list of trained spells for this class at this level
    std::list<TrainedSpell> spells;
    std::vector<TrainedSpell>::const_iterator it = m_TrainedSpells.begin();
    while (it != m_TrainedSpells.end())
    {
        if ((*it).Class() == classType
                && (*it).Level() == level)
        {
            spells.push_back((*it));
        }
        ++it;
    }
    return spells;
}

void Build::TrainSpell(
        const std::string& ct,
        size_t level,
        const std::string& spellName)
{
    // add the spell to the list of those trained
    TrainedSpell spell(ct, level, spellName);
    m_TrainedSpells.push_back(spell);
    std::sort(m_TrainedSpells.begin(), m_TrainedSpells.end());
    //??NotifySpellTrained(spell);
    SetModifiedFlag(TRUE);

    std::stringstream ss;
    ss << "Trained class \"" << ct << "\" spell \"" << spellName << "\" at spell level " << level;
    GetLog().AddLogEntry(ss.str().c_str());
    ApplySpellEffects(ct, spellName);
}

void Build::RevokeSpell(
        const std::string& ct,
        size_t level,
        const std::string& spellName,
        bool bSuppressLog)
{
    // add the spell to the list of those trained
    TrainedSpell spell(ct, level, spellName);
    // remove it from the list
    bool found = false;
    std::vector<TrainedSpell>::iterator it = m_TrainedSpells.begin();
    while (!found && it != m_TrainedSpells.end())
    {
        if ((*it) == spell)
        {
            m_TrainedSpells.erase(it);
            found = true;
        }
        else
        {
            ++it;
        }
    }
    ASSERT(found);
    //??NotifySpellRevoked(spell);
    SetModifiedFlag(TRUE);

    if (!bSuppressLog)
    {
        std::stringstream ss;
        ss << "Revoked class \"" << ct << "\" spell \"" << spellName << "\" at spell level " << level;
        GetLog().AddLogEntry(ss.str().c_str());
    }
    RevokeSpellEffects(ct, spellName);
}

bool Build::IsSpellTrained(
        const std::string& spellName) const
{
    bool found = false;
    std::vector<TrainedSpell>::const_iterator it = m_TrainedSpells.begin();
    while (!found && it != m_TrainedSpells.end())
    {
        if ((*it).SpellName() == spellName)
        {
            found = true;
            break;
        }
        else
        {
            ++it;
        }
    }
    return found;
}

void Build::ApplySpellEffects()
{
    // apply any effects from any trained spells (always assumed to be active)
    for (auto&& tsit: m_TrainedSpells)
    {
        ApplySpellEffects(tsit.Class(), tsit.SpellName());
    }
}

void Build::ApplySpellEffects(const std::string& ct, const std::string& spellName)
{
    const Spell& spell = FindSpellByName(spellName);
    for (auto&& seit: spell.Effects())
    {
        Effect copy = seit;
        if (copy.AType() == Amount_ClassLevel)
        {
            copy.SetStackSource(ct);
        }
        copy.SetDisplayName(std::string("Spell: ") + spellName);
        copy.SetApplyAsItemEffect();
        NotifyEnhancementEffectApplied(copy);
    }
}

void Build::RevokeSpellEffects(const std::string& ct, const std::string& spellName)
{
    const Spell& spell = FindSpellByName(spellName);
    for (auto&& seit : spell.Effects())
    {
        Effect copy = seit;
        if (copy.AType() == Amount_ClassLevel)
        {
            copy.SetStackSource(ct);
        }
        copy.SetDisplayName(std::string("Spell: ") + spellName);
        copy.SetApplyAsItemEffect();
        NotifyEnhancementEffectRevoked(copy);
    }
}

void Build::UpdateFeats()
{
    // we start with any special feats (past lives etc)
    std::list<TrainedFeat> allFeats = SpecialFeats();
    // for each level, determine what automatic feats are trained
    // also, update each level with the trainable feat types
    for (size_t level = 0; level < Level(); ++level)
    {
        UpdateFeats(level, &allFeats);
    }
    NotifyBuildAutomaticFeatsChanged((size_t)-1);
    //KeepGrantedFeatsUpToDate();
}

void Build::UpdateFeats(
        size_t level,
        std::list<TrainedFeat>* allFeats)
{
    std::list<LevelTraining>::iterator it = m_Levels.begin();
    std::advance(it, level);
    std::list<TrainedFeat> oldFeats = (*it).AutomaticFeats();

    // add the trained feats for this level as they can affect the available automatic feats
    const std::list<TrainedFeat> & trainedFeats = (*it).TrainedFeats();
    allFeats->insert(allFeats->end(), trainedFeats.begin(), trainedFeats.end());

    std::list<TrainedFeat> automaticFeats = AutomaticFeats(level, *allFeats);
    allFeats->insert(allFeats->end(), automaticFeats.begin(), automaticFeats.end());

    // have the automatic feats at this level changed?
    if (automaticFeats != oldFeats)
    {
        // first revoke the feats at this level then apply the new ones
        for (auto&& ofit: oldFeats)
        {
            const Feat & feat = FindFeat(ofit.FeatName());
            RevokeFeatEffects(feat);
        }
        (*it).Set_AutomaticFeats(automaticFeats);
        // now apply the new automatic feats
        for (auto&& afit: automaticFeats)
        {
            const Feat & feat = FindFeat(afit.FeatName());
            ApplyFeatEffects(feat);
        }
        NotifyBuildAutomaticFeatsChanged(level);
    }
}

std::list<TrainedFeat> Build::AutomaticFeats(
    size_t level,
    const std::list<TrainedFeat>& currentFeats) const
{
    // determine which feats are automatically gained at this level/class and race
    std::list<TrainedFeat> feats;
    // need to know how many and of what classes have been trained up to here
    std::vector<size_t> classLevels = ClassLevels(level);
    const std::map<std::string, Feat>& allFeats = StandardFeats();

    for (auto&& fit: allFeats)
    {
        // check every feats automatic acquisition entries (if any)
        auto aa = fit.second.AutomaticAssignment();
        bool acquire = false;
        std::list<AutomaticAcquisition>::const_iterator aait = aa.begin();
        while (!acquire && aait != aa.end())
        {
            acquire |= (*aait).Met(*this, level, true, Weapon_Unknown, Weapon_Unknown);
            if (acquire
                    && !(*aait).HasIgnoreRequirements())
            {
                if (fit.second.HasRequirementsToTrain())
                {
                    // must also meet the standard feat requirements
                    acquire = fit.second.RequirementsToTrain().Met(*this, level, true, Weapon_Unknown, Weapon_Unknown);
                }
            }
            ++aait;
        }
        if (acquire)
        {
            // first count how many times this feat has been acquired already
            size_t count = TrainedCount(
                    currentFeats,
                    fit.second.Name());
            // by default unless specified by the feat otherwise, any feat
            // can only be acquired once
            if (count < fit.second.MaxTimesAcquire()) // MaxTimesAcquire defaults to 1 if not specified in the file
            {
                // this feat is automatic at this level
                // train it!
                TrainedFeat feat(fit.second.Name(), "Automatic", level);
                feats.push_back(feat);
            }
        }
    }
    return feats;
}

void Build::AutoTrainSingleSelectionFeats()
{
    for (size_t level = 0; level < Level(); ++level)
    {
        std::vector<FeatSlot> tfts = TrainableFeatTypeAtLevel(level);
        for (size_t i = 0; i < tfts.size(); ++i)
        {
            if (tfts[i].HasAutoPopulate())
            {
                TrainedFeat tf = GetTrainedFeat(
                        level,
                        tfts[i].FeatType());
                if (tf.FeatName() == "")
                {
                    std::vector<Feat> availableFeats = TrainableFeats(
                        tfts[i].FeatType(),
                        level,
                        "");
                    if (availableFeats.size() == 1)
                    {
                        TrainFeat(
                            availableFeats[0].Name(),
                            tfts[i].FeatType(),
                            level,
                            true);
                        // add log entry
                        std::stringstream ss;
                        ss << "Auto trained feat at level " << level + 1
                                << " in slot type \"" << tfts[i].FeatType().c_str()
                                << "\" with feat \"" << availableFeats[0].Name().c_str() << "\"";
                        GetLog().AddLogEntry(ss.str().c_str());
                    }
                }
            }
        }
    }
}

void Build::VerifyGear()
{
    EquippedGear gear = ActiveGearSet();
    bool revokeOccurred = false;
    // need to know how many levels and of what classes they have trained
    std::vector<size_t> classLevels = ClassLevels(Level()-1);
    // need to know which feats have already been trained by this point
    // include any feats also trained at the current level
    std::list<TrainedFeat> currentFeats = CurrentFeats(Level());
    // check every item
    for (size_t i = Inventory_Unknown + 1; i < Inventory_Count; ++i)
    {
        if (gear.HasItemInSlot((InventorySlotType)i))
        {
            Item item = gear.ItemInSlot((InventorySlotType)i);
            bool bRemoveItem = (item.MinLevel() > Level());
            if (item.HasRequirementsToUse())
            {
                if (!item.RequirementsToUse().Met(*this, Level()-1, true, item.HasWeapon() ? item.Weapon() : Weapon_Unknown, Weapon_Unknown))
                {
                    bRemoveItem |= true;
                }
            }
            if (bRemoveItem)
            {
                CString text("Item in slot ");
                revokeOccurred = true;
                ClearGearInSlot(gear.Name(), (InventorySlotType)i);
                CString itemName;
                itemName = EnumEntryText((InventorySlotType)i, InventorySlotTypeMap);
                itemName += ": ";
                itemName += item.Name().c_str();
                itemName += " removed as requirements no longer met.\r\n";
                text += itemName;
                GetLog().AddLogEntry(text);
            }
        }
    }
    if (revokeOccurred)
    {
        //UpdateGreensteelStances();      // only on gear changes
    }
}

void Build::SetModifiedFlag(BOOL modified)
{
    m_pLife->SetModifiedFlag(modified);
}

void Build::NotifyStanceActivated(const std::string& name)
{
    NotifyAll(&BuildObserver::UpdateStanceActivated, this, name);
}

void Build::NotifyStanceDeactivated(const std::string& name)
{
    NotifyAll(&BuildObserver::UpdateStanceDeactivated, this, name);
}

void Build::NotifyStanceDisabled(const std::string& name)
{
    NotifyAll(&BuildObserver::UpdateStanceDisabled, this, name);
}

void Build::NotifySliderChanged(const std::string& sliderName, int newValue)
{
    NotifyAll(&BuildObserver::UpdateSliderChanged, this, sliderName, newValue);
}

void Build::ApplyFeatEffects(const Feat & feat)
{
    // if we have just trained a feat that is also a stance
    // add a stance selection button for each
    const std::list<Stance> & stances = feat.Stances();
    for (auto&& sit: stances)
    {
        NotifyNewStance(sit);
    }
    // if we have any DC objects notify about them
    const std::list<DC> & dcs = feat.DCs();
    for (auto&& dcit: dcs)
    {
        NotifyNewDC(dcit);
    }
    // apply the feats effects
    const std::list<Effect> & effects = feat.Effects();
    for (auto&& eit : effects)
    {
        NotifyFeatEffectApplied(eit);
    }
}

void Build::RevokeFeatEffects(const Feat & feat)
{
    // revoke the feats effects
    const std::list<Effect>& effects = feat.Effects();
    for (auto&& eit : effects)
    {
        NotifyFeatEffectRevoked(eit);
    }
    // revoke any stances this feat has
    const std::list<Stance> & stances = feat.Stances();
    for (auto&& sit : stances)
    {
        NotifyRevokeStance(sit);
    }
    // if we have any DC objects notify about them
    const std::list<DC> & dcs = feat.DCs();
    for (auto&& dcit: dcs)
    {
        NotifyRevokeDC(dcit);
    }
    NotifyFeatRevoked(feat.Name());
}

const SpendInTree* Build::FindSpendInTree(const std::string& treeName) const
{
    const SpendInTree* pTree = NULL;

    std::list<EnhancementSpendInTree>::const_iterator eit = m_EnhancementTreeSpend.begin();
    while (pTree == NULL
        && eit != m_EnhancementTreeSpend.end())
    {
        if ((*eit).TreeName() == treeName)
        {
            pTree = &(*eit);
            break;
        }
        ++eit;
    }
    if (pTree == NULL)
    {
        std::list<ReaperSpendInTree>::const_iterator rit = m_ReaperTreeSpend.begin();
        while (pTree == NULL
            && rit != m_ReaperTreeSpend.end())
        {
            if ((*rit).TreeName() == treeName)
            {
                pTree = &(*rit);
                break;
            }
            ++rit;
        }
    }
    if (pTree == NULL)
    {
        std::list<DestinySpendInTree>::const_iterator dit = m_DestinyTreeSpend.begin();
        while (pTree == NULL
            && dit != m_DestinyTreeSpend.end())
        {
            if ((*dit).TreeName() == treeName)
            {
                pTree = &(*dit);
                break;
            }
            ++dit;
        }
    }
    return pTree;
}

SpendInTree* Build::FindSpendInTree(const std::string& treeName)
{
    SpendInTree* pTree = NULL;

    std::list<EnhancementSpendInTree>::iterator eit = m_EnhancementTreeSpend.begin();
    while (pTree == NULL
        && eit != m_EnhancementTreeSpend.end())
    {
        if ((*eit).TreeName() == treeName)
        {
            pTree = &(*eit);
            break;
        }
        ++eit;
    }
    if (pTree == NULL)
    {
        std::list<ReaperSpendInTree>::iterator rit = m_ReaperTreeSpend.begin();
        while (pTree == NULL
            && rit != m_ReaperTreeSpend.end())
        {
            if ((*rit).TreeName() == treeName)
            {
                pTree = &(*rit);
                break;
            }
            ++rit;
        }
    }
    if (pTree == NULL)
    {
        std::list<DestinySpendInTree>::iterator dit = m_DestinyTreeSpend.begin();
        while (pTree == NULL
            && dit != m_DestinyTreeSpend.end())
        {
            if ((*dit).TreeName() == treeName)
            {
                pTree = &(*dit);
                break;
            }
            ++dit;
        }
    }
    return pTree;
}

void Build::RemoveTreeSpend(const std::string& treeName)
{
    SpendInTree* pTree = NULL;

    std::list<EnhancementSpendInTree>::iterator eit = m_EnhancementTreeSpend.begin();
    while (pTree == NULL
        && eit != m_EnhancementTreeSpend.end())
    {
        if ((*eit).TreeName() == treeName)
        {
            m_EnhancementTreeSpend.erase(eit);
            break;
        }
        ++eit;
    }
    if (pTree == NULL)
    {
        std::list<ReaperSpendInTree>::iterator rit = m_ReaperTreeSpend.begin();
        while (pTree == NULL
            && rit != m_ReaperTreeSpend.end())
        {
            if ((*rit).TreeName() == treeName)
            {
                m_ReaperTreeSpend.erase(rit);
                break;
            }
            ++rit;
        }
    }
    if (pTree == NULL)
    {
        std::list<DestinySpendInTree>::iterator dit = m_DestinyTreeSpend.begin();
        while (pTree == NULL
            && dit != m_DestinyTreeSpend.end())
        {
            if ((*dit).TreeName() == treeName)
            {
                m_DestinyTreeSpend.erase(dit);
                break;
            }
            ++dit;
        }
    }
}

void Build::NotifySkillSpendChanged(size_t level, SkillType skill)
{
    NotifyAll(&BuildObserver::UpdateSkillSpendChanged, this, level, skill);
}

void Build::NotifyAbilityValueChanged(AbilityType ability)
{
    NotifyAll(&BuildObserver::UpdateAbilityValueChanged, this, ability);
}

void Build::NotifyEnhancementTrained(const EnhancementItemParams& item)
{
    NotifyAll(
        &BuildObserver::UpdateEnhancementTrained,
        this,
        item);
}

void Build::NotifyEnhancementRevoked(const EnhancementItemParams& item)
{
    NotifyAll(
        &BuildObserver::UpdateEnhancementRevoked,
        this,
        item);
}
void Build::NotifyEnhancementEffectApplied(
    const Effect& effect)
{
    if (effect.IsType(Effect_SpellListAddition))
    {
        AddSpellListAddition(effect);
    }
    NotifyAll(&BuildObserver::UpdateEnhancementEffectApplied, this, effect);
}

void Build::NotifyEnhancementEffectRevoked(
    const Effect& effect)
{
    if (effect.IsType(Effect_SpellListAddition))
    {
        RevokeSpellListAddition(effect);
    }
    NotifyAll(&BuildObserver::UpdateEnhancementEffectRevoked, this, effect);
}

void Build::NotifyEnhancementTreeOrderChanged()
{
    NotifyAll(&BuildObserver::UpdateEnhancementTreeOrderChanged, this);
}

void Build::NotifyActionPointsChanged()
{
    NotifyAll(&BuildObserver::UpdateActionPointsChanged, this);
}

void Build::NotifyNewStance(const Stance& stance)
{
    NotifyAll(&BuildObserver::UpdateNewStance, this, stance);
}

void Build::NotifyRevokeStance(const Stance& stance)
{
    NotifyAll(&BuildObserver::UpdateRevokeStance, this, stance);
}

void Build::NotifyBuildAutomaticFeatsChanged(size_t level)
{
    NotifyAll(&BuildObserver::UpdateBuildAutomaticFeatsChanged, this, level);
}

void Build::NotifyNewDC(const DC& dc)
{
    NotifyAll(&BuildObserver::UpdateNewDC, this, dc);
}

void Build::NotifyRevokeDC(const DC& dc)
{
    NotifyAll(&BuildObserver::UpdateRevokeDC, this, dc);
}

void Build::SetSkillTome(SkillType skill, size_t value)
{
    m_pLife->SetSkillTome(skill, value);
}

size_t Build::SkillTomeValue(SkillType skill, size_t level) const
{
    return m_pLife->SkillTomeValue(skill, level);
}

void Build::SetAbilityTome(
        AbilityType ability,
        size_t value)
{
    m_pLife->SetAbilityTome(ability, value);
}

size_t Build::AbilityTomeValue(
        AbilityType ability) const
{
    return m_pLife->AbilityTomeValue(ability);
}

int Build::TomeAtLevel(
        AbilityType ability,
        size_t level) const
{
    return m_pLife->TomeAtLevel(ability, level);
}

size_t Build::StrTome() const
{
    return m_pLife->StrTome();
}

size_t Build::DexTome() const
{
    return m_pLife->DexTome();
}

size_t Build::ConTome() const
{
    return m_pLife->ConTome();
}

size_t Build::IntTome() const
{
    return m_pLife->IntTome();
}

size_t Build::WisTome() const
{
    return m_pLife->WisTome();
}

size_t Build::ChaTome() const
{
    return m_pLife->ChaTome();
}

AbilityType Build::Level4() const
{
    return m_pLife->Level4();
}

AbilityType Build::Level8() const
{
    return m_pLife->Level8();
}

AbilityType Build::Level12() const
{
    return m_pLife->Level12();
}

AbilityType Build::Level16() const
{
    return m_pLife->Level16();
}

AbilityType Build::Level20() const
{
    return m_pLife->Level20();
}

AbilityType Build::Level24() const
{
    return m_pLife->Level24();
}

AbilityType Build::Level28() const
{
    return m_pLife->Level28();
}

AbilityType Build::Level32() const
{
    return m_pLife->Level32();
}

AbilityType Build::Level36() const
{
    return m_pLife->Level36();
}

AbilityType Build::Level40() const
{
    return m_pLife->Level40();
}

void Build::SetAbilityLevelUp(size_t level, AbilityType ability)
{
    AbilityType old = m_pLife->SetAbilityLevelUp(level, ability);
    if (old != ability)
    {
        // add the log entry
        std::stringstream ss;
        ss << "Ability level up at level " << level
                << " changed from \"" << EnumEntryText(old, abilityTypeMap)
                << "\" to \"" << EnumEntryText(ability, abilityTypeMap) << "\"";
        GetLog().AddLogEntry(ss.str().c_str());
        if (ability == Ability_Intelligence
            || old == Ability_Intelligence)
        {
            UpdateSkillPoints();
        }
        NotifyAbilityValueChanged(old);
        NotifyAbilityValueChanged(ability);
    }
}

AbilityType Build::AbilityLevelUp(size_t level) const
{
    AbilityType at = Ability_Unknown;
    switch (level)
    {
        case 4: at = Level4();   break;
        case 8: at = Level8();   break;
        case 12: at = Level12();   break;
        case 16: at = Level16();   break;
        case 20: at = Level20();   break;
        case 24: at = Level24();   break;
        case 28: at = Level28();   break;
        case 32: at = Level32();   break;
        case 36: at = Level36();   break;
        case 40: at = Level40();   break;
    }
    return at;
}


size_t Build::BaseAbilityValue(AbilityType ability) const
{
    const AbilitySpend & as = BuildPoints();
    const ::Race & race = FindRace(Race());
    size_t value = 8 + race.RacialModifier(ability) + as.GetAbilitySpend(ability);
    return value;
}

void Build::SpendOnAbility(AbilityType ability)
{
    const AbilitySpend & as = BuildPoints();
    size_t nextCost = as.NextPointsSpentCost(ability);
    // add the log entry
    std::stringstream ss;
    ss << nextCost << " Build point(s) spent on ability \"" << EnumEntryText(ability, abilityTypeMap) << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
    m_BuildPoints.SpendOnAbility(ability);
    // if Intelligence has changed, update the available skill points
    if (ability == Ability_Intelligence)
    {
        UpdateSkillPoints();
    }
    NotifyAbilityValueChanged(ability);
    SetModifiedFlag(TRUE);
}

void Build::RevokeSpendOnAbility(AbilityType ability)
{
    const AbilitySpend & as = BuildPoints();
    size_t lastCost = as.LastPointsSpentCost(ability);
    // add the log entry
    std::stringstream ss;
    ss << lastCost << " Build point(s) revoked on ability \"" << EnumEntryText(ability, abilityTypeMap) << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
    m_BuildPoints.RevokeSpendOnAbility(ability);
    // if Intelligence has changed, update the available skill points
    if (ability == Ability_Intelligence)
    {
        UpdateSkillPoints();
    }
    NotifyAbilityValueChanged(ability);
    SetModifiedFlag(TRUE);
    // revoking an ability point in theory can invalidate a feat selection
    VerifyTrainedFeats();
}

size_t Build::BuildPointsPastLifeCount() const
{
    // note that we only consider heroic and racial past lives for
    // the past lives count to determine build points because:
    //   Epic past lives do not add to build points
    //   Iconic past lives indirectly do, as you also get a heroic past life
    size_t numPastLifeFeats = 0;
    const std::list<TrainedFeat> & feats = SpecialFeats();
    std::list<TrainedFeat>::const_iterator it = feats.begin();
    while (it != feats.end())
    {
        const Feat & feat = FindFeat((*it).FeatName());
        if (feat.Acquire() == FeatAcquisition_HeroicPastLife
                || feat.Acquire() == FeatAcquisition_RacialPastLife)
        {
            // it is a trained feat that affects number of available
            // build points
            ++numPastLifeFeats;
        }
        ++it;
    }
    return numPastLifeFeats;
}

size_t Build::DetermineBuildPoints()
{
    // determine how many build points the current past life count
    // allows this character to have
    size_t numPastLifeFeats = BuildPointsPastLifeCount();
    // no past lives gives a choice between build point entries [0] and [1]
    // 1 past lives gives [2]
    // 2+ past lives gives entry [3]
    const ::Race & race = FindRace(Race()); // ask the race for its build point
    const std::vector<size_t> & buildPointsVector = race.BuildPoints();
    size_t buildPoints = 0;
    switch (numPastLifeFeats)
    {
        case 0:
            buildPoints = buildPointsVector[m_BuildPoints.HasChampion() ? 1 : 0];
            break;
        case 1:
            buildPoints = buildPointsVector[2];
            break;
        // all other past life counts is the maximum build points
        default:
            buildPoints = buildPointsVector[3];
            break;
    }
    if (m_BuildPoints.AvailableSpend() != buildPoints)
    {
        // add the log entry
        std::stringstream ss;
        ss << "Build points changed from " << m_BuildPoints.AvailableSpend() << " to " << buildPoints;
        GetLog().AddLogEntry(ss.str().c_str());
        // we have had a change in the number of available build points
        m_BuildPoints.SetAvailableSpend(buildPoints);
        //NotifyAvailableBuildPointsChanged();
    }
    return buildPoints;
}

void Build::SetBuildPoints(size_t buildPoints)
{
    const ::Race & race = FindRace(Race()); // ask the race for its build point
    const std::vector<size_t> & buildPointsVector = race.BuildPoints();
    if (buildPoints == buildPointsVector[0])
    {
        m_BuildPoints.ClearChampion();
    }
    else
    {
        m_BuildPoints.SetChampion();
    }
    // add the log entry
    std::stringstream ss;
    ss << "Build points changed from " << m_BuildPoints.AvailableSpend() << " to " << buildPoints;
    GetLog().AddLogEntry(ss.str().c_str());
    m_BuildPoints.SetAvailableSpend(buildPoints);
    SetModifiedFlag(TRUE);
    //NotifyAvailableBuildPointsChanged();
}

size_t Build::GetSpecialFeatTrainedCount(const std::string& featName) const
{
    // this info is stored at the life level, pass through
    return m_pLife->GetSpecialFeatTrainedCount(featName);
}

void Build::TrainSpecialFeat(const std::string& featName)
{
    // this is handled at the life level, pass through for all  but favor feats
    const Feat& feat = FindFeat(featName);
    if (feat.Name() != "Feat not found")
    {
        if (feat.Acquire() == FeatAcquisition_Favor)
        {
            // just add a copy of the feat name to the current list
            std::list<TrainedFeat> trainedFeats = FavorFeats().Feats();
            TrainedFeat tf(featName, (LPCTSTR)EnumEntryText(feat.Acquire(), featAcquisitionMap), 0);
            trainedFeats.push_back(tf);

            FeatsListObject flo(L"FavorFeats", trainedFeats);
            Set_FavorFeats(flo);

            // notify about the feat effects
            ApplyFeatEffects(feat);

            m_pLife->NotifyLifeFeatTrained(featName);
            m_pLife->m_pCharacter->SetModifiedFlag(TRUE);

            // add log entry
            std::stringstream ss;
            ss << "Trained the favor feat \"" << featName.c_str() << "\"";
            GetLog().AddLogEntry(ss.str().c_str());
            SetModifiedFlag(TRUE);
        }
        else
        {
            m_pLife->TrainSpecialFeat(featName);
        }
        if (feat.Acquire() == FeatAcquisition_RacialPastLife
            || feat.Acquire() == FeatAcquisition_HeroicPastLife)
        {
            // either of the completionist feats may now be available
            std::list<TrainedFeat> allFeats = SpecialFeats();
            UpdateFeats(0, &allFeats);      // racial completionist state may have changed
            if (Level() >= 3)
            {
                UpdateFeats(2, &allFeats);      // completionist state may have changed
            }
        }
    }
}

void Build::RevokeSpecialFeat(const std::string& featName, bool bOverride)
{
    // this is handled at the life level, pass through for all but favor feats
    const Feat& feat = FindFeat(featName);
    if (feat.Acquire() == FeatAcquisition_Favor
            || bOverride == true)
    {
        // just remove the first copy of the feat name from the current list
        std::list<TrainedFeat> trainedFeats = FavorFeats().Feats();
        std::list<TrainedFeat>::iterator it = trainedFeats.begin();
        bool found = false;
        while (!found && it != trainedFeats.end())
        {
            if ((*it).FeatName() == featName)
            {
                // this is the first occurrence, remove it
                it = trainedFeats.erase(it);
                found = true;
            }
            else
            {
                ++it;
            }
        }
        if (found)
        {
            FeatsListObject flo(L"FavorFeats", trainedFeats);
            Set_FavorFeats(flo);
            // notify about the feat effects
            RevokeFeatEffects(feat);
            m_pLife->NotifyLifeFeatRevoked(featName);
            // add log entry
            std::stringstream ss;
            ss << "Revoked the favor feat \"" << featName.c_str() << "\"";
            GetLog().AddLogEntry(ss.str().c_str());
            SetModifiedFlag(TRUE);
        }
        else
        {
            // try and remove it as a special feat
            m_pLife->RevokeSpecialFeat(featName);
        }
    }
    else
    {
        m_pLife->RevokeSpecialFeat(featName);
    }
    if (feat.Acquire() == FeatAcquisition_RacialPastLife
        || feat.Acquire() == FeatAcquisition_HeroicPastLife)
    {
        // either of the completionist feats may no longer be available
        std::list<TrainedFeat> allFeats = SpecialFeats();
        UpdateFeats(0, &allFeats);      // racial completionist state may have changed
        if (Level() >= 3)
        {
            UpdateFeats(2, &allFeats);      // completionist state may have changed
        }
    }
}

void Build::Enhancement_ResetEnhancementTree(std::string treeName)
{
    // add the log entry
    std::stringstream ss;
    ss << "Enhancement tree \"" << treeName << "\" reset";
    GetLog().AddLogEntry(ss.str().c_str());
    // a whole tree is being reset
    SpendInTree* pItem = FindSpendInTree(treeName);
    if (pItem != NULL)
    {
        // clear all the enhancements trained by revoking them until none left
        while (pItem != NULL && pItem->Enhancements().size() > 0)
        {
            Enhancement_RevokeEnhancement(treeName, pItem->Enhancements().back().EnhancementName());
            pItem = FindSpendInTree(treeName);
        }
        // now remove the tree entry from the list (not present if no spend in tree)
        std::list<EnhancementSpendInTree>::iterator it = m_EnhancementTreeSpend.begin();
        while (it != m_EnhancementTreeSpend.end())
        {
            if ((*it).TreeName() == treeName)
            {
                // done once we find it
                m_EnhancementTreeSpend.erase(it);
                break;
            }
            ++it;
        }
        NotifyActionPointsChanged();
    }
}

void Build::Enhancement_SetSelectedTrees(const Enhancement_SelectedTrees& trees)
{
    m_EnhancementSelectedTrees = trees;
}

void Build::Enhancement_SetSelectedTrees(const LegacyEnhancementSelectedTrees& trees)
{
    m_EnhancementSelectedTrees.SetTree(0, trees.Tree(0));
    m_EnhancementSelectedTrees.SetTree(1, trees.Tree(1));
    m_EnhancementSelectedTrees.SetTree(2, trees.Tree(2));
    m_EnhancementSelectedTrees.SetTree(3, trees.Tree(3));
    m_EnhancementSelectedTrees.SetTree(4, trees.Tree(4));
    m_EnhancementSelectedTrees.SetTree(5, trees.Tree(5));
    m_EnhancementSelectedTrees.SetTree(6, trees.Tree(6));
    if (trees.HasTier5Tree())
    {
        std::string name = trees.Tier5Tree();
        static std::string nameTranslations[] =
        {
            // old tree name                        new tree name
            "Ravager",                              "Ravager (Barbarian)",
            "Ravager (Ftr)",                        "Ravager (Fighter)",
            "Arch-Mage",                            "Archmage"
        };
        size_t count = sizeof(nameTranslations) / sizeof(std::string);
        if (count % 2 != 0)
        {
            throw "Must be an multiple of 2";
        }
        for (size_t i = 0; i < count; i += 2)
        {
            if (name == nameTranslations[i])
            {
                name = nameTranslations[i + 1];
                break;
            }
        }
        m_EnhancementSelectedTrees.SetTier5Tree(name);
    }
}

void Build::Enhancement_TrainEnhancement(
    const std::string& treeName,
    const std::string& enhancementName,
    const std::string& selection,
    const std::vector<size_t>& cost)
{
    // Find the tree tracking amount spent and enhancements trained
    SpendInTree* pItem = FindSpendInTree(treeName);
    const EnhancementTree& eTree = EnhancementTree::GetTree(treeName);
    if (pItem == NULL)
    {
        // no tree item available for this tree, its a first time spend
        // create an object to handle this tree
        EnhancementSpendInTree tree(treeName, eTree.Version());
        m_EnhancementTreeSpend.push_back(tree);
        pItem = &m_EnhancementTreeSpend.back();
    }
    size_t ranks = 0;
    const EnhancementTreeItem* pTreeItem = FindEnhancement(treeName, enhancementName);
    int spent = pItem->TrainEnhancement(
            enhancementName,
            selection,
            cost,
            pTreeItem->MinSpent(selection),
            pTreeItem->HasTier5(),
            &ranks);
    if (eTree.HasIsRacialTree())
    {
        m_racialTreeSpend += spent;
    }
    else if (eTree.HasIsUniversalTree())
    {
        m_universalTreeSpend += spent;
    }
    else
    {
        m_classTreeSpend += spent;
    }
    // track whether this is a tier 5 enhancement
    ASSERT(pTreeItem != NULL);
    if (m_EnhancementSelectedTrees.HasTier5Tree() && pTreeItem->HasTier5())
    {
        ASSERT(m_EnhancementSelectedTrees.Tier5Tree() == treeName);    // should only be able to train other tier 5's in the same tree
    }
    else
    {
        // no tier 5 enhancement yet selected
        if (pTreeItem->HasTier5())
        {
            // this is a tier 5, lockout all other tree's tier 5 enhancements
            m_EnhancementSelectedTrees.SetTier5Tree(treeName);
        }
    }
    // now notify all and sundry about the enhancement effects
    ApplyEnhancementEffects(treeName, enhancementName, selection, ranks);
    EnhancementItemParams item;
    item.tree = treeName;
    item.enhancementName = enhancementName;
    item.selection = selection;
    item.isTier5 = pTreeItem->HasTier5();
    NotifyEnhancementTrained(item);
    NotifyActionPointsChanged();
    //NotifyAPSpentInTreeChanged(treeName);
    std::stringstream ss;
    ss << "Trained enhancement from tree \""
            << treeName
            << "\": Rank "
            << ranks
            << " of \""
            << pTreeItem->DisplayName(selection)
            << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
}

void Build::Enhancement_RevokeEnhancement(
    const std::string& treeName,
    std::string enhancementName)
{
    SpendInTree* pItem = FindSpendInTree(treeName);
    if (pItem != NULL
            && pItem->Enhancements().size() > 0)
    {
        bool wasTier5 = pItem->HasTier5();
        // return points available to spend also
        std::string revokedEnhancementSelection;
        size_t ranks = 0;
        size_t spent = pItem->RevokeEnhancement(
                enhancementName,
                &revokedEnhancementSelection,
                &ranks);
        const EnhancementTree& eTree = EnhancementTree::GetTree(treeName);
        if (eTree.Name() == "")
        {
            RemoveTreeSpend(treeName);
            // this tree no longer exists at all in any capacity, just delete the entry
            std::stringstream ss;
            ss << "Enhancement tree \"" << treeName << "\" not found. Removed from build.";
            GetLog().AddLogEntry(ss.str().c_str());
        }
        else
        {
            if (eTree.HasIsRacialTree())
            {
                m_racialTreeSpend -= spent;
            }
            else
            {
                if (eTree.HasIsUniversalTree())
                {
                    m_universalTreeSpend -= spent;
                }
                else
                {
                    m_classTreeSpend -= spent;
                }
            }
            // now notify all and sundry about the enhancement effects
            // get the list of effects this enhancement has
            RevokeEnhancementEffects(treeName, enhancementName, revokedEnhancementSelection, ranks);
            // determine whether we still have a tier 5 enhancement trained if the tree just had one
            // revoked in it
            if (m_EnhancementSelectedTrees.HasTier5Tree()
                    && m_EnhancementSelectedTrees.Tier5Tree() == treeName)
            {
                // may have lost the tier 5 status, check the tree to see if any tier 5 are still trained
                if (!pItem->HasTier5())
                {
                    m_EnhancementSelectedTrees.ClearTier5Tree();  // no longer a tier 5 trained
                }
            }
            EnhancementItemParams item;
            item.tree = treeName;
            item.enhancementName = enhancementName;
            item.selection = revokedEnhancementSelection;
            item.isTier5 = wasTier5;
            NotifyEnhancementRevoked(item);
            NotifyActionPointsChanged();
            std::stringstream ss;
            const EnhancementTreeItem* pTreeItem = FindEnhancement(treeName, enhancementName);
            if (pTreeItem != NULL)
            {
                ss << "Revoked enhancement from tree \""
                    << treeName
                    << "\": Rank "
                    << ranks
                    << " of \""
                    << pTreeItem->DisplayName(revokedEnhancementSelection)
                    << "\"";
            }
            else
            {
                ss << "Revoked enhancement from tree \""
                    << treeName
                    << "\": Rank "
                    << ranks
                    << " of \"Unknown\"";
            }
            GetLog().AddLogEntry(ss.str().c_str());
        }
    }
}

bool Build::IsEnhancementTrained(
        const std::string& enhancementName,
        const std::string& selection,
        TreeType type) const
{
    UNREFERENCED_PARAMETER(type);
    // return true if this enhancement and selection is trained
    bool isTrained = (IsTrained(enhancementName, selection) != NULL);
    return isTrained;
}

bool Build::IsExclusiveEnhancement(
        const std::string& enhancementId,
        const std::string& group) const
{
    // look through the list of ExclusionGroup objects to see if this exists
    // returns true if it matches or is not found
    bool found = false;
    bool isUs = false;
    for (auto&& it : m_exclusiveEnhancements)
    {
        found |= (it.Name() == group);
        if (it.EnhancementId() == enhancementId
                && it.Name() == group)
        {
            // this is us
            isUs = true;
        }
    }
    return isUs || !found;
}

void Build::ApplyEnhancementEffects(
        const std::string& treeName,
        const std::string& enhancementName,
        const std::string& selection,
        size_t rank)
{
    const EnhancementTreeItem* pItem = FindEnhancement(treeName, enhancementName);
    if (pItem != NULL)
    {
        std::list<Effect> effects = pItem->GetEffects(selection, rank);
        std::string name = enhancementName;
        if (selection != "")
        {
            name = selection;
        }
        for (auto&& eit : effects)
        {
            for (auto&& tit : eit.Type())
            {
                Effect copy = eit;
                copy.SetType(tit);
                if (copy.IsType(Effect_AddGroupWeapon))
                {
                    AddWeaponToGroup(copy);
                }
                if (copy.IsType(Effect_MergeGroups))
                {
                    MergeGroups(copy);
                }
                NotifyEnhancementEffectApplied(copy);
                // handle exclusive enhancement items (regular enhancements only)
                if (copy.IsType(Effect_ExclusionGroup))
                {
                    if (IsExclusiveEnhancement(copy.Item().front(), copy.Item().back()))
                    {
                        // need to add a stack
                        auto it = m_exclusiveEnhancements.begin();
                        bool found = false;
                        while (it != m_exclusiveEnhancements.end())
                        {
                            if ((*it).Name() == copy.Item().back())
                            {
                                (*it).AddStack();
                                found = true;
                            }
                            it++;
                        }
                        if (!found)
                        {
                            // group does not currently exist. add it
                            ExclusionGroup eg(copy.Item().back(), copy.Item().front());
                            eg.AddStack();
                            m_exclusiveEnhancements.push_back(eg);
                        }
                    }
                }
            }
        }

        // apply any stances
        std::list<Stance> stances = pItem->GetStances(selection, rank);
        for (auto&& sit : stances)
        {
            NotifyNewStance(sit);
        }

        std::list<DC> dcs = pItem->GetDCs(selection);
        for (auto&& dcit: dcs)
        {
            NotifyNewDC(dcit);
        }
    }
}

void Build::RevokeEnhancementEffects(
        const std::string& treeName,
        const std::string& enhancementName,
        const std::string& selection,
        size_t rank)
{
    bool bVerifyGear = false;
    const EnhancementTreeItem* pItem = FindEnhancement(treeName, enhancementName);
    if (pItem != NULL)
    {
        std::list<Effect> effects = pItem->GetEffects(selection, rank);
        for (auto&& eit : effects)
        {
            for (auto&& tit : eit.Type())
            {
                Effect copy = eit;
                copy.SetType(tit);
                if (copy.IsType(Effect_GrantFeat))
                {
                    bVerifyGear = true;
                }
                if (copy.IsType(Effect_AddGroupWeapon))
                {
                    RemoveFromWeaponGroup(copy);
                }
                if (copy.IsType(Effect_MergeGroups))
                {
                    RemoveMergeGroup(copy);
                }
                NotifyEnhancementEffectRevoked(copy);
                // handle exclusive enhancement items (regular enhancements only)
                if (copy.IsType(Effect_ExclusionGroup))
                {
                    if (IsExclusiveEnhancement(copy.Item().front(), copy.Item().back()))
                    {
                        // need to remove a stack
                        auto it = m_exclusiveEnhancements.begin();
                        while (it != m_exclusiveEnhancements.end())
                        {
                            if ((*it).Name() == copy.Item().back())
                            {
                                bool last = (*it).RevokeStack();
                                if (last)
                                {
                                    it = m_exclusiveEnhancements.erase(it);
                                }
                                break;
                            }
                            it++;
                        }
                    }
                }
            }
        }

        // revoke any stances
        std::list<Stance> stances = pItem->GetStances(selection, rank);
        for (auto&& sit : stances)
        {
            NotifyRevokeStance(sit);
        }

        std::list<DC> dcs = pItem->GetDCs(selection);
        for (auto&& dcit: dcs)
        {
            NotifyRevokeDC(dcit);
        }
    }
    if (bVerifyGear)
    {
        VerifyGear();
    }
}

void Build::ActivateStance(
    const Stance& stance,
    StanceGroup* pStanceGroup)
{
    // first activate the stance if not already active
    if (!IsStanceActive(stance.Name()))
    {
        // add the log entry
        std::stringstream ss;
        ss << "Stance \"" << stance.Name() << "\" activated";
        GetLog().AddLogEntry(ss.str().c_str());

        m_Stances.AddActiveStance(stance.Name());
        NotifyStanceActivated(stance.Name());
        // now revoke any stances that cannot be active at the same time as this stance
        // these stances are in the same stance group which is not "User"
        if (pStanceGroup->GroupName() != "User"
                && pStanceGroup->GroupName() != "Auto")
        {
            pStanceGroup->DeactivateOtherStancesExcept(stance.Name(), this);
        }

        //const std::list<std::string>& incompatibles = stance.IncompatibleStance();
        //std::list<std::string>::const_iterator isit = incompatibles.begin();
        //while (isit != incompatibles.end())
        //{
            //m_Stances.RevokeStance((*isit));
            //NotifyStanceDeactivated((*isit));
            //++isit;
        //}
    }
}

void Build::DeactivateStance(
    const Stance& stance,
    StanceGroup* pStanceGroup)
{
    UNREFERENCED_PARAMETER(pStanceGroup);
    // deactivate the stance if active
    if (IsStanceActive(stance.Name()))
    {
        // add the log entry
        std::stringstream ss;
        ss << "Stance \"" << stance.Name() << "\" deactivated";
        GetLog().AddLogEntry(ss.str().c_str());
        // deactivation of a stance only affects that stance
        m_Stances.RevokeStance(stance.Name());
        NotifyStanceDeactivated(stance.Name());
    }
    NotifyStanceDisabled(stance.Name());
}

void Build::DisableStance(
    const Stance& stance,
    StanceGroup* pStanceGroup)
{
    UNREFERENCED_PARAMETER(pStanceGroup);
    // deactivate the stance if active
    if (IsStanceActive(stance.Name()))
    {
        // add the log entry
        std::stringstream ss;
        ss << "Stance \"" << stance.Name() << "\" deactivated";
        GetLog().AddLogEntry(ss.str().c_str());
        // deactivation of a stance only affects that stance
        m_Stances.RevokeStance(stance.Name());
        NotifyStanceDeactivated(stance.Name());
    }
}

bool Build::IsStanceActive(const std::string& name, WeaponType wt) const
{
    bool ret = false;
    // some special stances are based on a slider position
    // all these stances start with a numeric with the format of
    // "<number>% <stance name>"
    CWnd* pWnd = AfxGetMainWnd();
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    const CStancesPane* pStancesPane = dynamic_cast<const CStancesPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CStancesPane)));
    if (name.find("%") != std::string::npos)
    {
        ret = pStancesPane->IsStanceActive(name, wt);
    }
    else
    {
        ret = m_Stances.IsStanceActive(name);
        ret |= pStancesPane->IsStanceActive(name, wt);
    }
    if (name == "FavoredWeapon")
    {
        ret = m_Stances.IsStanceActive("Favored Weapon");   // check the weapon group
        // must also have at least 10 favored soul levels for this to apply
        // for Grace/Knowledge of battle
        //??
        //if (ClassLevels(Class_FavoredSoul) < 10)
        //{
        //    // not enough heroic levels for this to apply
        //    ret = false;
        //}
    }
    return ret;
}

void Build::StanceSliderChanged(const std::string& sliderName, int value)
{
    NotifySliderChanged(sliderName, value);
}

void Build::Destiny_SetSelectedTrees(const Destiny_SelectedTrees& trees)
{
    m_DestinySelectedTrees = trees;
}

void Build::Destiny_SetSelectedTrees(const LegacyDestinySelectedTrees& trees)
{
    m_DestinySelectedTrees.SetTree(0, trees.Tree(0));
    m_DestinySelectedTrees.SetTree(1, trees.Tree(1));
    m_DestinySelectedTrees.SetTree(2, trees.Tree(2));
    m_DestinySelectedTrees.SetTree(3, trees.Tree(3));
    if (trees.HasTier5Tree())
    {
        m_DestinySelectedTrees.SetTier5Tree(trees.Tier5Tree());
    }
}

void Build::Destiny_TrainEnhancement(
        const std::string& treeName,
        const std::string& enhancementName,
        const std::string& selection,
        const std::vector<size_t>& cost)
{
    // Find the tree tracking amount spent and enhancements trained
    SpendInTree * pItem = FindSpendInTree(treeName);
    if (pItem == NULL)
    {
        // no tree item available for this tree, its a first time spend
        // create an object to handle this tree
        const EnhancementTree & eTree = GetEnhancementTree(treeName);
        DestinySpendInTree tree;
        tree.SetTree(treeName, eTree.Version());  // need to track version of tree spent in
        m_DestinyTreeSpend.push_back(tree);
        pItem = &m_DestinyTreeSpend.back();
    }
    size_t ranks = 0;
    const EnhancementTreeItem* pTreeItem = FindEnhancement(treeName, enhancementName);
    int spent = pItem->TrainEnhancement(
            enhancementName,
            selection,
            cost,
            pTreeItem->MinSpent(selection),
            pTreeItem->HasTier5(),
            &ranks);
    m_destinyTreeSpend += spent;
    // track whether this is a tier 5 enhancement
    ASSERT(pTreeItem != NULL);
    if (m_DestinySelectedTrees.HasTier5Tree() && pTreeItem->HasTier5())
    {
        ASSERT(m_DestinySelectedTrees.Tier5Tree() == treeName);    // should only be able to train other tier 5's in the same tree
    }
    else
    {
        // no tier 5 enhancement yet selected
        if (pTreeItem->HasTier5())
        {
            // this is a tier 5, lockout all other tree's tier 5 enhancements
            m_DestinySelectedTrees.SetTier5Tree(treeName);
        }
    }
    // now notify all and sundry about the enhancement effects
    ApplyEnhancementEffects(treeName, enhancementName, selection, ranks);
    EnhancementItemParams p;
    p.enhancementName = enhancementName;
    p.selection = selection;
    p.isTier5 = pTreeItem->HasTier5();
    NotifyEnhancementTrained(p);

    //??NotifyAPSpentInTreeChanged(treeName);
    SetModifiedFlag(TRUE);
    std::stringstream ss;
    ss << "Trained destiny enhancement from tree \""
            << treeName
            << "\": Rank "
            << ranks
            << " of \""
            << pTreeItem->DisplayName(selection)
            << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
}

void Build::Destiny_RevokeEnhancement(
        const std::string& treeName,
        std::string enhancementName)
{
    SpendInTree * pItem = FindSpendInTree(treeName);
    if (pItem != NULL
            && pItem->Enhancements().size() > 0)
    {
        bool wasTier5 = pItem->HasTier5();
        // return points available to spend also
        std::string revokedEnhancementSelection;
        size_t ranks = 0;
        int spent = pItem->RevokeEnhancement(
                enhancementName,
                &revokedEnhancementSelection,
                &ranks);
        m_destinyTreeSpend -= spent;

        // determine whether we still have a tier 5 enhancement trained if the tree just had one
        // revoked in it
        if (m_DestinySelectedTrees.HasTier5Tree()
            && m_DestinySelectedTrees.Tier5Tree() == treeName)
        {
            // may have lost the tier 5 status, check the tree to see if any tier 5 are still trained
            if (!pItem->HasTier5())
            {
                m_DestinySelectedTrees.ClearTier5Tree();  // no longer a tier 5 trained
            }
        }

        // now notify all and sundry about the enhancement effects
        RevokeEnhancementEffects(treeName, enhancementName, revokedEnhancementSelection, ranks);
        EnhancementItemParams p;
        p.enhancementName = enhancementName;
        p.selection = revokedEnhancementSelection;
        p.isTier5 = wasTier5;
        NotifyEnhancementRevoked(p);

        const EnhancementTreeItem * pTreeItem = FindEnhancement(enhancementName);
        //??NotifyAPSpentInTreeChanged(treeName);
        SetModifiedFlag(TRUE);
        std::stringstream ss;
        ss << "Revoked destiny enhancement from tree \""
                << treeName
                << "\": Rank "
                << ranks
                << " of \""
                << pTreeItem->DisplayName(revokedEnhancementSelection)
                << "\"";
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

void Build::Destiny_ResetEnhancementTree(std::string treeName)
{
    // add the log entry
    std::stringstream ss;
    ss << "Destiny tree \"" << treeName << "\" reset";
    GetLog().AddLogEntry(ss.str().c_str());

    // a whole tree is being reset
    SpendInTree * pItem = FindSpendInTree(treeName);
    if (pItem != NULL)
    {
        // clear all the enhancements trained by revoking them until none left
        while (pItem->Enhancements().size() > 0)
        {
            Destiny_RevokeEnhancement(treeName, pItem->Enhancements().back().EnhancementName());
            pItem = FindSpendInTree(treeName);
        }
        // now remove the tree entry from the list (not present if no spend in tree)
        std::list<DestinySpendInTree>::iterator it = m_DestinyTreeSpend.begin();
        while (it != m_DestinyTreeSpend.end())
        {
            if ((*it).TreeName() == treeName)
            {
                // done once we find it
                m_DestinyTreeSpend.erase(it);
                break;
            }
            ++it;
        }
        //??NotifyEnhancementTreeReset();
        //??NotifyAPSpentInTreeChanged(treeName);
        SetModifiedFlag(TRUE);
    }
}

void Build::Reaper_TrainEnhancement(
        const std::string& treeName,
        const std::string& enhancementName,
        const std::string& selection,
        const std::vector<size_t>& cost)
{
    // Find the tree tracking amount spent and enhancements trained
    SpendInTree * pItem = FindSpendInTree(treeName);
    const EnhancementTree & eTree = GetEnhancementTree(treeName);
    if (pItem == NULL)
    {
        // no tree item available for this tree, its a first time spend
        // create an object to handle this tree
        ReaperSpendInTree tree(treeName, eTree.Version());
        m_ReaperTreeSpend.push_back(tree);
        pItem = &m_ReaperTreeSpend.back();
    }
    size_t ranks = 0;
    const EnhancementTreeItem* pTreeItem = FindEnhancement(treeName, enhancementName);
    pItem->TrainEnhancement(
            enhancementName,
            selection,
            cost,
            pTreeItem->MinSpent(selection),
            pTreeItem->HasTier5(),
            &ranks);
    // now notify all and sundry about the enhancement effects
    ApplyEnhancementEffects(treeName, enhancementName, selection, ranks);
    EnhancementItemParams item;
    item.tree = treeName;
    item.enhancementName = enhancementName;
    item.selection = selection;
    item.isTier5 = pTreeItem->HasTier5();
    NotifyEnhancementTrained(item);
    //NotifyAPSpentInTreeChanged(treeName);
    SetModifiedFlag(TRUE);
    std::stringstream ss;
    ss << "Trained Reaper enhancement from tree \""
            << treeName
            << "\": Rank "
            << ranks
            << " of \""
            << pTreeItem->DisplayName(selection)
            << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
}

void Build::Reaper_RevokeEnhancement(
        const std::string& treeName,
        std::string enhancementName)
{
    SpendInTree * pItem = FindSpendInTree(treeName);
    if (pItem != NULL
            && pItem->Enhancements().size() > 0)
    {
        // return points available to spend also
        std::string revokedEnhancementSelection;
        size_t ranks = 0;
        pItem->RevokeEnhancement(
                enhancementName,
                &revokedEnhancementSelection,
                &ranks);
        // now notify all and sundry about the enhancement effects
        // get the list of effects this enhancement has
        RevokeEnhancementEffects(treeName, enhancementName, revokedEnhancementSelection, ranks);
        EnhancementItemParams item;
        item.tree = treeName;
        item.enhancementName = enhancementName;
        item.selection = revokedEnhancementSelection;
        item.isTier5 = false;
        NotifyEnhancementRevoked(item);
        NotifyActionPointsChanged();
        //NotifyAPSpentInTreeChanged(treeName);
        SetModifiedFlag(TRUE);
        const EnhancementTreeItem* pTreeItem = FindEnhancement(treeName, enhancementName);
        std::stringstream ss;
        ss << "Revoked Reaper enhancement from tree \""
                << treeName
                << "\": Rank "
                << ranks
                << " of \""
                << pTreeItem->DisplayName(revokedEnhancementSelection)
                << "\"";
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

void Build::Reaper_ResetEnhancementTree(std::string treeName)
{
    // add the log entry
    std::stringstream ss;
    ss << "Reaper Enhancement tree \"" << treeName << "\" reset";
    GetLog().AddLogEntry(ss.str().c_str());
    // a whole tree is being reset
    SpendInTree * pItem = FindSpendInTree(treeName);
    if (pItem != NULL)
    {
        // clear all the enhancements trained by revoking them until none left
        while (pItem->Enhancements().size() > 0)
        {
            Reaper_RevokeEnhancement(treeName, pItem->Enhancements().back().EnhancementName());
            pItem = FindSpendInTree(treeName);
        }
        // now remove the tree entry from the list (not present if no spend in tree)
        std::list<ReaperSpendInTree>::iterator it = m_ReaperTreeSpend.begin();
        while (it != m_ReaperTreeSpend.end())
        {
            if ((*it).TreeName() == treeName)
            {
                // done once we find it
                m_ReaperTreeSpend.erase(it);
                break;
            }
            ++it;
        }
        NotifyActionPointsChanged();
    }
}

// gear support
void Build::AddGearSet(const EquippedGear & gear)
{
    m_bSwitchingBuildsOrGear = true;
    // add the new gear set to the end of the list
    m_GearSetups.push_back(gear);
    SetModifiedFlag(TRUE);
    std::stringstream ss;
    ss << "Gear set \"" << gear.Name() << "\" created.";
    GetLog().AddLogEntry(ss.str().c_str());
    m_bSwitchingBuildsOrGear = false;
    UpdateTotalChanged(NULL, Breakdown_Strength);       // ensure snapshots are current
    UpdateTotalChanged(NULL, Breakdown_Intelligence);
    UpdateTotalChanged(NULL, Breakdown_Wisdom);
    UpdateTotalChanged(NULL, Breakdown_Dexterity);
    UpdateTotalChanged(NULL, Breakdown_Constitution);
    UpdateTotalChanged(NULL, Breakdown_Charisma);
}

void Build::DeleteGearSet(const std::string& name)
{
    m_bSwitchingBuildsOrGear = true;
    RevokeGearEffects();        // always for active gear (one being deleted)
    std::stringstream ss;
    ss << "Gear set \"" << name << "\" deleted.";
    // find the gear set and delete it
    std::list<EquippedGear>::const_iterator it = m_GearSetups.begin();
    while (it != m_GearSetups.end())
    {
        if ((*it).Name() == name)
        {
            m_GearSetups.erase(it);
            break;  // were done
        }
        ++it;
    }
    // as we just deleted the active gear set, switch to the first gear set remaining (if any)
    if (m_GearSetups.size() > 0)
    {
        it = m_GearSetups.begin();
        Set_ActiveGear((*it).Name());
        ss << "\r\n    New active gear set is \"" << (*it).Name() << "\"";
    }
    else
    {
        // no gear sets, no name
        Set_ActiveGear("");
        ss << "\r\n    There are no other gear sets in this build";
    }
    ApplyGearEffects();         // always for active gear
    NotifyGearChanged(Inventory_Weapon1);   // updates both in breakdowns
    m_bSwitchingBuildsOrGear = false;
    UpdateTotalChanged(NULL, Breakdown_Strength);       // ensure snapshots are current
    UpdateTotalChanged(NULL, Breakdown_Intelligence);
    UpdateTotalChanged(NULL, Breakdown_Wisdom);
    UpdateTotalChanged(NULL, Breakdown_Dexterity);
    UpdateTotalChanged(NULL, Breakdown_Constitution);
    UpdateTotalChanged(NULL, Breakdown_Charisma);
    SetModifiedFlag(TRUE);
    GetLog().AddLogEntry(ss.str().c_str());
}

bool Build::DoesGearSetExist(const std::string& name) const
{
    std::list<EquippedGear>::const_iterator it = m_GearSetups.begin();
    bool found = false;
    while (!found && it != m_GearSetups.end())
    {
        if ((*it).Name() == name)
        {
            found = true;
        }
        ++it;
    }
    return found;
}

EquippedGear Build::GetGearSet(const std::string& name) const
{
    EquippedGear gear;
    std::list<EquippedGear>::const_iterator it = m_GearSetups.begin();
    bool found = false;
    while (!found && it != m_GearSetups.end())
    {
        if ((*it).Name() == name)
        {
            gear = (*it);
            found = true;
        }
        ++it;
    }
    return gear;
}

void Build::SetActiveGearSet(const std::string& name)
{
    m_bSwitchingBuildsOrGear = true;
    RevokeGearEffects();        // always for active gear
    Set_ActiveGear(name);
    ApplyGearEffects();         // always for active gear
    NotifyGearChanged(Inventory_Weapon1);   // updates both in breakdowns
    SetModifiedFlag(TRUE);
    m_bSwitchingBuildsOrGear = false;
    UpdateTotalChanged(NULL, Breakdown_Strength);       // ensure snapshots are current
    UpdateTotalChanged(NULL, Breakdown_Intelligence);
    UpdateTotalChanged(NULL, Breakdown_Wisdom);
    UpdateTotalChanged(NULL, Breakdown_Dexterity);
    UpdateTotalChanged(NULL, Breakdown_Constitution);
    UpdateTotalChanged(NULL, Breakdown_Charisma);
}

EquippedGear Build::ActiveGearSet() const
{
    return GetGearSet(ActiveGear());
}

void Build::SetNumFiligrees(size_t count)
{
    m_bSwitchingBuildsOrGear = true;
    // first revoke all gear effects as the gear is about to change
    RevokeGearEffects();        // always for active gear
    // update the gear
    std::list<EquippedGear>::iterator it = m_GearSetups.begin();
    bool found = false;
    while (!found && it != m_GearSetups.end())
    {
        if ((*it).Name() == ActiveGear())
        {
            (*it).SetNumFiligrees(count);
            found = true;
        }
        ++it;
    }
    // now apply the gear effects if its the active gear set
    ApplyGearEffects();         // always for active gear
    SetModifiedFlag(TRUE);
    m_bSwitchingBuildsOrGear = false;
    UpdateTotalChanged(NULL, Breakdown_Strength);       // ensure snapshots are current
    UpdateTotalChanged(NULL, Breakdown_Intelligence);
    UpdateTotalChanged(NULL, Breakdown_Wisdom);
    UpdateTotalChanged(NULL, Breakdown_Dexterity);
    UpdateTotalChanged(NULL, Breakdown_Constitution);
    UpdateTotalChanged(NULL, Breakdown_Charisma);
}

void Build::SetGearSetSnapshot(const std::string& setName)
{
    std::string old = GearSetSnapshot();
    Set_GearSetSnapshot(setName);
    std::stringstream ss;
    ss << "Gear Set Snapshot changed from \"" << old << "\" to \"" <<  setName.c_str() << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
    SetModifiedFlag(TRUE);
    NotifyAbilityValueChanged(Ability_Strength);
    NotifyAbilityValueChanged(Ability_Intelligence);
    NotifyAbilityValueChanged(Ability_Wisdom);
    NotifyAbilityValueChanged(Ability_Dexterity);
    NotifyAbilityValueChanged(Ability_Constitution);
    NotifyAbilityValueChanged(Ability_Charisma);
}

int Build::SnapshotAbilityValue(AbilityType at) const
{
    int total = 0;
    if (HasGearSetSnapshot())
    {
        EquippedGear gear = GetGearSet(GearSetSnapshot());
        switch (at)
        {
            case Ability_Strength:      total = gear.SnapshotStrength(); break;
            case Ability_Intelligence:  total = gear.SnapshotIntelligence(); break;
            case Ability_Wisdom:        total = gear.SnapshotWisdom(); break;
            case Ability_Dexterity:     total = gear.SnapshotDexterity(); break;
            case Ability_Constitution:  total = gear.SnapshotConstitution(); break;
            case Ability_Charisma:      total = gear.SnapshotCharisma(); break;
        }
    }
    else
    {
        // if we do not have a snapshot, we fall back to the current breakdown total
        BreakdownType bt = StatToBreakdown(at);
        BreakdownItem* pBI = FindBreakdown(bt);
        if (pBI != NULL)
        {
            total = static_cast<int>(pBI->Total());
        }
    }
    return total;
}

void Build::UpdateGearToLatestVersions()
{
    // needs to be done for all gear sets and all items
    std::list<EquippedGear>::iterator it = m_GearSetups.begin();
    while (it != m_GearSetups.end())
    {
        // update all the items
        for (size_t i = Inventory_Unknown + 1; i < Inventory_Count; ++i)
        {
            if ((*it).HasItemInSlot((InventorySlotType)i))
            {
                Item latestVersion = GetLatestVersionOfItem((InventorySlotType)i, (*it).ItemInSlot((InventorySlotType)i));
                (*it).SetItem((InventorySlotType)i, this, latestVersion);
            }
        }
        ++it;
    }
}

Item Build::GetLatestVersionOfItem(InventorySlotType slot, Item original)
{
    AddSpecialSlots(slot, original); // only adds if they are missing
    Item newVersion = original; // assume unchanged
    const Item& foundItem = FindItem(original.Name());
    // no name if not found
    if (foundItem.Name() == original.Name())
    {
        int originalLevel = original.MinLevel();
        // this is the item we want to copy
        newVersion = foundItem;
        AddSpecialSlots(slot, newVersion); // only adds if they are missing
        // now copy across specific fields from the source item
        newVersion.CopyUserSetValues(original);
        if (original.HasUserSetsLevel())
        {
            // retain item level if its cannith crafted
            newVersion.Set_MinLevel(originalLevel);
        }
        // make sure all the selected augments are valid
        std::vector<ItemAugment> originalAugments = original.Augments();
        std::vector<ItemAugment> newAugments = newVersion.Augments();
        std::vector<SlotUpgrade> newUpgrades = foundItem.SlotUpgrades();
        for (size_t i = 0; i < originalAugments.size(); ++i)
        {
            bool bFound = false;
            if (originalAugments[i].HasSelectedAugment()
                    && originalAugments[i].SelectedAugment() != "")
            {
                // find it in the newAugments
                for (size_t j = 0; !bFound && j < newAugments.size(); ++j)
                {
                    // match by augment type. If not matched, its ignored
                    if (newAugments[j].Type() == originalAugments[i].Type())
                    {
                        newAugments[j].Set_SelectedAugment(originalAugments[i].SelectedAugment());
                        if (originalAugments[i].HasSelectedLevelIndex())
                        {
                            newAugments[j].Set_SelectedLevelIndex(originalAugments[i].SelectedLevelIndex());
                        }
                        if (newAugments[j].Type().find("Cannith") != std::string::npos)
                        {
                            newAugments[j].Set_SelectedLevelIndex(newVersion.MinLevel());
                        }
                        bFound = true;
                    }
                }
            }
            if (!bFound)
            {
                // we have an augment (populated or not) that not in the regular
                // augment list. This could be a SlotUpgrade augment.
                for (size_t j = 0; !bFound && j < newUpgrades.size(); ++j)
                {
                    if (newUpgrades[j].HasSlotType(originalAugments[i].Type()))
                    {
                        AddAugment(&newAugments, originalAugments[i].Type(), false);
                        newUpgrades.erase(newUpgrades.begin()+j);
                        j--;
                        for (size_t k = 0; !bFound && k < newAugments.size(); ++k)
                        {
                            // match by augment type. If not matched, its ignored
                            if (newAugments[k].Type() == originalAugments[i].Type())
                            {
                                newAugments[k].Set_SelectedAugment(originalAugments[i].SelectedAugment());
                                if (originalAugments[i].HasSelectedLevelIndex())
                                {
                                    newAugments[k].Set_SelectedLevelIndex(originalAugments[i].SelectedLevelIndex());
                                }
                                bFound = true;
                            }
                        }
                    }
                }
            }
        }
        newVersion.Set_Augments(newAugments);
        newVersion.Set_SlotUpgrades(newUpgrades);
    }
    return newVersion;
}

Item Build::GetLatestVersionOfItem(InventorySlotType slot, LegacyItem original)
{
    Item foundItem = FindItem(original.Name());
    // no name if not found
    if (foundItem.Name() == original.Name())
    {
        // this is the item we want to copy
        AddSpecialSlots(slot, foundItem); // only adds if they are missing
        // make sure all the selected augments are valid
        std::vector<ItemAugment> originalAugments = original.Augments();
        std::vector<ItemAugment> newAugments = foundItem.Augments();
        std::vector<SlotUpgrade> newUpgrades = foundItem.SlotUpgrades();
        for (size_t i = 0; i < originalAugments.size(); ++i)
        {
            bool bFound = false;
            if (originalAugments[i].HasSelectedAugment()
                && originalAugments[i].SelectedAugment() != "")
            {
                // find it in the newAugments
                for (size_t j = 0; !bFound && j < newAugments.size(); ++j)
                {
                    // match by augment type. If not matched, its ignored
                    if (newAugments[j].Type() == originalAugments[i].Type()
                            || originalAugments[i].Type().find(newAugments[j].Type()) != std::string::npos)
                    {
                        newAugments[j].Set_SelectedAugment(originalAugments[i].SelectedAugment());
                        // find the nearest level from the list available
                        size_t levelIndex = 0;
                        const Augment& aug = FindAugmentByName(originalAugments[i].SelectedAugment(), NULL);
                        if (aug.HasChooseLevel())
                        {
                            bool bFoundIndex = false;
                            if (aug.HasLevels())
                            {
                                if (aug.HasLevelValue())
                                {
                                    // switch to the index that matches the value
                                    const std::vector<double>& levelValue = aug.LevelValue();
                                    for (size_t index = 0; index < levelValue.size(); ++index)
                                    {
                                        if (levelValue[index] == originalAugments[i].Value())
                                        {
                                            levelIndex = index;
                                            bFoundIndex = true;
                                        }
                                    }
                                }
                                if (!bFoundIndex)
                                {
                                    // switch to the index thats closest to this items actual level or lower
                                    const std::vector<int>& levels = aug.Levels();
                                    for (size_t index = 0; index < levels.size(); ++index)
                                    {
                                        if (levels[index] <= (int)foundItem.MinLevel())
                                        {
                                            levelIndex = index;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                // its a cannith type augment, lookup by item level
                                levelIndex = foundItem.MinLevel();
                            }
                            newAugments[j].Set_SelectedLevelIndex(levelIndex);
                        }
                        bFound = true;
                    }
                }
            }
            if (!bFound)
            {
                // we have an augment (populated or not) thats not in the regular
                // augment list. This could be a SlotUpgrade augment.
                for (size_t j = 0; !bFound && j < newUpgrades.size(); ++j)
                {
                    if (newUpgrades[j].HasSlotType(originalAugments[i].Type()))
                    {
                        AddAugment(&newAugments, originalAugments[i].Type(), false);
                        newUpgrades.erase(newUpgrades.begin() + j);
                        j--;
                        if (originalAugments[i].HasSelectedAugment())
                        {
                            for (size_t k = 0; !bFound && k < newAugments.size(); ++k)
                            {
                                // match by augment type. If not matched, its ignored
                                if (newAugments[k].Type() == originalAugments[i].Type())
                                {
                                    newAugments[k].Set_SelectedAugment(originalAugments[i].SelectedAugment());
                                    // find the nearest level from the list available
                                    size_t levelIndex = 0;
                                    const Augment& aug = FindAugmentByName(originalAugments[i].SelectedAugment(), NULL);
                                    if (aug.HasChooseLevel())
                                    {
                                        if (aug.HasLevels())
                                        {
                                            // switch to the index thats closest to this items actual level or lower
                                            const std::vector<int>& levels = aug.Levels();
                                            for (size_t index = 0; index < levels.size(); ++index)
                                            {
                                                if (levels[index] <= (int)foundItem.MinLevel())
                                                {
                                                    levelIndex = index;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // its a cannith type augment, lookup by item level
                                            levelIndex = foundItem.MinLevel();
                                        }
                                        newAugments[k].Set_SelectedLevelIndex(levelIndex);
                                    }
                                    bFound = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        foundItem.Set_Augments(newAugments);
        foundItem.Set_SlotUpgrades(newUpgrades);
    }
    return foundItem;
}

void Build::UpdateActiveGearSet(const EquippedGear& newGear)
{
    m_bSwitchingBuildsOrGear = true;
    // first revoke all gear effects as the gear is about to change
    RevokeGearEffects();        // always for active gear
    // update the gear
    std::list<EquippedGear>::iterator it = m_GearSetups.begin();
    bool found = false;
    while (!found && it != m_GearSetups.end())
    {
        if ((*it).Name() == ActiveGear())
        {
            (*it) = newGear;
            found = true;
        }
        ++it;
    }
    // now apply the gear effects if its the active gear set
    ApplyGearEffects();         // always for active gear
    SetModifiedFlag(TRUE);
    m_bSwitchingBuildsOrGear = false;
    UpdateTotalChanged(NULL, Breakdown_Strength);       // ensure snapshots are current
    UpdateTotalChanged(NULL, Breakdown_Intelligence);
    UpdateTotalChanged(NULL, Breakdown_Wisdom);
    UpdateTotalChanged(NULL, Breakdown_Dexterity);
    UpdateTotalChanged(NULL, Breakdown_Constitution);
    UpdateTotalChanged(NULL, Breakdown_Charisma);
}

void Build::SetGear(
        const std::string& name,
        InventorySlotType slot,
        const Item & item)
{
    if (name == ActiveGear())
    {
        // revoke specific equipped item only
        if (ActiveGearSet().HasItemInSlot(slot))
        {
            const Item& oldItem = ActiveGearSet().ItemInSlot(slot);
            RevokeItem(oldItem, slot);
        }
    }
    // update the gear entry
    std::list<EquippedGear>::iterator it = m_GearSetups.begin();
    bool found = false;
    while (!found && it != m_GearSetups.end())
    {
        if ((*it).Name() == name)
        {
            std::list<Item> revokedItems = (*it).SetItem(slot, this, item);
            found = true;
            // clear any items from restricted gear slots if required
            if (item.HasRestrictedSlots())
            {
                for (size_t i = Inventory_Unknown + 1; i < Inventory_Count; ++i)
                {
                    if (item.RestrictedSlots().HasSlot((InventorySlotType)i))
                    {
                        if (name == ActiveGear())
                        {
                            revokedItems.push_back(ActiveGearSet().ItemInSlot((InventorySlotType)i));
                        }
                        (*it).ClearItem((InventorySlotType)i);
                    }
                }
            }
            for (auto&& riit : revokedItems)
            {
                InventorySlotType ist = Inventory_Unknown;
                if (riit.HasArmor()) ist = Inventory_Armor;
                else if (riit.HasWeapon()) ist = Inventory_Weapon1;

                std::stringstream ss;
                ss << "Item revoked as slot restricted: \""
                    << riit.Name() << "\"\r\n";
                GetLog().AddLogEntry(ss.str().c_str());
                RevokeItem(riit, ist);
            }
        }
        ++it;
    }
    ASSERT(found);

    // now apply the gear effects if its the active gear set
    if (name == ActiveGear())
    {
        // apply specific equipped item only
        if (ActiveGearSet().HasItemInSlot(slot))
        {
            ApplyItem(item, slot);
        }
    }
    NotifyGearChanged(slot);
    //UpdateGreensteelStances();
    SetModifiedFlag(TRUE);
}

void Build::ClearGearInSlot(const std::string& name, InventorySlotType slot)
{
    if (name == ActiveGear())
    {
        // revoke specific equipped item only
        if (ActiveGearSet().HasItemInSlot(slot))
        {
            const Item& oldItem = ActiveGearSet().ItemInSlot(slot);
            RevokeItem(oldItem, slot);
        }
    }
    // update the gear entry
    std::list<EquippedGear>::iterator it = m_GearSetups.begin();
    bool found = false;
    while (!found && it != m_GearSetups.end())
    {
        if ((*it).Name() == name)
        {
            (*it).ClearItem(slot);
            found = true;
        }
        ++it;
    }
    ASSERT(found);

    NotifyGearChanged(slot);
    SetModifiedFlag(TRUE);
}

void Build::RevokeGearEffects()
{
    EquippedGear gear = ActiveGearSet(); // empty if no gear found
    // iterate the items
    for (size_t i = Inventory_Unknown + 1; i < Inventory_Count; ++i)
    {
        InventorySlotType ist = (InventorySlotType)i;
        if (gear.HasItemInSlot(ist))
        {
            Item item = gear.ItemInSlot(ist);
            RevokeItem(item, ist);
        }
    }
    // revoke this gears sentient weapon Filigrees
    for (size_t si = 0 ; si < MAX_FILIGREE; ++si)
    {
        std::string filigree = gear.GetFiligree(si);
        RevokeFiligree(filigree, si, gear.IsRareFiligree(si), false);
    }
    if (gear.HasMinorArtifact())
    {
        for (size_t si = 0 ; si < MAX_ARTIFACT_FILIGREE; ++si)
        {
            std::string filigree = gear.GetArtifactFiligree(si);
            RevokeFiligree(filigree, si, gear.IsRareArtifactFiligree(si), true);
        }
    }
}

void Build::RevokeFiligree(
        const std::string& filigreeName,
        size_t si,
        bool bRare,
        bool bArtifact)
{
    if (filigreeName != "")
    {
        // there is a Filigree in this position
        const Filigree& filigree = FindFiligreeByName(filigreeName);
        // name is:
        // Filigree <index>: <Filigree name>
        std::stringstream ss;
        if (bArtifact) ss << "Artifact ";
        ss << "Filigree " << (si +  1) << ": " << filigree.Name();
        // now revoke the filigree effects
        std::string name;
        name = ss.str();
        for (auto&& feit : filigree.NormalEffects())
        {
            NotifyItemEffectRevoked(name, feit, Inventory_Unknown);
        }
        // now do any rare effects
        // now do any rare effects
        if (bRare)
        {
            for (auto&& feit : filigree.RareEffects())
            {
                NotifyItemEffectRevoked(name, feit, Inventory_Unknown);
            }
        }
        // revoke any filigree stances
        //const std::list<Stance> & stances = filigree.StanceData();
        //std::list<Stance>::const_iterator sit = stances.begin();
        //while (sit != stances.end())
        //{
            //NotifyRevokeStance((*sit));
            //++sit;
        //}
        // revoke any filigree set bonuses
        for (auto&& sbit: filigree.SetBonus())
        {
            RevokeSetBonus(sbit, name);
        }
    }
}

void Build::ApplyGearEffects()
{
    EquippedGear gear = ActiveGearSet();
    // iterate the items
    for (size_t i = Inventory_Unknown + 1; i < Inventory_Count; ++i)
    {
        InventorySlotType ist = (InventorySlotType)i;
        if (gear.HasItemInSlot(ist))
        {
            Item item = gear.ItemInSlot(ist);
            ApplyItem(item, ist);
        }
    }
    // apply this gears weapon and artifact Filigrees
    for (size_t si = 0 ; si < MAX_FILIGREE; ++si)
    {
        std::string filigree = gear.GetFiligree(si);
        ApplyFiligree(filigree, si, gear.IsRareFiligree(si), false);
    }
    if (gear.HasMinorArtifact())
    {
        for (size_t si = 0 ; si < MAX_ARTIFACT_FILIGREE; ++si)
        {
            std::string filigree = gear.GetArtifactFiligree(si);
            ApplyFiligree(filigree, si, gear.IsRareArtifactFiligree(si), true);
        }
    }
}

void Build::ApplyItem(const Item& item, InventorySlotType ist)
{
    bool bSuppressSetBonuses = false;
    switch (ist)
    {
        case Inventory_Weapon1:
        case Inventory_Weapon2:
            ApplyWeaponEffects(item, ist);
            break;
        case Inventory_Armor:
            ApplyArmorEffects(item);
            break;
    }
    // apply the items effects
    for (auto&& ibit : item.Buffs())
    {
        const Buff& buff = FindBuff(ibit.Type());
        std::list<Effect> effects = buff.Effects();
        ibit.UpdatedEffects(&effects, buff.HasNegativeValues());
        for (auto&& eit : effects)
        {
            if (buff.HasApplyToWeaponOnly())
            {
                if (item.HasWeapon())
                {
                    NotifyItemWeaponEffect(item.Name(), eit, item.Weapon(), ist);
                }
                else
                {
                    NotifyItemWeaponEffect(item.Name(), eit, Weapon_All, ist);
                }
            }
            else
            {
                NotifyItemEffect(item.Name(), eit, ist);
            }
        }
        for (auto&& sit: buff.Stances())
        {
            NotifyNewStance(sit);
        }
    }
    // apply the items effects
    for (auto&& eit : item.Effects())
    {
        ApplyItemEffect(item.Name(), eit, ist);
    }
    //for (auto&& dcit: item.EffectDC())
    //{
    //    NotifyNewDC(dcit);
    //}
    // do the same for any augment slots
    for (auto&& ait: item.Augments())
    {
        if (ait.HasSelectedAugment())
        {
            // there is an augment in this position
            const Augment& augment = ait.GetSelectedAugment();
            bSuppressSetBonuses |= augment.HasSuppressSetBonus();
            ApplyAugment(augment, ait, item.Name(), item.MinLevel(), item.HasWeapon() ? item.Weapon() : Weapon_Unknown);
        }
    }
    // apply any item set bonuses if not suppressed
    if (!bSuppressSetBonuses)
    {
        for (auto&& sbit: item.SetBonus())
        {
            ApplySetBonus(sbit, item.Name());
        }
    }
}

void Build::ApplyAugment(
        const Augment& augment,
        const ItemAugment& itemAugment,
        const std::string& itemName,
        size_t itemLevel,
        WeaponType wt)
{
    // name is:
    // <item>:<augment type>:<Augment name>
    std::stringstream ss;
    ss << itemName << " : " << itemAugment.Type() << " : " << augment.Name();
    // now notify the augments effects
    std::string name;
    name = ss.str();
    size_t effectIndex = 0;
    auto effects = augment.Effects();
    for (auto&& eit: effects)
    {
        // we use a copy of the effect object here to be able to update it
        // augment may have user entered/item level specific values
        if (augment.HasEnterValue())
        {
            if (augment.HasDualValues())
            {
                if (effectIndex == 0)
                {
                    if (itemAugment.HasValue())
                    {
                        eit.SetAmount(itemAugment.Value());
                    }
                }
                else
                {
                    if (itemAugment.HasValue2())
                    {
                        eit.SetAmount(itemAugment.Value2());
                    }
                }
            }
            else if (itemAugment.HasValue())
            {
                eit.SetAmount(itemAugment.Value());
            }
        }
        if (augment.HasChooseLevel())
        {
            if (itemAugment.HasSelectedLevelIndex())
            {
                if (augment.HasDualValues())
                {
                    if (effectIndex == 0)
                    {
                        eit.SetAmount(augment.LevelValue()[itemAugment.SelectedLevelIndex()]);
                    }
                    else
                    {
                        eit.SetAmount(augment.LevelValue2()[itemAugment.SelectedLevelIndex()]);
                    }
                }
                else
                {
                    eit.SetAmount(augment.LevelValue()[itemAugment.SelectedLevelIndex()]);
                }
            }
            else
            {
                // use the item level to choose the effect value
                if (augment.HasDualValues())
                {
                    if (effectIndex == 0)
                    {
                        eit.SetAmount(augment.LevelValue()[itemLevel]);
                    }
                    else
                    {
                        eit.SetAmount(augment.LevelValue2()[itemLevel]);
                    }
                }
                else
                {
                    eit.SetAmount(augment.LevelValue()[itemLevel]);
                }
            }
        }
        if (eit.IsType(Effect_AddGroupWeapon))
        {
            if (eit.Item().back() == "ReplacedDynamically")
            {
                CString weapon = EnumEntryText(wt, weaponTypeMap);
                eit.ReplaceLastItem((LPCTSTR)weapon);
            }
        }
        eit.SetApplyAsItemEffect();
        NotifyItemEffect(name, eit, Inventory_Unknown);
        ++effectIndex;
    }
    // apply any augment stances
    for (auto&& sit: augment.StanceData())
    {
        NotifyNewStance(sit);
    }
    // apply any augment set bonuses
    for (auto&& sbit: augment.SetBonus())
    {
        ApplySetBonus(sbit, name);
    }
}

void Build::RevokeItem(const Item& item, InventorySlotType ist)
{
    bool bSuppressSetBonuses = false;
    switch (ist)
    {
        case Inventory_Weapon1:
        case Inventory_Weapon2:
            RevokeWeaponEffects(item, ist);
            break;
        case Inventory_Armor:
            RevokeArmorEffects(item);
            break;
    }
    // revoke the items effects
    for (auto&& ibit : item.Buffs())
    {
        const Buff& buff = FindBuff(ibit.Type());
        std::list<Effect> effects = buff.Effects();
        ibit.UpdatedEffects(&effects, buff.HasNegativeValues());
        for (auto&& eit : effects)
        {
            if (buff.HasApplyToWeaponOnly())
            {
                if (item.HasWeapon())
                {
                    NotifyItemWeaponEffectRevoked(item.Name(), eit, item.Weapon(), ist);
                }
                else
                {
                    NotifyItemWeaponEffectRevoked(item.Name(), eit, Weapon_All, ist);
                }
            }
            else
            {
                NotifyItemEffectRevoked(item.Name(), eit, ist);
            }
        }
        for (auto&& sit : buff.Stances())
        {
            NotifyRevokeStance(sit);
        }
    }
    // apply the items effects
    for (auto&& eit : item.Effects())
    {
        RevokeItemEffect(item.Name(), eit, ist);
    }
    //for (auto&& dcit: item.EffectDC())
    //{
    //    NotifyRevokeDC(dcit);
    //}
    // do the same for any augment slots
    for (auto&& ait: item.Augments())
    {
        if (ait.HasSelectedAugment())
        {
            // there is an augment in this position
            const Augment & augment = ait.GetSelectedAugment();
            bSuppressSetBonuses |= augment.HasSuppressSetBonus();
            RevokeAugment(augment, ait, item.Name(), item.MinLevel(), item.HasWeapon() ? item.Weapon() : Weapon_Unknown);
        }
    }
    if (!bSuppressSetBonuses)
    {
        // revoke any item set bonuses
        for (auto&& sbit: item.SetBonus())
        {
            RevokeSetBonus(sbit, item.Name());
        }
    }
}

void Build::RevokeAugment(
        const Augment& augment,
        const ItemAugment& itemAugment,
        const std::string& itemName,
        size_t itemLevel,
        WeaponType wt)
{
    // name is:
    // <item>:<augment type>:<Augment name>
    std::stringstream ss;
    ss << itemName << " : " << itemAugment.Type()  << " : " << augment.Name();
    // now revoke the augments effects
    std::string name;
    name = ss.str();
    size_t effectIndex = 0;
    auto effects = augment.Effects();
    for (auto&& eit : effects)
    {
        // we use a copy of the effect object here to be able to update it
        // augment may have user entered/item level specific values
        if (augment.HasEnterValue())
        {
            if (augment.HasDualValues())
            {
                if (effectIndex == 0)
                {
                    if (itemAugment.HasValue())
                    {
                        eit.SetAmount(itemAugment.Value());
                    }
                }
                else
                {
                    if (itemAugment.HasValue2())
                    {
                        eit.SetAmount(itemAugment.Value2());
                    }
                }
            }
            else if (itemAugment.HasValue())
            {
                eit.SetAmount(itemAugment.Value());
            }
        }
        if (augment.HasChooseLevel())
        {
            if (itemAugment.HasSelectedLevelIndex())
            {
                if (augment.HasDualValues())
                {
                    if (effectIndex == 0)
                    {
                        eit.SetAmount(augment.LevelValue()[itemAugment.SelectedLevelIndex()]);
                    }
                    else
                    {
                        eit.SetAmount(augment.LevelValue2()[itemAugment.SelectedLevelIndex()]);
                    }
                }
                else
                {
                    eit.SetAmount(augment.LevelValue()[itemAugment.SelectedLevelIndex()]);
                }
            }
            else
            {
                // use the item level to choose the effect value
                if (augment.HasDualValues())
                {
                    if (effectIndex == 0)
                    {
                        eit.SetAmount(augment.LevelValue()[itemLevel]);
                    }
                    else
                    {
                        eit.SetAmount(augment.LevelValue2()[itemLevel]);
                    }
                }
                else
                {
                    eit.SetAmount(augment.LevelValue()[itemLevel]);
                }
            }
        }
        if (eit.IsType(Effect_AddGroupWeapon))
        {
            if (eit.Item().back() == "ReplacedDynamically")
            {
                CString weapon = EnumEntryText(wt, weaponTypeMap);
                eit.ReplaceLastItem((LPCTSTR)weapon);
            }
        }
        eit.SetApplyAsItemEffect();
        NotifyItemEffectRevoked(name, eit, Inventory_Unknown);
        ++effectIndex;
    }
    // clear any augment stances
    for (auto&& sit: augment.StanceData())
    {
        NotifyRevokeStance(sit);
    }
    // revoke any augment set bonuses
    for (auto&& sbit: augment.SetBonus())
    {
        RevokeSetBonus(sbit, name);
    }
}

void Build::ApplyFiligree(
        const std::string& filigreeName,
        size_t si,
        bool bRare,
        bool bArtifact)
{
    if (filigreeName != "")
    {
        // there is a filigree in this position
        const Filigree& filigree = FindFiligreeByName(filigreeName);
        // name is:
        // Filigree <slot>: <Filigree name>
        std::stringstream ss;
        if (bArtifact) ss << "Artifact ";
        ss << "Filigree " << (si + 1) << ": " << filigree.Name();
        // now apply the filigree effects
        std::string name(ss.str());
        for (auto&& feit: filigree.NormalEffects())
        {
            NotifyItemEffect(name, feit, Inventory_Unknown);
        }
        // now do any rare effects
        if (bRare)
        {
            for (auto&& feit: filigree.RareEffects())
            {
                NotifyItemEffect(name, feit, Inventory_Unknown);
            }
        }
        // apply any filigree stances
        //for (auto&& fsit: filigree.StanceData())
        //{
            //NotifyNewStance(fsit);
        //}
        // apply any filigree set bonuses
        for (auto&& fsbit: filigree.SetBonus())
        {
            ApplySetBonus(fsbit, name);
        }
    }
}

void Build::ApplySetBonus(
        const std::string& set,
        const std::string& name)
{
    size_t stacks = AddSetBonusStack(set);
    const SetBonus& setObject = FindSetBonus(set);
    // sets effects
    for (auto&& it: setObject.ActiveEffects(stacks))
    {
        NotifyItemEffect(name, it, Inventory_Unknown);
    }
    // notify about the new set being trained (or new stack of it)
    NotifyNewSetBonusStack(setObject);

    //// they may also have regular stance objects
    //const std::vector<Stance> & stances = setObject.Stances();
    //std::vector<Stance>::const_iterator sit = stances.begin();
    //while (sit != stances.end())
    //{
    //    NotifyNewStance((*sit));
    //    ++sit;
    //}
}

void Build::RevokeSetBonus(
        const std::string& set,
        const std::string& name)
{
    size_t stacks = RevokeSetBonusStack(set);
    const SetBonus& setObject = FindSetBonus(set);
    // sets effects
    for (auto&& it: setObject.ActiveEffects(stacks+1))
    {
        NotifyItemEffectRevoked(name, it, Inventory_Unknown);
    }
    // notify about the new set being trained (or new stack of it)
    NotifyRevokeSetBonusStack(setObject);

    //// they may also have regular stance objects
    //const std::vector<Stance> & stances = setObject.Stances();
    //std::vector<Stance>::const_iterator sit = stances.begin();
    //while (sit != stances.end())
    //{
    //    NotifyRevokeStance((*sit));
    //    ++sit;
    //}
}

size_t Build::AddSetBonusStack(const std::string& set)
{
    size_t stacks = 0;
    bool bFound = false;
    // if the set is already in the list find it and increment the count
    // else create a new entry
    for (auto&& it : m_setBonusStacks)
    {
        if (!bFound && it.Name() == set)
        {
            it.AddStack();
            stacks = it.Stacks();
            bFound = true;
        }
    }
    if (!bFound)
    {
        m_setBonusStacks.push_back(StackTracking(set));
        m_setBonusStacks.back().AddStack();
        stacks = 1;         // always 1 stack when created new
    }
    return stacks;
}

size_t Build::RevokeSetBonusStack(const std::string& set)
{
    size_t stacks = 0;
    // if we find it, decrement the stack count
    // if it becomes empty, remove it from the list
    std::list<StackTracking>::iterator it = m_setBonusStacks.begin();
    while (it != m_setBonusStacks.end())
    {
        if ((*it).Name() == set)
        {
            (*it).RevokeStack();
            stacks = (*it).Stacks();
            if (stacks == 0)
            {
                it = m_setBonusStacks.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
    return stacks;
}

const std::list<StackTracking>& Build::ActiveSets() const
{
    return m_setBonusStacks;
}

void Build::SetupDefaultWeaponGroups()
{
    m_weaponGroups.clear();
    // use the defaults loaded from the files as the base
    m_weaponGroups = WeaponGroups();
}

bool Build::IsWeaponInGroup(const std::string& group, WeaponType wt) const
{
    bool isPresent = false;
    for (auto&& wgit : m_weaponGroups)
    {
        if (wgit.Name() == group)
        {
            isPresent = wgit.HasWeapon(wt, *this);
            break;  // no need to check the rest
        }
    }
    return isPresent;
}

void Build::AddWeaponToGroup(const Effect& effect)
{
    // see if the weapon group already exists. If so, add the list of weapons to it
    // else create the weapon group and add the weapons to it
    std::string group = effect.Item().front();
    std::list<WeaponGroup>::iterator wgit = m_weaponGroups.begin();
    while (wgit != m_weaponGroups.end()
            && (*wgit).Name() != group)
    {
        ++wgit;
    }
    if (wgit != m_weaponGroups.end())
    {
        for (auto&& iit : effect.Item())
        {
            WeaponType wt = TextToEnumEntry(iit, weaponTypeMap, false);
            if (wt != Weapon_Unknown)
            {
                if (effect.HasRequirementsToBeActive())
                {
                    (*wgit).AddWeapon(wt, effect.RequirementsToBeActive());
                }
                else
                {
                    (*wgit).AddWeapon(wt);
                }
            }
        }
    }
    else
    {
        // need to create new weapon group
        WeaponGroup wg(group);
        for (auto&& iit : effect.Item())
        {
            WeaponType wt = TextToEnumEntry(iit, weaponTypeMap, false);
            if (wt != Weapon_Unknown)
            {
                if (effect.HasRequirementsToBeActive())
                {
                    wg.AddWeapon(wt, effect.RequirementsToBeActive());
                }
                else
                {
                    wg.AddWeapon(wt);
                }
            }
        }
        m_weaponGroups.push_back(wg);
    }
    NotifyStanceActivated("cause an update");
}

void Build::RemoveFromWeaponGroup(const Effect& effect)
{
    std::string group = effect.Item().front();
    std::list<WeaponGroup>::iterator wgit = m_weaponGroups.begin();
    while (wgit != m_weaponGroups.end()
            && (*wgit).Name() != group)
    {
        ++wgit;
    }
    if (wgit != m_weaponGroups.end())
    {
        for (auto&& iit : effect.Item())
        {
            WeaponType wt = TextToEnumEntry(iit, weaponTypeMap, false);
            if (wt != Weapon_Unknown)
            {
                if (effect.HasRequirementsToBeActive())
                {
                    (*wgit).RemoveWeapon(wt, effect.RequirementsToBeActive());
                }
                else
                {
                    (*wgit).RemoveWeapon(wt);
                }
            }
        }
        // its ok if the group is now empty
    }
    else
    {
        // can't remove from a weapon group that does not exist
    }
    NotifyStanceActivated("cause an update");
}

void Build::MergeGroups(const Effect& effect)
{
    std::string group = effect.Item().front();
    std::list<WeaponGroup>::iterator wgit = m_weaponGroups.begin();
    while (wgit != m_weaponGroups.end()
        && (*wgit).Name() != group)
    {
        ++wgit;
    }
    if (wgit != m_weaponGroups.end())
    {
        wgit->AddMergeGroup(effect.Item().back());
    }
    NotifyStanceActivated("cause an update");
}

void Build::RemoveMergeGroup(const Effect& effect)
{
    std::string group = effect.Item().front();
    std::list<WeaponGroup>::iterator wgit = m_weaponGroups.begin();
    while (wgit != m_weaponGroups.end()
        && (*wgit).Name() != group)
    {
        ++wgit;
    }
    if (wgit != m_weaponGroups.end())
    {
        wgit->RemoveMergeGroup(effect.Item().back());
    }
    NotifyStanceActivated("cause an update");
}

void Build::DumpWeaponGroups() const
{
    for (auto&& wgit : m_weaponGroups)
    {
        wgit.DumpToLog(*this);
    }
}

WeaponType Build::MainHandWeapon() const
{
    WeaponType wt = Weapon_Empty;
    EquippedGear gs = ActiveGearSet();
    if (gs.HasItemInSlot(Inventory_Weapon1))
    {
        wt = gs.ItemInSlot(Inventory_Weapon1).Weapon();
    }
    return wt;
}

WeaponType Build::OffhandWeapon() const
{
    WeaponType wt = Weapon_Empty;
    EquippedGear gs = ActiveGearSet();
    if (gs.HasItemInSlot(Inventory_Weapon2))
    {
        wt = gs.ItemInSlot(Inventory_Weapon2).Weapon();
    }
    return wt;
}

void Build::ApplyWeaponEffects(const Item& item, InventorySlotType ist)
{
    std::string wt = (LPCTSTR)EnumEntryText(item.Weapon(), weaponTypeMap);
    if (item.HasWeaponDamage())
    {
        Effect effect(
            Effect_Weapon_BaseDamage,
            "Base Weapon Damage",
            "Base",
            item.WeaponDamage());
        // need to add the weapon type
        effect.AddItem(wt);
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffect(item.Name(), effect, item.Weapon(), ist);
    }
    if (item.HasCriticalThreatRange())
    {
        Effect effect(
                Effect_Weapon_CriticalRange,
                "Base Weapon Range",
                "Base",
                item.CriticalThreatRange());
        effect.AddItem(wt);
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffect(item.Name(), effect, item.Weapon(), ist);
    }
    if (item.HasCriticalMultiplier())
    {
        Effect effect(
            Effect_Weapon_CriticalMultiplier,
            "Base Weapon Multiplier",
            "Base",
            item.CriticalMultiplier());
        effect.AddItem(wt);
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffect(item.Name(), effect, item.Weapon(), ist);
    }
    for (auto&& itDr: item.DRBypass())
    {
        Effect effect(
            Effect_DRBypass,
            item.Name(),
            "Weapon DR",
            0);
        effect.SetAType(Amount_NotNeeded);
        effect.AddItem(wt);
        effect.AddValue((LPCTSTR)EnumEntryText(itDr, drTypeMap));
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffect(item.Name(), effect, item.Weapon(), ist);
    }
    if (item.HasShieldBonus())
    {
        Effect effect(
            Effect_ACBonus,
            item.Name(),
            "Shield",
            item.ShieldBonus());
        effect.AddItem(wt);
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffect(item.Name(), effect, item.Weapon(), ist);
    }
}

void Build::RevokeWeaponEffects(const Item& item, InventorySlotType ist)
{
    std::string wt = (LPCTSTR)EnumEntryText(item.Weapon(), weaponTypeMap);
    if (item.HasWeaponDamage())
    {
        Effect effect(
                Effect_Weapon_BaseDamage,
                "Base Weapon Damage",
                "Base",
                item.WeaponDamage());
        effect.AddItem(wt);
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffectRevoked(item.Name(), effect, item.Weapon(), ist);
    }
    if (item.HasCriticalThreatRange())
    {
        Effect effect(
                Effect_Weapon_CriticalRange,
                "Base Weapon Range",
                "Base",
                item.CriticalThreatRange());
        effect.AddItem(wt);
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffectRevoked(item.Name(), effect, item.Weapon(), ist);
    }
    if (item.HasCriticalMultiplier())
    {
        Effect effect(
                Effect_Weapon_CriticalMultiplier,
                "Base Weapon Multiplier",
                "Base",
                item.CriticalMultiplier());
        effect.AddItem(wt);
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffectRevoked(item.Name(), effect, item.Weapon(), ist);
    }
    for (auto&& itDr : item.DRBypass())
    {
        Effect effect(
            Effect_DRBypass,
            item.Name(),
            "Weapon DR",
            0);
        effect.SetAType(Amount_NotNeeded);
        effect.AddItem(wt);
        effect.AddValue((LPCTSTR)EnumEntryText(itDr, drTypeMap));
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffectRevoked(item.Name(), effect, item.Weapon(), ist);
    }
    if (item.HasShieldBonus())
    {
        Effect effect(
            Effect_ACBonus,
            item.Name(),
            "Shield",
            item.ShieldBonus());
        effect.AddItem(wt);
        effect.SetIsItemSpecific();
        NotifyItemWeaponEffectRevoked(item.Name(), effect, item.Weapon(), ist);
    }
}

void Build::ApplyArmorEffects(const Item& item)
{
    if (item.HasMithralBody())
    {
        if (item.HasArmorBonus())
        {
            Effect effect(
                Effect_ACBonus,
                "Armor Bonus (Composite Plating)",
                "Armor",
                item.ArmorBonus());
            Requirements req;
            Requirement featRequirement(Requirement_Feat, "Composite Plating");
            req.AddRequirement(featRequirement);
            effect.SetRequirements(req);
            NotifyItemEffect(item.Name(), effect, Inventory_Armor);
        }
        Effect effect(
            Effect_ACBonus,
            "Armor Bonus (Mithral Body)",
            "Armor",
            item.MithralBody());
        Requirements req;
        Requirement featRequirement(Requirement_Feat, "Mithral Body");
        req.AddRequirement(featRequirement);
        effect.SetRequirements(req);
        NotifyItemEffect(item.Name(), effect, Inventory_Armor);
    }
    else
    {
        // non-war/bladeforged item
        if (item.HasArmorBonus())
        {
            Effect effect(
                Effect_ACBonus,
                "Armor Bonus",
                "Armor",
                item.ArmorBonus());
            NotifyItemEffect(item.Name(), effect, Inventory_Armor);
        }
    }
    if (item.HasAdamantineBody())
    {
        Effect effect(
            Effect_ACBonus,
            "Armor Bonus (Adamantine Body)",
            "Armor",
            item.AdamantineBody());
        Requirements req;
        Requirement featRequirement(Requirement_Feat, "Adamantine Body");
        req.AddRequirement(featRequirement);
        effect.SetRequirements(req);
        NotifyItemEffect(item.Name(), effect, Inventory_Armor);
    }
    if (item.HasMaximumDexterityBonus())
    {
        Effect effect(
            Effect_MaxDexBonus,
            "Armor Max Dex Bonus",
            "Armor",
            item.MaximumDexterityBonus());
        NotifyItemEffect(item.Name(), effect, Inventory_Armor);
    }
    if (item.HasArmorCheckPenalty())
    {
        Effect effect(
            Effect_ArmorCheckPenalty,
            "Armor Check Penalty",
            "Armor",
            item.ArmorCheckPenalty());
        NotifyItemEffect(item.Name(), effect, Inventory_Armor);
    }
    if (item.HasArcaneSpellFailure())
    {
        Effect effect(
            Effect_ArcaneSpellFailure,
            "Armor Arcane Spell Failure",
            "Armor",
            item.ArcaneSpellFailure());
        NotifyItemEffect(item.Name(), effect, Inventory_Armor);
    }
}

void Build::RevokeArmorEffects(const Item& item)
{
    if (item.HasMithralBody())
    {
        if (item.HasArmorBonus())
        {
            Effect effect(
                Effect_ACBonus,
                "Armor Bonus (Composite Plating)",
                "Armor",
                item.ArmorBonus());
            Requirements req;
            Requirement featRequirement(Requirement_Feat, "Composite Plating");
            req.AddRequirement(featRequirement);
            effect.SetRequirements(req);
            NotifyItemEffectRevoked(item.Name(), effect, Inventory_Armor);
        }
        Effect effect(
            Effect_ACBonus,
            "Armor Bonus (Mithral Body)",
            "Armor",
            item.MithralBody());
        Requirements req;
        Requirement featRequirement(Requirement_Feat, "Mithral Body");
        req.AddRequirement(featRequirement);
        effect.SetRequirements(req);
        NotifyItemEffectRevoked(item.Name(), effect, Inventory_Armor);
    }
    else
    {
        // non-war/bladeforged item
        if (item.HasArmorBonus())
        {
            Effect effect(
                Effect_ACBonus,
                "Armor Bonus",
                "Armor",
                item.ArmorBonus());
            NotifyItemEffectRevoked(item.Name(), effect, Inventory_Armor);
        }
    }
    if (item.HasAdamantineBody())
    {
        Effect effect(
            Effect_ACBonus,
            "Armor Bonus (Adamantine Body)",
            "Armor",
            item.AdamantineBody());
        Requirements req;
        Requirement featRequirement(Requirement_Feat, "Adamantine Body");
        req.AddRequirement(featRequirement);
        effect.SetRequirements(req);
        NotifyItemEffectRevoked(item.Name(), effect, Inventory_Armor);
    }
    if (item.HasMaximumDexterityBonus())
    {
        Effect effect(
            Effect_MaxDexBonus,
            "Armor Max Dex Bonus",
            "Armor",
            item.MaximumDexterityBonus());
        NotifyItemEffectRevoked(item.Name(), effect, Inventory_Armor);
    }
    if (item.HasArmorCheckPenalty())
    {
        Effect effect(
            Effect_ArmorCheckPenalty,
            "Armor Check Penalty",
            "Armor",
            item.ArmorCheckPenalty());
        NotifyItemEffectRevoked(item.Name(), effect, Inventory_Armor);
    }
    if (item.HasArcaneSpellFailure())
    {
        Effect effect(
            Effect_ArcaneSpellFailure,
            "Armor Arcane Spell Failure",
            "Armor",
            item.ArcaneSpellFailure());
        NotifyItemEffectRevoked(item.Name(), effect, Inventory_Armor);
    }
}

void Build::VerifySpecialFeats()
{
    std::list<TrainedFeat> feats = SpecialFeats();
    std::list<TrainedFeat>::iterator fit = feats.begin();
    while (fit != feats.end())
    {
        const Feat& feat = FindFeat(fit->FeatName());
        if (fit->FeatName() != feat.Name())
        {
            // this special feat no longer exists, remove it
            RevokeSpecialFeat(fit->FeatName(), true);
        }
        ++fit;
    }
}

bool Build::operator<(const Build& other) const
{
    return m_Level < other.Level();
}

void Build::ApplyGuildBuffs(bool bApply)
{
    // we only apply or revoke the effects within the m_previousGuildLevel to
    // GuildLevel range
    if (bApply)
    {
        if (m_previousGuildLevel != m_pLife->GuildLevel())
        {
            // there has been a change in guild level find the list of guild buffs
            // that have changed. Then either apply or revoke them depending
            // on the direction of level change
            bool revoke = (m_previousGuildLevel > m_pLife->GuildLevel());
            size_t minLevel = min(m_previousGuildLevel, m_pLife->GuildLevel());
            size_t maxLevel = max(m_previousGuildLevel, m_pLife->GuildLevel());
            std::list<GuildBuff> guildBuffs = GuildBuffs(); // all known guild buffs
            std::list<GuildBuff>::iterator it = guildBuffs.begin();
            while (it != guildBuffs.end())
            {
                // remove the buff if it is not in the required level range of change
                if ((*it).Level() <= minLevel
                    || (*it).Level() > maxLevel)
                {
                    // this buff is not in the range of those changed, exclude it
                    it = guildBuffs.erase(it);
                }
                else
                {
                    // on to the next buff to check
                    ++it;
                }
            }
            // now apply or revoke the buffs left in the list
            it = guildBuffs.begin();
            while (it != guildBuffs.end())
            {
                const std::list<Effect>& effects = (*it).Effects();
                std::list<Effect>::const_iterator eit = effects.begin();
                while (eit != effects.end())
                {
                    if (revoke)
                    {
                        NotifyItemEffectRevoked((*it).Name(), (*eit), Inventory_Unknown);
                    }
                    else
                    {
                        NotifyItemEffect((*it).Name(), (*eit), Inventory_Unknown);
                    }
                    ++eit;
                }
                ++it;
            }
            m_previousGuildLevel = m_pLife->GuildLevel();
        }
    }
    else
    {
        // guild buffs are no longer applied all, existing buffs need to be revoked
        size_t minLevel = 0;
        size_t maxLevel = m_pLife->GuildLevel();
        std::list<GuildBuff> guildBuffs = GuildBuffs(); // all known guild buffs
        std::list<GuildBuff>::iterator it = guildBuffs.begin();
        while (it != guildBuffs.end())
        {
            // remove the buff if it is not in the required level range of revoke
            if ((*it).Level() <= minLevel
                || (*it).Level() > maxLevel)
            {
                // this buff is not in the range of those changed, exclude it
                it = guildBuffs.erase(it);
            }
            else
            {
                // on to the next buff to check
                ++it;
            }
        }
        // now revoke the buffs left in the list
        it = guildBuffs.begin();
        while (it != guildBuffs.end())
        {
            const std::list<Effect>& effects = (*it).Effects();
            std::list<Effect>::const_iterator eit = effects.begin();
            while (eit != effects.end())
            {
                NotifyItemEffectRevoked((*it).Name(), (*eit), Inventory_Unknown);
                ++eit;
            }
            ++it;
        }
        m_previousGuildLevel = 0;
    }
}

void Build::GuildLevelChange()
{
    ApplyGuildBuffs(m_pLife->ApplyGuildBuffs());
}

void Build::NotifyOptionalBuff(const std::string& name)
{
    // find the buff and notify its effects
    const OptionalBuff& buff = FindOptionalBuff(name);
    // get the list of effects this OptionalBuff has
    const std::vector<Effect>& effects = buff.Effects();
    std::vector<Effect>::const_iterator feit = effects.begin();
    for (auto&& it: effects)
    {
        NotifyItemEffect(name, it, Inventory_Unknown);
    }
}

void Build::RevokeOptionalBuff(const std::string& name)
{
    // find the buff and clear its effects
    const OptionalBuff& buff = FindOptionalBuff(name);
    // get the list of effects this OptionalBuff has
    const std::vector<Effect>& effects = buff.Effects();
    for (auto&& it : effects)
    {
        NotifyItemEffectRevoked(name, it, Inventory_Unknown);
    }
}

void Build::ApplySelfAndPartyBuffs()
{
    const std::list<std::string>& opBuffs = m_pLife->SelfAndPartyBuffs();
    for (auto&& it : opBuffs)
    {
        NotifyOptionalBuff(it);
    }
}

void Build::SetNotes(const std::string& newNotes)
{
    Set_Notes(newNotes);
    SetModifiedFlag(TRUE);
}

void Build::ApplyItemEffect(const std::string& name, const Effect& effect, InventorySlotType ist)
{
    for (auto&& tit : effect.Type())
    {
        Effect copy = effect;
        copy.SetType(tit);
        NotifyItemEffect(name, copy, ist);
    }
}

void Build::RevokeItemEffect(const std::string& name, const Effect& effect, InventorySlotType ist)
{
    for (auto&& tit : effect.Type())
    {
        Effect copy = effect;
        copy.SetType(tit);
        NotifyItemEffectRevoked(name, copy, ist);
    }
}

void Build::SetQuestsCompletions(const std::list<CompletedQuest>& quests)
{
    std::list<CompletedQuest> currentQuests = CompletedQuests();
    for (auto&& qit : quests)
    {
        std::list<CompletedQuest>::iterator cqit = currentQuests.begin();
        while (cqit != currentQuests.end())
        {
            if (qit.SameQuestAndLevel(*cqit))
            {
                // this is the one we need to update
                break;
            }
            cqit++;
        }
        if (cqit != currentQuests.end())
        {
            // we have a Q to update to a new difficulty
            if (qit.Difficulty() != QD_notRun)
            {
                cqit->Set_Difficulty(qit.Difficulty()); // update
            }
            else
            {
                currentQuests.erase(cqit);  // remove as no longer been "run"
            }
        }
        else
        {
            // this quest has never been completed before, add it unless we're setting to QD_notRun
            if (qit.Difficulty() != QD_notRun)
            {
                currentQuests.push_back(qit);   // add it
            }
            else
            {
                // just ignore as its being set to Not Run and its not in the list anyway
            }
        }
    }
    Set_CompletedQuests(currentQuests);
    // and were done
    SetModifiedFlag(TRUE);
}

void Build::AddSpellListAddition(const Effect& effect)
{
    // check to make sure it is not already present
    std::string spellName = effect.Item().front();
    std::string className = effect.Item().back();
    int spellLevel = static_cast<int>(effect.Amount()[0]);
    //int spellSPCost = static_cast<int>(effect.Amount()[1]);
    //int spellMCL = static_cast<int>(effect.Amount()[2]);
    if (!IsSpellInSpellListAdditionList(className, spellLevel, spellName))
    {
        SpellListAddition sla(className, spellLevel, spellName);
        m_additionalSpells.push_back(sla);
    }
    else
    {
        // its already present, add to its count
        for (size_t i = 0; i < m_additionalSpells.size(); ++i)
        {
            if (m_additionalSpells[i].AddsToSpellList(className, spellLevel)
                    && m_additionalSpells[i].SpellName() == spellName)
            {
                m_additionalSpells[i].AddReference();
            }
        }
    }
}

void Build::RevokeSpellListAddition(const Effect& effect)
{
    bool bUpdate = false;
    // if its present remove a stack and revoke if now all gone
    std::string spellName = effect.Item().front();
    std::string className = effect.Item().back();
    int spellLevel = static_cast<int>(effect.Amount()[0]);
    //int spellSPCost = static_cast<int>(effect.Amount()[1]);
    //int spellMCL = static_cast<int>(effect.Amount()[2]);
    for (size_t i = 0; i < m_additionalSpells.size(); ++i)
    {
        if (m_additionalSpells[i].AddsToSpellList(className, spellLevel)
            && m_additionalSpells[i].SpellName() == spellName)
        {
            bool erase = m_additionalSpells[i].RemoveReference();
            if (erase)
            {
                m_additionalSpells.erase(m_additionalSpells.begin() + i);
                --i;        // keep index right
                bUpdate = true;
            }
        }
    }
    if (bUpdate)
    {
        // make sure trained spell lists are correct on spell list changes
        UpdateSpells();
    }
}

bool Build::IsSpellInSpellListAdditionList(
    const std::string& ct,
    size_t spellLevel,
    const std::string& spellName) const
{
    bool bPresent = false;
    for (size_t i = 0; !bPresent && i < m_additionalSpells.size(); ++i)
    {
        bPresent = m_additionalSpells[i].AddsToSpellList(ct, spellLevel)
            && m_additionalSpells[i].SpellName() == spellName;
    }
    return bPresent;
}

Spell Build::AdditionalClassSpell(
        const std::string& className,
        const std::string& spellName) const
{
    Spell spell;
    for (auto&& it: m_additionalSpells)
    {
        if (it.AddsToSpellList(className, spellName))
        {
            spell = FindSpellByName(spellName);
        }
    }
    return spell;
}

void Build::AppendSpellListAdditions(
        std::list<Spell>& spells,
        const std::string& ct,
        int spellLevel)
{
    for (auto&& it : m_additionalSpells)
    {
        if (it.AddsToSpellList(ct, spellLevel))
        {
            Spell spell = FindSpellByName(it.SpellName());
            spells.push_back(spell);
        }
    }
    spells.sort();
}

void Build::UpdateCachedClassLevels()
{
    std::map<std::string, int> cacheAtCurrentLevel;
    std::list<LevelTraining>::const_iterator clit = m_Levels.begin();
    for (size_t level = 0; level < MAX_GAME_LEVEL; ++level)
    {
        if (clit != m_Levels.end())
        {
            std::string c = clit->HasClass() ? clit->Class() : Class_Unknown;
            if (cacheAtCurrentLevel.find(c) != cacheAtCurrentLevel.end())
            {
                // already present, increment the class count
                cacheAtCurrentLevel[c] = cacheAtCurrentLevel[c] + 1;
            }
            else
            {
                // first encounter for this class, add an entry
                cacheAtCurrentLevel[c] = 1;
            }
            ++clit;
        }
        m_cachedClassLevels[level] = cacheAtCurrentLevel;
    }
}

void Build::UpdateTotalChanged(BreakdownItem* /*could be NULL*/, BreakdownType bt)
{
    if (!m_bSwitchingBuildsOrGear)
    {
        if (m_GearSetups.size() > 0)
        {
            BreakdownItem* pBI = FindBreakdown(bt);
            if (pBI != NULL)
            {
                int total = static_cast<int>(pBI->Total());
                // update the gear set snapshot value
                std::list<EquippedGear>::iterator it = m_GearSetups.begin();
                bool found = false;
                while (!found && it != m_GearSetups.end())
                {
                    if ((*it).Name() == ActiveGear())
                    {
                        switch (bt)
                        {
                            case Breakdown_Strength:        it->Set_SnapshotStrength(total); break;
                            case Breakdown_Intelligence:    it->Set_SnapshotIntelligence(total); break;
                            case Breakdown_Wisdom:          it->Set_SnapshotWisdom(total); break;
                            case Breakdown_Dexterity:       it->Set_SnapshotDexterity(total); break;
                            case Breakdown_Constitution:    it->Set_SnapshotConstitution(total); break;
                            case Breakdown_Charisma:        it->Set_SnapshotCharisma(total); break;
                        }
                        found = true;
                    }
                    ++it;
                }
            }
        }
    }
}

