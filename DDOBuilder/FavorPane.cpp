// FavorPane.cpp : implementation file
//

#include "stdafx.h"
#include "FavorPane.h"
#include "GlobalSupportFunctions.h"
#include "DDOBuilderDoc.h"
#include "LogPane.h"
#include "Quest.h"

// CFavorPane

IMPLEMENT_DYNCREATE(CFavorPane, CFormView)
BEGIN_MESSAGE_MAP(CFavorPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
END_MESSAGE_MAP()

CFavorPane::CFavorPane() :
    CFormView(CFavorPane::IDD),
    m_pDoc(NULL),
    m_pCharacter(NULL),
    m_bLoadComplete(false)
{
}

CFavorPane::~CFavorPane()
{
}

LRESULT CFavorPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDDOBuilderDoc * pDoc = (CDDOBuilderDoc*)(wParam);
    m_pDoc = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        PopulateQuestList();
    }
    return 0L;
}

LRESULT CFavorPane::OnLoadComplete(WPARAM, LPARAM)
{
    m_bLoadComplete = true;
    // add the columns to the list control
    m_listQuests.InsertColumn(0, "Quest Name", LVCFMT_LEFT, 400);
    m_listQuests.InsertColumn(1, "Level", LVCFMT_LEFT, 40);
    m_listQuests.InsertColumn(2, "Favor", LVCFMT_LEFT, 40);
    m_listQuests.InsertColumn(3, "Run@", LVCFMT_LEFT, 60);
    return 0;
}

void CFavorPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_QUESTS, m_listQuests);
}

void CFavorPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_listQuests.GetSafeHwnd()))
    {
        m_listQuests.MoveWindow(c_controlSpacing, c_controlSpacing,
                cx - c_controlSpacing * 2, cy - c_controlSpacing * 2, TRUE);
    }
    SetScaleToFitSize(CSize(cx, cy));
}

BOOL CFavorPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CFavorPane::OnInitialUpdate()
{
    __super::OnInitialUpdate();
}

void CFavorPane::PopulateQuestList()
{
    m_listQuests.LockWindowUpdate();
    m_listQuests.DeleteAllItems();
    const std::list<Quest>& quests = Quests();
    DWORD dwQuestindex = 0;
    for (auto&& qit : quests)
    {
        // add one list entry for every Levels entry this quest has
        const std::vector<int>& levels = qit.Levels();
        for (auto&& lit: levels)
        {
            int iIndex = m_listQuests.InsertItem(m_listQuests.GetItemCount(), qit.Name().c_str());
            m_listQuests.SetItemData(iIndex, dwQuestindex);
            CString text;
            text.Format("%d", lit);
            m_listQuests.SetItemText(iIndex, 1, text);
        }
        ++dwQuestindex;
    }
    m_listQuests.UnlockWindowUpdate();
}

void CFavorPane::UpdateActiveBuildChanged(Character*)
{
    if (m_pCharacter != NULL)
    {
        const Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            PopulateQuestList();
        }
        else
        {
            m_listQuests.DeleteAllItems();
        }
    }
    else
    {
        m_listQuests.DeleteAllItems();
    }
}

