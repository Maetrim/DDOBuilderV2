// Dice.h
//
// An XML object wrapper that holds information on dice to be rolled
#pragma once
#include "XmlLib\DLMacros.h"
#include "BaseDice.h"
#include "BonusDice.h"
#include "DamageTypes.h"
#include "SpellPowerTypes.h"

class SpellDice :
    public XmlLib::SaxContentElement
{
    public:
        SpellDice(void);
        void Write(XmlLib::SaxWriter * writer) const;

        CString AverageDamageText(double spellPower, int casterLevel) const;
        CString CriticalDamageText(double spellPower, int casterLevel, double criticalMultiplier) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        // gives possible dice such as 1d6+4 + xdy+x per n caster levels
        // with a damage type done and the spell power it scales by
        #define SpellDice_PROPERTIES(_) \
                DL_OPTIONAL_OBJECT(_, BaseDice, StandardDice) \
                DL_OPTIONAL_SIMPLE(_, size_t, PerCasterLevels, 1) \
                DL_OPTIONAL_OBJECT(_, BonusDice, DicePerCasterLevel) \
                DL_OPTIONAL_ENUM(_, DamageType, Damage, Damage_Unknown, damageTypeMap) \
                DL_OPTIONAL_ENUM(_, SpellPowerType, SpellPower, SpellPower_Unknown, spellPowerTypeMap)

        DL_DECLARE_ACCESS(SpellDice_PROPERTIES)
        DL_DECLARE_VARIABLES(SpellDice_PROPERTIES)
};
