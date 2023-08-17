// EffectCallbackItem.h
//
#pragma once
#include "AbilityTypes.h"
#include "InventorySlotTypes.h"
#include "SkillTypes.h"
#include <string>

class Build;
class Character;
class Effect;
struct EnhancementItemParams;
class Life;

class EffectCallbackItem
{
    public:
        EffectCallbackItem() {};
        virtual ~EffectCallbackItem() {};

        virtual void AbilityTomeChanged(Life*, AbilityType) = 0;
        virtual void AbilityValueChanged(Build*, AbilityType) = 0;
        virtual void AbilityTotalChanged(Build*, AbilityType) = 0;
        virtual void BuildLevelChanged(Build*) = 0;
        virtual void ClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) = 0;
        virtual void FeatTrained(Build*, const std::string&) = 0;
        virtual void FeatRevoked(Build*, const std::string&) = 0;
        virtual void RaceChanged(Life*, const std::string& race) = 0;
        virtual void SkillTomeChanged(Life*, SkillType) = 0;
        virtual void SkillSpendChanged(Build*, size_t, SkillType) = 0;

        virtual void FeatEffectApplied(Build*, const Effect& effect) = 0;
        virtual void FeatEffectRevoked(Build*, const Effect& effect) = 0;
        virtual void ItemEffectApplied(Build*, const Effect& effect) = 0;
        virtual void ItemEffectRevoked(Build*, const Effect& effect) = 0;
        virtual void ItemEffectApplied(Build*, const Effect& effect, WeaponType wt) = 0;
        virtual void ItemEffectRevoked(Build*, const Effect& effect, WeaponType wt) = 0;
        virtual void EnhancementTrained(Build*, const EnhancementItemParams& item) = 0;
        virtual void EnhancementRevoked(Build*, const EnhancementItemParams& item) = 0;
        virtual void EnhancementEffectApplied(Build*, const Effect& effect) = 0;
        virtual void EnhancementEffectRevoked(Build*, const Effect& effect) = 0;
        virtual void StanceActivated(Build*, const std::string& stanceName) = 0;
        virtual void StanceDeactivated(Build*, const std::string& stanceName) = 0;
        virtual void SliderChanged(Build*, const std::string& sliderName, int newValue) = 0;
        virtual void GearChanged(Build*, InventorySlotType slot) = 0;
};
