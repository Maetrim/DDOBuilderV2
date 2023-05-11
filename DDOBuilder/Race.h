// Race.h
//
// An XML object wrapper that holds information on a defined race
#pragma once
#include "XmlLib\DLMacros.h"
#include "AbilityTypes.h"
#include "SkillTypes.h"
#include "FeatSlot.h"
#include "Feat.h"

class Feat;

class Race :
    public XmlLib::SaxContentElement
{
    public:
        Race(void);
        void Write(XmlLib::SaxWriter * writer) const;

        int RacialModifier(const AbilityType ability) const;
        bool IsIconic() const;
        void AddFeatSlots(size_t level, std::vector<FeatSlot> * tfts) const;
        void ClearRacialFeats();

        void SetFilename(const std::string&) {};
        void VerifyObject() const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Race_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_FLAG(_, IsConstruct) \
                DL_FLAG(_, NoPastLife) \
                DL_VECTOR(_, size_t, BuildPoints) \
                DL_OPTIONAL_STRING(_, IconicClass) \
                DL_OBJECT_LIST(_, FeatSlot, RaceSpecificFeat) \
                DL_OPTIONAL_SIMPLE(_, int, Strength, 0) \
                DL_OPTIONAL_SIMPLE(_, int, Dexterity, 0) \
                DL_OPTIONAL_SIMPLE(_, int, Constitution, 0) \
                DL_OPTIONAL_SIMPLE(_, int, Intelligence, 0) \
                DL_OPTIONAL_SIMPLE(_, int, Wisdom, 0) \
                DL_OPTIONAL_SIMPLE(_, int, Charisma, 0) \
                DL_OPTIONAL_SIMPLE(_, int, SkillPoints, 0) \
                DL_OPTIONAL_ENUM(_, SkillType, Skill, Skill_Unknown, skillTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, Ability, Ability_Unknown, abilityTypeMap) \
                DL_ENUM_LIST(_, SkillType, AutoBuySkill, Skill_Unknown, skillTypeMap) \
                DL_OBJECT_LIST(_, Feat, RacialFeats) \
                DL_STRING_LIST(_, GrantedFeat)

        DL_DECLARE_ACCESS(Race_PROPERTIES)
        DL_DECLARE_VARIABLES(Race_PROPERTIES)
};
