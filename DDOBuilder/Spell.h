// Spell.h
//
// An XML object wrapper that holds information on a spell
#pragma once
#include "XmlLib\DLMacros.h"
#include "SpellSchoolTypes.h"
#include "Effect.h"
#include "SpellDC.h"
#include "SpellDamage.h"
#include "Stance.h"

class Build;
class ClassSpell;
class FixedSpell;

class Spell :
    public XmlLib::SaxContentElement
{
    public:
        Spell(void);
        void Write(XmlLib::SaxWriter* writer) const;

        bool operator<(const Spell& other) const;

        size_t MetamagicCount() const;
        void VerifyObject() const;

        size_t DC(const Build& build) const;
        const std::string& Class() const;
        void SetClass(const std::string& ct);
        std::list<Effect> UpdatedEffects(size_t castingLevel) const;
        void UpdateSpell(const ClassSpell& cs, const std::string& ct, int spellLevel);
        void UpdateSpell(const FixedSpell& fs, const std::string& ct);

        int ActualCasterLevel(const Build& build) const;
        int ActualMaxCasterLevel(const Build& build) const;
        CString ActualCasterLevelText(const Build& build) const;
        CString ActualMaxCasterLevelText(const Build& build) const;
        void SetCost(int cost);
        int TotalCost(const Build& build) const;

        void SetIconIndex(size_t index);
        size_t IconIndex() const;

        void SetMaxCasterLevel(int mcl);
        void SetCooldown(double cooldown);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Spell_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_OPTIONAL_STRING(_, Icon) \
                DL_OPTIONAL_SIMPLE(_, int, Level, 0) \
                DL_ENUM_LIST(_, SpellSchoolType, School, SpellSchool_Unknown, spellSchoolTypeMap) \
                DL_OPTIONAL_ENUM(_, PrimerType, Primer, Primer_Unknown, primerTypeMap) \
                DL_OPTIONAL_SIMPLE(_, int, Cost, 0) \
                DL_OBJECT_LIST(_, Effect, Effects) \
                DL_OBJECT_LIST(_, ::SpellDC, DCs) \
                DL_OBJECT_LIST(_, SpellDamage, SpellDamageEffects) \
                DL_OBJECT_LIST(_, Stance, Stances) \
                DL_FLAG(_, Accelerate) \
                DL_FLAG(_, Embolden) \
                DL_FLAG(_, Empower) \
                DL_FLAG(_, EmpowerHealing) \
                DL_FLAG(_, Enlarge) \
                DL_FLAG(_, Extend) \
                DL_FLAG(_, Heighten) \
                DL_FLAG(_, Intensify) \
                DL_FLAG(_, Maximize) \
                DL_FLAG(_, Quicken) \
                DL_OPTIONAL_SIMPLE(_, int, MaxCasterLevel, 0) \
                DL_OPTIONAL_SIMPLE(_, double, Cooldown, 0.0)

        DL_DECLARE_ACCESS(Spell_PROPERTIES)
        DL_DECLARE_VARIABLES(Spell_PROPERTIES)
    private:
        std::string m_class;
        size_t m_iconIndex;
};
