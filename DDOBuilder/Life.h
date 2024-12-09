// Life.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "ObserverSubject.h"
#include "AbilityTypes.h"
#include "AlignmentTypes.h"
#include "Build.h"
#include "Feat.h"
#include "SkillTomes.h"

class Character;
class Life;

class LifeObserver :
    public Observer<Life>
{
    public:
        virtual void UpdateAbilityTomeChanged(Life*, AbilityType) {};
        virtual void UpdateLifeFeatTrained(Life*, const std::string&) {};
        virtual void UpdateLifeFeatRevoked(Life*, const std::string&) {};
        virtual void UpdateAlignmentChanged(Life*, AlignmentType) {};
        virtual void UpdateRaceChanged(Life*, const std::string&) {};
        virtual void UpdateActionPointsChanged(Life*) {};
        virtual void UpdateSkillTomeChanged(Life*, SkillType) {};
};

class Life :
    public XmlLib::SaxContentElement,
    public Subject<LifeObserver>
{
    public:
        Life(Character* pCharacter);
        void Write(XmlLib::SaxWriter* writer) const;
        void LoadComplete();

        CString UIDescription(size_t index) const;
        void SetName(const std::string& name);

        void SetRace(const std::string& race);
        void SetAlignment(AlignmentType alignment);

        // ability tomes
        size_t StrTome() const;
        size_t DexTome() const;
        size_t ConTome() const;
        size_t IntTome() const;
        size_t WisTome() const;
        size_t ChaTome() const;
        void SetAbilityTome(AbilityType ability, size_t value);
        size_t AbilityTomeValue(AbilityType ability) const;
        int TomeAtLevel(AbilityType ability, size_t level) const;

        // build support
        const Build& GetBuild(size_t buildIndex) const;
        Build* GetBuildPointer(size_t buildIndex);
        const Build* GetBuildPointer(size_t buildIndex) const;
        size_t AddBuild(size_t buildIndex);
        void DeleteBuild(size_t buildIndex);
        void SetBuildLevel(size_t buildIndex, size_t level);

        void UpdateSkillPoints();
        void VerifyTrainedFeats();
        void SetModifiedFlag(BOOL modified);

        int BonusRacialActionPoints() const;
        int BonusUniversalActionPoints() const;
        int BonusDestinyActionPoints() const;

        // skill tomes
        void SetSkillTome(SkillType skill, size_t value);
        size_t SkillTomeValue(SkillType skill, size_t level) const;

        // special feats
        size_t GetSpecialFeatTrainedCount(const std::string& featName) const;
        void TrainSpecialFeat(const std::string& featName);
        void RevokeSpecialFeat(const std::string& featName);
        const FeatsListObject& SpecialFeats() const;

        // guild buffs
        bool ApplyGuildBuffs() const;
        size_t GuildLevel() const;

        // self and party buffs
        void EnableSelfAndPartyBuff(const std::string& name);
        void DisableSelfAndPartyBuff(const std::string& name);

        void UpdateBuildLifePointers(Character* pCharacter);

        void NotifyActiveBuildChanged();
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Life_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Race) \
                DL_ENUM(_, AlignmentType, Alignment, Alignment_LawfulGood, alignmentTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level4, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level8, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level12, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level16, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level20, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level24, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level28, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level32, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level36, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Level40, Ability_Strength, abilityTypeMap) \
                DL_THIS_OBJECT_LIST(_, Build, Builds) \
                DL_STRING_LIST(_, SelfAndPartyBuffs) \
                DL_FLAG(_, TreeCollapsed)

        DL_DECLARE_ACCESS(Life_PROPERTIES)
        DL_DECLARE_VARIABLES(Life_PROPERTIES)

    private:
        void NotifyAbilityTomeChanged(AbilityType ability);
        void NotifyLifeFeatTrained(const std::string& featName);
        void NotifyLifeFeatRevoked(const std::string& featName);
        void NotifyAlignmentChanged(AlignmentType alignment);
        void NotifyRaceChanged(const std::string& race);
        void NotifyActionPointsChanged();
        void NotifySkillTomeChanged(SkillType skill);

        void ApplyFeatEffects(const Feat& feat);
        void RevokeFeatEffects(const Feat& feat);
        void CountBonusRacialAP();
        void CountBonusUniversalAP();

        Character* m_pCharacter;
        int m_bonusRacialActionPoints;
        int m_bonusUniversalActionPoints;
        int m_bonusDestinyActionPoints;
        // backwards compatibility (moved to Character from Life)
        FeatsListObject m_SpecialFeats;
        size_t m_StrTome;
        size_t m_DexTome;
        size_t m_ConTome;
        size_t m_IntTome;
        size_t m_WisTome;
        size_t m_ChaTome;
        SkillTomes m_SkillTomes;

        friend class Build;
        friend class CDDOBuilderApp;
};
