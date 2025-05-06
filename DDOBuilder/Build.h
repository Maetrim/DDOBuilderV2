// Build.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "ObserverSubject.h"

#include "AbilitySpend.h"
#include "ActiveStances.h"
#include "AlignmentTypes.h"
#include "AttackChain.h"
#include "CompletedQuest.h"
#include "DestinySpendInTree.h"
#include "EnhancementItemParams.h"
#include "EnhancementSpendInTree.h"
#include "EquippedGear.h"
#include "ExclusionGroup.h"
#include "FeatsListObject.h"
#include "FeatSlot.h"
#include "InventorySlotTypes.h"
#include "LevelTraining.h"
#include "LegacyItem.h"
#include "ReaperSpendInTree.h"
#include "Destiny_SelectedTrees.h"
#include "Enhancement_SelectedTrees.h"
#include "Reaper_SelectedTrees.h"
#include "TrainedSpell.h"
#include "WeaponGroup.h"
#include "WeaponTypes.h"
#include "StackTracking.h"
#include "SpellListAddition.h"
#include "Spell.h"
#include <map>
#include "BreakdownItem.h"

class Attack;
class Augment;
class Build;
class DC;
class Effect;
class Feat;
class ItemAugment;
class Life;
class Race;
class SetBonus;
class Stance;
class StanceGroup;
class TrainedEnhancement;
class LegacyEnhancementSelectedTrees;
class LegacyDestinySelectedTrees;

class BuildObserver :
    public Observer<Build>
{
    public:
        virtual void UpdateBuildLevelChanged(Build*) {};
        virtual void UpdateClassChoiceChanged(Build*) {};
        virtual void UpdateClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) {UNREFERENCED_PARAMETER(classFrom);UNREFERENCED_PARAMETER(classTo);UNREFERENCED_PARAMETER(level);};
        virtual void UpdateFeatTrained(Build*, const std::string& featName) {UNREFERENCED_PARAMETER(featName);};
        virtual void UpdateFeatRevoked(Build*, const std::string& featName) {UNREFERENCED_PARAMETER(featName);};
        virtual void UpdateFeatEffectApplied(Build*, const Effect& effect) {UNREFERENCED_PARAMETER(effect);};
        virtual void UpdateFeatEffectRevoked(Build*, const Effect& effect) {UNREFERENCED_PARAMETER(effect);};
        virtual void UpdateStanceActivated(Build*, const std::string& stanceName) {UNREFERENCED_PARAMETER(stanceName);};
        virtual void UpdateStanceDeactivated(Build*, const std::string& stanceName) {UNREFERENCED_PARAMETER(stanceName);};
        virtual void UpdateStanceDisabled(Build*, const std::string& stanceName) { UNREFERENCED_PARAMETER(stanceName); };
        virtual void UpdateSliderChanged(Build*, const std::string& sliderName, int newValue) { UNREFERENCED_PARAMETER(sliderName); UNREFERENCED_PARAMETER(newValue); };
        virtual void UpdateSkillSpendChanged(Build*, size_t level, SkillType skill) {UNREFERENCED_PARAMETER(level);UNREFERENCED_PARAMETER(skill);};
        virtual void UpdateAbilityValueChanged(Build*, AbilityType ability) {UNREFERENCED_PARAMETER(ability);};
        virtual void UpdateEnhancementTrained(Build*, const EnhancementItemParams& item) {UNREFERENCED_PARAMETER(item);};
        virtual void UpdateEnhancementRevoked(Build*, const EnhancementItemParams& item) {UNREFERENCED_PARAMETER(item);};
        virtual void UpdateEnhancementEffectApplied(Build*, const Effect& effect) {UNREFERENCED_PARAMETER(effect);};
        virtual void UpdateEnhancementEffectRevoked(Build*, const Effect& effect) {UNREFERENCED_PARAMETER(effect);};
        virtual void UpdateEnhancementTreeOrderChanged(Build*) {};
        virtual void UpdateActionPointsChanged(Build*) {};
        virtual void UpdateNewStance(Build*, const Stance& stance) {UNREFERENCED_PARAMETER (stance);};
        virtual void UpdateRevokeStance(Build*, const Stance& stance) {UNREFERENCED_PARAMETER(stance);};
        virtual void UpdateItemEffectApplied(Build*, const Effect& effect) { UNREFERENCED_PARAMETER(effect); };
        virtual void UpdateItemEffectRevoked(Build*, const Effect& effect) { UNREFERENCED_PARAMETER(effect); };
        virtual void UpdateItemWeaponEffectApplied(Build*, const Effect& effect, WeaponType wt, InventorySlotType ist) { UNREFERENCED_PARAMETER(effect); UNREFERENCED_PARAMETER(wt); UNREFERENCED_PARAMETER(ist); };
        virtual void UpdateItemWeaponEffectRevoked(Build*, const Effect& effect, WeaponType wt, InventorySlotType ist) { UNREFERENCED_PARAMETER(effect); UNREFERENCED_PARAMETER(wt); UNREFERENCED_PARAMETER(ist); };
        virtual void UpdateBuildAutomaticFeatsChanged(Build*, size_t level) { UNREFERENCED_PARAMETER(level); };
        virtual void UpdateNewDC(Build*, const DC&) {};
        virtual void UpdateRevokeDC(Build*, const DC&) {};
        virtual void UpdateGearChanged(Build*, InventorySlotType) {};
        virtual void UpdateNewSetBonusStack(Build*, const SetBonus&) {};
        virtual void UpdateRevokeSetBonusStack(Build*, const SetBonus&) {};
        virtual void UpdateNewAttack(Build*, const Attack&) {};
        virtual void UpdateRevokeAttack(Build*, const Attack&) {};
};

class Build :
    public XmlLib::SaxContentElement,
    public Subject<BuildObserver>,
    public BreakdownObserver
{
    public:
        Build(Life* pParentLife);
        void Write(XmlLib::SaxWriter* writer) const;
        void LoadComplete();

        void SetLifePointer(Life* pLife);
        Life* GetLife();

        CString UIDescription(size_t buildIndex) const;
        std::string ComplexUIDescription() const;

        void BuildNowActive();      // applies all feats/enhancements etc
        void SetLevel(size_t level);

        // name
        void SetName(const std::string& name);
        const std::string& Name() const;

        // race
        void SetRace(const std::string& race);
        const std::string& Race() const;

        // alignment
        AlignmentType Alignment() const;
        void SetAlignment(AlignmentType alignment);
        void CheckClasses();

        // class selection
        void SetClass1(const std::string& ct);
        void SetClass2(const std::string& ct);
        void SetClass3(const std::string& ct);
        void SetClass(size_t level, const std::string& ct);
        bool RevokeClass(const std::string& ct);
        std::vector<size_t> BaseClassLevels(size_t level) const;
        size_t BaseClassLevels(const std::string& ct, size_t level) const;
        const std::string& Class(size_t index) const;
        std::vector<size_t> ClassLevels(size_t level) const;
        size_t ClassLevels(const std::string& ct, size_t level) const;
        std::string ClassAtLevel(size_t level) const;
        std::string BaseClassAtLevel(size_t level) const;

        // ability tome support
        void SetAbilityTome(AbilityType ability, size_t value);
        size_t AbilityTomeValue(AbilityType ability) const;
        int TomeAtLevel(AbilityType ability, size_t level) const;
        size_t StrTome() const;
        size_t DexTome() const;
        size_t ConTome() const;
        size_t IntTome() const;
        size_t WisTome() const;
        size_t ChaTome() const;

        // ability point spend
        void SetAbilityLevelUp(size_t level, AbilityType ability, bool bSuppressNotify = false);
        AbilityType AbilityLevelUp(size_t level) const;

        // stances
        void ActivateStance(const Stance& stance, StanceGroup* pStanceGroup);
        void DeactivateStance(const Stance& stance);
        void DisableStance(const Stance& stance);
        bool IsStanceActive(const std::string& name, WeaponType wt = Weapon_Unknown) const;
        void StanceSliderChanged(const std::string& sliderName, int newValue);

        // skill points
        bool IsClassSkill(SkillType skill, size_t level) const;
        void SpendSkillPoint(size_t level, SkillType skill, bool suppressUpdate = false);
        void RevokeSkillPoint(size_t level, SkillType skill, bool suppressUpdate);
        double MaxSkillForLevel(SkillType skill, size_t level) const;
        size_t SpentAtLevel(SkillType skill, size_t level) const; // level is 0 based
        double SkillAtLevel(SkillType skill, size_t level, bool includeTome) const; // level is 0 based
        void SkillsUpdated();
        void UpdateSkillPoints();
        void UpdateSkillPoints(size_t level);

        // Enhancements
        void Enhancement_ResetEnhancementTree(std::string treeName);
        void Enhancement_SetSelectedTrees(const Enhancement_SelectedTrees& trees);
        void Enhancement_SetSelectedTrees(const LegacyEnhancementSelectedTrees& trees);
        void Enhancement_TrainEnhancement(
                const std::string& treeName,
                const std::string& enhancementName,
                const std::string& selection,
                const std::vector<size_t>& cost);
        void Enhancement_RevokeEnhancement(
                const std::string& treeName,
                std::string enhancementName);
        bool IsEnhancementTrained(
                const std::string& enhancementName,
                const std::string& selection,
                TreeType type) const;

        // reaper enhancement support
        void Reaper_TrainEnhancement(
                const std::string& treeName,
                const std::string& enhancementName,
                const std::string& selection,
                const std::vector<size_t>& cost);
        void Reaper_RevokeEnhancement(
                const std::string& treeName,
                std::string enhancementName);
        void Reaper_ResetEnhancementTree(std::string treeName);

        bool IsExclusiveEnhancement(const std::string& enhancementId, const std::string& group) const;

        // checks all possible tree types
        void Enhancement_SwapTrees(const std::string& tree1, const std::string& tree2);
        void Destiny_SwapTrees(const std::string& tree1, const std::string& tree2);
        void Reaper_SwapTrees(const std::string& tree1, const std::string& tree2);

        int AvailableActionPoints(size_t level, TreeType type) const;
        size_t BonusRacialActionPoints() const;
        size_t BonusUniversalActionPoints() const;
        int APSpentInTree(const std::string& treeName) const;
        const TrainedEnhancement* IsTrained(
                const std::string& enhancementName,
                const std::string& selection) const;

        std::vector<Feat> TrainableFeats(
                const std::string& type,
                size_t level,
                const std::string& includeThisFeat) const;
        std::list<TrainedFeat> CurrentFeats(size_t level) const;
        bool IsFeatTrainable(
                size_t level,
                const std::string& type,
                const Feat& feat,
                bool includeTomes,
                bool alreadyTrained = false,
                bool bIgnoreEpicOnly = false) const;
        std::list<TrainedFeat> SpecialFeats() const;
        std::vector<FeatSlot> TrainableFeatTypeAtLevel(size_t level) const;
        const LevelTraining& LevelData(size_t level) const;
        size_t BaseAttackBonus(size_t level) const;
        size_t AbilityAtLevel(AbilityType ability, size_t level, bool includeTomes) const;
        int LevelUpsAtLevel(AbilityType ability, size_t level) const;
        void RevokeLostLevelFeats(size_t targetLevel, size_t currentLevel);

        bool IsFeatTrained(const std::string& featName) const;
        TrainedFeat GetTrainedFeat(size_t level, const std::string& type) const;
        void TrainAlternateFeat(
            const std::string& featName,
            const std::string& type,
            size_t level);
        void TrainFeat(
                const std::string& featName,
                const std::string& type,
                size_t level,
                bool autoTrained = false);
        void UpdateFeats(bool bApplyEffects);
        void UpdateFeats(size_t level, std::list<TrainedFeat>* allFeats, bool bApplyEffects);
        void AutoTrainSingleSelectionFeats();
        void VerifyTrainedFeats();
        bool IsGrantedFeat(const std::string& featName) const;
        size_t FeatTrainedCount(const std::string& featName) const;

        // spells
        void UpdateSpells();
        std::list<TrainedSpell> TrainedSpells(const std::string& ct, size_t level) const;
        std::list<TrainedSpell> FixedSpells(const std::string& ct, size_t level) const;
        void TrainSpell(const std::string& ct, size_t level, const std::string& spellName);
        void RevokeSpell(const std::string& ct, size_t level, const std::string& spellName, bool bSuppressLog);
        bool IsSpellTrained(const std::string& ct, const std::string& spellName) const;
        void ApplySpellEffects();
        void ApplySpellEffects(const TrainedSpell& ts);
        void RevokeSpellEffects(const TrainedSpell& ts);
        Spell AdditionalClassSpell(const std::string& className, const std::string& spellName) const;
        void AppendSpellListAdditions(std::list<Spell>& spells, const std::string& ct, int spellLevel);
        int BonusCasterLevels(const std::string& spellName) const;
        int BonusMaxCasterLevels(const std::string& spellName) const;

        void SetModifiedFlag(BOOL modified);
        void ApplyFeatEffects(const Feat& feat);
        void RevokeFeatEffects(const Feat& feat);

        void Destiny_SetSelectedTrees(const Destiny_SelectedTrees& trees);
        void Destiny_SetSelectedTrees(const LegacyDestinySelectedTrees& trees);
        void Destiny_TrainEnhancement(
                const std::string& treeName,
                const std::string& enhancementName,
                const std::string& selection,
                const std::vector<size_t>& cost);
        void Destiny_RevokeEnhancement(
                const std::string& treeName,
                std::string enhancementName);
        void Destiny_ResetEnhancementTree(std::string treeName);

        const SpendInTree* FindSpendInTree(const std::string& treeName) const;
        SpendInTree* FindSpendInTree(const std::string& treeName);
        void RemoveTreeSpend(const std::string& treeName);

        void SetSkillTome(SkillType skill, size_t value);
        size_t SkillTomeValue(SkillType skill, size_t level) const;

        // active point build affected
        size_t BaseAbilityValue(AbilityType ability) const;
        void SpendOnAbility(AbilityType ability);
        void RevokeSpendOnAbility(AbilityType ability);
        size_t BuildPointsPastLifeCount() const;
        size_t DetermineBuildPoints();
        void SetBuildPoints(size_t buildPoints);

        // special feats
        size_t GetSpecialFeatTrainedCount(const std::string& featName) const;
        void TrainSpecialFeat(const std::string& featName);
        void RevokeSpecialFeat(const std::string& featName, bool bOverride = false);

        // gear support
        void UpdateGearToLatestVersions();
        Item GetLatestVersionOfItem(InventorySlotType slot, Item original);
        Item Build::GetLatestVersionOfItem(InventorySlotType slot, LegacyItem original);
        void VerifyGear();
        void AddGearSet(const EquippedGear& gear);
        void DeleteGearSet(const std::string& name);
        bool DoesGearSetExist(const std::string& name) const;
        void SetActiveGearSet(const std::string& name);
        EquippedGear GetGearSet(const std::string& name) const;
        EquippedGear ActiveGearSet() const;
        void UpdateActiveGearSet(const EquippedGear& newGear);
        void SetGear(const std::string& name, InventorySlotType slot, const Item& item);
        void ClearGearInSlot(const std::string& name, InventorySlotType slot);
        void SetNumFiligrees(size_t count);
        void SetGearSetSnapshot(const std::string& setName);
        int SnapshotAbilityValue(AbilityType at) const;

        const std::list<StackTracking>& ActiveSets() const;
        int SetBonusCount(const std::string& setName) const;

        // weapon groups
        void SetupDefaultWeaponGroups();
        bool IsWeaponInGroup(const std::string& group, WeaponType wt) const;
        void AddWeaponToGroup(const Effect& effect);
        void RemoveFromWeaponGroup(const Effect& effect);
        void MergeGroups(const Effect& effect);
        void RemoveMergeGroup(const Effect& effect);
        void DumpWeaponGroups() const;
        WeaponType MainHandWeapon() const;
        WeaponType OffhandWeapon() const;

        // guild buffs
        void ApplyGuildBuffs(bool bApply);
        void GuildLevelChange();
        void ApplySelfAndPartyBuffs();

        // self and party buffs
        void NotifyOptionalBuff(const std::string& name);
        void RevokeOptionalBuff(const std::string& name);

        // notes
        void SetNotes(const std::string& newNotes);

        // quests
        void SetQuestsCompletions(const std::list<CompletedQuest>& quests);

        // attack chains
        void AddAttackChain(const AttackChain& ac);
        void DeleteAttackChain(const std::string& acName);
        void UpdateAttackChain(const std::string& acName, const AttackChain& ac);
        const AttackChain& GetActiveAttackChain() const;

        bool operator<(const Build& other) const;

    protected:
        XmlLib::SaxContentElementInterface* StartElement(
                const XmlLib::SaxString& name,
                const XmlLib::SaxAttributes& attributes);

        virtual void EndElement();

        #define Build_PROPERTIES(_) \
                DL_SIMPLE(_, size_t, Level, BUILD_START_LEVEL) \
                DL_STRING(_, Class1) \
                DL_STRING(_, Class2) \
                DL_STRING(_, Class3) \
                DL_OBJECT(_, AbilitySpend, BuildPoints) \
                DL_OBJECT_LIST(_, LevelTraining, Levels) \
                DL_OBJECT_VECTOR(_, TrainedSpell, TrainedSpells) \
                DL_OBJECT(_, ActiveStances, Stances) \
                DL_OBJECT(_, Destiny_SelectedTrees, DestinySelectedTrees) \
                DL_OBJECT(_, Enhancement_SelectedTrees, EnhancementSelectedTrees) \
                DL_OBJECT(_, Reaper_SelectedTrees, ReaperSelectedTrees) \
                DL_OBJECT_LIST(_, EnhancementSpendInTree, EnhancementTreeSpend) \
                DL_OBJECT_LIST(_, ReaperSpendInTree, ReaperTreeSpend) \
                DL_OBJECT_LIST(_, DestinySpendInTree, DestinyTreeSpend) \
                DL_STRING(_, ActiveAttackChain) \
                DL_OBJECT_LIST(_, AttackChain, AttackChains) \
                DL_STRING(_, ActiveGear) \
                DL_OBJECT_LIST(_, EquippedGear, GearSetups) \
                DL_OPTIONAL_STRING(_, GearSetSnapshot) \
                DL_OBJECT(_, FeatsListObject, FavorFeats) \
                DL_OBJECT_LIST(_, CompletedQuest, CompletedQuests) \
                DL_STRING(_, Notes) \
                DL_ENUM(_, AbilityType, Level4, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level8, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level12, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level16, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level20, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level24, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level28, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level32, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level36, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level40, Ability_Strength, abilityTypeMap)

        DL_DECLARE_ACCESS(Build_PROPERTIES)
        DL_DECLARE_VARIABLES(Build_PROPERTIES)
    private:
        void NotifyBuildLevelChanged();
        void NotifyClassChoiceChanged();
        void NotifyClassChanged(
                const std::string& classFrom,
                const std::string& classTo,
                size_t level);
        void NotifyFeatTrained(const std::string& featName);
        void NotifyFeatRevoked(const std::string& featName);
        void NotifyFeatEffectApplied(const Effect& effect);
        void NotifyFeatEffectRevoked(const Effect& effect);
        void NotifyStanceActivated(const std::string& name);
        void NotifyStanceDeactivated(const std::string& name);
        void NotifyStanceDisabled(const std::string& name);
        void NotifySliderChanged(const std::string& sliderName, int newValue);
        void NotifySkillSpendChanged(size_t level, SkillType skill);
        void NotifyAbilityValueChanged(AbilityType ability);
        void NotifyEnhancementTrained(const EnhancementItemParams& item);
        void NotifyEnhancementRevoked(const EnhancementItemParams& item);
        void NotifyEnhancementEffectApplied(const Effect& effect);
        void NotifyEnhancementEffectRevoked(const Effect& effect);
        void NotifyEnhancementTreeOrderChanged();
        void NotifyActionPointsChanged();
        void NotifyNewStance(const Stance& stance);
        void NotifyRevokeStance(const Stance& stance);
        void NotifyBuildAutomaticFeatsChanged(size_t level);
        void NotifyNewDC(const DC& dc);
        void NotifyRevokeDC(const DC& dc);
        void NotifyItemEffect(const std::string& itemName, Effect effect, InventorySlotType ist);
        void NotifyItemEffectRevoked(const std::string& itemName, Effect effect, InventorySlotType ist);
        void NotifyItemWeaponEffect(const std::string& itemName, Effect effect, WeaponType wt, InventorySlotType ist);
        void NotifyItemWeaponEffectRevoked(const std::string& itemName, Effect effect, WeaponType wt, InventorySlotType ist);
        void NotifyGearChanged(InventorySlotType slot);
        void NotifyNewSetBonusStack(const SetBonus& setBonus);
        void NotifyRevokeSetBonusStack(const SetBonus& setBonus);
        void NotifyNewAttack(const Attack& attack);
        void NotifyRevokeAttack(const Attack& attack);

        std::list<TrainedFeat> AutomaticFeats(
            size_t level,
            const std::list<TrainedFeat>& currentFeats) const;
        void ApplyEnhancementEffects(
                const std::string& treeName,
                const std::string& enhancementName,
                const std::string& selection,
                size_t rank);
        void RevokeEnhancementEffects(
                const std::string& treeName,
                const std::string& enhancementName,
                const std::string& selection,
                size_t rank);
        void ApplyGearEffects();     // apply effects from equipped gear
        void RevokeGearEffects();     // clear effects from equipped gear
        void ApplyFiligree(const std::string& filigree, size_t si, bool bRare, bool bArtifact);
        void RevokeFiligree(const std::string& filigree, size_t si, bool bRare, bool bArtifact);
        void ApplySetBonus(const std::string& set, const std::string& name);
        void RevokeSetBonus(const std::string& set, const std::string& name);
        void ApplyItem(const Item& item, InventorySlotType ist);
        void RevokeItem(const Item& item, InventorySlotType ist);
        void ApplyWeaponEffects(const Item& item, InventorySlotType ist);
        void RevokeWeaponEffects(const Item& item, InventorySlotType ist);
        void ApplyArmorEffects(const Item& item);
        void RevokeArmorEffects(const Item& item);
        void ApplyAugment(const Augment& augment, const ItemAugment& itemAugment, const std::string& itemName, size_t itemLevel, WeaponType wt, InventorySlotType ist);
        void RevokeAugment(const Augment& augment, const ItemAugment& itemAugment, const std::string& itemName, size_t itemLevel, WeaponType wt, InventorySlotType ist);
        void ApplyItemEffect(const std::string& name, const Effect& effect, InventorySlotType ist);
        void RevokeItemEffect(const std::string& name, const Effect& effect, InventorySlotType ist);
        size_t AddSetBonusStack(const std::string& set);
        size_t RevokeSetBonusStack(const std::string& set);
        void VerifySpecialFeats();
        void AddSpellListAddition(const Effect& effect);
        void RevokeSpellListAddition(const Effect& effect);
        bool IsSpellInSpellListAdditionList(const std::string& ct, size_t spellLevel, const std::string& spellName) const;
        void UpdateCachedClassLevels();
        void AddSpellCasterLevelEffect(const Effect& effect);
        void AddSpellMaxCasterLevelEffect(const Effect& effect);
        void RemoveSpellCasterLevelEffect(const Effect& effect);
        void RemoveSpellMaxCasterLevelEffect(const Effect& effect);
        void AddEffectToList(std::list<Effect>& list, const Effect& effect);
        void RemoveEffectFromList(std::list<Effect>& list, const Effect& effect);
        void UpdateGreensteelStances();
        void ApplyAllAttacks(const std::list<Attack>& attacks);
        void RevokeAllAttacks(const std::list<Attack>& attacks);

        // BreakdownObserver
        virtual void UpdateTotalChanged(BreakdownItem* pBI, BreakdownType bt) override;

        Life* m_pLife;
        int m_racialTreeSpend;
        int m_universalTreeSpend;
        int m_classTreeSpend;
        int m_destinyTreeSpend;
        size_t m_previousGuildLevel;
        std::list<ExclusionGroup> m_exclusiveEnhancements;
        std::list<StackTracking> m_setBonusStacks;
        std::list<WeaponGroup> m_weaponGroups;
        std::vector<SpellListAddition> m_additionalSpells;
        std::map<std::string, int> m_cachedClassLevels[MAX_GAME_LEVEL]; // each entry has a max of 5 entries, "class name" and count
        std::list<Effect> m_spellCasterLevels;
        std::list<Effect> m_spellMaxCasterLevels;
        bool m_bSwitchingBuildsOrGear;

        friend class CStancesPane;
        friend class CStanceButton;
        friend class CDDOBuilderApp;
        friend class CGrantedFeatsPane;
};
