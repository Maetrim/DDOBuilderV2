// SpellDC.h
//
#pragma once
#include "XmlLib\DLMacros.h"

#include "SpellSchoolTypes.h"

class Build;
class Spell;

class SpellDC :
    public XmlLib::SaxContentElement
{
    public:
        SpellDC(void);
        void Write(XmlLib::SaxWriter* writer) const;
        int CalculateSpellDC(const Build* build, const Spell& spell) const;
        std::string SpellDCBreakdown(const Build* build, const Spell& spell) const;

        bool VerifyObject(std::stringstream* ss) const;

    protected:
        XmlLib::SaxContentElementInterface* StartElement(
                const XmlLib::SaxString& name,
                const XmlLib::SaxAttributes& attributes);

        virtual void EndElement();

        // note that if a SpellDC has an Amount field, it is a fixed DC value
        #define SpellDC_PROPERTIES(_) \
                DL_STRING(_, DCType) \
                DL_STRING(_, DCVersus) \
                DL_OPTIONAL_SIMPLE(_, size_t, Amount, 0) \
                DL_OPTIONAL_STRING(_, Other) \
                DL_FLAG(_, CastingStatMod) \
                DL_ENUM_LIST(_, SpellSchoolType, School, SpellSchool_Unknown, spellSchoolTypeMap)

        DL_DECLARE_ACCESS(SpellDC_PROPERTIES)
        DL_DECLARE_VARIABLES(SpellDC_PROPERTIES)
};
