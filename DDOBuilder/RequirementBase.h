// RequirementBlock.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "Requirement.h"
#include "RequiresNoneOf.h"
#include "RequiresOneOf.h"

class Build;
class Feat;
class TrainedFeat;

// Note that the reading and writing of the RequirementBlockBase_PROPERTIES
// is handled by classes that inherit from us

class RequirementBlock :
    public XmlLib::SaxContentElement
{
    public:
        RequirementBlock(const XmlLib::SaxString& elementName, unsigned verCurrent);
        void Write(XmlLib::SaxWriter* writer) const;

        bool Met(
                const Build & build,
                size_t level,
                bool includeTomes) const;

        void CreateRequirementStrings(
                const Build & build,
                size_t level,
                bool includeTomes,
                std::vector<CString> * requirements,
                std::vector<bool> * met) const;

        bool operator==(const RequirementBlock& other) const;

        bool VerifyObject(
                std::stringstream * ss) const;
    protected:
        XmlLib::SaxContentElementInterface* StartElement(
            const XmlLib::SaxString& name,
            const XmlLib::SaxAttributes& attributes);

        virtual void EndElement();

        #define RequirementBlock_PROPERTIES(_) \
                DL_OBJECT_LIST(_, Requirement, Requires) \
                DL_OBJECT_LIST(_, RequiresOneOf, OneOf) \
                DL_OPTIONAL_OBJECT(_, RequiresNoneOf, NoneOf)

        DL_DECLARE_ACCESS(RequirementBlock_PROPERTIES)
        DL_DECLARE_VARIABLES(RequirementBlock_PROPERTIES)

        friend class RequirementsBase;
};
