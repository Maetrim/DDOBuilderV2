// DDOBuilder.h
//
#pragma once
#include <list>
#include "Augment.h"
#include "Bonus.h"
#include "Buff.h"
#include "Class.h"
#include "EnhancementTree.h"
#include "Feat.h"
#include "Filigree.h"
#include "Gem.h"
#include "GuildBuff.h"
#include "Item.h"
#include "OptionalBuff.h"
#include "Patron.h"
#include "Quest.h"
#include "Race.h"
#include "SetBonus.h"
#include "Stance.h"
#include "Spell.h"
#include "WeaponGroup.h"

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "CustomContextMenuManager.h"

class LegacyCharacter;

// CDDOBuilderApp:
// See DDOBuilder.cpp for the implementation of this class
//

class CDDOBuilderApp :
    public CWinAppEx
{
public:
    CDDOBuilderApp();

    UINT  m_nAppLook;
    BOOL  m_bHiColorIcons;

    const std::list<Bonus>& BonusTypes() const;
    const std::list<Race>& Races() const;
    const std::list<Class>& Classes() const;
    const std::map<std::string, Feat>& AllFeats() const;
    const std::list<Feat>& HeroicPastLifeFeats() const;
    const std::list<Feat>& RacialPastLifeFeats() const;
    const std::list<Feat>& IconicPastLifeFeats() const;
    const std::list<Feat>& EpicPastLifeFeats() const;
    const std::list<Feat>& SpecialFeats() const;
    const std::list<Feat>& UniversalTreeFeats() const;
    const std::list<Feat>& DestinyTreeFeats() const;
    const std::list<Feat>& FavorFeats() const;
    const std::list<EnhancementTree>& EnhancementTrees() const;
    const std::list<Augment>& Augments() const;
    const std::list<Filigree>& Filigrees() const;
    const std::list<Item>& Items() const;
    const std::list<Item>& ItemsForSlot(InventorySlotType ist) const;
    CImageList& ItemImageList();
    CImageList& SpellImageList();
    const std::list<SetBonus>& SetBonuses() const;
    const std::list<Stance>& Stances() const;
    const std::list<Spell>& Spells() const;
    const std::list<Gem>& SentientGems() const;
    const std::list<WeaponGroup>& WeaponGroups() const;
    const std::list<Buff>& ItemBuffs() const;
    const std::list<Spell>& ItemClickies() const;
    const std::list<OptionalBuff>& OptionalBuffs() const;
    const std::list<Patron>& Patrons() const;
    const std::list<Quest>& Quests() const;
    const std::list<std::string>& IgnoreList() const;
    const std::list<GuildBuff>& GuildBuffs() const;
    void UpdateIgnoreList(const std::list<std::string>& itemList);

    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual void PreLoadState();
    virtual void LoadCustomState();
    virtual void SaveCustomState();

    // Retrieve an integer value from INI file or registry.
    virtual UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
    virtual BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
    virtual CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
    virtual BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
    virtual BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes);
    virtual BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);

    afx_msg void OnAppAbout();
    afx_msg void OnFileNew();
    afx_msg void OnVerifyLoadedData();
    afx_msg void OnUpdateDisabledDuringLoad(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDisabledDuringLoadSpecial(CCmdUI* pCmdUI);
    afx_msg void OnFileImport();
    DECLARE_MESSAGE_MAP()
private:
    void LoadData();
    void LoadBonusTypes(const std::string& path);
    void LoadRaces(const std::string& path);
    void LoadClasses(const std::string& path);
    void LoadFeats(const std::string& path);
    void LoadGuildBuffs(const std::string& path);
    void SeparateFeats();   // breaks them out into groups based on type
    void LoadEnhancements(const std::string& path);
    void LoadAugments(const std::string& path);
    void LoadSetBonuses(const std::string& path);
    void LoadFiligrees(const std::string& path);
    void LoadStances(const std::string& path);
    void LoadSpells(const std::string& path);
    void LoadSpellImages(const std::string& path);
    void LoadPatrons(const std::string& path);
    void LoadQuests(const std::string& path);
    void LoadSentientGems(const std::string& path);
    void LoadWeaponGroups(const std::string& path);
    void LoadItemBuffs(const std::string& path);
    void LoadItemClickies(const std::string& path);
    void LoadOptionalBuffs(const std::string& path);
    void LoadIgnoreList(const std::string& path);
    void UpdateFeats();
    void VerifyLoadedData();
    void VerifyClasses();
    void VerifyFeats();
    void VerifyEnhancements();     // verify that all enhancements are consistent
    void VerifyAugments();
    void VerifyFiligrees();
    void VerifySetBonuses();
    void VerifyStances();
    void VerifySpells();
    void VerifyItems();
    void VerifyBuffs();
    void VerifySentientGems();
    void VerifyWeaponGroups();
    void VerifyItemClickies();
    void VerifyQuests();
    void NotifyLoadComplete();
    static void LoadItemImage(CDDOBuilderApp* pApp, const std::string& localPath, std::string filename);
    void LoadSpellImage(std::string filename);

    static UINT ThreadedItemLoad(LPVOID pParam);
    void ConvertToNewDataStructure(LegacyCharacter& importedCharacter);

    CCustomContextMenuManager m_ourMenuManager; // construction of object replaces default implementation created in InitContextMenuManager
    std::list<Bonus> m_bonusTypes;
    std::list<Race> m_races;
    std::list<Class> m_classes;
    std::map<std::string, Feat> m_allFeats;
    std::list<Feat> m_heroicPastLifeFeats;
    std::list<Feat> m_racialPastLifeFeats;
    std::list<Feat> m_iconicPastLifeFeats;
    std::list<Feat> m_epicPastLifeFeats;
    std::list<Feat> m_specialFeats;
    std::list<Feat> m_universalTreeFeats;
    std::list<Feat> m_destinyTreeFeats;
    std::list<Feat> m_favorFeats;
    std::list<EnhancementTree> m_enhancementTrees;
    std::list<Augment> m_augments;
    std::list<Filigree> m_filigrees;
    std::list<SetBonus> m_setBonuses;
    std::list<Stance> m_stances;
    std::list<Spell> m_spells;
    std::list<Item> m_items;
    std::list<Gem> m_sentientGems;
    std::list<GuildBuff> m_guildBuffs;
    std::list<WeaponGroup> m_weaponGroups;
    std::list<Buff> m_itemBuffs;
    std::list<Spell> m_itemClickies;
    std::list<OptionalBuff> m_selfAndPartyBuffs;
    std::list<Quest> m_quests;
    std::list<Patron> m_patrons;
    std::list<std::string> m_ignoreList;
    std::list<Item> m_itemsForSlot[Inventory_FinalDrawnItem];
    CImageList m_itemImages;
    CImageList m_spellImages;
    std::map<std::string, int> m_itemImagesMap;
    std::map<std::string, int> m_spellImagesMap;
    bool m_bLoadComplete;
    CString m_iniFileFilename;

    // threaded item load variables
    bool m_bItemLoadThreadRunning;
    bool m_bKillItemLoadThread;

    friend class Item;
    friend class EquippedGear;
};

extern CDDOBuilderApp theApp;
