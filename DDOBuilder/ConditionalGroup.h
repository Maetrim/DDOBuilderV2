// ConditionalGroup.h
//
// An XML object wrapper that holds information on an ConditionalGroup that can be equipped
#pragma once
#include "XmlLib\DLMacros.h"
#include "Effect.h"
#include "Stance.h"
#include "Requirements.h"

class ConditionalGroup :
    public XmlLib::SaxContentElement
{
    public:
        ConditionalGroup(void);
        ConditionalGroup(const XmlLib::SaxString & objectName);
        void Write(XmlLib::SaxWriter * writer) const;

        bool VerifyObject(std::stringstream* ss) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define ConditionalGroup_PROPERTIES(_) \
                DL_STRING_LIST(_, Group) \
                DL_OPTIONAL_OBJECT(_, Requirements, RequirementsToUse)

        DL_DECLARE_ACCESS(ConditionalGroup_PROPERTIES)
        DL_DECLARE_VARIABLES(ConditionalGroup_PROPERTIES)
};
