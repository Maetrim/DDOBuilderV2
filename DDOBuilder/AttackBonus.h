// AttackBonus.h
//
// An XML object wrapper that holds information on which equipment slot(s)
// and item can be equipped to.
#pragma once
#include "XmlLib\DLMacros.h"

class AttackBonus :
    public XmlLib::SaxContentElement
{
    public:
        AttackBonus(const XmlLib::SaxString& elementName);
        void Write(XmlLib::SaxWriter * writer) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define AttackBonus_PROPERTIES(_) \
                DL_OPTIONAL_VECTOR(_, double, Duration) \
                DL_OPTIONAL_VECTOR(_, double, BonusW) \
                DL_OPTIONAL_VECTOR(_, double, BonusAttackBonus) \
                DL_OPTIONAL_VECTOR(_, double, BonusDamage) \
                DL_OPTIONAL_VECTOR(_, double, BonusThreatRange) \
                DL_OPTIONAL_VECTOR(_, double, BonusCriticalMultiplier) \
                DL_OPTIONAL_VECTOR(_, double, BonusDamagePercent) \
                DL_OPTIONAL_VECTOR(_, double, BonusAlacrity) \
                DL_OPTIONAL_VECTOR(_, double, BonusMeleePower) \
                DL_OPTIONAL_VECTOR(_, double, BonusRangedPower) \
                DL_OPTIONAL_VECTOR(_, double, FortificationLoss) \
                DL_FLAG(_, AllowSneakAttack)

        DL_DECLARE_ACCESS(AttackBonus_PROPERTIES)
        DL_DECLARE_VARIABLES(AttackBonus_PROPERTIES)
};
