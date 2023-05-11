#include "stdafx.h"
#include "OutlookControl.h"

/////////////////////////////////////////////////////////////////////////////
// COutlookControl

IMPLEMENT_SERIAL(COutlookControl, CWnd)

COutlookControl::COutlookControl()
{
	m_bMode2003 = FALSE;
	m_pFontButtons = NULL;
}

COutlookControl::~COutlookControl()
{
	while (!m_lstCustomPages.IsEmpty())
	{
		delete m_lstCustomPages.RemoveHead();
	}
}

BEGIN_MESSAGE_MAP(COutlookControl, CWnd)
	//{{AFX_MSG_MAP(COutlookControl)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutlookControl message handlers

void COutlookControl::GetTabArea(CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const
{
	rectTabAreaTop.SetRectEmpty();
	rectTabAreaBottom.SetRectEmpty();
	GetClientRect(rectTabAreaTop);
	ClientToScreen(rectTabAreaTop);
}

int COutlookControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectClient(0, 0, lpCreateStruct->cx, lpCreateStruct->cy);

	ENSURE(m_pTabWnd == NULL);
	m_pTabWnd = new CMFCOutlookBarTabCtrl;

	CMFCOutlookBarTabCtrl* pTabWnd = (CMFCOutlookBarTabCtrl*) m_pTabWnd;

	// enable this before create,a s it may change inside for dialog applications
	pTabWnd->m_bEnableWrapping = TRUE;

	// Create tabs window:
	if (!pTabWnd->Create(rectClient, this, 101))
	{
		TRACE0("Failed to create tab window\n");
		delete m_pTabWnd;
		m_pTabWnd = NULL;
		return -1;      // fail to create
	}

	pTabWnd->SetDockingBarWrapperRTC(RUNTIME_CLASS(CMFCOutlookBarPaneAdapter));

	if (CanFloat())
	{
		pTabWnd->HideSingleTab();
	}

	return 0;
}

BOOL COutlookControl::Create(LPCTSTR lpszCaption, CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	dwControlBarStyle &= ~AFX_CBRS_FLOAT;

	BOOL bResult = CBaseTabbedPane::Create(lpszCaption, pParentWnd, rect, FALSE, nID, dwStyle, AFX_CBRS_OUTLOOK_TABS, dwControlBarStyle, pContext);
	if (!bResult)
	{
		TRACE0("Failed to create COutlookControl\n");
		return FALSE;
	}

	if (dwControlBarStyle & AFX_CBRS_RESIZE)
	{
		EnableDocking(CBRS_ALIGN_ANY);
		DockPaneUsingRTTI(FALSE);
	}
	else
	{
		CDockingManager* pManager = afxGlobalUtils.GetDockingManager(pParentWnd);
		if (pManager != NULL)
		{
			pManager->AddPane(this);
		}
	}

	if (lpszCaption != NULL)
	{
		m_strBarName = lpszCaption;
	}

	return TRUE;
}

BOOL COutlookControl::CanAcceptPane(const CBasePane* pBar) const
{
	ASSERT_VALID(this);

	if (pBar == NULL || m_bMode2003)
	{
		return FALSE;
	}

	if (CanFloat())
	{
		return CBaseTabbedPane::CanAcceptPane(pBar);
	}

	return(pBar->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneAdapter)) || pBar->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPane)) || pBar->IsKindOf(RUNTIME_CLASS(COutlookControl)));
}

AFX_CS_STATUS COutlookControl::GetDockingStatus(CPoint pt, int nSensitivity)
{
	ASSERT_VALID(this);

	if (m_pTabWnd == NULL)
	{
		return CS_NOTHING;
	}

	if (m_pTabWnd->GetTabsNum() == 0 || m_pTabWnd->GetVisibleTabsNum() == 0)
	{
		return CS_DELAY_DOCK_TO_TAB;
	}

	// detect caption
	UINT nHitTest = HitTest(pt, TRUE);

	CRect rectTabAreaTop;
	CRect rectTabAreaBottom;
	GetTabArea(rectTabAreaTop, rectTabAreaBottom);

	if (!rectTabAreaTop.IsRectEmpty())
	{
		rectTabAreaTop.bottom += nSensitivity;
	}

	if (!rectTabAreaBottom.IsRectEmpty())
	{
		rectTabAreaBottom.top -= nSensitivity;
	}

	if (nHitTest == HTCAPTION || rectTabAreaTop.PtInRect(pt) || rectTabAreaBottom.PtInRect(pt))
	{
		// need to display "ready to create detachable tab" status
		return CS_DELAY_DOCK_TO_TAB;
	}

	AFX_CS_STATUS status = CDockablePane::GetDockingStatus(pt, nSensitivity);

	//if the bar can't float it's a static bar and it can't accept
	// anything but dock to tabs

	if (!CanFloat() && status == CS_DELAY_DOCK)
	{
		return CS_NOTHING;
	}

	return status;
}

void COutlookControl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(dc, this);

	CRect rectClient;
	GetClientRect(rectClient);

	CMFCVisualManager::GetInstance()->OnFillBarBackground(&memDC.GetDC(), this, rectClient, rectClient);
}

CMFCOutlookBarPane* COutlookControl::CreateCustomPage(LPCTSTR lpszPageName, BOOL bActivatePage, DWORD dwEnabledDocking, BOOL bEnableTextLabels)
{
	ENSURE(lpszPageName != NULL);

	if (m_bMode2003)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CMFCOutlookBarTabCtrl* pOutlookWnd = (CMFCOutlookBarTabCtrl*) GetUnderlyingWindow();

	ASSERT_VALID(pOutlookWnd);

	UINT uiPageID = FindAvailablePageID();

	if (uiPageID == 0xFFFF)
	{
		TRACE0("There is no page ID available!\n");
		return NULL;
	}

	CMFCOutlookBarPane* pNewPage = new CMFCOutlookBarPane;
	pNewPage->Create(this, AFX_DEFAULT_TOOLBAR_STYLE | CBRS_FLOAT_MULTI, uiPageID);
	pNewPage->SetOwner(GetOwner());
	pNewPage->EnableDocking(dwEnabledDocking);
	pNewPage->EnableTextLabels(bEnableTextLabels);
	pOutlookWnd->AddTab(pNewPage, lpszPageName);

	m_lstCustomPages.AddTail(pNewPage);
	g_arTakenIDs [uiPageID - g_nMinReservedPageID] = true;

	if (bActivatePage)
	{
		pOutlookWnd->SetActiveTab(pOutlookWnd->m_iTabsNum - 1);
	}

	return pNewPage;
}

BOOL COutlookControl::RemoveCustomPage(UINT uiPage, CMFCOutlookBarTabCtrl* pTargetWnd)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pTargetWnd);

	if (m_bMode2003)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCOutlookBarTabCtrl* pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBarTabCtrl, GetUnderlyingWindow());
	if (pOutlookBar == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCOutlookBarPane* pPage = DYNAMIC_DOWNCAST(CMFCOutlookBarPane, pTargetWnd->GetTabWndNoWrapper(uiPage));
	if (pPage == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	POSITION pos = m_lstCustomPages.Find(pPage);
	if (pos != NULL)
	{
		UINT uID = pPage->GetDlgCtrlID();

		if (uID >= g_nMinReservedPageID && uID < g_nMinReservedPageID + AFX_TOTAL_RESERVED_PAGES)
		{
			g_arTakenIDs [uID - g_nMinReservedPageID] = false;
			m_lstCustomPages.RemoveAt(pos);

			BOOL bSave = pTargetWnd->m_bAutoDestroyWindow;

			pTargetWnd->m_bAutoDestroyWindow = TRUE;
			pTargetWnd->RemoveTab(uiPage);
			pTargetWnd->m_bAutoDestroyWindow = bSave;

			delete pPage;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL COutlookControl::LoadState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CBaseTabbedPane::LoadState(lpszProfileName, nIndex, uiID);

	CString strProfileName = ::AFXGetRegPath(strOutlookBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	LPBYTE lpbData = NULL;
	UINT uiDataSize;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	if (!reg.Read(strRegCustomPages, &lpbData, &uiDataSize))
	{
		return FALSE;
	}

	CMFCOutlookBarTabCtrl* pOutlookBar = (CMFCOutlookBarTabCtrl*) GetUnderlyingWindow();

	try
	{
		CMemFile file(lpbData, uiDataSize);
		CArchive ar(&file, CArchive::load);

		int nCount = 0;
		ar >> nCount;

		for (int i = 0; i < nCount; i++)
		{
			int nID = 0;
			CString strName;
			ar >> nID;
			ar >> strName;

			CMFCOutlookBarPane* pPage = new CMFCOutlookBarPane();
			pPage->Create(this, AFX_DEFAULT_TOOLBAR_STYLE, nID);
			pPage->SetOwner(GetOwner());

			pPage->LoadState(lpszProfileName, nID,  nID);

			m_lstCustomPages.AddTail(pPage);
			pOutlookBar->AddTab(pPage, strName);

			g_arTakenIDs [nID - g_nMinReservedPageID] = true;
		}

		int nVisiblePages = 0;
		ar >> nVisiblePages;

		pOutlookBar->SetVisiblePageButtons(nVisiblePages);
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in COutlookControl::SaveState()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Archive exception in COutlookControl::LoadState()!\n"));
	}

	delete [] lpbData;
	return TRUE;
}

BOOL COutlookControl::SaveState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CBaseTabbedPane::SaveState(lpszProfileName, nIndex, uiID);

	for (POSITION pos = m_lstCustomPages.GetHeadPosition(); pos != NULL;)
	{
		CMFCOutlookBarPane* pPage = (CMFCOutlookBarPane*)m_lstCustomPages.GetNext(pos);
		ASSERT_VALID(pPage);
		int nID = pPage->GetDlgCtrlID();
		pPage->SaveState(lpszProfileName, nID, nID);
	}

	CString strProfileName = ::AFXGetRegPath(strOutlookBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	try
	{
		CMemFile file;

		{
			CArchive ar(&file, CArchive::store);

			ar <<(int) m_lstCustomPages.GetCount();
			for (POSITION pos = m_lstCustomPages.GetHeadPosition(); pos != NULL;)
			{
				CMFCOutlookBarPane* pPage = (CMFCOutlookBarPane*)m_lstCustomPages.GetNext(pos);
				ASSERT_VALID(pPage);

				ar << pPage->GetDlgCtrlID();

				CString strName;
				if (pPage->IsTabbed())
				{
					pPage->GetWindowText(strName);
				}
				else
				{
					pPage->GetParent()->GetWindowText(strName);
				}

				ar << strName;
			}

			CMFCOutlookBarTabCtrl* pOutlookBar = (CMFCOutlookBarTabCtrl*) GetUnderlyingWindow();
			if (pOutlookBar != NULL)
			{
				ar << pOutlookBar->GetVisiblePageButtons();
			}
			else
			{
				ar << -1;
			}

			ar.Flush();
		}

		UINT uiDataSize = (UINT) file.GetLength();
		LPBYTE lpbData = file.Detach();

		if (lpbData != NULL)
		{
			CSettingsStoreSP regSP;
			CSettingsStore& reg = regSP.Create(FALSE, FALSE);

			if (reg.CreateKey(strSection))
			{
				reg.Write(strRegCustomPages, lpbData, uiDataSize);
			}

			free(lpbData);
		}
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in COutlookControl::SaveState()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Archive exception in COutlookControl::SaveState()!\n"));
	}

	return TRUE;
}

UINT COutlookControl::FindAvailablePageID()
{
	for (UINT ui = 0; ui < AFX_TOTAL_RESERVED_PAGES; ui++)
	{
		if (!g_arTakenIDs [ui])
		{
			return ui + g_nMinReservedPageID;
		}
	}

	return 0xFFFF;
}

void COutlookControl::SetMode2003(BOOL bMode2003/* = TRUE*/)
{
	ASSERT_VALID(this);

	if (GetSafeHwnd() != NULL)
	{
		ASSERT(FALSE);
		return;
	}

	m_bMode2003 = bMode2003;
}

void COutlookControl::SetButtonsFont(CFont* pFont, BOOL bRedraw/* = TRUE*/)
{
	ASSERT_VALID(this);
	m_pFontButtons = pFont;

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}
}

BOOL COutlookControl::FloatTab(CWnd* pBar, int nTabID, AFX_DOCK_METHOD dockMethod, BOOL bHide)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	CMFCOutlookBarTabCtrl* pOutlookWnd = DYNAMIC_DOWNCAST(CMFCOutlookBarTabCtrl, GetUnderlyingWindow());

	ASSERT_VALID(pOutlookWnd);

	if (pOutlookWnd->GetTabsNum() > 1)
	{
		return CBaseTabbedPane::FloatTab(pBar, nTabID, dockMethod, bHide);
	}

	return FALSE;
}



