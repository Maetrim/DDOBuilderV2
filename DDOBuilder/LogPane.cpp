
#include "stdafx.h"

#include "LogPane.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

CLogPane::CLogPane() :
    CFormView(CLogPane::IDD)
{
}

CLogPane::~CLogPane()
{
}

IMPLEMENT_DYNCREATE(CLogPane, CFormView)
BEGIN_MESSAGE_MAP(CLogPane, CFormView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_COMMAND(ID_EDIT_COPYLOGTOCLIPBOARD, &CLogPane::OnCopyLogToClipboard)
    ON_COMMAND(ID_EDIT_CLEARLOG, &CLogPane::OnClearLog)
END_MESSAGE_MAP()

void CLogPane::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    UpdateFonts();
}

void CLogPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_LOG, m_wndOutputLog);
}

void CLogPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_wndOutputLog.GetSafeHwnd()))
    {
        m_wndOutputLog.MoveWindow(0, 0, cx, cy, TRUE);
    }
    SetScaleToFitSize(CSize(cx, cy));
}

void CLogPane::UpdateFonts()
{
    m_wndOutputLog.SetFont(&afxGlobalData.fontRegular);
}

void CLogPane::AddLogEntry(const CString & strText)
{
    // break up any lines (this is incredibility crude and inefficient but should not be used much)
    int index = 0;
    CString logStr;
    for (int i = 0; i < strText.GetLength(); i++)
    {
        if (strText[i] == '\n')
        {
            index = m_wndOutputLog.AddString(logStr);
            logStr = "";
        }
        else
        {
            if (strText[i] != '\r') logStr += strText[i];
        }
    }
    if (logStr != "")
    {
        index = m_wndOutputLog.AddString(logStr);
    }
    m_wndOutputLog.SetTopIndex(index);
    m_wndOutputLog.RedrawWindow();
}

void CLogPane::UpdateLastLogEntry(const CString & strText)
{
    m_wndOutputLog.LockWindowUpdate();
    int index = m_wndOutputLog.GetCount();
    m_wndOutputLog.DeleteString(index - 1);
    index = m_wndOutputLog.AddString(strText);
    m_wndOutputLog.SetTopIndex(index);
    m_wndOutputLog.UnlockWindowUpdate();
    m_wndOutputLog.RedrawWindow();
}


void CLogPane::OnCopyLogToClipboard()
{
    m_wndOutputLog.OnEditCopy();
}

void CLogPane::OnClearLog()
{
    m_wndOutputLog.OnEditClear();
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList()
{
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CMenu menu;
    menu.LoadMenu(IDR_OUTPUT_POPUP);

    CMenu* pSumMenu = menu.GetSubMenu(0);

    if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
    {
        CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

        if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
            return;

        ((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
        UpdateDialogControls(this, FALSE);
    }

    SetFocus();
}

void COutputList::OnEditCopy()
{
    CString strText;
    int numItems = GetCount();
    for (int index = 0; index < numItems; ++index)
    {
        CString strItem;
        GetText(index, strItem);
        strText += strItem;
        strText += "\r\n";
    }
    const size_t len = strText.GetLength() + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hMem != NULL)
    {
        LPVOID pVoid = GlobalLock(hMem);
        if (pVoid != NULL)
        {
            memcpy(pVoid, strText, len);
            GlobalUnlock(hMem);
            if (OpenClipboard())
            {
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hMem);
                CloseClipboard();
            }
        }
    }
}

void COutputList::OnEditClear()
{
    ResetContent();
}
