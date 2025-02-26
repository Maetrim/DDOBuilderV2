// Class.h
//
// An XML object wrapper that holds information on a defined class
#pragma once
#include "XmlLib\DLMacros.h"
#include "AbilityTypes.h"
#include "AlignmentTypes.h"
#include "AutomaticFeats.h"
#include "SaveTypes.h"
#include "SkillTypes.h"
#include "FeatSlot.h"
#include "ClassSpell.h"
#include "Spell.h"

class Build;
class Feat;

class Class :
    public XmlLib::SaxContentElement
{
    public:
        Class();
        virtual ~Class();
        void Write(XmlLib::SaxWriter * writer) const;

        std::string GetBaseClass() const;

        size_t SkillPoints(const std::string& race, const size_t intelligence, const size_t level) const;
        bool CanTrainClass(const AlignmentType alignment) const;
        bool IsClassSkill(const SkillType skill) const;
        size_t ClassSave(const SaveType st, const size_t level) const;
        AbilityType ClassCastingStat() const;
        size_t SpellPointsAtLevel(const size_t level) const;
        std::vector<size_t> SpellSlotsAtLevel(const size_t level) const;
        size_t Index() const;
        void AddSmallClassImage(CImageList * pImageList) const;
        void AddLargeClassImage(CImageList * pImageList) const;
        void AddFeatSlots(const Build& build, size_t classLevel, std::vector<FeatSlot> * tfts, size_t totalLevel) const;
        size_t MaxSpellLevel(size_t classLevel) const;
        const std::list<std::string>& GetAutoFeats(size_t level) const;

        std::list<Feat> ImprovedHeroicDurabilityFeats() const;

        bool operator<(const Class & other) const;
        void VerifyObject() const;
        static void CreateClassImageLists();
        static CImageList& SmallClassImageList();
        static CImageList& LargeClassImageList();
        static size_t TomeImageIndex();
        void ClearClassFeats();
        void CreateSpellLists();

        std::list<Spell> Spells(Build* pBuild, size_t spellLevel, bool bIncludeFixedSpells) const;
        Spell FindSpell(const std::string& name) const;

        void SetFilename(const std::string&) {};
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Class_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_OPTIONAL_STRING(_, BaseClass) \
                DL_STRING(_, Description) \
                DL_STRING_LIST(_, ClassSpecificFeatType) \
                DL_STRING(_, SmallIcon) \
                DL_STRING(_, LargeIcon) \
                DL_SIMPLE(_, int, SkillPoints, 0) \
                DL_SIMPLE(_, int, HitPoints, 0) \
                DL_ENUM_LIST(_, SkillType, ClassSkill, Skill_Unknown, skillTypeMap) \
                DL_ENUM_LIST(_, AlignmentType, Alignment, Alignment_Unknown, alignmentTypeMap) \
                DL_ENUM(_, ClassSaveType, Fortitude, SaveType_Unknown, classSaveTypeMap) \
                DL_ENUM(_, ClassSaveType, Reflex, SaveType_Unknown, classSaveTypeMap) \
                DL_ENUM(_, ClassSaveType, Will, SaveType_Unknown, classSaveTypeMap) \
                DL_VECTOR(_, int, SpellPointsPerLevel) \
                DL_ENUM_LIST(_, AbilityType, CastingStat, Ability_Unknown, abilityTypeMap) \
                DL_ENUM_LIST(_, SkillType, AutoBuySkill, Skill_Unknown, skillTypeMap) \
                DL_FLAG(_, NotHeroic) \
                DL_OPTIONAL_VECTOR(_, size_t, Level1) \
                DL_OPTIONAL_VECTOR(_, size_t, Level2) \
                DL_OPTIONAL_VECTOR(_, size_t, Level3) \
                DL_OPTIONAL_VECTOR(_, size_t, Level4) \
                DL_OPTIONAL_VECTOR(_, size_t, Level5) \
                DL_OPTIONAL_VECTOR(_, size_t, Level6) \
                DL_OPTIONAL_VECTOR(_, size_t, Level7) \
                DL_OPTIONAL_VECTOR(_, size_t, Level8) \
                DL_OPTIONAL_VECTOR(_, size_t, Level9) \
                DL_OPTIONAL_VECTOR(_, size_t, Level10) \
                DL_OPTIONAL_VECTOR(_, size_t, Level11) \
                DL_OPTIONAL_VECTOR(_, size_t, Level12) \
                DL_OPTIONAL_VECTOR(_, size_t, Level13) \
                DL_OPTIONAL_VECTOR(_, size_t, Level14) \
                DL_OPTIONAL_VECTOR(_, size_t, Level15) \
                DL_OPTIONAL_VECTOR(_, size_t, Level16) \
                DL_OPTIONAL_VECTOR(_, size_t, Level17) \
                DL_OPTIONAL_VECTOR(_, size_t, Level18) \
                DL_OPTIONAL_VECTOR(_, size_t, Level19) \
                DL_OPTIONAL_VECTOR(_, size_t, Level20) \
                DL_VECTOR(_, double, BAB) \
                DL_OBJECT_LIST(_, ClassSpell, ClassSpells) \
                DL_OBJECT_LIST(_, AutomaticFeats, AutoFeats) \
                DL_OBJECT_LIST(_, FeatSlot, FeatSlots) \
                DL_OBJECT_LIST(_, Feat, ClassFeats)

        DL_DECLARE_ACCESS(Class_PROPERTIES)
        DL_DECLARE_VARIABLES(Class_PROPERTIES)

        mutable size_t m_index;
        static CImageList sm_classImagesLarge;
        static CImageList sm_classImagesSmall;
        static size_t sm_tomeIndex;
        std::vector<std::list<Spell> > m_classSpells;
};
