// Class.cpp
//
#include "StdAfx.h"
#include "Class.h"
#include "XmlLib\SaxWriter.h"

#include "BreakdownItem.h"
#include "Build.h"
#include "GlobalSupportFunctions.h"
#include "Race.h"
#include "LogPane.h"

// Classes need to define a few things to be fully setup:
// Name          - The name of the class e.g. "Fighter"
// Description   - Description of the class displayed in the UI
// ClassSpecificFeatType - Any feat types that are specific to this class type
// SmallIcon     - The name of the file that contains the small icon for this class
// LargeIcon     - The name of the file that contains the large icon for this class
// SkillPoints   - The number of skill points this class gets per level
// HitPoints     - The number of hit points this class gets per level
// ClassSkill    - List of skills which are class skills for this class
// Alignment     - List of allowed alignments this class must have (Or "Any" for all)
// Fortitude     - The type of fortitude save for this class (type 1 or 2)
// Reflex        - The type of reflex save for this class (type 1 or 2)
// Will          - The type of will save for this class (type 1 or 2)
// SpellPointsPerLevel - Vector of actual spell points for that many levels in this class (size 21)
// CastingStat   - The main state used for DC calculation (list if use highest)
// AutoBuySkill  - List of skills to prioritize in the an auto buy action for this class
// Level1..Level20 - Number of spell slots at each level for spell casting classes
// BAB           - The amount of BAB awarded for each level in this class (size 21)
// ClassSpells   - The spells this class has at each spell level (-ve are auto assigned spells)
// FeatSlots     - List of trainable feats slots and types for this class

#define DL_ELEMENT Class

namespace
{
    const wchar_t f_saxElementName[] = L"Class";
    DL_DEFINE_NAMES(Class_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

// we have global image lists for the small and large icons for all classes
// these can be referenced where needed from elsewhere in the project
CImageList Class::sm_classImagesLarge;
CImageList Class::sm_classImagesSmall;
size_t Class::sm_tomeIndex;

Class::Class() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_index(999)
{
    DL_INIT(Class_PROPERTIES)
}

Class::~Class()
{
}

DL_DEFINE_ACCESS(Class_PROPERTIES)

XmlLib::SaxContentElementInterface * Class::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Class_PROPERTIES)

    return subHandler;
}

void Class::EndElement()
{
    // now check out our object
    SaxContentElement::EndElement();
    DL_END(Class_PROPERTIES)
}

void Class::Write(XmlLib::SaxWriter* writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Class_PROPERTIES)
    writer->EndElement();
}

std::string Class::GetBaseClass() const
{
    std::string baseClass = Name();
    if (HasBaseClass())
    {
        baseClass = BaseClass();
    }
    return baseClass;
}

bool Class::operator<(const Class& other) const
{
    // (assumes all class names are unique)
    // sort by name
    // make a combined name of "<BaseClass> <Actual Class>" for comparison
    CString ourName = GetBaseClass().c_str(); // same as class name when no base class
    if (HasBaseClass())
    {
        ourName += " ";
        ourName += Name().c_str();
    }
    CString theirName = other.GetBaseClass().c_str(); // same as class name when no base class
    if (other.HasBaseClass())
    {
        theirName += " ";
        theirName += other.Name().c_str();
    }

    bool ret = ourName < theirName;
    return ret;
}

void Class::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";

    if (BAB().size() != MAX_CLASS_LEVEL + 1)
    {
        ss << "Has incorrect BAB vector size\n";
        ok = false;
    }

    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

std::vector<size_t> Class::SpellSlotsAtLevel(const size_t level) const
{
    std::vector<size_t> slots;
    switch (level)
    {
    case 0: break;  // no spells
    case 1: if (HasLevel1()) slots = Level1(); break;
    case 2: if (HasLevel2()) slots = Level2(); break;
    case 3: if (HasLevel3()) slots = Level3(); break;
    case 4: if (HasLevel4()) slots = Level4(); break;
    case 5: if (HasLevel5()) slots = Level5(); break;
    case 6: if (HasLevel6()) slots = Level6(); break;
    case 7: if (HasLevel7()) slots = Level7(); break;
    case 8: if (HasLevel8()) slots = Level8(); break;
    case 9: if (HasLevel9()) slots = Level9(); break;
    case 10: if (HasLevel10()) slots = Level10(); break;
    case 11: if (HasLevel11()) slots = Level11(); break;
    case 12: if (HasLevel12()) slots = Level12(); break;
    case 13: if (HasLevel13()) slots = Level13(); break;
    case 14: if (HasLevel14()) slots = Level14(); break;
    case 15: if (HasLevel15()) slots = Level15(); break;
    case 16: if (HasLevel16()) slots = Level16(); break;
    case 17: if (HasLevel17()) slots = Level17(); break;
    case 18: if (HasLevel18()) slots = Level18(); break;
    case 19: if (HasLevel19()) slots = Level19(); break;
    case 20: if (HasLevel20()) slots = Level20(); break;
    }
    return slots;
}

size_t Class::SkillPoints(
        const std::string& race,
        const size_t intelligence,
        const size_t level) const
{
    // determine the number of skill points for this class at this level
    // for the given race and intelligence
    size_t skillPoints = 0;
    // basic number for the class first
    skillPoints += SkillPoints();

    // certain races get extra skill points per level
    const Race & r = FindRace(race);
    if (r.HasSkillPoints())
    {
        skillPoints += r.SkillPoints();
    }

    // int modifier
    int mod = BaseStatToBonus(intelligence);
    skillPoints += mod;
    if (skillPoints < 1)
    {
        // minimum of 1 skill point always
        skillPoints = 1;
    }
    // 4 times the skill points at 1st level
    if (level == 0) // level is 0 based
    {
        skillPoints *= 4;
    }
    return skillPoints;
}

bool Class::CanTrainClass(const AlignmentType alignment) const
{
    // can train if the alignment provided is present in the list
    bool canTrain = false;
    std::list<AlignmentType>::const_iterator it = m_Alignment.begin();
    while (!canTrain && it != m_Alignment.end())
    {
        if ((*it) == Alignment_Any
                || (*it) == alignment)
        {
            canTrain = true;
        }
        ++it;
    }
    return canTrain;
}

size_t Class::ClassSave(const SaveType st, const size_t level) const
{
    // saves due to class levels come in two types:
    // Level    1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
    // -------------------------------------------------------------------
    // Type1    0  0  1  1  1  2  2  2  3  3  3  4  4  4  5  5  5  6  6  6
    // Type2    2  3  3  4  4  5  5  6  6  7  7  8  8  9  9 10 10 11 11 12
    ClassSaveType type = SaveType_Unknown;
    switch (st)
    {
    case Save_Fortitude:    type = Fortitude(); break;
    case Save_Reflex:       type = Reflex(); break;
    case Save_Will:         type = Will(); break;
    default:
        ASSERT(FALSE);
        break;
    }
    size_t save = 0;
    switch (type)
    {
    case SaveType_None:     save = 0; break;
    case SaveType_Type1:    save = (size_t)floor(level / 3.0); break;
    case SaveType_Type2:    save = 2 + (size_t)floor(level / 2.0); break;
    default:                ASSERT(FALSE); break;
    }
    return save;
}

AbilityType Class::ClassCastingStat() const
{
    AbilityType at = Ability_Unknown;
    // use the casting stat if we have one. Use the highest one if we have many
    if (m_CastingStat.size() == 1)
    {
        at = m_CastingStat.front();
    }
    else
    {
        int best = -999;
        for (auto&& csi : m_CastingStat)
        {
            BreakdownType bt = StatToBreakdown(csi);
            BreakdownItem * pBreakdown = FindBreakdown(bt);
            if (pBreakdown != NULL)
            {
                int amount = static_cast<int>(pBreakdown->Total());
                if (amount > best)
                {
                    best = amount;
                    at = csi;
                }
            }
        }
    }
    return at;
}

size_t Class::SpellPointsAtLevel(const size_t level) const
{
    size_t spellPoints = 0;
    if (level >= 0 && level < m_SpellPointsPerLevel.size())
    {
        spellPoints = m_SpellPointsPerLevel[level];
    }
    return spellPoints;
}

size_t Class::Index() const
{
    return m_index;
}

void Class::AddSmallClassImage(CImageList * pImageList) const
{
    CImage image;
    HRESULT result = LoadImageFile(
            "DataFiles\\ClassImages\\",
            SmallIcon(),
            &image,
            CSize(16, 15));
    if (result == S_OK)
    {
        CBitmap bitmap;
        bitmap.Attach(image.Detach());
        pImageList->Add(&bitmap, c_transparentColour);  // standard mask color (purple)
    }
    else
    {
        CString str;
        str.Format("Failed to load small class image file \"DataFiles\\ClassImages\\%s\"", SmallIcon().c_str());
        GetLog().AddLogEntry(str);
    }
}

void Class::AddLargeClassImage(CImageList * pImageList) const
{
    CImage image;
    HRESULT result = LoadImageFile(
            "DataFiles\\ClassImages\\",
            LargeIcon(),
            &image,
            CSize(32, 32));
    if (result == S_OK)
    {
        CBitmap bitmap;
        bitmap.Attach(image.Detach());
        pImageList->Add(&bitmap, c_transparentColour);  // standard mask color (purple)
    }
    else
    {
        CString str;
        str.Format("Failed to load large class image file \"DataFiles\\ClassImages\\%s\"", LargeIcon().c_str());
        GetLog().AddLogEntry(str);
    }
}

void Class::AddFeatSlots(
        const Build& build,
        size_t classLevel,
        std::vector<FeatSlot> * tfts,
        size_t totalLevel) const
{
    // look through the list of FeatSlots and add those which are for the level in question
    for (auto&& cfli : m_FeatSlots)
    {
        if (cfli.Level() == classLevel)   // 1 based in file, count (1 based) in code
        {
            bool bAdd = true;       // assume to be added
            if (cfli.HasSingular())
            {
                // this feat slot type should only be added if it has not been
                // added at a previous level.
                for (size_t pLevel = 0; bAdd && pLevel < totalLevel; ++pLevel)
                {
                    std::vector<FeatSlot> slots = build.TrainableFeatTypeAtLevel(pLevel);
                    for (auto&& sit : slots)
                    {
                        // has it been added previously?
                        if (sit == cfli)
                        {
                            bAdd = false;
                        }
                    }
                }
            }
            // need to add this one?
            if (bAdd)
            {
                tfts->push_back(cfli);
            }
        }
    }
}

std::list<Feat> Class::ImprovedHeroicDurabilityFeats() const
{
    // all heroic classes need to create their Improved Heroic Durability (class name)
    // feats for level 5, 10 and 15. As there is no way to to define these directly
    // in the xml, we create them dynamically here and add them to the the total feat
    // list
    std::list<Feat> ihd;
    if (!HasNotHeroic())
    {
        const Feat & baseFeat = FindFeat("Improved Heroic Durability");
        // need to create the improved heroic durability feats for this class
        for (size_t level = 5; level <= 15; level += 5)     // 5, 10 and 15 only
        {
            std::stringstream ss;
            ss << "Improved Heroic Durability (" << Name() << " " << level << ")";
            // e.g. "Improved Heroic Durability (Paladin 5)"
            Feat ihdf(baseFeat);
            ihdf.SetName(ss.str());    // needs a unique name
            Requirement r(Requirement_ClassAtLevel, Name(), level);
            ihdf.AddAutomaticAcquisition(r);
            ihd.push_back(ihdf);
        }
    }
    return ihd;
}

bool Class::IsClassSkill(const SkillType skill) const
{
    bool bIsClassSkill = false;
    std::list<SkillType>::const_iterator cit = m_ClassSkill.begin();
    while (!bIsClassSkill && cit != m_ClassSkill.end())
    {
        bIsClassSkill = ((*cit) == skill);
        ++cit;
    }
    return bIsClassSkill;
}

void Class::CreateClassImageLists()
{
    const std::list<Class> & classes = Classes();
    sm_classImagesSmall.Create(16, 15, ILC_COLOR24, 0, classes.size());
    sm_classImagesLarge.Create(32, 32, ILC_COLOR24, 0, classes.size());
    // load all the small/large images for each class
    size_t index = 0;
    std::list<Class>::const_iterator cit = classes.begin();
    while (cit != classes.end())
    {
        (*cit).m_index = index;
        (*cit).AddSmallClassImage(&sm_classImagesSmall);
        (*cit).AddLargeClassImage(&sm_classImagesLarge);
        ++cit;
        ++index;
    }
    // finally, add the special "Tome" image that is used by the skills
    // view as part of the small class image list
    ::Class tomeImage;      // temp to load image only
    tomeImage.Set_SmallIcon("Tome");
    tomeImage.AddSmallClassImage(&sm_classImagesSmall);
    sm_tomeIndex = index;
}

CImageList& Class::SmallClassImageList()
{
    return sm_classImagesSmall;
}

CImageList& Class::LargeClassImageList()
{
    return sm_classImagesLarge;
}

size_t Class::TomeImageIndex()
{
    return sm_tomeIndex;
}

void Class::ClearClassFeats()
{
    m_ClassFeats.clear();
}

void Class::CreateSpellLists()
{
    std::vector<size_t> spellSlots = SpellSlotsAtLevel(MAX_CLASS_LEVEL);
    // reserve a list for each levels Spells this class has
    m_classSpells.resize(spellSlots.size());
    // now process all the ClassSpell objects and create the spell lists
    for (auto&& csit : m_ClassSpells)
    {
        std::string spellName = csit.Name();
        Spell spell = FindSpellByName(spellName);
        spell.UpdateSpell(csit, Name());
        m_classSpells[abs(csit.Level()) - 1].push_back(spell);
    }
    // were done with all class spells, clear data that is no longer required
    m_ClassSpells.clear();
}

std::list<Spell> Class::Spells(Build* pBuild, size_t spellLevel, bool bIncludeFixedSpells) const
{
    std::list<Spell> spells;
    if (spellLevel <= m_classSpells.size())
    {
        // spellLevel is 1 based
        auto it = m_classSpells.begin();
        std::advance(it, spellLevel - 1);

         spells = *it;
        if (!bIncludeFixedSpells)
        {
            // remove any automatic feats (negative levels)
            std::list<Spell>::iterator sit = spells.begin();
            while (sit != spells.end())
            {
                if ((*sit).Level() < 0)
                {
                    sit = spells.erase(sit);
                }
                else
                {
                    ++sit;
                }
            }
        }
        pBuild->AppendSpellListAdditions(spells, Name(), spellLevel);
    }
    return spells;
}

Spell Class::FindSpell(const std::string& name) const
{
    Spell spell;
    bool bFound = false;
    for (auto&& slit : m_classSpells)
    {
        for (auto&& sit : slit)
        {
            if (sit.Name() == name)
            {
                spell = sit;
                bFound = true;
                break;
            }
        }
        if (bFound)
        {
            break;
        }
    }
    return spell;
}
