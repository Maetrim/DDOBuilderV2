// RequiresOneOf.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "WeaponTypes.h"
#include "InventorySlotTypes.h"

class Build;
class Requirement;

class RequiresOneOf :
    public XmlLib::SaxContentElement
{
    public:
        RequiresOneOf(void);
        void Write(XmlLib::SaxWriter * writer) const;

        bool Met(
                const Build & build,
                size_t level,
                bool includeTomes,
                InventorySlotType slot,
                WeaponType wtMainHand,
                WeaponType wtOffhand) const;
        bool CanTrainEnhancement(
                const Build& build,
                size_t trainedRanks) const;
        bool MetEnhancements(const Build& build, size_t trainedRanks) const;
        bool MetHardRequirements(
                const Build & build,
                size_t level,
                bool includeTomes) const;

        void CreateRequirementStrings(
                const Build& build,
                size_t level,
                bool includeTomes,
                std::vector<CString>* requirementsArray,
                std::vector<bool>* metArray) const;

        void AddRequirement(const Requirement& req);
        bool RequiresAnEnhancement() const;

        bool operator==(const RequiresOneOf& other) const;

        bool VerifyObject(
                std::stringstream * ss) const;

        void UpdateEnhancementRequirements(const std::string& prepend);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define RequiresOneOf_PROPERTIES(_) \
                DL_OPTIONAL_STRING(_, DisplayDescription) \
                DL_OPTIONAL_SIMPLE(_, size_t, Count, 1) \
                DL_OBJECT_LIST(_, Requirement, Requirements)

        DL_DECLARE_ACCESS(RequiresOneOf_PROPERTIES)
        DL_DECLARE_VARIABLES(RequiresOneOf_PROPERTIES)
};
