// Requirement.h
//
#pragma once
#include "XmlLib\DLMacros.h"

#include "AbilityTypes.h"
#include "AlignmentTypes.h"
#include "RequirementTypes.h"
#include "SkillTypes.h"
#include "InventorySlotTypes.h"
#include "WeaponTypes.h"

class Build;
class Feat;
class TrainedFeat;

class Requirement :
    public XmlLib::SaxContentElement
{
    public:
        Requirement(void);
        Requirement(RequirementType type);
        Requirement(RequirementType type, const std::string& item);
        Requirement(RequirementType type, const std::string& item, size_t value);
        Requirement(RequirementType type, const std::string& item1, const std::string& item2);

        void Write(XmlLib::SaxWriter * writer) const;

        bool Met(
                const Build & build,
                size_t level,
                bool includeTomes,
                InventorySlotType slot = Inventory_Unknown,
                WeaponType wtMainHand = Weapon_Unknown,
                WeaponType wtOffHand = Weapon_Unknown) const;
        bool CanTrainEnhancement(
                const Build& build,
                size_t trainedRanks) const;
        bool MetHardRequirements(
                const Build & build,
                size_t level,
                bool includeTomes) const;
        bool MetEnhancements(
                const Build & build,
                size_t trainedRanks) const;

        void CreateRequirementStrings(
                const Build& build,
                size_t level,
                bool includeTomes,
                std::vector<CString>* requirementsArray,
                std::vector<bool>* metArray) const;

        bool RequiresEnhancement(const std::string& name) const;
        bool RequiresAnEnhancement() const;

        bool operator==(const Requirement& other) const;

        bool VerifyObject(
                std::stringstream * ss) const;

        void UpdateEnhancementRequirements(const std::string& prepend);

    private:
        bool EvaluateAbility(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateAbilityGreaterCondition(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateAlignment(const Build& build) const;
        bool EvaluateAlignmentType(const Build& build) const;
        bool EvaluateBAB(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateBaseClass(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateBaseClassAtLevel(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateBaseClassMinLevel(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateClass(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateClassAtLevel(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateClassMinLevel(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateEnhancement(const Build& build) const;
        bool EvaluateExclusive(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateFeat(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateFeatAnySource(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateWeaponGroupMember(const Build& build, WeaponType wtMain, WeaponType wtOffhand, bool bMainhand) const;
        bool EvaluateWeaponTypesEquipped(const Build& build, WeaponType wtMain, WeaponType wtOffhand) const;
        bool EvaluateItemInSlot(const Build& build) const;
        bool EvaluateLevel(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateNotConstruct(const Build& build) const;
        bool EvaluateRace(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateRaceConstruct(const Build& build) const;
        bool EvaluateSkill(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateSpecificLevel(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateStance(const Build& build, size_t level, bool includeTomes) const;
        bool EvaluateStartingWorld(const Build& build) const;
        bool EvaluateMaterialType(const Build& build) const;
        bool EvaluateItemSlot(InventorySlotType slot) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Requirement_PROPERTIES(_) \
                DL_ENUM(_, RequirementType, Type, Requirement_Unknown, requirementTypeMap) \
                DL_STRING_LIST(_, Item) \
                DL_OPTIONAL_SIMPLE(_, size_t, Value, 0)

        DL_DECLARE_ACCESS(Requirement_PROPERTIES)
        DL_DECLARE_VARIABLES(Requirement_PROPERTIES)
};
