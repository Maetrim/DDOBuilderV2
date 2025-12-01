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
    ON_UPDATE_COMMAND_UI(ID_NOT_SELECTABLE, OnUpdateNotSelectable)
    ON_UPDATE_COMMAND_UI(ID_SETFAVOR_NONE, OnUpdateFavorNone)
    ON_UPDATE_COMMAND_UI(ID_SETFAVOR_SOLO, OnUpdateFavorSolo)
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
    ON_CBN_SELENDOK(IDC_COMBO_MINLEVEL, OnSelendokMinLevel)
    ON_CBN_SELENDOK(IDC_COMBO_MAXLEVEL, OnSelendokMaxLevel)
    ON_CBN_SELENDOK(IDC_COMBO_PATRON, OnSelendokPatron)
    ON_EN_CHANGE(IDC_EDIT_SEARCHTEXT, OnChangeSearchText)
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
    if (NULL != pCharacter)
    {
        m_pCharacter = pCharacter;
        m_pCharacter->AttachObserver(this);
    }
    PopulateQuestList();
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
    m_listQuests.InsertColumn(4, "Patron", LVCFMT_LEFT, 140);
    m_listQuests.SetExtendedStyle(m_listQuests.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    m_sortHeader.SetSortArrow(1, TRUE);     // sort by level (increasing) by default

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
    // populate the drop list controls
    CString txt;
    for (size_t level = 1; level <= MAX_QUEST_LEVEL; ++level)
    {
        txt.Format("%d", level);
        m_comboMinLevel.AddString(txt);
        m_comboMaxLevel.AddString(txt);
    }
    m_comboMinLevel.SetCurSel(0);
    m_comboMaxLevel.SetCurSel(m_comboMaxLevel.GetCount() - 1);
    m_comboPatron.AddString("<No Filtering>");
    for (auto&& pit : patrons)
    {
        if (pit.Name() < Patron_TotalFavor)
        {
            CString patronName = EnumEntryText(pit.Name(), patronTypeMap);
            m_comboPatron.AddString(patronName);
        }
    }
    m_comboPatron.SetCurSel(0);
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
        DDX_Control(pDX, IDC_STATIC_FILTER_BY, m_staticFilterByLevel);
        DDX_Control(pDX, IDC_COMBO_MINLEVEL, m_comboMinLevel);
        DDX_Control(pDX, IDC_STATIC_TO, m_staticTo);
        DDX_Control(pDX, IDC_COMBO_MAXLEVEL, m_comboMaxLevel);
        DDX_Control(pDX, IDC_STATIC_PATRON, m_staticPatron);
        DDX_Control(pDX, IDC_COMBO_PATRON, m_comboPatron);
        DDX_Control(pDX, IDC_STATIC_TEXT, m_staticContains);
        DDX_Control(pDX, IDC_EDIT_SEARCHTEXT, m_editSearchText);
    }
}

void CFavorPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    CRect rectPatronItem(0, 0, 10, 10);
    if (IsWindow(m_listQuests.GetSafeHwnd())
            && IsWindowVisible())
    {
        m_favorItems[0].GetWindowRect(&rectPatronItem);
        rectPatronItem -= rectPatronItem.TopLeft();
        rectPatronItem += CPoint(c_controlSpacing, c_controlSpacing);
        rectPatronItem.right = min(280, cx / 2);    // limit x size
        // patron list on the left hand side
        for (size_t i = 0; i < m_patronCount; ++i)
        {
            m_favorItems[i].MoveWindow(rectPatronItem, TRUE);
            rectPatronItem += CPoint(0, c_controlSpacing + rectPatronItem.Height());
        }
        // measure and place the filter control to the right
        CRect rctSearchControls[8];
        m_staticFilterByLevel.GetWindowRect(rctSearchControls[0]);
        m_comboMinLevel.GetWindowRect(rctSearchControls[1]);
        m_staticTo.GetWindowRect(rctSearchControls[2]);
        m_comboMaxLevel.GetWindowRect(rctSearchControls[3]);
        m_staticPatron.GetWindowRect(rctSearchControls[4]);
        m_comboPatron.GetWindowRect(rctSearchControls[5]);
        m_staticContains.GetWindowRect(rctSearchControls[6]);
        m_editSearchText.GetWindowRect(rctSearchControls[7]);
        for (size_t i = 0; i < 8; i++)
        {
            rctSearchControls[i] -= rctSearchControls[i].TopLeft();
            if (i == 0)
            {
                rctSearchControls[i] += CSize(rectPatronItem.right + c_controlSpacing, c_controlSpacing);
            }
            else
            {
                rctSearchControls[i] += CSize(rctSearchControls[i-1].right + c_controlSpacing, c_controlSpacing);
            }
        }
        rctSearchControls[7].right = cx - c_controlSpacing;
        m_staticFilterByLevel.MoveWindow(rctSearchControls[0]);
        m_comboMinLevel.MoveWindow(rctSearchControls[1]);
        m_staticTo.MoveWindow(rctSearchControls[2]);
        m_comboMaxLevel.MoveWindow(rctSearchControls[3]);
        m_staticPatron.MoveWindow(rctSearchControls[4]);
        m_comboPatron.MoveWindow(rctSearchControls[5]);
        m_staticContains.MoveWindow(rctSearchControls[6]);
        m_editSearchText.MoveWindow(rctSearchControls[7]);
        m_listQuests.MoveWindow(rectPatronItem.right + c_controlSpacing, rctSearchControls[0].bottom + c_controlSpacing,
                cx - rectPatronItem.right - c_controlSpacing * 2, cy - c_controlSpacing * 3 - rctSearchControls[0].Height(), TRUE);
    }
    SetScrollSizes(MM_TEXT, CSize(cx- GetSystemMetrics(SM_CYHSCROLL), rectPatronItem.top));
}

BOOL CFavorPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
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
        IDC_STATIC_FILTER_BY,
        IDC_COMBO_MINLEVEL,
        IDC_STATIC_TO,
        IDC_COMBO_MAXLEVEL,
        IDC_STATIC_PATRON,
        IDC_COMBO_PATRON,
        IDC_STATIC_TEXT,
        IDC_EDIT_SEARCHTEXT,
        IDC_LIST_QUESTS,
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
    m_runQuests.clear();
    if (m_pCharacter != NULL)
    {
        // duplicates have been removed here
        m_runQuests = m_pCharacter->CompletedQuests();
    }
    size_t selMinLevel = m_comboMinLevel.GetCurSel() + 1;
    size_t selMaxLevel = m_comboMaxLevel.GetCurSel() + 1;
    int selPatron = m_comboPatron.GetCurSel();
    PatronType selectedPatron = Patron_Unknown;
    if (selPatron > 0)
    {
        CString patronName;
        m_comboPatron.GetLBText(selPatron, patronName);
        selectedPatron = TextToEnumEntry((LPCSTR)patronName, patronTypeMap);
    }
    CString strSearchText;
    m_editSearchText.GetWindowText(strSearchText);
    strSearchText.MakeLower();
    std::string strFindText = (LPCTSTR)strSearchText;

    int topIndex = m_listQuests.GetTopIndex();
    m_listQuests.LockWindowUpdate();
    m_listQuests.DeleteAllItems();
    std::list<Quest> quests = Quests();
    int column = 0;
    bool bAscending = false;
    m_sortHeader.GetSortArrow(&column, &bAscending);
    Quest::SetSortInfo(column, bAscending);
    quests.sort();
    for (auto&& qit : quests)
    {
        if (qit.HasDoNotShow()) continue;       // skip showing this one

        // level filtering
        if (selMinLevel <= qit.Levels()[0]
            && selMaxLevel >= qit.Levels()[0])
        {
            CString patron = EnumEntryText(qit.Patron(), patronTypeMap);
            // do we have patron filtering?
            if (selPatron == 0          // no filtering
                || qit.Patron() == selectedPatron)
            {
                if (strSearchText == ""
                        || SearchForText(qit.Name(), strFindText))
                {
                    // add one list entry for every Levels entry this quest has
                    CString questName = qit.Name().c_str();
                    int iIndex = m_listQuests.InsertItem(m_listQuests.GetItemCount(), questName);
                    CString text;
                    text.Format("%d", qit.Levels()[0]);
                    m_listQuests.SetItemText(iIndex, QLC_level, text);
                    text.Format("%d", qit.Favor());
                    m_listQuests.SetItemText(iIndex, QLC_favor, text);
                    m_listQuests.SetItemText(iIndex, QLC_patron, patron);
                    QuestDifficulty diff = QD_notRun;
                    for (auto&& rqi : m_runQuests)
                    {
                        if (rqi.Name() == qit.Name()
                            && rqi.Level() == qit.Levels()[0])
                        {
                            diff = max(diff, rqi.Difficulty());
                        }
                    }
                    CString difficulty = EnumEntryText(diff, questDifficultyTypeMap);
                    m_listQuests.SetItemText(iIndex, QLC_runAt, difficulty);
                }
            }
        }
    }
    // now sum the total favor per patron from all quests that have a run@
    std::vector<size_t> favorPerPatron(MAX_FAVOR_ITEMS, 0);
    // now add the favor to the total for this patron
    std::list<CompletedQuest> favorQs = m_runQuests;
    RemoveQuestDuplicates(favorQs); // only the highest favor for a given quest name regardless of level
    for (auto&& fqit : favorQs)
    {
        const Quest& quest = FindQuest(fqit.Name());
        PatronType pt = quest.Patron();
        int favor = quest.Favor(fqit.Difficulty());
        favorPerPatron[static_cast<size_t>(pt)] += favor;
        favorPerPatron[Patron_TotalFavor] += favor;
    }
    for (size_t i = 0; i < MAX_FAVOR_ITEMS; ++i)
    {
        m_favorItems[i].SetCurrentFavor(favorPerPatron[i]);
    }
    m_listQuests.EnsureVisible(m_listQuests.GetItemCount() - 1, FALSE);
    m_listQuests.EnsureVisible(topIndex, FALSE);
    m_listQuests.UnlockWindowUpdate();
    bool bEnableWindows = false;
    if (m_pCharacter != NULL)
    {
        bEnableWindows = (m_pCharacter->ActiveBuild() != NULL);
    }
    m_comboMinLevel.EnableWindow(bEnableWindows);
    m_comboMaxLevel.EnableWindow(bEnableWindows);
    m_comboPatron.EnableWindow(bEnableWindows);
    m_editSearchText.EnableWindow(bEnableWindows);
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
            m_comboMinLevel.EnableWindow(FALSE);
            m_comboMaxLevel.EnableWindow(FALSE);
            m_comboPatron.EnableWindow(FALSE);
            m_editSearchText.EnableWindow(FALSE);
        }
    }
    else
    {
        m_listQuests.DeleteAllItems();
        m_comboMinLevel.EnableWindow(FALSE);
        m_comboMaxLevel.EnableWindow(FALSE);
        m_comboPatron.EnableWindow(FALSE);
        m_editSearchText.EnableWindow(FALSE);
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
    PopulateQuestList();
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
        int sel = pApp->GetContextMenuManager()->TrackPopupMenu(
                pPopup->GetSafeHmenu(),
                menuPos.x,
                menuPos.y,
                this);
        if (sel != 0)
        {
            // the user has selected a difficulty for the selected quests
            QuestDifficulty qd = QD_notRun;
            switch (sel)
            {
                case ID_SETFAVOR_NONE:      qd = QD_notRun; break;
                case ID_SETFAVOR_SOLO:      qd = QD_solo; break;
                case ID_SETFAVOR_CASUAL:    qd = QD_casual; break;
                case ID_SETFAVOR_NORMAL:    qd = QD_normal; break;
                case ID_SETFAVOR_HARD:      qd = QD_hard; break;
                case ID_SETFAVOR_ELITE:     qd = QD_elite; break;
                case ID_REAPER_REAPER1:     qd = QD_reaper1; break;
                case ID_REAPER_REAPER2:     qd = QD_reaper2; break;
                case ID_REAPER_REAPER3:     qd = QD_reaper3; break;
                case ID_REAPER_REAPER4:     qd = QD_reaper4; break;
                case ID_REAPER_REAPER5:     qd = QD_reaper5; break;
                case ID_REAPER_REAPER6:     qd = QD_reaper6; break;
                case ID_REAPER_REAPER7:     qd = QD_reaper7; break;
                case ID_REAPER_REAPER8:     qd = QD_reaper8; break;
                case ID_REAPER_REAPER9:     qd = QD_reaper9; break;
                case ID_REAPER_REAPER10:    qd = QD_reaper10; break;
            }
            std::list<CompletedQuest> selectedQs = GetSelectedQuests();
            std::list<CompletedQuest>::iterator qit = selectedQs.begin();
            while (qit != selectedQs.end())
            {
                if (qit->Supports(qd))
                {
                    qit->Set_Difficulty(qd);
                    ++qit;
                }
                else
                {
                    // this quest does not support this difficulty, don't try and set it
                    qit = selectedQs.erase(qit);
                }
            }
            Build* pBuild = m_pCharacter->ActiveBuild();
            if (pBuild != NULL)
            {
                pBuild->SetQuestsCompletions(selectedQs);
            }
        }
        // update the display and total favor etc
        PopulateQuestList();
    }
    *pResult = 0;
}

void CFavorPane::OnUpdateNotSelectable(CCmdUI* pCmdUi)
{
    pCmdUi->Enable(FALSE);
}

void CFavorPane::OnUpdateFavorNone(CCmdUI* pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_None);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_None);
    pCmdUi->Enable(bEnabledState);
}

void CFavorPane::OnUpdateFavorSolo(CCmdUI* pCmdUi)
{
    unsigned int checkedState = GetCheckedState(Favor_Solo);
    pCmdUi->SetCheck(checkedState);
    bool bEnabledState = GetEnabledState(Favor_Solo);
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

unsigned int CFavorPane::GetCheckedState(FavorType)
{
    // look for the highest difficulty this quests has been run at (could
    // occur multiple times in the list)
    //QuestDifficulty diff = QD_notRun;
    //for (auto&& rqi : m_runQuests)
    //{
    //    if (rqi.Name() == qit.Name()
    //        && rqi.Level() == qit.Levels()[0])
    //    {
    //        diff = max(diff, rqi.Difficulty());
    //    }
    //}
    return FALSE;
}

bool CFavorPane::GetEnabledState(FavorType ft)
{
    std::list<CompletedQuest> quests = GetSelectedQuests();
    // at least one of the selected quests must support the given favor
    // tier for it to be enabled.
    bool bSupported = false;
    //if (quests.size() < 15)
    {
        // don't allow more than 15 quests at a time to be set
        for (auto&& qit : quests)
        {
            const Quest& q = FindQuest(qit.Name());
            switch (ft)
            {
                case Favor_None:    bSupported = true; break;
                case Favor_Solo:    bSupported = q.HasSolo(); break;
                case Favor_Casual:  bSupported = q.HasCasual(); break;
                case Favor_Normal:  bSupported = q.HasNormal(); break;
                case Favor_Hard:    bSupported = q.HasHard(); break;
                case Favor_Elite:   bSupported = q.HasElite(); break;
                case Favor_Reaper1:
                case Favor_Reaper2:
                case Favor_Reaper3:
                case Favor_Reaper4:
                case Favor_Reaper5:
                case Favor_Reaper6:
                case Favor_Reaper7:
                case Favor_Reaper8:
                case Favor_Reaper9:
                case Favor_Reaper10:    bSupported = q.HasReaper(); break;
            }
            if (bSupported) break;
        }
    }
    return bSupported;
}

std::list<CompletedQuest> CFavorPane::GetSelectedQuests()
{
    std::list<CompletedQuest> quests;
    int iCount = m_listQuests.GetSelectedCount();
    std::vector<int> indexes;
    indexes.reserve(iCount);
    POSITION pos = m_listQuests.GetFirstSelectedItemPosition();
    while (pos != NULL)
    {
        int sel = m_listQuests.GetNextSelectedItem(pos);
        indexes.push_back(sel);
    }
    for (size_t i = 0; i < static_cast<size_t>(iCount); ++i)
    {
        CString name = m_listQuests.GetItemText(indexes[i], QLC_name);
        int level = atoi(m_listQuests.GetItemText(indexes[i], QLC_level));
        CompletedQuest cq;
        cq.Set_Name((LPCTSTR)name);
        cq.Set_Level(static_cast<size_t>(level));
        quests.push_back(cq);
    }
    return quests;
}

void CFavorPane::RemoveQuestDuplicates(std::list<CompletedQuest>& favorQs)
{
    // only the highest favor for a given quest name regardless of level
    if (favorQs.size() > 1)
    {
        std::list<CompletedQuest>::iterator it = favorQs.begin();
        while (it != favorQs.end())
        {
            std::list<CompletedQuest>::iterator nit = it;
            const Quest& itq = FindQuest(it->Name());
            nit++;
            while (nit != favorQs.end())
            {
                const Quest& nitq = FindQuest(nit->Name());
                if (itq.Name() == nitq.Name())
                {
                    QuestDifficulty qd1 = it->Difficulty();
                    QuestDifficulty qd2 = nit->Difficulty();
                    QuestDifficulty nqd = max(qd1, qd2);
                    it->Set_Difficulty(nqd);
                    nit = favorQs.erase(nit);
                }
                else
                {
                    ++nit;
                }
            }
            ++it;
        }
    }
}

void CFavorPane::OnSelendokMinLevel()
{
    int selMin = m_comboMinLevel.GetCurSel();
    int selMax = m_comboMaxLevel.GetCurSel();
    if (selMax < selMin)
    {
        m_comboMaxLevel.SetCurSel(selMin);
    }
    PopulateQuestList();
}

void CFavorPane::OnSelendokMaxLevel()
{
    int selMin = m_comboMinLevel.GetCurSel();
    int selMax = m_comboMaxLevel.GetCurSel();
    if (selMin > selMax)
    {
        m_comboMinLevel.SetCurSel(selMax);
    }
    PopulateQuestList();
}

void CFavorPane::OnSelendokPatron()
{
    PopulateQuestList();
}

void CFavorPane::OnChangeSearchText()
{
    PopulateQuestList();
}
