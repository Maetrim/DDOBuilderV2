// Bonus.h
//
// An XML object wrapper that holds information on defined bonus types and
// their stacking rules
#pragma once
#include "XmlLib\DLMacros.h"

enum StackingType
{
    StackingType_Unknown = 0,
    StackingType_HighestOnly,
    StackingType_Always,
};
const XmlLib::enumMapEntry<StackingType> stackingTypeMap[] =
{
    {StackingType_Unknown, L"Unknown"},
    {StackingType_HighestOnly, L"Highest Only"},
    {StackingType_Always, L"Always"},
    {StackingType(0), NULL}
};

class Bonus :
    public XmlLib::SaxContentElement
{
    public:
        Bonus();
        void Write(XmlLib::SaxWriter * writer) const;

        void VerifyObject() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Bonus_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_ENUM(_, StackingType, Stacking, StackingType_Unknown, stackingTypeMap)
        DL_DECLARE_ACCESS(Bonus_PROPERTIES)
        DL_DECLARE_VARIABLES(Bonus_PROPERTIES)
};
