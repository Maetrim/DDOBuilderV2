// LegacyCharacter.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "AbilityTypes.h"
#include "AbilitySpend.h"
#include "ActiveStances.h"
#include "AlignmentTypes.h"
#include "DestinySpendInTree.h"
#include "EnhancementSpendInTree.h"
#include "LegacyDestinySelectedTrees.h"
#include "LegacyEnhancementSelectedTrees.h"
#include "FeatsListObject.h"
#include "LegacyEquippedGear.h"
#include "LevelTraining.h"
#include "ReaperSpendInTree.h"
#include "SkillTomes.h"
#include "TrainedSpell.h"

class LegacyCharacter :
    public XmlLib::SaxContentElement
{
    public:
        LegacyCharacter();

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define LegacyCharacter_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_ENUM(_, AlignmentType, Alignment, Alignment_LawfulGood, alignmentTypeMap) \
                DL_STRING(_, Race) \
                DL_OBJECT(_, AbilitySpend, BuildPoints) \
                DL_SIMPLE(_, size_t, GuildLevel, 0) \
                DL_FLAG(_, ApplyGuildBuffs) \
                DL_SIMPLE(_, size_t, StrTome, 0) \
                DL_SIMPLE(_, size_t, DexTome, 0) \
                DL_SIMPLE(_, size_t, ConTome, 0) \
                DL_SIMPLE(_, size_t, IntTome, 0) \
                DL_SIMPLE(_, size_t, WisTome, 0) \
                DL_SIMPLE(_, size_t, ChaTome, 0) \
                DL_OBJECT(_, SkillTomes, Tomes) \
                DL_OBJECT(_, FeatsListObject, SpecialFeats) \
                DL_OBJECT(_, ActiveStances, Stances) \
                DL_ENUM(_, AbilityType, Level4, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level8, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level12, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level16, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level20, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level24, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level28, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level32, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level36, Ability_Strength, abilityTypeMap) \
                DL_ENUM(_, AbilityType, Level40, Ability_Strength, abilityTypeMap) \
                DL_OPTIONAL_STRING(_, Class1) \
                DL_OPTIONAL_STRING(_, Class2) \
                DL_OPTIONAL_STRING(_, Class3) \
                DL_OBJECT(_, LegacyEnhancementSelectedTrees, SelectedTrees) \
                DL_OBJECT(_, LegacyDestinySelectedTrees, DestinyTrees) \
                DL_OPTIONAL_STRING(_, Tier5Tree) \
                DL_OPTIONAL_STRING(_, U51Destiny_Tier5Tree) \
                DL_OBJECT_LIST(_, LevelTraining, Levels) \
                DL_OBJECT_VECTOR(_, TrainedSpell, TrainedSpells) \
                DL_OBJECT_LIST(_, EnhancementSpendInTree, EnhancementTreeSpend) \
                DL_OBJECT_LIST(_, ReaperSpendInTree, ReaperTreeSpend) \
                DL_OBJECT_LIST(_, DestinySpendInTree, DestinyTreeSpend) \
                DL_STRING(_, ActiveGear) \
                DL_OBJECT_LIST(_, LegacyEquippedGear, GearSetups) \
                DL_STRING_LIST(_, SelfAndPartyBuffs) \
                DL_STRING(_, Notes)

        DL_DECLARE_ACCESS(LegacyCharacter_PROPERTIES)
        DL_DECLARE_VARIABLES(LegacyCharacter_PROPERTIES)
};
