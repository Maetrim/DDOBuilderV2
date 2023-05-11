// SpellDamage.h
//
// An XML object wrapper that holds information on a damage a spell does
#pragma once
#include "XmlLib\DLMacros.h"
#include "DamageTypes.h"
#include "SpellDice.h"

class Build;

class SpellDamage :
    public XmlLib::SaxContentElement
{
    public:
        SpellDamage(void);
        void Write(XmlLib::SaxWriter * writer) const;

        CString DiceDamageText() const;
        CString SpellDamageText(const Build& build, size_t casterLevel) const;

        bool VerifyObject(std::stringstream* ss, bool bHasMaxcasterLevel) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define SpellDamage_PROPERTIES(_) \
                DL_OBJECT(_, SpellDice, DamageDice) \
                DL_OPTIONAL_ENUM(_, DamageType, Damage, Damage_Unknown, damageTypeMap) \
                DL_OPTIONAL_ENUM(_, SpellPowerType, SpellPower, SpellPower_Unknown, spellPowerTypeMap)

        DL_DECLARE_ACCESS(SpellDamage_PROPERTIES)
        DL_DECLARE_VARIABLES(SpellDamage_PROPERTIES)
};
