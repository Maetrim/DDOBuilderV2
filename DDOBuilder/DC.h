// DC.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "Dice.h"

#include "AbilityTypes.h"
#include "SpellSchoolTypes.h"
#include "SkillTypes.h"
#include "TacticalTypes.h"

class Build;

class DC :
    public XmlLib::SaxContentElement
{
    public:
        DC(void);
        void Write(XmlLib::SaxWriter * writer) const;
        int CalculateDC(const Build * build) const;
        std::string DCBreakdown(const Build * build) const;

        void SetClass(const std::string& ct);

        void AddStack();
        void RevokeStack();
        size_t NumStacks() const;

        bool VerifyObject(std::stringstream * ss) const;
        bool operator==(const DC & other) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define DC_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_STRING(_, Icon) \
                DL_STRING(_, DCType) \
                DL_STRING(_, DCVersus) \
                DL_OPTIONAL_STRING(_, Other) \
                DL_OPTIONAL_VECTOR(_, int, Amount) \
                DL_OPTIONAL_VECTOR(_, double, ClassMultiplier) \
                DL_ENUM_LIST(_, AbilityType, FullAbility, Ability_Unknown, abilityTypeMap) \
                DL_ENUM_LIST(_, AbilityType, ModAbility, Ability_Unknown, abilityTypeMap) \
                DL_ENUM_LIST(_, SpellSchoolType, School, SpellSchool_Unknown, spellSchoolTypeMap) \
                DL_OPTIONAL_ENUM(_, SkillType, Skill, Skill_Unknown, skillTypeMap) \
                DL_OPTIONAL_ENUM(_, TacticalType, Tactical, Tactical_Unknown, tacticalTypeMap) \
                DL_OPTIONAL_ENUM(_, TacticalType, Tactical2, Tactical_Unknown, tacticalTypeMap) \
                DL_OPTIONAL_STRING(_, BaseClassLevel) \
                DL_OPTIONAL_STRING(_, ClassLevel) \
                DL_OPTIONAL_STRING(_, HalfClassLevel) \
                DL_OPTIONAL_STRING(_, HalfBaseClassLevel)

        DL_DECLARE_ACCESS(DC_PROPERTIES)
        DL_DECLARE_VARIABLES(DC_PROPERTIES)

        size_t m_stacks;
        std::string m_class;
};
