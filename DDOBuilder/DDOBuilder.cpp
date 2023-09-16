// DDOBuilder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "DDOBuilder.h"
#include "MainFrm.h"
#include "AboutDlg.h"
#include "GlobalSupportFunctions.h"

#include "BonusTypesFile.h"
#include "BuffFile.h"
#include "FeatsFile.h"
#include "MultiFileObjectLoader.h"
#include "PatronsFile.h"
#include "QuestsFile.h"
#include "SentientGemsFile.h"
#include "SetBonusFile.h"
#include "StancesFile.h"
#include "SpellsFile.h"
#include "WeaponGroupFile.h"
#include "IgnoredListFile.h"

#include "DDOBuilderDoc.h"
#include "DDOBuilderView.h"
#include "LocalSettingsStore.h"
#include "LogPane.h"

// CDDOBuilderApp

BEGIN_MESSAGE_MAP(CDDOBuilderApp, CWinAppEx)
    ON_COMMAND(ID_APP_ABOUT, &CDDOBuilderApp::OnAppAbout)
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, &CDDOBuilderApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
    // UI options disabled during start up
    ON_UPDATE_COMMAND_UI(ID_APP_ABOUT, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_UPDATE_COMMAND_UI(ID_FILE_NEW, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_UPDATE_COMMAND_UI(ID_DEVELOPMENT_VERIFYLOADEDDATA, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_LAST, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_COMMAND(ID_DEVELOPMENT_VERIFYLOADEDDATA, OnVerifyLoadedData)
END_MESSAGE_MAP()

// CDDOBuilderApp construction
CDDOBuilderApp::CDDOBuilderApp() :
    m_nAppLook(0),
    m_bLoadComplete(false)
{
    EnableHtmlHelp();

    m_bHiColorIcons = TRUE;
    SetAppID(_T("DDOBuilder.AppID.NoVersion"));

    m_itemImages.Create(
        32,
        32,
        ILC_COLOR32,
        2000,
        0);
    g_bShowIgnoredItems = false;
}

// The one and only CDDOBuilderApp object

CDDOBuilderApp theApp;

// CDDOBuilderApp initialization
BOOL CDDOBuilderApp::InitInstance()
{
    //  store settings in a file in local ini file, not in the registry
    CSettingsStoreSP::SetRuntimeClass(RUNTIME_CLASS(CLocalSettingsStore));

    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinAppEx::InitInstance();

    // Initialize OLE libraries
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }

    AfxEnableControlContainer();

    EnableTaskbarInteraction(FALSE);

    // AfxInitRichEdit2() is required to use RichEdit control	
    // AfxInitRichEdit2();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("DDOBuilder"));
    LoadStdProfileSettings(16);  // Load standard INI file options (including MRU)

    //InitContextMenuManager(); // we construct our own custom one
    InitShellManager();

    InitKeyboardManager();

    InitTooltipManager();
    CMFCToolTipInfo ttParams;
    ttParams.m_bVislManagerTheme = TRUE;
    theApp.GetTooltipManager()->SetTooltipParams(
            AFX_TOOLTIP_TYPE_ALL,
            RUNTIME_CLASS(CMFCToolTipCtrl),
            &ttParams);

    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views
    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
            IDR_MAINFRAME,
            RUNTIME_CLASS(CDDOBuilderDoc),
            RUNTIME_CLASS(CMainFrame),       // main SDI frame window
            RUNTIME_CLASS(CDDOBuilderView));
    if (!pDocTemplate)
        return FALSE;
    AddDocTemplate(pDocTemplate);

    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    // Enable DDE Execute open
    EnableShellOpen();
    RegisterShellFileTypes(TRUE);

    // Dispatch commands specified on the command line.  Will return FALSE if
    // app was launched with /RegServer, /Register, /Unregserver or /Unregister.
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    AfxGetApp()->m_pMainWnd = m_pMainWnd;
    // The one and only window has been initialized, so show and update it
    m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
    m_pMainWnd->UpdateWindow();
    // call DragAcceptFiles only if there's a suffix
    //  In an SDI app, this should occur after ProcessShellCommand
    // Enable drag/drop open
    m_pMainWnd->DragAcceptFiles();

    OnIdle(0);  // get UI to update
    LoadData();

    NotifyLoadComplete();
    GetLog().AddLogEntry("Ready");

    return TRUE;
}

int CDDOBuilderApp::ExitInstance()
{
    //TODO: handle additional resources you may have added
    AfxOleTerm(FALSE);

    return CWinAppEx::ExitInstance();
}

// CDDOBuilderApp message handlers

// App command to run the dialog
void CDDOBuilderApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

void CDDOBuilderApp::OnFileNew()
{
    CWinAppEx::OnFileNew();
    if (m_bLoadComplete)
    {
        CWnd* pWnd = AfxGetApp()->m_pMainWnd;
        CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
        CBuildsPane* pBuildsPane = dynamic_cast<CBuildsPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CBuildsPane)));
        if (pBuildsPane != NULL)
        {
            pBuildsPane->OnButtonNewLife();
        }
    }
}

void CDDOBuilderApp::OnVerifyLoadedData()
{
    VerifyLoadedData();
}

// CDDOBuilderApp customization load/save methods

void CDDOBuilderApp::PreLoadState()
{
    BOOL bNameValid;
    CString strName;
    bNameValid = strName.LoadString(IDS_EDIT_MENU);
    ASSERT(bNameValid);
    GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CDDOBuilderApp::LoadCustomState()
{
}

void CDDOBuilderApp::SaveCustomState()
{
}

// CDDOBuilderApp message handlers
const std::list<Bonus>& CDDOBuilderApp::BonusTypes() const
{
    return m_bonusTypes;
}

const std::list<Race>& CDDOBuilderApp::Races() const
{
    return m_races;
}

const std::list<Class>& CDDOBuilderApp::Classes() const
{
    return m_classes;
}

void CDDOBuilderApp::LoadData()
{
    std::string folderPath = DataFolder();
    LoadBonusTypes(folderPath);
    LoadEnhancements(folderPath);
    LoadSpells(folderPath);         // spells must load before classes
    LoadRaces(folderPath);
    LoadClasses(folderPath);
    LoadFeats(folderPath);
    LoadAugments(folderPath);
    LoadSetBonuses(folderPath);
    LoadFiligrees(folderPath);
    LoadStances(folderPath);
    LoadSentientGems(folderPath);
    LoadWeaponGroups(folderPath);
    LoadItemBuffs(folderPath);
    LoadItemClickies(folderPath);
    LoadPatrons(folderPath);
    LoadQuests(folderPath);
    LoadItems(folderPath);
    LoadIgnoreList(folderPath);

    // all loaded feats need to be consolidated into a single map
    // and also have their groups updated so they match any additional
    // feat slots they are now compatible with
    UpdateFeats();
}

void CDDOBuilderApp::LoadBonusTypes(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "BonusTypes.xml";

    GetLog().AddLogEntry("Loading Bonus Types...");
    BonusTypesFile file(filename);
    file.Read();
    m_bonusTypes = file.BonusTypes();
}

void CDDOBuilderApp::LoadRaces(const std::string& path)
{
    std::string localPath(path);
    localPath += "Races\\";
    MultiFileObjectLoader<Race> file(L"Races", localPath, "*.race.xml");
    file.ReadFiles("Loading Races...");
    m_races = file.LoadedObjects();
}

void CDDOBuilderApp::LoadClasses(const std::string& path)
{
    std::string localPath(path);
    localPath += "Classes\\";
    MultiFileObjectLoader<Class> file(L"Classes", localPath, "*.class.xml");
    file.ReadFiles("Loading Classes...");
    m_classes = file.LoadedObjects();
    m_classes.sort();       // sort alphabetically
    Class::CreateClassImageLists();
}

void CDDOBuilderApp::LoadFeats(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "Feats.xml";

    GetLog().AddLogEntry("Loading Feats...");
    FeatsFile file(filename);
    file.Read();
    m_allFeats = file.Feats();

    // also include all class specific feats
    for (auto&& cit: m_classes)
    {
        std::list<Feat> ihdfs = cit.ImprovedHeroicDurabilityFeats();
        for (auto&& it: ihdfs)
        {
            m_allFeats.insert(std::pair<std::string, Feat>(it.Name(), it));
            CString strFeatCount;
            strFeatCount.Format("Loading Feats...%d", m_allFeats.size());
            GetLog().UpdateLastLogEntry(strFeatCount);
        }
        // also add any class specific feats
        for (auto&& cfit : cit.ClassFeats())
        {
            m_allFeats.insert(std::pair<std::string, Feat>(cfit.Name(), cfit));
            CString strFeatCount;
            strFeatCount.Format("Loading Feats...%d", m_allFeats.size());
            GetLog().UpdateLastLogEntry(strFeatCount);
        }
        // no need to keep 2 copies of the feats
        cit.ClearClassFeats();
        // make sure the classes have all their spell info
        // (note that spells must be loaded before classes for this to work,
        // and can only be done on the list in the app object)
        cit.CreateSpellLists();
    }
    for (auto&& rit: m_races)
    {
        for (auto&& rfit : rit.RacialFeats())
        {
            m_allFeats.insert(std::pair<std::string, Feat>(rfit.Name(), rfit));
            CString strFeatCount;
            strFeatCount.Format("Loading Feats...%d", m_allFeats.size());
            GetLog().UpdateLastLogEntry(strFeatCount);
        }
        // no need to keep 2 copies of the feats
        rit.ClearRacialFeats();
    }
    // update the class completionist feat to include all the required
    // past lives for classes
    auto fit = m_allFeats.find("Completionist");
    if (fit != m_allFeats.end())
    {
        Feat* completionist = &fit->second;
        // we need to build arrays or archetypes so we can build the requirements correctly
        Requirements req;
        std::vector<bool> bDone;                // true if this class has had its entry done
        bDone.resize(m_classes.size(), false);
        size_t ci = 0;
        for (auto&& cit : m_classes)
        {
            size_t architypeCount = 1;          // this is the first of this one
            if (!cit.HasNotHeroic()
                    && cit.GetBaseClass() != Class_Unknown
                    && !bDone[ci])
            {
                bDone[ci] = true;
                std::string baseClass = cit.GetBaseClass();
                RequiresOneOf roo;
                std::stringstream ss;
                ss << "Past Life: " << cit.Name();
                Requirement classRequirement(Requirement_Feat, ss.str(), 1);
                roo.AddRequirement(classRequirement);
                // see if any of the following classes are the same archetype base class
                size_t aci = 0;
                for (auto&& acit : m_classes)
                {
                    if (!acit.HasNotHeroic()
                            && !bDone[aci]
                            && acit.GetBaseClass() == baseClass)
                    {
                        // same base class
                        std::stringstream ass;
                        ass << "Past Life: " << acit.Name();
                        Requirement architypeClassRequirement(Requirement_Feat, ass.str(), 1);
                        roo.AddRequirement(architypeClassRequirement);
                        ++architypeCount;
                        bDone[aci] = true;
                    }
                    ++aci;
                }
                if (architypeCount == 1)
                {
                    req.AddRequirement(classRequirement);
                }
                else
                {
                    req.AddRequiresOneOf(roo);
                }
            }
            bDone[ci] = true;
            ++ci;
        }
        completionist->SetRequirements(req);
    }
    else
    {
        GetLog().AddLogEntry("Error: Failed to find \"Completionist\" feat.");
    }
    // update the racial completionist feat to include all non-iconic races loaded
    fit = m_allFeats.find("Racial Completionist");
    if (fit != m_allFeats.end())
    {
        Feat* racialCompletionist = &fit->second;
        Requirements req;
        for (auto&& rit : m_races)
        {
            if (!rit.IsIconic()
                    && !rit.HasNoPastLife())
            {
                std::stringstream ss;
                ss << "Past Life: " << rit.Name();
                Requirement raceRequirement(Requirement_Feat, ss.str(), 3);
                req.AddRequirement(raceRequirement);
            }
        }
        racialCompletionist->SetRequirements(req);
    }
    else
    {
        GetLog().AddLogEntry("Error: Failed to find \"Racial Completionist\" feat.");
    }
    SeparateFeats();
}

void CDDOBuilderApp::SeparateFeats()
{
    // break each feat out into special sub groups
    auto it = m_allFeats.begin();
    while (it != m_allFeats.end())
    {
        if (it->second.Acquire() == FeatAcquisition_EpicPastLife)
        {
            m_epicPastLifeFeats.push_back(it->second);
        }
        else if (it->second.Acquire() == FeatAcquisition_HeroicPastLife)
        {
            m_heroicPastLifeFeats.push_back(it->second);
        }
        else if (it->second.Acquire() == FeatAcquisition_RacialPastLife)
        {
            m_racialPastLifeFeats.push_back(it->second);
        }
        else if (it->second.Acquire() == FeatAcquisition_IconicPastLife)
        {
            m_iconicPastLifeFeats.push_back(it->second);
        }
        else if (it->second.Acquire() == FeatAcquisition_Special)
        {
            m_specialFeats.push_back(it->second);
        }
        else if (it->second.Acquire() == FeatAcquisition_UniversalTree)
        {
            m_universalTreeFeats.push_back(it->second);
        }
        else if (it->second.Acquire() == FeatAcquisition_EpicDestinyTree)
        {
            m_destinyTreeFeats.push_back(it->second);
        }
        else if (it->second.Acquire() == FeatAcquisition_Favor)
        {
            m_favorFeats.push_back(it->second);
        }
        else
        {
            // has to be a bog standard feat, leave it in the map
        }
        ++it;
    }
    // now sort them in to order
    m_epicPastLifeFeats.sort();
    m_heroicPastLifeFeats.sort();
    m_racialPastLifeFeats.sort();
    m_iconicPastLifeFeats.sort();
    m_specialFeats.sort();
    m_universalTreeFeats.sort();
    m_destinyTreeFeats.sort();
    m_favorFeats.sort();
}

const std::map<std::string, Feat> & CDDOBuilderApp::AllFeats() const
{
    return m_allFeats;
}

const std::list<Feat> & CDDOBuilderApp::HeroicPastLifeFeats() const
{
    return m_heroicPastLifeFeats;
}

const std::list<Feat> & CDDOBuilderApp::RacialPastLifeFeats() const
{
    return m_racialPastLifeFeats;
}

const std::list<Feat> & CDDOBuilderApp::IconicPastLifeFeats() const
{
    return m_iconicPastLifeFeats;
}

const std::list<Feat> & CDDOBuilderApp::EpicPastLifeFeats() const
{
    return m_epicPastLifeFeats;
}

const std::list<Feat> & CDDOBuilderApp::SpecialFeats() const
{
    return m_specialFeats;
}

const std::list<Feat> & CDDOBuilderApp::UniversalTreeFeats() const
{
    return m_universalTreeFeats;
}

const std::list<Feat> & CDDOBuilderApp::DestinyTreeFeats() const
{
    return m_destinyTreeFeats;
}

const std::list<Feat> & CDDOBuilderApp::FavorFeats() const
{
    return m_favorFeats;
}

const std::list<EnhancementTree> & CDDOBuilderApp::EnhancementTrees() const
{
    return m_enhancementTrees;
}

const std::list<Augment>& CDDOBuilderApp::Augments() const
{
    return m_augments;
}

const std::list<Filigree>& CDDOBuilderApp::Filigrees() const
{
    return m_filigrees;
}

const std::list<Item>& CDDOBuilderApp::Items() const
{
    return m_items;
}

const std::list<Patron>& CDDOBuilderApp::Patrons() const
{
    return m_patrons;
}

const std::list<Quest>& CDDOBuilderApp::Quests() const
{
    return m_quests;
}

CImageList& CDDOBuilderApp::ItemImageList()
{
    return m_itemImages;
}

const std::list<SetBonus>& CDDOBuilderApp::SetBonuses() const
{
    return m_setBonuses;
}

const std::list<Stance>& CDDOBuilderApp::Stances() const
{
    return m_stances;
}

const std::list<Spell>& CDDOBuilderApp::Spells() const
{
    return m_spells;
}

const std::list<Gem>& CDDOBuilderApp::SentientGems() const
{
    return m_sentientGems;
}

const std::list<WeaponGroup>& CDDOBuilderApp::WeaponGroups() const
{
    return m_weaponGroups;
}

const std::list<Buff>& CDDOBuilderApp::ItemBuffs() const
{
    return m_itemBuffs;
}

const std::list<Spell>& CDDOBuilderApp::ItemClickies() const
{
    return m_itemClickies;
}

void CDDOBuilderApp::VerifyLoadedData()
{
    VerifyClasses();
    VerifyFeats();
    VerifyEnhancements();
    VerifyAugments();
    VerifyFiligrees();
    VerifyItems();
    //VerifyOptionalBuffs();
    VerifySetBonuses();
    VerifyStances();
    VerifySpells();
    VerifySentientGems();
    VerifyWeaponGroups();
    VerifyItemClickies();
    VerifyQuests();
}

void CDDOBuilderApp::VerifyClasses()
{
    for (auto&& cit: m_classes)
    {
        cit.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyFeats()
{
    for (auto&& fit : m_allFeats)
    {
        fit.second.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyEnhancements()
{
    std::map<std::string, int> names;
    for (auto&& eit : m_enhancementTrees)
    {
        eit.VerifyObject(&names, m_enhancementTrees);
    }
}

void CDDOBuilderApp::VerifyAugments()
{
    for (auto&& ait : m_augments)
    {
        ait.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyFiligrees()
{
    for (auto&& fit : m_filigrees)
    {
        fit.VerifyObject();
    }
}

void CDDOBuilderApp::LoadEnhancements(const std::string& path)
{
    std::string localPath(path);
    localPath += "EnhancementTrees\\";
    MultiFileObjectLoader<EnhancementTree> file(L"Enhancements", localPath, "*.tree.xml");
    file.ReadFiles("Loading Enhancement Trees...");
    m_enhancementTrees = file.LoadedObjects();
}

void CDDOBuilderApp::LoadAugments(const std::string& path)
{
    std::string localPath(path);
    localPath += "Augments\\";
    MultiFileObjectLoader<Augment> file(L"Augments", localPath, "*.augments.xml");
    file.ReadFiles("Loading Augments...");
    m_augments = file.LoadedObjects();
}

void CDDOBuilderApp::LoadSetBonuses(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "SetBonuses.xml";

    GetLog().AddLogEntry("Loading Gear Set Bonuses...");
    SetBonusFile file(filename);
    file.Read();
    m_setBonuses = file.SetBonuses();
}

void CDDOBuilderApp::LoadFiligrees(const std::string& path)
{
    std::string localPath(path);
    localPath += "FiligreeSets\\";
    {
        MultiFileObjectLoader<Filigree> file(L"Filigrees", localPath, "*.filigree.xml");
        file.ReadFiles("Loading Filigrees...");
        m_filigrees = file.LoadedObjects();
        m_filigrees.sort();
    }

    {
        MultiFileObjectLoader<SetBonus> file(L"Filigrees", localPath, "*.filigree.xml");
        file.ReadFiles("Loading Filigrees Set Bonuses...");
        std::list<SetBonus> filigreeSetBonuses = file.LoadedObjects();
        m_setBonuses.insert(m_setBonuses.end(), filigreeSetBonuses.begin(), filigreeSetBonuses.end());
    }
}

void CDDOBuilderApp::LoadStances(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "Stances.xml";

    GetLog().AddLogEntry("Loading Stances...");
    StancesFile file(filename);
    file.Read();
    m_stances = file.Stances();
}

void CDDOBuilderApp::LoadSpells(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "Spells.xml";

    GetLog().AddLogEntry("Loading Spells...");
    SpellsFile file(filename, false);
    file.Read();
    m_spells = file.Spells();
}

void CDDOBuilderApp::LoadItems(const std::string& path)
{
    {
        // item images are in sub folders. look in each sub folder
        CDDOBuilderApp::LoadImage("", "NoImage");    // always first index
        std::string folders[] =
        {
            "Armor_Cloth\\",
            "Armor_Docent\\",
            "Armor_Heavy\\",
            "Armor_Light\\",
            "Armor_Medium\\",
            "Belts\\",
            "Boots\\",
            "Bracers\\",
            "Cloaks\\",
            "Gloves\\",
            "Goggles\\",
            "Helmets\\",
            "Necklace\\",
            "Orbs\\",
            "Rings\\",
            "Quiver\\",
            "Shields\\",
            "Trinkets\\",
            "Weapon_HeavyCrossbow\\",
            "Weapon_BastardSword\\",
            "Weapon_BattleAxe\\",
            "Weapon_Club\\",
            "Weapon_Dagger\\",
            "Weapon_Dart\\",
            "Weapon_DwarvenAxe\\",
            "Weapon_Falchion\\",
            "Weapon_GreatAxe\\",
            "Weapon_GreatClub\\",
            "Weapon_GreatCrossbow\\",
            "Weapon_GreatSword\\",
            "Weapon_HandAxe\\",
            "Weapon_Handwraps\\",
            "Weapon_HeavyMace\\",
            "Weapon_HeavyPick\\",
            "Weapon_Kama\\",
            "Weapon_Khopesh\\",
            "Weapon_Kukri\\",
            "Weapon_LightCrossbow\\",
            "Weapon_LightHammer\\",
            "Weapon_LightMace\\",
            "Weapon_LightPick\\",
            "Weapon_Longbow\\",
            "Weapon_Longsword\\",
            "Weapon_Maul\\",
            "Weapon_Morningstar\\",
            "Weapon_Quarterstaff\\",
            "Weapon_Rapier\\",
            "Weapon_RepeatingHeavyCrossbow\\",
            "Weapon_RepeatingLightCrossbow\\",
            "Weapon_RuneArm\\",
            "Weapon_Scimitar\\",
            "Weapon_Shortbow\\",
            "Weapon_Shortsword\\",
            "Weapon_Shuriken\\",
            "Weapon_Sickle\\",
            "Weapon_ThrowingAxe\\",
            "Weapon_ThrowingDagger\\",
            "Weapon_ThrowingHammer\\",
            "Weapon_Warhammer\\"
        };
        size_t numFolders = sizeof(folders) / sizeof(std::string);
        int imageCount = 0;
        GetLog().AddLogEntry("Loading Item Images...");
        for (size_t fi = 0; fi < numFolders; ++fi)
        {
            std::string fileFilter(path);
            fileFilter += "ItemImages\\";
            fileFilter += folders[fi];
            fileFilter += "*.png";
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(fileFilter.c_str(), &findFileData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                CDDOBuilderApp::LoadImage(folders[fi], findFileData.cFileName);
                while (FindNextFile(hFind, &findFileData))
                {
                    ++imageCount;
                    CString text;
                    text.Format("Loading Item Images...%d", imageCount);
                    GetLog().UpdateLastLogEntry(text);
                    CDDOBuilderApp::LoadImage(folders[fi], findFileData.cFileName);
                }
                FindClose(hFind);
            }
        }
    }
    {
        // all the items are in the same folder
        std::string localPath(path);
        localPath += "Items\\";
        MultiFileObjectLoader<Item> file(L"Items", localPath, "*.item");
        file.ReadFiles("Loading Items...");
        m_items = file.LoadedObjects();
        // update all the items with their correct image index
        for (auto&& iit : m_items)
        {
            if (iit.HasIcon())
            {
                std::string icon = iit.Icon();
                if (m_imagesMap.find(icon) != m_imagesMap.end())
                {
                    iit.SetIconIndex(m_imagesMap[icon]);
                }
            }
        }
    }
}

void CDDOBuilderApp::LoadImage(const std::string& localPath, std::string filename)
{
    CImage image;
    std::string fullPath("DataFiles\\ItemImages\\");
    fullPath += localPath;
    size_t pos = filename.find(".png");
    if (pos != std::string::npos)
    {
        filename.replace(pos, 4, "");
    }
    if (S_OK == LoadImageFile(fullPath, filename, &image, CSize(32, 32), false))
    {
        CBitmap bitmap;
        bitmap.Attach(image.Detach());
        int imageIndex = m_itemImages.Add(&bitmap, c_transparentColour);
        m_imagesMap[filename] = imageIndex;
    }
}

void CDDOBuilderApp::LoadSentientGems(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "Sentient.gems.xml";

    GetLog().AddLogEntry("Loading Sentient Gems...");
    SentientGemsFile file(filename);
    file.Read();
    m_sentientGems = file.SentientGems();
    m_sentientGems.sort();
}

void CDDOBuilderApp::LoadWeaponGroups(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "WeaponGroupings.xml";

    GetLog().AddLogEntry("Loading Weapon Groups...");
    WeaponGroupFile file(filename);
    file.Read();
    m_weaponGroups = file.WeaponGroups();
    m_weaponGroups.sort();
}

void CDDOBuilderApp::LoadItemBuffs(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "ItemBuffs.xml";

    GetLog().AddLogEntry("Loading Item Buffs...");
    BuffFile file(filename);
    file.Read();
    m_itemBuffs = file.ItemBuffs();
}

void CDDOBuilderApp::LoadItemClickies(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "ItemClickies.xml";

    GetLog().AddLogEntry("Loading Item Clickies...");
    SpellsFile file(filename, true);
    file.Read();
    m_itemClickies = file.Spells();
}

void CDDOBuilderApp::LoadPatrons(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "Patrons.xml";

    GetLog().AddLogEntry("Loading Patron List...");
    PatronsFile file(filename);
    file.Read();
    m_patrons = file.Patrons();
}

void CDDOBuilderApp::LoadQuests(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "Quests.xml";

    GetLog().AddLogEntry("Loading Quest List...");
    QuestsFile file(filename);
    file.Read();
    m_quests = file.Quests();

    // update the loaded patrons with the max favor for each
    int patronMaxFavor[Patron_Count];
    memset(patronMaxFavor, 0, sizeof(patronMaxFavor[0]) * Patron_Count);
    for (auto&& qit: m_quests)
    {
        PatronType ePatron = qit.Patron();
        patronMaxFavor[ePatron] += qit.MaxFavor();
    }
    std::list<Patron>::iterator pit = m_patrons.begin();
    for (size_t i = 0; i < Patron_Count; ++i)
    {
        pit->SetMaxFavor(patronMaxFavor[i]);
        ++pit;
    }
}

void CDDOBuilderApp::UpdateFeats()
{
    // go through every class and race loaded and update
    // all feats that they define
    for (auto&& rit : m_races)
    {
        // first add the race specific feats to the main list
        for (auto&& rfit : rit.RacialFeats())
        {
            m_allFeats.insert(std::pair<std::string, Feat>(rfit.Name(), rfit));
        }
    }
    for (auto&& cit : m_classes)
    {
        // first add the race specific feats to the main list
        for (auto&& cfit : cit.ClassFeats())
        {
            m_allFeats.insert(std::pair<std::string, Feat>(cfit.Name(), cfit));
        }
    }
    // now update any feats to be in the race specific groupings as required
    for (auto&& rit : m_races)
    {
        for (auto&& fsit : rit.RaceSpecificFeat())
        {
            for (auto&& fulit : fsit.FeatUpdateList())
            {
                const Feat& feat = FindFeat(fulit);
                if (feat.Name() != "")
                {
                    // ok, a const cast isn't nice, but there's no other easy way to
                    // get access to the feat to update it.
                    Feat* pFeat = const_cast<Feat*>(&feat);
                    pFeat->AddGroup(fsit.FeatType());
                }
                else
                {
                    // the named feat was not found, we need to log an error
                    CString err;
                    err.Format("Race \"%s\", Feat Slot \"%s\" referenced unknown feat \"%s\"",
                            rit.Name().c_str(),
                            fsit.FeatType().c_str(),
                            fulit.c_str());
                    GetLog().AddLogEntry(err);
                }
            }
        }
    }
    // now update any feats to be in the class specific groupings as required
    for (auto&& cit : m_classes)
    {
        for (auto&& fsit : cit.FeatSlots())
        {
            for (auto&& fulit : fsit.FeatUpdateList())
            {
                const Feat& feat = FindFeat(fulit);
                if (feat.Name() != "")
                {
                    // ok, a const cast isn't nice, but there's no other easy way to
                    // get access to the feat to update it.
                    Feat* pFeat = const_cast<Feat*>(&feat);
                    pFeat->AddGroup(fsit.FeatType());
                }
                else
                {
                    // the named feat was not found, we need to log an error
                    CString err;
                    err.Format("Class \"%s\", Feat Slot \"%s\" referenced unknown feat \"%s\"",
                            cit.Name().c_str(),
                            fsit.FeatType().c_str(),
                            fulit.c_str());
                    GetLog().AddLogEntry(err);
                }
            }
        }
    }
    // and finally make sure we have all the feat icons setup
    Feat::CreateFeatImageList();
}

void CDDOBuilderApp::VerifySetBonuses()
{
    for (auto&& sbit : m_setBonuses)
    {
        sbit.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyStances()
{
    for (auto&& sit : m_stances)
    {
        std::stringstream ss;
        ss << "=====Stance " << sit.Name() << "=====\r\n";
        bool ok = sit.VerifyObject(&ss);
        if (!ok)
        {
            GetLog().AddLogEntry(ss.str().c_str());
        }
    }
}

void CDDOBuilderApp::VerifySpells()
{
    for (auto&& sit : m_spells)
    {
        sit.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyItems()
{
    for (auto&& iit : m_items)
    {
        iit.VerifyObject();
    }
}

void CDDOBuilderApp::VerifySentientGems()
{
    for (auto&& sit : m_sentientGems)
    {
        sit.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyWeaponGroups()
{
    for (auto&& wgit : m_weaponGroups)
    {
        wgit.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyItemClickies()
{
    for (auto&& icit : m_itemClickies)
    {
        icit.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyQuests()
{
    for (auto&& qit : m_quests)
    {
        qit.VerifyObject();
    }
}

void CDDOBuilderApp::NotifyLoadComplete()
{
    // let all dock views and open documents know that the load of program data
    // has completed, so they can initialise correctly
    CMainFrame * pMainFrame = dynamic_cast<CMainFrame*>(m_pMainWnd);
    pMainFrame->LoadComplete();
    m_bLoadComplete = true;
    // create a default life
    CWnd* pWnd = AfxGetApp()->m_pMainWnd;
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    CBuildsPane* pBuildsPane = dynamic_cast<CBuildsPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CBuildsPane)));
    if (pBuildsPane != NULL)
    {
        pBuildsPane->OnButtonNewLife();
    }
}

void CDDOBuilderApp::OnUpdateDisabledDuringLoad(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bLoadComplete);
}

void CDDOBuilderApp::UpdateIgnoreList(const std::list<std::string>& itemList)
{
    m_ignoreList = itemList;
}

const std::list<std::string>& CDDOBuilderApp::IgnoreList() const
{
    return m_ignoreList;
}

void CDDOBuilderApp::LoadIgnoreList(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "IgnoredList.xml";

    bool exists = false;
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(filename.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        exists = true;
    }

    if (exists)
    {
        IgnoredListFile file(filename);
        file.Read();
        m_ignoreList = file.IgnoredItems();
    }
}
