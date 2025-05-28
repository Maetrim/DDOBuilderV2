// RequiresNoneOf.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "WeaponTypes.h"
#include "InventorySlotTypes.h"

class Build;
class Requirement;

class RequiresNoneOf :
    public XmlLib::SaxContentElement
{
    public:
        RequiresNoneOf(void);
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

        bool operator==(const RequiresNoneOf& other) const;

        bool VerifyObject(
                std::stringstream * ss) const;

        void UpdateEnhancementRequirements(const std::string& prepend);
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define RequiresNoneOf_PROPERTIES(_) \
                DL_OPTIONAL_STRING(_, DisplayDescription) \
                DL_OBJECT_LIST(_, Requirement, Requirements)

        DL_DECLARE_ACCESS(RequiresNoneOf_PROPERTIES)
        DL_DECLARE_VARIABLES(RequiresNoneOf_PROPERTIES)
};
