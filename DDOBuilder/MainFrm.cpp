// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainFrm.h"

#include "DDOBuilder.h"
#include "DDOBuilderDoc.h"
#include "CustomDockablePane.h"
#include "GlobalSupportFunctions.h"
#include "WikiDownloader.h"
#include "WikiItemFileProcessor.h"

#include "AutomaticFeatsPane.h"
#include "BreakdownsPane.h"
#include "BuildsPane.h"
#include "ClassAndFeatPane.h"
#include "DCPane.h"
#include "DestinyPane.h"
#include "EquipmentPane.h"
#include "EnhancementsPane.h"
#include "FavorPane.h"
#include "GrantedFeatsPane.h"
#include "LogPane.h"
#include "ReaperEnhancementsPane.h"
#include "SkillsPane.h"
#include "SpecialFeatsPane.h"
#include "SpellsPane.h"
#include "StancesPane.h"
#include "CItemImageDialog.h"
#include "CWeaponImageDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
    ON_WM_CREATE()
    // Global help commands
    ON_COMMAND(ID_HELP_FINDER, &CFrameWndEx::OnHelpFinder)
    ON_COMMAND(ID_HELP, &CFrameWndEx::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, &CFrameWndEx::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, &CFrameWndEx::OnHelpFinder)
    ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
    ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
    ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
    ON_WM_SETTINGCHANGE()
    ON_UPDATE_COMMAND_UI_RANGE(ID_DOCKING_WINDOWS_START, ID_DOCKING_WINDOWS_END, OnUpdateDockPane)
    ON_COMMAND_RANGE(ID_DOCKING_WINDOWS_START, ID_DOCKING_WINDOWS_END, OnDockPane)
    ON_COMMAND(ID_DEVELOPMENT_RUNWIKIITEMCRAWLER, &CMainFrame::OnDevelopmentRunwWikiItemCrawler)
    ON_COMMAND(ID_DEVELOPMENT_PROCESSWIKIFILES, &CMainFrame::OnDevelopmentProcessWikiFiles)
    ON_COMMAND(ID_DEVELOPMENT_UPDATEITEMIMAGES, &CMainFrame::OnDevelopmentUpdateItemImages)
    ON_COMMAND(ID_DEVELOPMENT_UPDATEWEAPONITEMIMAGES, &CMainFrame::OnDevelopmentUpdateWeaponImages)
    //ON_COMMAND(ID_DEVELOPMENT_UPDATEARMORITEMIMAGES, &CMainFrame::OnDevelopmentUpdateWeaponImages)
    ON_UPDATE_COMMAND_UI(ID_DEVELOPMENT_RUNWIKIITEMCRAWLER, &CMainFrame::OnUpdateDevelopmentRunwWikiItemCrawler)
    ON_UPDATE_COMMAND_UI(ID_DEVELOPMENT_PROCESSWIKIFILES, &CMainFrame::OnUpdateDevelopmentProcessWikiFiles)
    ON_UPDATE_COMMAND_UI(ID_DEVELOPMENT_UPDATEITEMIMAGES, &CMainFrame::OnUpdateDevelopmentUpdateItemImages)
    ON_UPDATE_COMMAND_UI(ID_DEVELOPMENT_UPDATEWEAPONITEMIMAGES, &CMainFrame::OnUpdateDevelopmentUpdateWeaponImages)
    ON_COMMAND(ID_EDIT_IGNORELIST_ACTIVE, &CMainFrame::OnEditIgnorelistActive)
    ON_UPDATE_COMMAND_UI(ID_EDIT_IGNORELIST_ACTIVE, &CMainFrame::OnUpdateEditIgnorelistActive)
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
    m_bLoadComplete(false),
    m_bWikiProcessing(false)
{
    CopyDefaultIniToDDOBuilderIni();
    theApp.m_nAppLook = theApp.GetInt(
            _T("ApplicationLook"),
            ID_VIEW_APPLOOK_OFF_2007_BLUE);
}

CMainFrame::~CMainFrame()
{
    for (size_t x = 0; x < m_dockablePanes.size(); x++)
    {
        delete m_dockablePanes[x];
    }
    m_dockablePanes.clear();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    AfxGetApp()->m_pMainWnd = this;

    BOOL bNameValid;
    // set the visual manager and style based on persisted value
    OnApplicationLook(theApp.m_nAppLook);

    if (!m_wndMenuBar.Create(this))
    {
        TRACE0("Failed to create menu bar\n");
        return -1;      // fail to create
    }

    m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

    // prevent the menu bar from taking the focus on activation
    CMFCPopupMenu::SetForceMenuFocus(FALSE);

    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
    {
        TRACE0("Failed to create tool bar\n");
        return -1;      // fail to create
    }

    CString strToolBarName;
    bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
    ASSERT(bNameValid);
    m_wndToolBar.SetWindowText(strToolBarName);

    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);
    m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

    // Allow user-defined tool bars operations:
    InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

    if (!m_wndStatusBar.Create(this))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }
    m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

    // TODO: Delete these five lines if you don't want the tool bar and menu bar to be dockable
    m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndMenuBar);
    DockPane(&m_wndToolBar);

    // enable Visual Studio 2005 style docking window behavior
    CDockingManager::SetDockingMode(DT_SMART);
    // enable Visual Studio 2005 style docking window auto-hide behavior
    EnableAutoHidePanes(CBRS_ALIGN_ANY);

    // Navigation pane will be created at left, so temporary disable docking at the left side:
    EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT);

    // Outlook bar is created and docking on the left side should be allowed.
    EnableDocking(CBRS_ALIGN_LEFT);
    EnableAutoHidePanes(CBRS_ALIGN_RIGHT);

    // create docking windows
    if (!CreateDockingWindows())
    {
        TRACE0("Failed to create docking windows\n");
        return -1;
    }

    // Enable tool bar and docking window menu replacement
    EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

    // enable quick (Alt+drag) tool bar customization
    CMFCToolBar::EnableQuickCustomization();

    if (CMFCToolBar::GetUserImages() == NULL)
    {
        // load user-defined tool bar toolbarImages
        if (m_UserImages.Load(_T(".\\UserImages.bmp")))
        {
            CMFCToolBar::SetUserImages(&m_UserImages);
        }
    }

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWndEx::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
         | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_SYSMENU;

    return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
    // create the floating views
    CCustomDockablePane * pLogPane = CreateDockablePane(
            "Log",
            GetActiveDocument(),
            RUNTIME_CLASS(CLogPane),
            ID_DOCK_LOG);
    pLogPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // has to be created before builds bane for correct operation
    // on build switch/load
    CCustomDockablePane* pBreakdownsPane = CreateDockablePane(
            "Breakdowns",
            GetActiveDocument(),
            RUNTIME_CLASS(CBreakdownsPane),
            ID_DOCK_BREAKDOWNS);
    pBreakdownsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pStancesPane = CreateDockablePane(
            "Stances",
            GetActiveDocument(),
            RUNTIME_CLASS(CStancesPane),
            ID_DOCK_STANCES);
    pStancesPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pBuildsPane = CreateDockablePane(
            "Builds",
            GetActiveDocument(),
            RUNTIME_CLASS(CBuildsPane),
            ID_DOCK_BUILDS);
    pBuildsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane * pClassAndLevel = CreateDockablePane(
            "Class and Levels",
            GetActiveDocument(),
            RUNTIME_CLASS(CClassAndFeatPane),
            ID_DOCK_CLASSFEATS);
    pClassAndLevel->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane * pSkills = CreateDockablePane(
            "Skills",
            GetActiveDocument(),
            RUNTIME_CLASS(CSkillsPane),
            ID_DOCK_SKILLS);
    pSkills->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane * pSpecialFeats = CreateDockablePane(
            "Past Lives and Special Feats",
            GetActiveDocument(),
            RUNTIME_CLASS(CSpecialFeatPane),
            ID_DOCK_SPECIALFEATS);
    pSpecialFeats->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pEnhancementsPane = CreateDockablePane(
            "Enhancements",
            GetActiveDocument(),
            RUNTIME_CLASS(CEnhancementsPane),
            ID_DOCK_ENHANCEMENTS);
    pEnhancementsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pDestinyPane = CreateDockablePane(
            "Epic Destinies",
            GetActiveDocument(),
            RUNTIME_CLASS(CDestinyPane),
            ID_DOCK_DESTINY);
    pDestinyPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pAutomaticFeatsPane = CreateDockablePane(
            "Automatic Feats",
            GetActiveDocument(),
            RUNTIME_CLASS(CAutomaticFeatsPane),
            ID_DOCK_AUTOMATICFEATS);
    pAutomaticFeatsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pGrantedFeatsPane = CreateDockablePane(
            "Granted Feats",
            GetActiveDocument(),
            RUNTIME_CLASS(CGrantedFeatsPane),
            ID_DOCK_GRANTEDFEATS);
    pGrantedFeatsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pReaperEnhancementsPane = CreateDockablePane(
            "Reaper Enhancements",
            GetActiveDocument(),
            RUNTIME_CLASS(CReaperEnhancementsPane),
            ID_DOCK_REAPERENHANCEMENTS);
    pReaperEnhancementsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pDCsPane = CreateDockablePane(
            "DCs",
            GetActiveDocument(),
            RUNTIME_CLASS(CDCPane),
            ID_DOCK_DCS);
    pDCsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pEquipmentPane = CreateDockablePane(
            "Gear and Filigrees",
            GetActiveDocument(),
            RUNTIME_CLASS(CEquipmentPane),
            ID_DOCK_EQUIPMENT);
    pEquipmentPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pSpellsPane = CreateDockablePane(
            "Spells and SLAs",
            GetActiveDocument(),
            RUNTIME_CLASS(CSpellsPane),
            ID_DOCK_SPELLS);
    pSpellsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    CCustomDockablePane* pFavorPane = CreateDockablePane(
            "Quests and Favor",
            GetActiveDocument(),
            RUNTIME_CLASS(CFavorPane),
            ID_DOCK_FAVOR);
    pFavorPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    return TRUE;
}

// CMainFrame message handlers

void CMainFrame::OnViewCustomize()
{
    CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
    pDlgCust->EnableUserDefinedToolbars();
    pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
    LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
    if (lres == 0)
    {
        return 0;
    }

    CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
    ASSERT_VALID(pUserToolbar);

    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
    return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
    CWaitCursor wait;

    theApp.m_nAppLook = id;

    switch (theApp.m_nAppLook)
    {
    case ID_VIEW_APPLOOK_WIN_2000:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
        break;

    case ID_VIEW_APPLOOK_OFF_XP:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
        break;

    case ID_VIEW_APPLOOK_WIN_XP:
        CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
        break;

    case ID_VIEW_APPLOOK_OFF_2003:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_VS_2005:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_VS_2008:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_WINDOWS_7:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    default:
        switch (theApp.m_nAppLook)
        {
        case ID_VIEW_APPLOOK_OFF_2007_BLUE:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_BLACK:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_SILVER:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_AQUA:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
            break;
        }

        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
        CDockingManager::SetDockingMode(DT_SMART);
    }

    RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

    theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
    // base class does the real work
    if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
    {
        return FALSE;
    }

    // enable customization button for all user tool bars
    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    for (int i = 0; i < iMaxUserToolbars; i ++)
    {
        CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
        if (pUserToolbar != NULL)
        {
            pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
        }
    }

    // ensure log window is visible
    int logIndex = ID_DOCK_LOG - ID_DOCKING_WINDOWS_START;
    m_dockablePanes[logIndex]->ShowPane(TRUE, FALSE, TRUE);

    return TRUE;
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    CFrameWndEx::OnSettingChange(uFlags, lpszSection);
    // do the same for all docked windows also
    for (size_t i = 0; i < m_dockablePanes.size(); ++i)
    {
        //CView * pView = m_dockablePanes[i]->GetView();
        //pView->SendMessage(WM_SETTINGCHANGE, uFlags, lpszSection);
    }
}

void CMainFrame::NewDocument(CDDOBuilderDoc * pDoc)
{
    // make sure all the windows know who is the active document
    // there is always a document as this is an SDI application
    POSITION pos = pDoc->GetFirstViewPosition();
    while (pos != NULL)
    {
        CView* pView = pDoc->GetNextView(pos);
        if (pView != NULL)
        {
            pView->SendMessage(UWM_NEW_DOCUMENT, (WPARAM)pDoc, (LPARAM)pDoc->GetCharacter());
        }
    }
    for (size_t x = 0; x < m_dockablePanes.size(); x++)
    {
        m_dockablePanes[x]->SetDocumentAndCharacter(pDoc, pDoc->GetCharacter());
    }
}

CCustomDockablePane* CMainFrame::CreateDockablePane(
        const char* paneTitle, 
        CDocument* doc, 
        CRuntimeClass* runtimeClass,
        UINT viewID)
{
    CCreateContext createContext;
    createContext.m_pCurrentDoc = doc;
    createContext.m_pNewViewClass = runtimeClass;

    CCustomDockablePane* pane = new CCustomDockablePane(viewID);
    // Assorted functionality is applied to all panes
    m_dockablePanes.push_back(pane);

    pane->Create(
            _T(paneTitle),
            this,
            CRect(0,0,400,400),
            TRUE,
            viewID,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI, 
            32UL,
            15UL,
            &createContext);
    pane->EnableDocking(CBRS_ALIGN_ANY);
    DockPane(pane);

    return pane;
}

CLogPane & CMainFrame::GetLog()
{
    CView * pView = m_dockablePanes[0]->GetView();
    CLogPane * pLogPane = dynamic_cast<CLogPane*>(pView);
    return *pLogPane;
}

BOOL CMainFrame::OnCmdMsg(
        UINT nID,
        int nCode,
        void * pExtra,
        AFX_CMDHANDLERINFO * pHandlerInfo)
{
    BOOL bReturn = FALSE;

    // see if a child pane can handle it
    if (FALSE == bReturn)
    {
        // Offer command to child views by looking in each docking window which
        // holds a view until we get the 1st view that handles the message.
        for (size_t x = 0; bReturn == FALSE && x < m_dockablePanes.size(); x++)
        {
            CView * pView = m_dockablePanes[x]->GetView();
            if (pView != NULL)
            {
                bReturn = pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
            }
        }
    }

    if (FALSE == bReturn)
    {
        // also offer the message to any views for the active open document
        POSITION pos = AfxGetApp()->m_pDocManager->GetFirstDocTemplatePosition();
        CDocTemplate * pTemplate = AfxGetApp()->m_pDocManager->GetNextDocTemplate(pos);
        pos = pTemplate->GetFirstDocPosition();
        CDocument * pDoc = pTemplate->GetNextDoc(pos);

        if (pDoc != NULL)
        {
            pos = pDoc->GetFirstViewPosition();
            while (pos != NULL && bReturn == FALSE)
            {
                CView * pView = pDoc->GetNextView(pos);
                if (pView != NULL)
                {
                    bReturn = pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
                }
            }
        }
    }

    // try to handle the message ourselves
    if (FALSE == bReturn)
    {
        bReturn = CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    return bReturn;
}

void CMainFrame::OnUpdateDockPane(CCmdUI* pCmdUI)
{
    size_t index = pCmdUI->m_nID - ID_DOCKING_WINDOWS_START;  // now 0...n
    ASSERT(index < m_dockablePanes.size());
    pCmdUI->SetCheck(m_dockablePanes[index]->IsVisible());
    pCmdUI->Enable(TRUE);           // always enabled
}

void CMainFrame::OnDockPane(UINT nID)
{
    size_t index = nID - ID_DOCKING_WINDOWS_START;
    ASSERT(index < m_dockablePanes.size());
    m_dockablePanes[index]->ShowPane(!m_dockablePanes[index]->IsVisible(), FALSE, TRUE);
}

void CMainFrame::AddSmallClassImageMenuIcons()
{
    // make sure the images in the tool bar match the lookups for the menus
    // as we add the class images dynamically later, we end up replacing the tool bar
    // menu commands images as they use the same index otherwise
    // TBD: Sort out the transparent background colour for these images
    CMFCToolBarImages * toolbarImages = CMFCToolBar::GetMenuImages();
    {
        CBitmap bitmap;
        bitmap.LoadBitmap(IDR_MAINFRAME_256);
        toolbarImages->AddImage((HBITMAP)bitmap, FALSE);
    }

    // add all the class images to the available tool bar toolbarImages
    // so they show correctly in drop menus
    const std::list<Class> & classes = Classes();
    // load all the small images for each class
    CCommandManager * commandManager = GetCmdMgr(); 
    const CSize menuimgsize = CMFCToolBar::GetMenuImageSize(); 
    ASSERT(menuimgsize.cx == 16 && menuimgsize.cy == 15);

    std::list<Class>::const_iterator it = classes.begin();
    UINT classCommandId = ID_CLASS_PROMPT1;
    while (it != classes.end())
    {
        int classImageIndex = commandManager->GetCmdImage(classCommandId, FALSE);
        ASSERT(classImageIndex == -1); // should not exist
        CImage classImage;
        HRESULT result = LoadImageFile(
                "DataFiles\\ClassImages\\",
                (*it).SmallIcon(),
                &classImage,
                menuimgsize);
        if (result == S_OK)
        {
            CBitmap bitmap;
            bitmap.Attach(classImage.Detach());
            classImageIndex = toolbarImages->AddImage((HBITMAP)bitmap, FALSE);
            commandManager->SetCmdImage(classCommandId, classImageIndex, FALSE);
        }
        ++classCommandId;
        ++it;
    }
}

void CMainFrame::CopyDefaultIniToDDOBuilderIni()
{
    // before any settings are read from the ini file, we need to check to see if it
    // exists or not. If it does not exist, we copy the "Default.ini" file as
    // the ini file to be used. This allows new and existing users to get good
    // default settings on startup (new users get the defaults, old users get
    // their previous settings)
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
    std::string filename = path;
    filename += "DDOBuilder.ini";
    bool exists = false;
    WIN32_FIND_DATA findFileData1;
    HANDLE hFind = FindFirstFile(filename.c_str(), &findFileData1);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        exists = true;
    }
    if (!exists)
    {
        // copy the "Default.ini" file to "DDOBuilder.ini"
        std::string defaultIni = path;
        defaultIni += "Default.ini";
        // does Default.ini file exist?
        WIN32_FIND_DATA findFileData2;
        hFind = FindFirstFile(defaultIni.c_str(), &findFileData2);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
            exists = true;
        }
        if (exists)
        {
            // we can do the copy step
            ::CopyFile(defaultIni.c_str(), filename.c_str(), TRUE);
        }
    }
}

void CMainFrame::LoadComplete()
{
    AddSmallClassImageMenuIcons();

    // do the documents views (usually only 1)
    CDocument * pDoc = GetActiveDocument();
    if (pDoc != NULL)
    {
        POSITION pos = pDoc->GetFirstViewPosition();
        while (pos != NULL)
        {
            CView * pView = pDoc->GetNextView(pos);
            pView->SendMessage(UWM_LOAD_COMPLETE, 0, 0L);
        }
    }
    // do the same for all docked windows also
    for (size_t i = 0; i < m_dockablePanes.size(); ++i)
    {
        CView * pView = m_dockablePanes[i]->GetView();
        pView->SendMessage(UWM_LOAD_COMPLETE, 0, 0L);
    }
    m_bLoadComplete = true;
    //m_menuToolbar.LoadToolBar(IDR_MENUICONS_TOOLBAR);
}

MouseHook* CMainFrame::GetMouseHook()
{
    return &m_mouseHook;
}

BreakdownItem* CMainFrame::FindBreakdown(BreakdownType type)
{
    BreakdownItem* pItem = NULL;
    for (auto&& pit: m_dockablePanes)
    {
        CWnd* pWnd = pit->GetView();
        CBreakdownsPane* pView = dynamic_cast<CBreakdownsPane*>(pWnd);
        if (pView != NULL)
        {
            pItem = pView->FindBreakdown(type);
        }
    }
    return pItem;
}

CFormView* CMainFrame::GetPane(const CRuntimeClass* c)
{
    CFormView* pView = NULL;
    for (size_t i = 0; pView == NULL && i < m_dockablePanes.size(); ++i)
    {
        const CRuntimeClass* pPaneRTC = m_dockablePanes[i]->GetView()->GetRuntimeClass();
        if (strcmp(pPaneRTC->m_lpszClassName, c->m_lpszClassName) == 0)
        {
            // this it the pane we want
            pView = dynamic_cast<CFormView*>(m_dockablePanes[i]->GetView());
        }
    }
    return pView;
}

void CMainFrame::OnDevelopmentRunwWikiItemCrawler()
{
    UINT ret = AfxMessageBox(
        "This will start a process that will crawl the DDO Wiki and enumerate all the pages with item data.\r\n"
        "The downloaded files can later be processed using other Development command options.\r\n"
        "\r\n"
        "Are you sure you wish to do this?", MB_ICONEXCLAMATION | MB_YESNO);
    if (ret == IDYES)
    {
        WikiDownloader wiki;
        wiki.Start();
    }
}

void CMainFrame::OnDevelopmentProcessWikiFiles()
{
    UINT ret = AfxMessageBox(
        "This will start a process that will process all the DDO Wiki item html files, extracting\r\n"
        "the data and either creating or updating the existing .item files.\r\n"
        "On completion you will have to restart the software to pick up the new/updated items.\r\n"
        "\r\n"
        "Are you sure you wish to do this?", MB_ICONEXCLAMATION | MB_YESNO);
    if (ret == IDYES)
    {
        WikiItemFileProcessor fileProcessor;
        fileProcessor.Start();
    }
}

void CMainFrame::OnDevelopmentUpdateItemImages()
{
    CItemImageDialog dlg(this);
    dlg.DoModal();
}

void CMainFrame::OnDevelopmentUpdateWeaponImages()
{
    CWeaponImageDialog dlg(this);
    dlg.DoModal();
}

void CMainFrame::OnUpdateDevelopmentRunwWikiItemCrawler(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bLoadComplete && !m_bWikiProcessing);
}

void CMainFrame::OnUpdateDevelopmentProcessWikiFiles(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bLoadComplete && !m_bWikiProcessing);
}

void CMainFrame::OnUpdateDevelopmentUpdateItemImages(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bLoadComplete && !m_bWikiProcessing);
}

void CMainFrame::OnUpdateDevelopmentUpdateWeaponImages(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bLoadComplete && !m_bWikiProcessing);
}

void CMainFrame::OnEditIgnorelistActive()
{
    g_bShowIgnoredItems = !g_bShowIgnoredItems;
}


void CMainFrame::OnUpdateEditIgnorelistActive(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(g_bShowIgnoredItems);
    pCmdUI->Enable(m_bLoadComplete && !m_bWikiProcessing);
}
