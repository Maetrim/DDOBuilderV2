// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "Resource.h"
#include "BreakdownTypes.h"
#include "BuildsPane.h"
#include "MouseHook.h"
#include "CCustomMFCStatusBar.h"

class BreakdownItem;
class CDDOBuilderDoc;
class CCustomDockablePane;
class CLogPane;
class CStancesPane;

class CMainFrame : public CFrameWndEx
{
public:
    virtual ~CMainFrame();
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL LoadFrame(
            UINT nIDResource,
            DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
            CWnd* pParentWnd = NULL,
            CCreateContext* pContext = NULL);
    virtual BOOL OnCmdMsg(
            UINT nID,
            int nCode,
            void* pExtra,
            AFX_CMDHANDLERINFO* pHandlerInfo);

    MouseHook* GetMouseHook();
    void NewDocument(CDDOBuilderDoc * pDoc);
    CLogPane & GetLog();
    void LoadComplete();
    BreakdownItem* FindBreakdown(BreakdownType type);
    CFormView* GetPaneView(const CRuntimeClass* c);
    void ResizeWindows();

    void BeginProgress(CString fixedText);
    void SetProgress(int nProg);
    void EndProgress();

protected:
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnViewCustomize();
    afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
    afx_msg void OnApplicationLook(UINT id);
    afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg void OnUpdateDockPane(CCmdUI* pCmdUI);
    afx_msg void OnDockPane(UINT nID);
    afx_msg void OnDevelopmentShow0ValueBreakdowns();
    afx_msg void OnDevelopmentRunwWikiItemCrawler();
    afx_msg void OnDevelopmentCrawlSpecificWikiLink();
    afx_msg void OnDevelopmentProcessWikiFiles();
    afx_msg void OnDevelopmentProcessSpecificWikiFile();
    afx_msg void OnDevelopmentUpdateItemImages();
    afx_msg void OnDevelopmentUpdateWeaponImages();
    afx_msg void OnUpdateDevelopmentShow0ValueBreakdowns(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDevelopmentRunwWikiItemCrawler(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDevelopmentCrawlSpecificWikiLink(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDevelopmentProcessWikiFiles(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDevelopmentUpdateItemImages(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDevelopmentUpdateWeaponImages(CCmdUI* pCmdUI);
    afx_msg void OnEditIgnorelistActive();
    afx_msg void OnUpdateEditIgnorelistActive(CCmdUI* pCmdUI);
    afx_msg void OnResetScreenLayout();
    afx_msg LRESULT OnStartProgress(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnSetProgress(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnEndProgress(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    afx_msg LRESULT OnLogMessage(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()

    BOOL CreateDockingWindows();
    CCustomDockablePane* CreateDockablePane(
            const char* paneTitle, 
            CDocument* doc, 
            CRuntimeClass* runtimeClass,
            UINT viewID);
    void CopyDefaultIniToDDOBuilderIni();
    void AddSmallClassImageMenuIcons();
    CCustomDockablePane* GetPane(UINT nID);

    CProgressCtrl     m_ctlProgress;
    CMFCMenuBar       m_wndMenuBar;
    CMFCToolBar       m_wndToolBar;
    CCustomMFCStatusBar m_wndStatusBar;
    CMFCToolBarImages m_UserImages;
    CMFCToolBar       m_menuToolbar;
    std::vector<CCustomDockablePane*> m_dockablePanes;
    MouseHook m_mouseHook;
    bool m_bLoadComplete;
    bool m_bWikiProcessing;

    friend class WikiDownloader;
};
