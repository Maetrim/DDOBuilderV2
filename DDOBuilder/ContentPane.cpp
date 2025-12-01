// ContentPane.cpp
//
#include "stdafx.h"

#include "ContentPane.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

CContentPane::CContentPane() :
    CFormView(CContentPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL),
    m_bHadInitialUpdate(false),
    m_bPopulating(false)
{
}

CContentPane::~CContentPane()
{
}

IMPLEMENT_DYNCREATE(CContentPane, CFormView)
BEGIN_MESSAGE_MAP(CContentPane, CFormView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_WM_ERASEBKGND()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_CONTENT_LIST, OnItemchangedAdventurePack)
END_MESSAGE_MAP()

void CContentPane::OnInitialUpdate()
{
    if (!m_bHadInitialUpdate)
    {
        m_bHadInitialUpdate = true;
        CFormView::OnInitialUpdate();
        UpdateFonts();
    }
}

LRESULT CContentPane::OnLoadComplete(WPARAM, LPARAM)
{
    m_bPopulating = true;
    m_listContent.InsertColumn(0, "Adventure Pack", LVCFMT_LEFT, 100);
    m_listContent.SetExtendedStyle(m_listContent.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
    const std::list<std::string>& packs = AdventurePacks();
    for (auto&& pit: packs)
    {
        int index = m_listContent.InsertItem(m_listContent.GetItemCount(), pit.c_str());
        BOOL enabled = TRUE;
        m_listContent.SetItemState(index, INDEXTOSTATEIMAGEMASK(enabled + 1), LVIS_STATEIMAGEMASK);
    }
    //m_listContent.SortItems();
    m_bPopulating = false;
    return 0;
}

LRESULT CContentPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDocument* pDoc = (CDocument*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character* pCharacter = (Character*)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        // for all items in the character, set/clear the check box
        if (IsWindow(m_listContent.GetSafeHwnd()) != 0)
        {
            m_listContent.DeleteAllItems();
            const std::list<std::string>& packs = AdventurePacks();
            const std::list<std::string>& contentIDontOwn = m_pCharacter->ContentIDontOwn();
            for (auto&& pit: packs)
            {
                int index = m_listContent.InsertItem(m_listContent.GetItemCount(), pit.c_str());
                BOOL enabled = TRUE;
                if (std::find(contentIDontOwn.begin(), contentIDontOwn.end(), pit) != contentIDontOwn.end())
                {
                    enabled = FALSE;
                }
                m_listContent.SetItemState(index, INDEXTOSTATEIMAGEMASK(enabled + 1), LVIS_STATEIMAGEMASK);
            }
            //m_listContent.SortItems();
        }
    }
    return 0L;
}

void CContentPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CONTENT_LIST, m_listContent);
}

void CContentPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_listContent.GetSafeHwnd())
            && IsWindowVisible())
    {
        m_listContent.MoveWindow(0, 0, cx, cy, TRUE);
        m_listContent.SetColumnWidth(0, cx
                - GetSystemMetrics(SM_CYBORDER) * 4     // control border
                - GetSystemMetrics(SM_CYHSCROLL));      // vertical scrollbar
    }
    SetScaleToFitSize(CSize(cx, cy));
    CWnd *pWnd = this;
    while (pWnd != NULL)
    {
        pWnd = pWnd->GetParent();
    }
}

void CContentPane::UpdateFonts()
{
    m_listContent.SetFont(&afxGlobalData.fontRegular);
}

BOOL CContentPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_CONTENT_LIST,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CContentPane::OnItemchangedAdventurePack(NMHDR* pNMHDR, LRESULT* pResult)
{
    // stop control updates done by populating the control causing changes
    if (!m_bPopulating)
    {
        NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
        BOOL oldCheckState = ((pNMListView->uOldState & LVIS_STATEIMAGEMASK) >> 12) - 1;
        BOOL newCheckState = ((pNMListView->uNewState & LVIS_STATEIMAGEMASK) >> 12) - 1;

        bool bIgnore = false;
        if (oldCheckState != -1
            && newCheckState != -1
            && oldCheckState != newCheckState)
        {
            CString logEntry;
            CString item = m_listContent.GetItemText(pNMListView->iItem, 0);
            std::list<std::string> contentIDontWon = m_pCharacter->ContentIDontOwn();
            if (newCheckState == TRUE)
            {
                auto it = std::find(contentIDontWon.begin(), contentIDontWon.end(), (LPCSTR)item);
                if (it != contentIDontWon.end())
                {
                    contentIDontWon.erase(it);
                }
                else
                {
                    bIgnore = true;
                }
                logEntry.Format("Adventure pack \"%s\" add to the list of content you own.", (LPCTSTR)item);
            }
            else
            {
                contentIDontWon.push_back((LPCTSTR)item);
                logEntry.Format("Adventure pack \"%s\" removed from the list of content you own.", (LPCTSTR)item);
            }
            if (!bIgnore)
            {
                m_pCharacter->SetContentIDontOwn(contentIDontWon);
                GetLog().AddLogEntry(logEntry);
                m_pCharacter->SetModifiedFlag(TRUE);
            }
        }
    }
    *pResult = 0;
}
