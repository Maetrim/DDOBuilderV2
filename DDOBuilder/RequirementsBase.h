// RequirementsBase.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "Requirement.h"
#include "RequiresNoneOf.h"
#include "RequiresOneOf.h"

class Build;
class Feat;
class TrainedFeat;
class InfoTipItem;

class RequirementsBase :
    public XmlLib::SaxContentElement
{
    public:
        RequirementsBase(const XmlLib::SaxString& elementName, unsigned verCurrent);
        void Write(XmlLib::SaxWriter* writer) const;

        bool Met(
                const Build & build,
                size_t level,
                bool includeTomes,
                WeaponType wtMainHand,
                WeaponType wtOffhand) const;
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
                const Build & build,
                size_t level,
                bool includeTomes,
                std::vector<CString> * requirements,
                std::vector<bool> * met) const;

        void AddItemRequirements(std::vector<InfoTipItem*>& infoTip) const;

        void SetRequirement(const Requirement& r);
        void GetExclusionGroup(std::string * enhancementId, std::string * group) const;
        bool RequiresEnhancement(const std::string& name) const;
        void AddRequirement(const Requirement& req);
        void AddRequiresOneOf(const RequiresOneOf& roo);
        void AddRequiresNoneOf(const RequiresNoneOf& rno);

        bool operator==(const RequirementsBase& other) const;

        bool VerifyObject(
                std::stringstream * ss) const;
    protected:
        XmlLib::SaxContentElementInterface* StartElement(
            const XmlLib::SaxString& name,
            const XmlLib::SaxAttributes& attributes);

        virtual void EndElement();

        #define RequirementsBase_PROPERTIES(_) \
                DL_OBJECT_LIST(_, Requirement, Requires) \
                DL_OBJECT_LIST(_, RequiresOneOf, OneOf) \
                DL_OBJECT_LIST(_, RequiresNoneOf, NoneOf) \
                DL_FLAG(_, IgnoreRequirements)

        DL_DECLARE_ACCESS(RequirementsBase_PROPERTIES)
        DL_DECLARE_VARIABLES(RequirementsBase_PROPERTIES)
};
