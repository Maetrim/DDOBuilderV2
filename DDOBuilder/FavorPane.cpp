// FavorPane.cpp : implementation file
//

#include "stdafx.h"
#include "FavorPane.h"
#include "GlobalSupportFunctions.h"
#include "DDOBuilderDoc.h"
#include "LogPane.h"
#include "Quest.h"
#include "Patron.h"

// CFavorPane
namespace
{
    enum QuestListColumns
    {
        QLC_name = 0,
        QLC_level,
        QLC_favor,
        QLC_runAt,
        QLC_patron
    };
}

IMPLEMENT_DYNCREATE(CFavorPane, CFormView)
BEGIN_MESSAGE_MAP(CFavorPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_QUESTS, OnColumnclickListQuests)
    ON_NOTIFY(NM_RCLICK, IDC_LIST_QUESTS, OnRightClickListQuests)
    ON_UPDATE_COMMAND_UI(ID_SETFAVOR_NONE, OnUpdateFavorNone)
    ON_UPDATE_COMMAND_UI(ID_SETFAVOR_CASUAL, OnUpdateFavorCasual)
    ON_UPDATE_COMMAND_UI(ID_SETFAVOR_NORMAL, OnUpdateFavorNormal)
    ON_UPDATE_COMMAND_UI(ID_SETFAVOR_HARD, OnUpdateFavorHard)
    ON_UPDATE_COMMAND_UI(ID_SETFAVOR_ELITE, OnUpdateFavorElite)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER1, OnUpdateFavorReaper1)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER2, OnUpdateFavorReaper2)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER3, OnUpdateFavorReaper3)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER4, OnUpdateFavorReaper4)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER5, OnUpdateFavorReaper5)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER6, OnUpdateFavorReaper6)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER7, OnUpdateFavorReaper7)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER8, OnUpdateFavorReaper8)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER9, OnUpdateFavorReaper9)
    ON_UPDATE_COMMAND_UI(ID_REAPER_REAPER10, OnUpdateFavorReaper10)
END_MESSAGE_MAP()

CFavorPane::CFavorPane() :
    CFormView(CFavorPane::IDD),
    m_pDoc(NULL),
    m_pCharacter(NULL),
    m_bLoadComplete(false),
    m_patronCount(0),
    m_bHadIntialise(false)
{
    m_favorMenu.LoadMenu(IDR_SELECT_FAVOR_MENU);
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
    m_listQuests.InsertColumn(0, "Quest Name", LVCFMT_LEFT, 200);
    m_listQuests.InsertColumn(1, "Level", LVCFMT_LEFT, 40);
    m_listQuests.InsertColumn(2, "Favor", LVCFMT_LEFT, 40);
    m_listQuests.InsertColumn(3, "Run@", LVCFMT_LEFT, 60);
    m_listQuests.InsertColumn(4, "Patron", LVCFMT_LEFT, 100);
    m_listQuests.SetExtendedStyle(m_listQuests.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    m_sortHeader.SetSortArrow(1, FALSE);     // sort by level by default

    const std::list<Patron>& patrons = Patrons();
    m_patronCount = 0;
    for (auto&& pit : patrons)
    {
        CString patronName = EnumEntryText(pit.Name(), patronTypeMap);
        m_favorItems[m_patronCount].SetPatronName(patronName);
        m_favorItems[m_patronCount].SetFavorTiers(pit.FavorTiers(), pit.MaxFavor());
        m_favorItems[m_patronCount].ShowWindow(SW_SHOW);
        ++m_patronCount;
    }
    CRect rect;
    GetClientRect(rect);
    OnSize(SIZE_RESTORED, rect.Width(), rect.Height());
    return 0;
}

void CFavorPane::DoDataExchange(CDataExchange* pDX)
{
    if (!m_bHadIntialise)
    {
        m_bHadIntialise = true;
        CFormView::DoDataExchange(pDX);
        DDX_Control(pDX, IDC_LIST_QUESTS, m_listQuests);
        for (size_t i = 0; i < MAX_FAVOR_ITEMS; ++i)
        {
            DDX_Control(pDX, IDC_FAVOR_1 + i, m_favorItems[i]);
        }
        if (!pDX->m_bSaveAndValidate)
        {
            VERIFY(m_sortHeader.SubclassWindow(m_listQuests.GetHeaderCtrl()->GetSafeHwnd()));
        }
    }
}

void CFavorPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    CRect rectPatronItem(0, 0, 10, 10);
    if (IsWindow(m_listQuests.GetSafeHwnd()))
    {
        m_favorItems[0].GetWindowRect(&rectPatronItem);
        rectPatronItem -= rectPatronItem.TopLeft();
        rectPatronItem += CPoint(c_controlSpacing, c_controlSpacing);
        rectPatronItem.right = cx / 2;
        // patron list on the left hand side
        for (size_t i = 0; i < m_patronCount; ++i)
        {
            m_favorItems[i].MoveWindow(rectPatronItem, TRUE);
            rectPatronItem += CPoint(0, c_controlSpacing + rectPatronItem.Height());
        }
        m_listQuests.MoveWindow(cx / 2 + c_controlSpacing, c_controlSpacing,
                cx / 2 - c_controlSpacing, cy - c_controlSpacing * 2, TRUE);
    }
    SetScrollSizes(MM_TEXT, CSize(cx- GetSystemMetrics(SM_CYHSCROLL), rectPatronItem.top));
}

BOOL CFavorPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_LIST_QUESTS,
        IDC_FAVOR_1,
        IDC_FAVOR_2,
        IDC_FAVOR_3,
        IDC_FAVOR_4,
        IDC_FAVOR_5,
        IDC_FAVOR_6,
        IDC_FAVOR_7,
        IDC_FAVOR_8,
        IDC_FAVOR_9,
        IDC_FAVOR_10,
        IDC_FAVOR_11,
        IDC_FAVOR_12,
        IDC_FAVOR_13,
        IDC_FAVOR_14,
        IDC_FAVOR_15,
        IDC_FAVOR_16,
        IDC_FAVOR_17,
        IDC_FAVOR_18,
        IDC_FAVOR_19,
        IDC_FAVOR_20,
        IDC_FAVOR_21,
        IDC_FAVOR_22,
        IDC_FAVOR_23,
        IDC_FAVOR_24,
        IDC_FAVOR_25,
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
        CString patron = EnumEntryText(qit.Patron(), patronTypeMap);
        for (auto&& lit: levels)
        {
            CString questName = qit.Name().c_str();
            if (qit.HasEpicName() && lit > MAX_CLASS_LEVEL)
            {
                questName = qit.EpicName().c_str();
            }
            int iIndex = m_listQuests.InsertItem(m_listQuests.GetItemCount(), questName);
            m_listQuests.SetItemData(iIndex, dwQuestindex);
            CString text;
            text.Format("%d", lit);
            m_listQuests.SetItemText(iIndex, QLC_level, text);
            ++dwQuestindex;
            m_listQuests.SetItemText(iIndex, QLC_patron, patron);
        }
    }
    m_listQuests.SortItems(
            CFavorPane::SortCompareFunction,
            (long)GetSafeHwnd());
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

void CFavorPane::OnColumnclickListQuests(NMHDR* pNMHDR, LRESULT* pResult)
{
    // allow the user to sort the item list based on the selected column
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    int columnToSort = pNMListView->iSubItem;
    int column;
    bool bAscending;
    m_sortHeader.GetSortArrow(&column, &bAscending);
    if (column == columnToSort)
    {
        // just change sort direction
        bAscending = !bAscending;
    }
    else
    {
        // just change the column
        column = columnToSort;
    }
    m_sortHeader.SetSortArrow(column, bAscending);
    m_listQuests.SortItems(CFavorPane::SortCompareFunction, (long)GetSafeHwnd());

    *pResult = 0;
}

void CFavorPane::OnRightClickListQuests(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    //NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    unsigned int uCount = m_listQuests.GetSelectedCount();
    if (uCount > 0)
    {
        CMenu* pPopup = m_favorMenu.GetSubMenu(0);
        CPoint menuPos;
        GetCursorPos(&menuPos);
        CWinAppEx * pApp = dynamic_cast<CWinAppEx*>(AfxGetApp());
        /*int sel = */pApp->GetContextMenuManager()->TrackPopupMenu(
                pPopup->GetSafeHmenu(),
                menuPos.x,
                menuPos.y,
                this);
    }
    *pResult = 0;
}

int CFavorPane::SortCompareFunction(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort)
{
    // this is a static function so we need to make our own this pointer
    CWnd* pWnd = CWnd::FromHandle((HWND)lParamSort);
    CFavorPane* pThis = static_cast<CFavorPane*>(pWnd);

    int sortResult = 0;
    size_t index1 = lParam1; // item data index
    size_t index2 = lParam2; // item data index

    // need to find the actual current entry in the list control to compare the
    // text content
    index1 = FindItemIndexByItemData(&pThis->m_listQuests, index1);
    index2 = FindItemIndexByItemData(&pThis->m_listQuests, index2);

    // get the current sort settings
    int column;
    bool bAscending;
    pThis->m_sortHeader.GetSortArrow(&column, &bAscending);

    // get the control text entries for the column being sorted
    CString index1Text = pThis->m_listQuests.GetItemText(index1, column);
    CString index2Text = pThis->m_listQuests.GetItemText(index2, column);

    // some columns are converted to numeric to do the sort
    // while others are compared as ASCII
    switch (column)
    {
    case QLC_patron:
        // ASCII sorts
        sortResult = (index1Text < index2Text) ? -1 : 1;
        if (index1Text == index2Text)
        {
            // if we have the same patron, sort by quest level
            index1Text = pThis->m_listQuests.GetItemText(index1, QLC_level);
            index2Text = pThis->m_listQuests.GetItemText(index2, QLC_level);
            // numeric sorts
            double val1 = atof(index1Text);
            double val2 = atof(index2Text);
            if (val1 == val2)
            {
                // if numeric match, sort by item name instead
                index1Text = pThis->m_listQuests.GetItemText(index1, QLC_name);
                index2Text = pThis->m_listQuests.GetItemText(index2, QLC_name);
                sortResult = (index1Text < index2Text) ? -1 : 1;
            }
            else
            {
                sortResult = (val1 < val2) ? -1 : 1;
            }
        }
        break;
    case QLC_name:
        // ASCII sorts
        sortResult = (index1Text < index2Text) ? -1 : 1;
        break;
    case QLC_level:
    case QLC_favor:
        {
            // numeric sorts
            double val1 = atof(index1Text);
            double val2 = atof(index2Text);
            if (val1 == val2)
            {
                // if numeric match, sort by item name instead
                index1Text = pThis->m_listQuests.GetItemText(index1, QLC_name);
                index2Text = pThis->m_listQuests.GetItemText(index2, QLC_name);
                sortResult = (index1Text < index2Text) ? -1 : 1;
            }
            else
            {
                sortResult = (val1 < val2) ? -1 : 1;
            }
        }
        break;
    }
    if (!bAscending)
    {
        // switch sort direction result
        sortResult = -sortResult;
    }
    return sortResult;
}

void CFavorPane::OnUpdateFavorNone(CCmdUI* pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_None);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_None);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorCasual(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Casual);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Casual);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorNormal(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Normal);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Normal);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorHard(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Hard);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Hard);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorElite(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Elite);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Elite);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper1(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper1);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper1);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper2(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper2);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper2);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper3(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper3);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper3);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper4(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper4);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper4);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper5(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper5);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper5);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper6(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper6);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper6);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper7(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper7);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper7);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper8(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper8);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper8);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper9(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper9);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper9);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorReaper10(CCmdUI * pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Reaper10);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Reaper10);
    pCmdUi->Enable(bEnabledState);
}

unsigned int CFavorPane::GetCheckedState(FavorType ft)
{
    return (ft % 3);
}

bool CFavorPane::GetEnabledState(FavorType /*ft*/)
{
    return true;
}
