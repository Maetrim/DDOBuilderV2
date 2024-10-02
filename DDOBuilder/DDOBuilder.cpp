// DDOBuilder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <dwmapi.h>

#include "afxwinappex.h"
#include "afxdialogex.h"
#include "DDOBuilder.h"
#include "MainFrm.h"
#include "AboutDlg.h"
#include "GlobalSupportFunctions.h"

#include "BonusTypesFile.h"
#include "BuffFile.h"
#include "FeatsFile.h"
#include "GuildBuffsFile.h"
#include "LegacyCharacter.h"
#include "MultiFileObjectLoader.h"
#include "OptionalBuffFile.h"
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
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CDDOBuilderApp::OnUpdateDisabledDuringLoadSpecial)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CDDOBuilderApp::OnUpdateDisabledDuringLoadSpecial)
    ON_UPDATE_COMMAND_UI(ID_FILE_IMPORT, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_UPDATE_COMMAND_UI(ID_DEVELOPMENT_VERIFYLOADEDDATA, &CDDOBuilderApp::OnUpdateDisabledDuringLoadSpecial)
    ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_LAST, &CDDOBuilderApp::OnUpdateDisabledDuringLoad)
    ON_COMMAND(ID_DEVELOPMENT_VERIFYLOADEDDATA, OnVerifyLoadedData)
    ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
END_MESSAGE_MAP()

// CDDOBuilderApp construction
CDDOBuilderApp::CDDOBuilderApp() :
    m_nAppLook(0),
    m_bLoadComplete(false),
    m_bItemLoadThreadRunning(false),
    m_bKillItemLoadThread(false)
{
    EnableHtmlHelp();

    m_bHiColorIcons = TRUE;
    SetAppID(_T("DDOBuilder.AppID.NoVersion"));
    g_bShowIgnoredItems = false;

    // make the ini file filename
    char fullPath[MAX_PATH];
    ::GetModuleFileName(
        NULL,
        fullPath,
        MAX_PATH);

    char drive[_MAX_DRIVE];
    char folder[_MAX_PATH];
    _splitpath_s(fullPath,
        drive, _MAX_DRIVE,
        folder, _MAX_PATH,
        NULL, 0,        // filename
        NULL, 0);       // extension

    char path[_MAX_PATH];
    _makepath_s(path, _MAX_PATH, drive, folder, NULL, NULL);
    m_iniFileFilename = path;
    m_iniFileFilename += "DDOBuilder.ini";
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
    SetRegistryKey(_T("DDOBuilder"));
    LoadStdProfileSettings(16);  // Load standard INI file options (including MRU)
    // Change extension for help file
    CString strHelpFile = m_pszHelpFilePath;
#ifdef _DEBUG
    strHelpFile.Replace("D.HLP", ".HLP"); // convert to non-debug help file name
#endif
    strHelpFile.Replace(".HLP", ".chm");
    free((void*)m_pszHelpFilePath);
    m_pszHelpFilePath = _tcsdup(strHelpFile);

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
    m_bKillItemLoadThread = true;
    // wait for the load item thread to terminate if closed during start up
    while (m_bItemLoadThreadRunning)
    {
        Sleep(5);
    }
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
    POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
    CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(pos);
    pos = pTemplate->GetFirstDocPosition();
    CDocument* pDoc = pTemplate->GetNextDoc(pos);
    pos = pDoc->GetFirstViewPosition();
    CView* pView = pDoc->GetNextView(pos);

    if (m_bLoadComplete)
    {
        CWnd* pWnd = AfxGetApp()->m_pMainWnd;
        CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
        pMainWnd->SetActiveView(pView, FALSE);
        CBuildsPane* pBuildsPane = dynamic_cast<CBuildsPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CBuildsPane)));
        if (pBuildsPane != NULL)
        {
            pBuildsPane->OnButtonNewLife();
            pMainWnd->NewDocument(dynamic_cast<CDDOBuilderDoc*>(pDoc));
        }
    }
}

void CDDOBuilderApp::OnFileImport()
{
    CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, "DDOBuilder V1 Files (*.ddocp)|*.ddocp||");
    if (IDOK == dlg.DoModal())
    {
        CString filename = dlg.GetPathName();
        CWaitCursor longOperation;
        LegacyCharacter importedCharacter;
        // set up a reader with this as the expected root node
        XmlLib::SaxReader reader(&importedCharacter, L"DDOCharacterData");
        // read in the xml from a file (fully qualified path)
        bool ok = reader.Open((LPCTSTR)filename);
        if (ok)
        {
            std::stringstream ss;
            ss << "Legacy DDOBuilder V1 Document \"" << (LPCTSTR)filename << "\" loaded.";
            GetLog().AddLogEntry(ss.str().c_str());
            ConvertToNewDataStructure(importedCharacter);
        }
        else
        {
            std::string errorMessage = reader.ErrorMessage();
            // document has failed to load. Tell the user what we can about it
            CString text;
            text.Format("The document %s\n"
                "failed to load. The XML parser reported the following problem:\n"
                "\n", (LPCTSTR)filename);
            text += errorMessage.c_str();
            AfxMessageBox(text, MB_ICONERROR);
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
    VERIFY(bNameValid);
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
    int lastImageCount = GetProfileInt("ItemLoad", "LastImageCount", 2173);
    m_itemImages.Create(
        32,
        32,
        ILC_COLOR32,
        lastImageCount,
        100);
    int lastSpellCount = GetProfileInt("SpellLoad", "LastSpellCount", 747);
    m_spellImages.Create(
        32,
        32,
        ILC_COLOR32,
        lastSpellCount,
        100);

    std::string folderPath = DataFolder();
    LoadBonusTypes(folderPath);
    LoadEnhancements(folderPath);
    LoadSpells(folderPath);         // spells must load before classes
    LoadSpellImages(folderPath);
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
    LoadOptionalBuffs(folderPath);
    LoadPatrons(folderPath);
    LoadQuests(folderPath);
    LoadGuildBuffs(folderPath);
    LoadIgnoreList(folderPath);
    // done last as not thread safe
    AfxBeginThread(CDDOBuilderApp::ThreadedItemLoad, this);

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
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Bonus Types...%d", m_bonusTypes.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadRaces(const std::string& path)
{
    std::string localPath(path);
    localPath += "Races\\";
    MultiFileObjectLoader<Race> file(L"Races", localPath, "*.race.xml");
    GetLog().AddLogEntry("Loading Races...");
    file.ReadFiles("");
    m_races = file.LoadedObjects();
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Races...%d", m_races.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadClasses(const std::string& path)
{
    std::string localPath(path);
    localPath += "Classes\\";
    MultiFileObjectLoader<Class> file(L"Classes", localPath, "*.class.xml");
    GetLog().AddLogEntry("Loading Classes...");
    file.ReadFiles("");
    m_classes = file.LoadedObjects();
    m_classes.sort();       // sort alphabetically and by base classes
    // now move the "Unknown" class to the top of the list
    Class unknown = FindClass(Class_Unknown);
    size_t pos = ClassIndex(Class_Unknown);
    std::list<Class>::iterator cit = m_classes.begin();
    std::advance(cit, pos);
    m_classes.erase(cit);
    m_classes.push_front(unknown);

    Class::CreateClassImageLists();
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Classes...%d", m_classes.size());
    GetLog().UpdateLastLogEntry(strUpdate);
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
        }
        // also add any class specific feats
        for (auto&& cfit : cit.ClassFeats())
        {
            m_allFeats.insert(std::pair<std::string, Feat>(cfit.Name(), cfit));
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
        }
        // no need to keep 2 copies of the feats
        rit.ClearRacialFeats();
    }
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Feats...%d", m_allFeats.size());
    GetLog().UpdateLastLogEntry(strUpdate);

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
                        ass << "Past Life: " << acit.BaseClass() << " - " << acit.Name();
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

CImageList& CDDOBuilderApp::SpellImageList()
{
    return m_spellImages;
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

const std::list<OptionalBuff>& CDDOBuilderApp::OptionalBuffs() const
{
    return m_selfAndPartyBuffs;
}

void CDDOBuilderApp::VerifyLoadedData()
{
    VerifyClasses();
    VerifyFeats();
    VerifyEnhancements();
    VerifyAugments();
    VerifyFiligrees();
    VerifyItems();
    VerifyBuffs();
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
    GetLog().AddLogEntry("Loading Enhancement Trees...");
    file.ReadFiles("");
    m_enhancementTrees = file.LoadedObjects();
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Enhancement Trees...%d", m_enhancementTrees.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadAugments(const std::string& path)
{
    std::string localPath(path);
    localPath += "Augments\\";
    MultiFileObjectLoader<Augment> file(L"Augments", localPath, "*.augments.xml");
    GetLog().AddLogEntry("Loading Augments...");
    file.ReadFiles("");
    m_augments = file.LoadedObjects();
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Augments...%d", m_augments.size());
    GetLog().UpdateLastLogEntry(strUpdate);
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

    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Gear Set Bonuses...%d", m_setBonuses.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadFiligrees(const std::string& path)
{
    std::string localPath(path);
    localPath += "FiligreeSets\\";
    {
        MultiFileObjectLoader<Filigree> file(L"Filigrees", localPath, "*.filigree.xml");
        GetLog().AddLogEntry("Loading Filigrees...");
        file.ReadFiles("");
        m_filigrees = file.LoadedObjects();
        m_filigrees.sort();

        // update log after load action
        CString strUpdate;
        strUpdate.Format("Loading Filigrees...%d", m_filigrees.size());
        GetLog().UpdateLastLogEntry(strUpdate);
    }

    {
        MultiFileObjectLoader<SetBonus> file(L"Filigrees", localPath, "*.filigree.xml");
        GetLog().AddLogEntry("Loading Filigree Set Bonuses...");
        file.ReadFiles("");
        std::list<SetBonus> filigreeSetBonuses = file.LoadedObjects();
        m_setBonuses.insert(m_setBonuses.end(), filigreeSetBonuses.begin(), filigreeSetBonuses.end());
        // update log after load action
        CString strUpdate;
        strUpdate.Format("Loading Filigree Set Bonuses...%d", filigreeSetBonuses.size());
        GetLog().UpdateLastLogEntry(strUpdate);
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
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Stances...%d", m_stances.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadSpells(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "Spells.xml";

    GetLog().AddLogEntry("Loading Spells...");
    SpellsFile file(filename);
    file.Read();
    m_spells = file.Spells();
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Spells...%d", m_spells.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadSpellImages(const std::string& path)
{
    GetLog().AddLogEntry("Loading Spell Images...");
    // item images are in sub folders. look in each sub folder
    LoadSpellImage("NoImage");    // always first index
    int imageCount = 0;
    std::string filePath(path);
    filePath += "SpellImages\\";
    std::string fileFilter(filePath);
    fileFilter += "*.png";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(fileFilter.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        LoadSpellImage(findFileData.cFileName);
        while (FindNextFile(hFind, &findFileData))
        {
            ++imageCount;
            LoadSpellImage(findFileData.cFileName);
        }
        FindClose(hFind);
    }
    CString strUpdate;
    strUpdate.Format("Loading Spell Images...%d", imageCount);
    GetLog().UpdateLastLogEntry(strUpdate);
    WriteProfileInt("SpellLoad", "LastSpellCount", imageCount);

    // update all the spells with their correct image index
    for (auto&& iit : m_spells)
    {
        if (iit.HasIcon())
        {
            std::string icon = iit.Icon();
            if (m_spellImagesMap.find(icon) != m_spellImagesMap.end())
            {
                iit.SetIconIndex(m_spellImagesMap[icon]);
            }
        }
    }
}

void CDDOBuilderApp::LoadItemImage(CDDOBuilderApp* pApp, const std::string& localPath, std::string filename)
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
        int imageIndex = pApp->m_itemImages.Add(&bitmap, c_transparentColour);
        pApp->m_itemImagesMap[filename] = imageIndex;
    }
}

void CDDOBuilderApp::LoadSpellImage(std::string filename)
{
    CImage image;
    std::string fullPath("DataFiles\\SpellImages\\");
    size_t pos = filename.find(".png");
    if (pos != std::string::npos)
    {
        filename.replace(pos, 4, "");
    }
    if (S_OK == LoadImageFile(fullPath, filename, &image, CSize(32, 32), false))
    {
        CBitmap bitmap;
        bitmap.Attach(image.Detach());
        int imageIndex = m_spellImages.Add(&bitmap, c_transparentColour);
        m_spellImagesMap[filename] = imageIndex;
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
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Sentient Gems...%d", m_sentientGems.size());
    GetLog().UpdateLastLogEntry(strUpdate);
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
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Weapon Groups...%d", m_weaponGroups.size());
    GetLog().UpdateLastLogEntry(strUpdate);
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
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Item Buffs...%d", m_itemBuffs.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadItemClickies(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "ItemClickies.xml";

    GetLog().AddLogEntry("Loading Item Clickies...");
    SpellsFile file(filename);
    file.Read();
    m_itemClickies = file.Spells();
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Item Clickies...%d", m_itemClickies.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadOptionalBuffs(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "SelfAndPartyBuffs.xml";

    GetLog().AddLogEntry("Loading Self and Party Buffs...");
    OptionalBuffFile file(filename);
    file.Read();
    m_selfAndPartyBuffs = file.OptionalBuffs();
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Self and Party Buffs...%d", m_selfAndPartyBuffs.size());
    GetLog().UpdateLastLogEntry(strUpdate);
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
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Patron List...%d", m_patrons.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadQuests(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "Quests.xml";

    GetLog().AddLogEntry("Loading Quest List...");
    QuestsFile file(filename);
    file.Read();
    std::list<Quest> quests = file.Quests();

    // update the loaded patrons with the max favor for each
    int totalFavor = 0;
    int patronMaxFavor[Patron_Count];
    memset(patronMaxFavor, 0, sizeof(patronMaxFavor[0]) * Patron_Count);
    for (auto&& qit: quests)
    {
        // some quests (Devil Assault) appear multiple times and should only be counted once
        if (!qit.HasIgnoreForTotalFavor())
        {
            PatronType ePatron = qit.Patron();
            patronMaxFavor[ePatron] += qit.MaxFavor();
            totalFavor += qit.MaxFavor();
        }
        // now unpack the quests so we have multiple entries for each level
        const std::vector<size_t>& levels = qit.Levels();
        for (size_t li = 0; li < levels.size(); ++li)
        {
            Quest copy = qit;
            std::vector<size_t> singleLevel;
            singleLevel.push_back(levels[li]);
            copy.Set_Levels(singleLevel);
            if (li == 1 && copy.HasEpicName())
            {
                copy.Set_Name(copy.EpicName());
                copy.Clear_EpicName();
            }
            m_quests.push_back(copy);
        }
    }
    patronMaxFavor[Patron_TotalFavor] = totalFavor;
    std::list<Patron>::iterator pit = m_patrons.begin();
    for (size_t i = 0; i < Patron_Count; ++i)
    {
        pit->SetMaxFavor(patronMaxFavor[i]);
        ++pit;
    }
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Quests...%d", m_quests.size());
    GetLog().UpdateLastLogEntry(strUpdate);
}

void CDDOBuilderApp::LoadGuildBuffs(const std::string& path)
{
    // create the filename to load from
    std::string filename = path;
    filename += "GuildBuffs.xml";

    GetLog().AddLogEntry("Loading Guild Buffs List...");
    GuildBuffsFile file(filename);
    file.Read();
    m_guildBuffs = file.GuildBuffs();
    // update log after load action
    CString strUpdate;
    strUpdate.Format("Loading Guild Buffs List...%d", m_guildBuffs.size());
    GetLog().UpdateLastLogEntry(strUpdate);
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
    // make sure we do not have any spell duplicates
    std::list<Spell>::const_iterator it = m_spells.begin();
    while (it != m_spells.end())
    {
        std::list<Spell>::const_iterator nit = it;
        nit++;
        while (nit != m_spells.end())
        {
            if ((*it).Name() == (*nit).Name())
            {
                std::stringstream ss;
                ss << "===Duplicate spell " << (*nit).Name() << "=====";
                GetLog().AddLogEntry(ss.str().c_str());
            }
            ++nit;
        }
        ++it;
    }
}

void CDDOBuilderApp::VerifyItems()
{
    for (auto&& iit : m_items)
    {
        iit.VerifyObject();
    }
}

void CDDOBuilderApp::VerifyBuffs()
{
    for (auto&& ibit : m_itemBuffs)
    {
        ibit.VerifyObject();
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

void CDDOBuilderApp::OnUpdateDisabledDuringLoadSpecial(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bLoadComplete && !m_bItemLoadThreadRunning);
}

void CDDOBuilderApp::UpdateIgnoreList(const std::list<std::string>& itemList)
{
    m_ignoreList = itemList;
}

const std::list<std::string>& CDDOBuilderApp::IgnoreList() const
{
    return m_ignoreList;
}

const std::list<GuildBuff>& CDDOBuilderApp::GuildBuffs() const
{
    return m_guildBuffs;
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

UINT CDDOBuilderApp::ThreadedItemLoad(LPVOID pParam)
{
    CDDOBuilderApp* pApp = static_cast<CDDOBuilderApp*>(pParam);
    pApp->m_bItemLoadThreadRunning = true;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    VERIFY(hr == ERROR_SUCCESS);
    CString* pLoadingImages = new CString("Loading Item Images...");
    HWND hwndMainFrame = pApp->m_pMainWnd->GetSafeHwnd();
    ::SendMessage(hwndMainFrame,
            UWM_STARTPROGRESS,
            reinterpret_cast<WPARAM>(pLoadingImages),
            0L);
    int lastImageCount = pApp->GetProfileInt("ItemLoad", "LastImageCount", 2173);
    std::string folderPath = DataFolder();
    {
        // item images are in sub folders. look in each sub folder
        CDDOBuilderApp::LoadItemImage(pApp, "", "NoImage");    // always first index
        std::string folders[] =
        {
            "Armor_Cloth\\",
            "Armor_Docent\\",
            "Armor_Heavy\\",
            "Armor_Light\\",
            "Armor_Medium\\",
            "Armor_Cosmetic\\",
            "Belts\\",
            "Boots\\",
            "Bracers\\",
            "Collar\\",
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
        for (size_t fi = 0; fi < numFolders; ++fi)
        {
            std::string fileFilter(folderPath);
            fileFilter += "ItemImages\\";
            fileFilter += folders[fi];
            fileFilter += "*.png";
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(fileFilter.c_str(), &findFileData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                CDDOBuilderApp::LoadItemImage(pApp, folders[fi], findFileData.cFileName);
                while (FindNextFile(hFind, &findFileData))
                {
                    ++imageCount;
                    CDDOBuilderApp::LoadItemImage(pApp, folders[fi], findFileData.cFileName);
                    if (imageCount % 10 == 0)
                    {
                        // update the progress control
                        int percent = (imageCount * 100) / lastImageCount;
                        percent = min(100, percent);
                        ::SendMessage(hwndMainFrame, UWM_SETPROGRESS, percent, 0L);
                    }
                }
                FindClose(hFind);
            }
        }
        SendMessage(hwndMainFrame, UWM_ENDPROGRESS, 0L, 0L);
        // update image count for next run
        pApp->WriteProfileInt("ItemLoad", "LastImageCount", imageCount);
        // finally log to UI
        CString* pLoadedImages = new CString;
        pLoadedImages->Format("Item Images Loaded: %d", imageCount);
        SendMessage(hwndMainFrame,
                UWM_LOG_MESSAGE,
                reinterpret_cast<WPARAM>(pLoadedImages),
                0L);
    }
    // flag all items which drop in a raid so they can be excluded if required
    const std::list<Quest>& quests = pApp->Quests();
    std::vector<std::string> raidQuests;
    for (auto&& qit : quests)
    {
        if (qit.HasIsRaid())
        {
            raidQuests.push_back(qit.Name());
        }
    }
    {
        // all the items are in the same folder
        std::string localPath(folderPath);
        localPath += "Items\\";
        int lastItemCount = pApp->GetProfileInt("ItemLoad", "LastItemCount", 7030);
        MultiFileObjectLoader<Item> file(L"Items", localPath, "*.item", max(7030, lastItemCount), hwndMainFrame);
        file.ReadFiles("Loading Items...");
        pApp->m_items = file.LoadedObjects();
        pApp->WriteProfileInt("ItemLoad", "LastItemCount", pApp->m_items.size());
        // update all the items with their correct image index
        for (auto&& iit : pApp->m_items)
        {
            if (iit.HasIcon())
            {
                std::string icon = iit.Icon();
                if (pApp->m_itemImagesMap.find(icon) != pApp->m_itemImagesMap.end())
                {
                    iit.SetIconIndex(pApp->m_itemImagesMap[icon]);
                }
            }
            bool isRaidItem = false;
            if (iit.HasDropLocation())
            {
                for (auto&& rqit: raidQuests)
                {
                    if (iit.DropLocation().find(rqit) != std::string::npos)
                    {
                        isRaidItem = true;
                        break;
                    }
                }
            }
            iit.SetIsRaidItem(isRaidItem);
        }
        CString* pLoadedItems = new CString;
        pLoadedItems->Format("Items Loaded: %d", pApp->m_items.size());
        SendMessage(hwndMainFrame,
                UWM_LOG_MESSAGE,
                reinterpret_cast<WPARAM>(pLoadedItems),
                0L);
    }
    CoUninitialize();
    pApp->m_bItemLoadThreadRunning = false;
    ::SendMessage(hwndMainFrame, UWM_LOAD_COMPLETE, 0, 0L);
    return 0;
}

UINT CDDOBuilderApp::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
    return ::GetPrivateProfileInt(lpszSection, lpszEntry, nDefault, m_iniFileFilename);
}

BOOL CDDOBuilderApp::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
    TCHAR szT[16];
    _stprintf_s(szT, _countof(szT), _T("%d"), nValue);
    return ::WritePrivateProfileString(lpszSection, lpszEntry, szT, m_iniFileFilename);
}

CString CDDOBuilderApp::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
    if (lpszDefault == NULL)
    {
        lpszDefault = _T(""); // don't pass in NULL
    }
    TCHAR szT[4096];
    DWORD dw = ::GetPrivateProfileString(lpszSection, lpszEntry, lpszDefault, szT, _countof(szT), m_iniFileFilename);
    VERIFY(dw < 4095);
    return szT;
}

BOOL CDDOBuilderApp::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
    VERIFY(AtlStrLen(m_iniFileFilename) < 4095); // can't read in bigger
    return ::WritePrivateProfileString(lpszSection, lpszEntry, lpszValue, m_iniFileFilename);
}

BOOL CDDOBuilderApp::GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes)
{
    CString str = GetProfileString(lpszSection, lpszEntry, NULL);
    if (str.IsEmpty())
    {
        return FALSE;
    }
    VERIFY(str.GetLength() % 2 == 0);
    INT_PTR nLen = str.GetLength();
    *pBytes = UINT(nLen) / 2;
    *ppData = new BYTE[*pBytes+5];
    for (int i = 0; i < nLen; i += 2)
    {
        (*ppData)[i / 2] = (BYTE)(((str[i + 1] - 'A') << 4) + (str[i] - 'A'));
    }
    return TRUE;
}

BOOL CDDOBuilderApp::WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
    // convert to string and write out
    LPTSTR lpsz = new TCHAR[nBytes * 2 + 1];
    UINT i;
    for (i = 0; i < nBytes; i++)
    {
        lpsz[i * 2] = (TCHAR)((pData[i] & 0x0F) + 'A'); //low nibble
        lpsz[i * 2 + 1] = (TCHAR)(((pData[i] >> 4) & 0x0F) + 'A'); //high nibble
    }
    lpsz[i * 2] = 0;

    BOOL bResult = WriteProfileString(lpszSection, lpszEntry, lpsz);
    delete[] lpsz;
    return bResult;
}

void CDDOBuilderApp::ConvertToNewDataStructure(LegacyCharacter& importedCharacter)
{
    // create a new life with a single build to receive the imported character data
    CWnd* pWnd = AfxGetApp()->m_pMainWnd;
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    CBuildsPane* pBuildsPane = dynamic_cast<CBuildsPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CBuildsPane)));
    if (pBuildsPane != NULL)
    {
        Build* pBuild = pBuildsPane->OnNewImportLife();
        pBuild->SetName(importedCharacter.Name());
        pBuild->SetRace(importedCharacter.Race());
        pBuild->SetAlignment(importedCharacter.Alignment());
        pBuild->SetAbilityTome(Ability_Strength, importedCharacter.StrTome());
        pBuild->SetAbilityTome(Ability_Dexterity, importedCharacter.DexTome());
        pBuild->SetAbilityTome(Ability_Constitution, importedCharacter.ConTome());
        pBuild->SetAbilityTome(Ability_Intelligence, importedCharacter.IntTome());
        pBuild->SetAbilityTome(Ability_Wisdom, importedCharacter.WisTome());
        pBuild->SetAbilityTome(Ability_Charisma, importedCharacter.ChaTome());
        pBuild->SetAbilityLevelUp(4, importedCharacter.Level4());
        pBuild->SetAbilityLevelUp(8, importedCharacter.Level8());
        pBuild->SetAbilityLevelUp(12, importedCharacter.Level12());
        pBuild->SetAbilityLevelUp(16, importedCharacter.Level16());
        pBuild->SetAbilityLevelUp(20, importedCharacter.Level20());
        pBuild->SetAbilityLevelUp(24, importedCharacter.Level24());
        pBuild->SetAbilityLevelUp(28, importedCharacter.Level28());
        pBuild->SetAbilityLevelUp(32, importedCharacter.Level32());
        pBuild->SetAbilityLevelUp(36, importedCharacter.Level36());
        pBuild->SetAbilityLevelUp(40, importedCharacter.Level40());
        pBuild->Set_BuildPoints(importedCharacter.BuildPoints());
        pBuild->m_pLife->Set_Tomes(importedCharacter.Tomes());
        GetLog().AddLogEntry("Skill Tomes imported.");
        if (importedCharacter.HasClass1() && importedCharacter.Class1() != Class_Unknown) pBuild->SetClass1(importedCharacter.Class1());
        if (importedCharacter.HasClass2() && importedCharacter.Class2() != Class_Unknown) pBuild->SetClass2(importedCharacter.Class2());
        if (importedCharacter.HasClass3() && importedCharacter.Class3() != Class_Unknown) pBuild->SetClass3(importedCharacter.Class3());
        // need to set level up classes and feats before enhancements
        for (auto&& sfit: importedCharacter.SpecialFeats().Feats())
        {
            pBuild->TrainSpecialFeat(sfit.FeatName());
        }
        pBuild->Set_Levels(importedCharacter.Levels());
        pBuild->Set_Level(importedCharacter.Levels().size());
        pBuild->UpdateCachedClassLevels();
        size_t level = 0;
        for (auto&& lit: importedCharacter.Levels())
        {
            for (auto&& tfit: lit.TrainedFeats())
            {
                pBuild->TrainFeat(tfit.FeatName(), tfit.Type(), level);
            }
            ++level;
        }
        if (importedCharacter.HasTier5Tree())
        {
            LegacyEnhancementSelectedTrees trees = importedCharacter.SelectedTrees();
            trees.SetTier5Tree(importedCharacter.Tier5Tree());
            importedCharacter.Set_SelectedTrees(trees);
        }
        pBuild->Enhancement_SetSelectedTrees(importedCharacter.SelectedTrees());
        pBuild->Set_EnhancementTreeSpend(importedCharacter.EnhancementTreeSpend());
        if (importedCharacter.HasU51Destiny_Tier5Tree())
        {
            LegacyDestinySelectedTrees trees = importedCharacter.DestinyTrees();
            trees.SetTier5Tree(importedCharacter.U51Destiny_Tier5Tree());
            importedCharacter.Set_DestinyTrees(trees);
        }
        pBuild->Destiny_SetSelectedTrees(importedCharacter.DestinyTrees());
        pBuild->Set_DestinyTreeSpend(importedCharacter.DestinyTreeSpend());
        pBuild->Set_ReaperTreeSpend(importedCharacter.ReaperTreeSpend());
        pBuild->Set_TrainedSpells(importedCharacter.TrainedSpells());
        std::string notes = importedCharacter.Notes();
        // notes text is not saved as \r\n's replace all text
        notes = ReplaceAll(notes, "\n", "\r\n");
        pBuild->SetNotes(notes);
        std::list<EquippedGear> gearSets;
        for (auto&& gsit: importedCharacter.GearSetups())
        {
            EquippedGear gearSet;
            gearSet.SetName(gsit.Name());
            std::stringstream ss;
            ss << "Converting gear set \"" << gsit.Name() << "\"";
            GetLog().AddLogEntry(ss.str().c_str());
            for (size_t i = Inventory_Unknown + 1; i < Inventory_Count; ++i)
            {
                if (gsit.HasItemInSlot(static_cast<InventorySlotType>(i)))
                {
                    LegacyItem legacyItem = gsit.ItemInSlot(static_cast<InventorySlotType>(i));
                    Item item = pBuild->GetLatestVersionOfItem(static_cast<InventorySlotType>(i), legacyItem);
                    if (item.Name() == "")
                    {
                        std::stringstream().swap(ss);   // clear it
                        ss << "---Item \"" << legacyItem.Name() << "\" not found, skipped in gear set.";
                        GetLog().AddLogEntry(ss.str().c_str());
                    }
                    else
                    {
                        gearSet.SetItem(static_cast<InventorySlotType>(i), pBuild, item);
                    }
                }
            }
            // sentient jewel
            if (gsit.SentientIntelligence().HasPersonality())
            {
                gearSet.SetPersonality(gsit.SentientIntelligence().Personality());
            }
            // weapon filigrees now
            gearSet.SetNumFiligrees(gsit.SentientIntelligence().NumFiligrees());
            size_t fi = 0;
            std::list<WeaponFiligree> wfs = gsit.SentientIntelligence().Filigrees();
            for (auto&& fit: wfs)
            {
                if (fit.Name() != "")
                {
                    fit.TranslateOldNamesFromV1();
                    const Filigree& fili = FindFiligreeByNameComponents(fit.Name());
                    if (fili.Name() == "")
                    {
                        std::stringstream().swap(ss);   // clear it
                        ss << "---Weapon Filigree \"" << fit.Name() << "\" not found, skipped in gear set.";
                        GetLog().AddLogEntry(ss.str().c_str());
                    }
                    else
                    {
                        gearSet.SetFiligree(fi, fili.Name());
                        gearSet.SetFiligreeRare(fi, fit.HasRare());
                    }
                }
                fi++;
            }
            // artifact filigrees
            fi = 0;
            std::list<ArtifactFiligree> afs = gsit.SentientIntelligence().ArtifactFiligrees();
            for (auto&& fit : afs)
            {
                if (fit.Name() != "")
                {
                    fit.TranslateOldNamesFromV1();
                    const Filigree& fili = FindFiligreeByNameComponents(fit.Name());
                    if (fili.Name() == "")
                    {
                        std::stringstream().swap(ss);   // clear it
                        ss << "---Artifact Filigree \"" << fit.Name() << "\" not found, skipped in gear set.";
                        GetLog().AddLogEntry(ss.str().c_str());
                    }
                    else
                    {
                        gearSet.SetArtifactFiligree(fi, fili.Name());
                        gearSet.SetArtifactFiligreeRare(fi, fit.HasRare());
                    }
                }
                fi++;
            }
            gearSets.push_back(gearSet);
        }
        pBuild->Set_GearSetups(gearSets);
        pBuild->SetActiveGearSet(importedCharacter.ActiveGear());
        //pBuild->BuildNowActive();
        //pBuildsPane->PopulateBuildsList();
        pBuildsPane->ReselectCurrentItem();
    }
}
