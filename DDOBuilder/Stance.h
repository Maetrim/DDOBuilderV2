// Stance.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "Requirements.h"

// Requirements must be present if we have the AutoControlled flag
class Stance :
    public XmlLib::SaxContentElement
{
    public:
        Stance(void);
        explicit Stance(
                const std::string& name,
                const std::string& icon,
                const std::string& description,
                const std::string& group,
                bool bAutoControlled = false);
        void Write(XmlLib::SaxWriter * writer) const;

        bool VerifyObject(std::stringstream * ss) const;
        bool operator==(const Stance & other) const;

        void AddRequirement(const Requirement& requirement);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Stance_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Icon) \
                DL_STRING(_, Description) \
                DL_OPTIONAL_STRING(_, Group) \
                DL_FLAG(_, AutoControlled) \
                DL_OPTIONAL_OBJECT(_, Requirements, ActiveRequirements) \
                DL_STRING_LIST(_, IncompatibleStance)

        DL_DECLARE_ACCESS(Stance_PROPERTIES)
        DL_DECLARE_VARIABLES(Stance_PROPERTIES)
};
