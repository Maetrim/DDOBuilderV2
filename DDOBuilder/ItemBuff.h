// ItemBuff.h
//
// An XML object wrapper that holds information on an ItemBuff that can be equipped
#pragma once
#include "XmlLib\DLMacros.h"

class ItemBuff :
    public XmlLib::SaxContentElement
{
    public:
        ItemBuff(void);
        ItemBuff(const XmlLib::SaxString & objectName);
        void Write(XmlLib::SaxWriter * writer) const;

        void VerifyObject() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define ItemBuff_PROPERTIES(_) \
                DL_STRING(_, Type) \
                DL_STRING(_, Bonus) \
                DL_STRING_LIST(_, Item) \
                DL_OPTIONAL_VECTOR(_, double, Values)

        DL_DECLARE_ACCESS(ItemBuff_PROPERTIES)
        DL_DECLARE_VARIABLES(ItemBuff_PROPERTIES)
};
