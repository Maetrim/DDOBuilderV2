// GlobalSupportFunctions.h
//
#pragma once

#include <string>
#include <list>
#include <map>
#include "XmlLib\SaxContentElement.h"
#include "AbilityTypes.h"
#include "BreakdownTypes.h"
#include "DamageTypes.h"
#include "InventorySlotTypes.h"
#include "SkillTypes.h"
#include "SpellPowerTypes.h"
#include "SpellSchoolTypes.h"
#include "TacticalTypes.h"
#include "WeaponTypes.h"
#include "LogPane.h"

class Augment;
class Bonus;
class BreakdownItem;
class Buff;
class Build;
class Class;
class EnhancementTree;
class EnhancementTreeItem;
class Feat;
class Filigree;
class GuildBuff;
class Item;
class ItemAugment;
class Gem;
class MouseHook;
class OptionalBuff;
class Patron;
class Quest;
class Race;
class SetBonus;
class Spell;
class Stance;
class TrainedFeat;
class WeaponGroup;
class CLogPane;

const std::list<Bonus>& BonusTypes();
const std::list<Race> & Races();
const std::list<Class> & Classes();
const std::map<std::string, Feat> & StandardFeats();
const std::list<Feat> & HeroicPastLifeFeats();
const std::list<Feat> & RacialPastLifeFeats();
const std::list<Feat> & IconicPastLifeFeats();
const std::list<Feat> & EpicPastLifeFeats();
const std::list<Feat> & SpecialFeats();
const std::list<Feat> & UniversalTreeFeats();
const std::list<Feat> & DestinyTreeFeats();
const std::list<Feat> & FavorFeats();
const std::list<EnhancementTree> & EnhancementTrees();
const std::list<Filigree>& Filigrees();
const std::list<Augment>& Augments();
const std::list<SetBonus>& SetBonuses();
const std::list<Stance>& Stances();
const std::list<Spell>& Spells();
const std::list<Item>& Items();
const std::list<Item>& ItemsForSlot(InventorySlotType ist);
const std::list<Quest>& Quests();
const std::list<Patron>& Patrons();
const std::list<GuildBuff>& GuildBuffs();
CImageList& ItemsImages();
CImageList& SpellImages();
const std::list<Gem>& SentientGems();
const std::list<WeaponGroup>& WeaponGroups();
const std::list<Buff>& ItemBuffs();
const std::list<Spell>& ItemClickies();
const std::list<OptionalBuff>& OptionalBuffs();

const Bonus& FindBonus(const std::string& bonus);
const Feat& FindFeat(const std::string& featName);
const Race& FindRace(const std::string& raceName);
const Class& FindClass(const std::string& className);
const Buff& FindBuff(const std::string& buffName);
const Patron& FindPatron(const std::string& patronName);
const Quest& FindQuest(const std::string& questName);
const OptionalBuff& FindOptionalBuff(const std::string& name);
const EnhancementTreeItem* FindEnhancement(const std::string& internalName, std::string * treeName);
const EnhancementTree& GetEnhancementTree(const std::string& treeName);
size_t ClassIndex(const std::string& className);
const Class & ClassFromIndex(size_t index);
Spell FindClassSpellByName(const Build* pBuild, const std::string& ct, const std::string& name);
Spell FindSpellByName(const std::string& spell, bool bSuppressError = false);
Spell FindItemClickieByName(const std::string& ct, bool bSuppressError = false);
const Item& FindItem(const std::string& itemName);

int BaseStatToBonus(double ability);

MouseHook* GetMouseHook();
BOOL OnEraseBackground(
        CWnd * pWnd,
        CDC * pDC,
        const int controlsNotToBeErased[]);
HTREEITEM GetNextTreeItem(const CTreeCtrl& treeCtrl, HTREEITEM hItem);

CLogPane & GetLog();
HRESULT LoadImageFile(
        const std::string& folder,
        const std::string& filename,
        CImage * pImage,
        const CSize & expectedImageSize,
        bool bReportIssue = true);
bool ImageFileExists(
        const std::string& folder,
        const std::string& filename);
void MakeGrayScale(CImage * pImage, COLORREF transparent = RGB(0, 0, 0));

template <typename T>
CString EnumEntryText(const T & t, const XmlLib::enumMapEntry<T> * m)
{
    const XmlLib::enumMapEntry<T> * p;
    for (p = m; p->name != NULL; ++p)
    {
        if (p->value == t)
        {
            break;
        }
    }
    if (p->name == NULL) // stopped at end of list
    {
        AfxMessageBox("EnumEntryText failed to find entry", MB_ICONERROR);
    }
    XmlLib::SaxString entry = p->name;
    CString asAscii = ((std::string)entry).c_str();
    return asAscii;
}

template <typename T>
T TextToEnumEntry(const std::string& name, const XmlLib::enumMapEntry<T> * m, bool bThrowOnNotFound = true)
{
    const XmlLib::enumMapEntry<T> * p;
    for (p = m; p->name != NULL; ++p)
    {
        XmlLib::SaxString entry = p->name;
        CString asAscii = ((std::string)entry).c_str();
        if (name == (LPCTSTR)asAscii)
        {
            break;
        }
    }
    if (bThrowOnNotFound
            && p->name == NULL) // stopped at end of list
    {
        std::stringstream ss;
        ss << "TextToEnumEntry failed to find entry name " << name;
        GetLog().AddLogEntry(name.c_str());
        return static_cast<T>(0);
    }
    else
    {
        return p->value;
    }
}

size_t TrainedCount(
        const std::list<TrainedFeat> & currentFeats,
        const std::string& featName);

const EnhancementTreeItem* FindEnhancement(const std::string& internalName);
const EnhancementTreeItem* FindEnhancement(const std::string& treeName, const std::string& internalName);

void SelectComboboxEntry(size_t itemData, CComboBox * pCombo);
void SelectComboboxEntry(const std::string& strItem, CComboBox * pCombo);
size_t GetComboboxSelection(CComboBox * pCombo);
std::string DataFolder();
BreakdownItem* FindBreakdown(BreakdownType type);
BreakdownType StatToBreakdown(AbilityType ability);
BreakdownType SkillToBreakdown(SkillType skill);
BreakdownItem* CasterLevelBreakdown(const std::string& ct);
AbilityType StatFromSkill(SkillType skill);
int ArmorCheckPenalty_Multiplier(SkillType skill);
bool IsVowel(char ch);
bool IsOffHandWeapon(WeaponType wt);
BreakdownType TacticalToBreakdown(TacticalType tactical);
BreakdownType SchoolToBreakdown(SpellSchoolType school);
BreakdownType CasterLevelSchoolToBreakdown(SpellSchoolType school);
BreakdownType MaxCasterLevelSchoolToBreakdown(SpellSchoolType school);
BreakdownType SpellPowerToBreakdown(SpellPowerType sp);
BreakdownType SpellPowerToCriticalChanceBreakdown(SpellPowerType sp);
BreakdownType SpellPowerToCriticalMultiplierBreakdown(SpellPowerType sp);
const Augment & FindAugmentByName(const std::string& name, const Item* pItem);
const Filigree & FindFiligreeByName(const std::string& name);
const Filigree & FindFiligreeByNameComponents(const std::string& name);
const Gem& FindSentientGemByName(const std::string& name);
const SetBonus& FindSetBonus(const std::string& name);
bool AddMenuItem(HMENU hTargetMenu, const CString& itemText, UINT itemID, bool bEnabled);
int FindItemIndexByItemData(CListCtrl* pControl, DWORD itemData);
bool LimitToRuneArm(Build* pBuild);
bool CanEquipTo2ndWeapon(Build* pBuild, const Item& item);
void BreakUpLongLines(CString& line);
void AddToIgnoreList(const std::string& name);
void RemoveFromIgnoreList(const std::string& name);
bool IsInIgnoreList(const std::string& name);
CString ExtractLine(size_t index, CString lines);
void AddSpecialSlots(InventorySlotType slot, Item& item);
void AddAugment(std::vector<ItemAugment>* augments, const std::string& name, bool atEnd = false);
void LoadColumnWidthsByName(CListCtrl* control, const std::string& fmtString);
void SaveColumnWidthsByName(CListCtrl* control, const std::string& fmtString);
void FormatExportData(std::string* exportData);
void FormatExportData(CString* exportData);
void MakeIconTransparent(const CString& iconFilename, bool bPlusTopLeft);
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);
std::string ReplaceFirst(std::string str, const std::string& from, const std::string& to);
std::string ExtractBlock(std::string str, const std::string& from, const std::string& to);
bool DarkModeEnabled();
bool SearchForText(std::string source, const std::string& find);
std::vector<Spell> FilterSpells(Build* pBuild, std::string& ct, int level);
size_t WeaponBaseCriticalRange(WeaponType wt);
std::list<Augment> CompatibleAugments(const ItemAugment& aug, size_t level, const std::string& selectedAugment);
bool IsShield(WeaponType wt);
