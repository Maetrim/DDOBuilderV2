// Buff.h
//
// An XML object wrapper that holds information on an Buff that can be equipped
#pragma once
#include "XmlLib\DLMacros.h"
#include "Effect.h"
#include "Stance.h"
#include "Requirements.h"

class Buff :
    public XmlLib::SaxContentElement
{
    public:
        Buff(void);
        Buff(const XmlLib::SaxString & objectName);
        void Write(XmlLib::SaxWriter * writer) const;

        CString MakeDescription() const;
        void UpdatedEffects(std::list<Effect>* effects) const;

        void VerifyObject() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Buff_PROPERTIES(_) \
                DL_STRING(_, Type) \
                DL_FLAG(_, ApplyToWeaponOnly) \
                DL_STRING_LIST(_, DisplayText) \
                DL_OPTIONAL_STRING(_, Ignore) \
                DL_OPTIONAL_SIMPLE(_, double, Value1, 0.0) \
                DL_OPTIONAL_SIMPLE(_, double, Value2, 0.0) \
                DL_OPTIONAL_STRING(_, BonusType) \
                DL_OPTIONAL_STRING(_, Description1) \
                DL_OPTIONAL_STRING(_, Item) \
                DL_OPTIONAL_STRING(_, Item2) \
                DL_OBJECT_LIST(_, Effect, Effects) \
                DL_OBJECT_LIST(_, Stance, Stances) \
                DL_OPTIONAL_OBJECT(_, Requirements, RequirementsToUse)

        DL_DECLARE_ACCESS(Buff_PROPERTIES)
        DL_DECLARE_VARIABLES(Buff_PROPERTIES)

        friend class WikiItemFileProcessor;
        friend class Item;
};
