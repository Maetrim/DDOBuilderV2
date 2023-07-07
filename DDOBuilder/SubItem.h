// SubItem.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "Requirements.h"

// Requirements must be present if we have the AutoControlled flag
class SubItem :
    public XmlLib::SaxContentElement
{
    public:
        SubItem(void);
        void Write(XmlLib::SaxWriter * writer) const;

        bool VerifyObject(std::stringstream * ss) const;
        bool operator==(const SubItem & other) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define SubItem_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Icon) \
                DL_STRING(_, Description)

        DL_DECLARE_ACCESS(SubItem_PROPERTIES)
        DL_DECLARE_VARIABLES(SubItem_PROPERTIES)
};
