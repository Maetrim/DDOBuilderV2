// SetBonus.h
//
// An XML object wrapper that holds information on an affect that a SetBonus has.
#pragma once
#include "XmlLib\DLMacros.h"
#include "SetBonusBuff.h"

class SetBonus :
    public XmlLib::SaxContentElement
{
    public:
        SetBonus(void);
        void Write(XmlLib::SaxWriter * writer) const;

        std::list<Effect> ActiveEffects(size_t stacks) const;

        void SetFilename(const std::string&) {};
        void VerifyObject() const;
        bool operator==(const SetBonus& other) const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define SetBonus_PROPERTIES(_) \
                DL_STRING(_, Type) \
                DL_STRING(_, Icon) \
                DL_FLAG(_, IgnoreForParse) \
                DL_OBJECT_LIST(_, SetBonusBuff, Buffs)

        DL_DECLARE_ACCESS(SetBonus_PROPERTIES)
        DL_DECLARE_VARIABLES(SetBonus_PROPERTIES)
};
