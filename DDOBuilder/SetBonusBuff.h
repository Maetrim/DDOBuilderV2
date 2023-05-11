// SetBonusBuffBuff.h
//
// An XML object wrapper that holds information on an affect that a SetBonusBuff has.
#pragma once
#include "XmlLib\DLMacros.h"
#include "Effect.h"

class SetBonusBuff :
    public XmlLib::SaxContentElement
{
    public:
        SetBonusBuff(void);
        void Write(XmlLib::SaxWriter * writer) const;

        void VerifyObject() const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define SetBonusBuff_PROPERTIES(_) \
                DL_SIMPLE(_, size_t, EquippedCount, 0) \
                DL_STRING(_, Description) \
                DL_OBJECT_LIST(_, Effect, Effects)

        DL_DECLARE_ACCESS(SetBonusBuff_PROPERTIES)
        DL_DECLARE_VARIABLES(SetBonusBuff_PROPERTIES)
};
