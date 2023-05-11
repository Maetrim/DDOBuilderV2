// AutomaticAcquision.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "RequirementsBase.h"

class AutomaticAcquision :
    public RequirementsBase
{
    public:
        AutomaticAcquision(void);
        void Write(XmlLib::SaxWriter* writer) const;

        void SetRequirement(const Requirement& requirement);

    protected:
        XmlLib::SaxContentElementInterface* StartElement(
            const XmlLib::SaxString& name,
            const XmlLib::SaxAttributes& attributes);

        virtual void EndElement();

        #define AutomaticAcquision_PROPERTIES(_) \
                DL_FLAG(_, IgnoreRequirements)

        DL_DECLARE_ACCESS(AutomaticAcquision_PROPERTIES)
        DL_DECLARE_VARIABLES(AutomaticAcquision_PROPERTIES)
};
