// FeatSlot.h
//
// An XML object wrapper that holds information on a defined feat slot
#pragma once
#include "XmlLib\DLMacros.h"

class FeatSlot :
    public XmlLib::SaxContentElement
{
    public:
        FeatSlot(void);
        FeatSlot(
                size_t level,
                const std::string& featType,
                bool bSingular,
                bool bAutoPopulate);
        void Write(XmlLib::SaxWriter * writer) const;
        bool operator==(const FeatSlot& other) const;

        void VerifyObject() const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define FeatSlot_PROPERTIES(_) \
                DL_SIMPLE(_, size_t, Level, 0) \
                DL_STRING(_, FeatType) \
                DL_FLAG(_, Singular) \
                DL_FLAG(_, AutoPopulate) \
                DL_STRING_LIST(_, FeatUpdateList)

        DL_DECLARE_ACCESS(FeatSlot_PROPERTIES)
        DL_DECLARE_VARIABLES(FeatSlot_PROPERTIES)
};
