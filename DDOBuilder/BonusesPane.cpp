// BonusesPane.cpp
//
#include "stdafx.h"

#include "BonusesPane.h"
#include "GlobalSupportFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBonusesPane
namespace
{
    enum Columns    // must match order items added to control later
    {
        CI_Name = 0,
        CI_Enhancement,
        CI_Insightful,
        CI_Artifact,
        CI_Quality,
        CI_Profane,
        CI_Equipment,
        CI_Competence,
        CI_Exceptional,
        CI_Festive,
        CI_Fortune,
        CI_Columns
    };
};

CBonusesPane::CBonusesPane() :
    CFormView(CBonusesPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL),
    m_bHadInitialUpdate(false),
    m_bPopulating(false)
{
}

CBonusesPane::~CBonusesPane()
{
}

IMPLEMENT_DYNCREATE(CBonusesPane, CFormView)
BEGIN_MESSAGE_MAP(CBonusesPane, CFormView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_WM_ERASEBKGND()
    ON_BN_CLICKED(IDC_BUTTON_ADD, &CBonusesPane::OnAddBreakdown)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CBonusesPane::OnRemoveBreakdown)
    ON_BN_CLICKED(IDC_BUTTON_MOVEUP, &CBonusesPane::OnMoveUpBreakdown)
    ON_BN_CLICKED(IDC_BUTTON_MOVEDOWN, &CBonusesPane::OnMoveDownBreakdown)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_BONUSES_LIST, OnItemchangedBonusesList)
END_MESSAGE_MAP()

void CBonusesPane::OnInitialUpdate()
{
    if (!m_bHadInitialUpdate)
    {
        m_bHadInitialUpdate = true;
        CFormView::OnInitialUpdate();
        UpdateFonts();
        m_buttonAdd.SetImage(IDB_BITMAP_ADD);
        m_buttonRemove.SetImage(IDB_BITMAP_REMOVE);
        m_buttonMoveUp.SetImage(IDB_BITMAP_MOVEUP);
        m_buttonMoveDown.SetImage(IDB_BITMAP_MOVEDOWN);
    }
}

LRESULT CBonusesPane::OnLoadComplete(WPARAM, LPARAM)
{
    m_bPopulating = true;
    m_listBonuses.InsertColumn(CI_Name, "Statistic", LVCFMT_LEFT, 100);
    m_listBonuses.InsertColumn(CI_Enhancement, "Enhancement", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Insightful, "Insightful", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Artifact, "Artifact", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Quality, "Quality", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Profane, "Profane", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Equipment, "Equipment", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Competence, "Competence", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Exceptional, "Exceptional", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Festive, "Festive", LVCFMT_CENTER, 65);
    m_listBonuses.InsertColumn(CI_Fortune, "Fortune", LVCFMT_CENTER, 65);
    m_listBonuses.SetExtendedStyle(m_listBonuses.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    m_bPopulating = false;
    return 0;
}

LRESULT CBonusesPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
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
    ClearItems();
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        PopulateItems();
    }
    return 0L;
}

void CBonusesPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BONUSES_LIST, m_listBonuses);
    DDX_Control(pDX, IDC_BUTTON_ADD, m_buttonAdd);
    DDX_Control(pDX, IDC_BUTTON_REMOVE, m_buttonRemove);
    DDX_Control(pDX, IDC_BUTTON_MOVEUP, m_buttonMoveUp);
    DDX_Control(pDX, IDC_BUTTON_MOVEDOWN, m_buttonMoveDown);
}

void CBonusesPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_listBonuses.GetSafeHwnd())
            && IsWindowVisible())
    {
        // position all the windows
        // +-----------------------------------------------+
        // | +----------------------+--------------------+ |
        // | |                                           | |
        // | | Ability List                              | |
        // | |                                           | |
        // | |                                           | |
        // | |                                           | |
        // | |                                           | |
        // | |                                           | |
        // | |                                           | |
        // | |                                           | |
        // | +-------------------------------------------+ |
        // | [+][-][^][v]                                  |
        // +-----------------------------------------------+
        CRect rctList(c_controlSpacing, c_controlSpacing, cx - c_controlSpacing, cy - c_controlSpacing);
        CRect rctAdd;
        CRect rctRemove;
        CRect rctMoveUp;
        CRect rctMoveDown;
        m_buttonAdd.GetWindowRect(&rctAdd);
        m_buttonRemove.GetWindowRect(&rctRemove);
        m_buttonMoveUp.GetWindowRect(&rctMoveUp);
        m_buttonMoveDown.GetWindowRect(&rctMoveDown);
        rctAdd -= rctAdd.TopLeft();
        rctRemove -= rctRemove.TopLeft();
        rctMoveUp -= rctMoveUp.TopLeft();
        rctMoveDown -= rctMoveDown.TopLeft();
        rctAdd += CPoint(c_controlSpacing, cy - rctAdd.Height() - c_controlSpacing);
        rctRemove += CPoint(c_controlSpacing + rctAdd.right, cy - rctRemove.Height() - c_controlSpacing);
        rctMoveUp += CPoint(rctRemove.right + c_controlSpacing, cy - rctMoveUp.Height() - c_controlSpacing);
        rctMoveDown += CPoint(rctMoveUp.right + c_controlSpacing, cy - rctMoveDown.Height() - c_controlSpacing);
        rctList.bottom = rctAdd.top - c_controlSpacing;
        m_listBonuses.MoveWindow(rctList, TRUE);
        m_buttonAdd.MoveWindow(rctAdd, TRUE);
        m_buttonRemove.MoveWindow(rctRemove, TRUE);
        m_buttonMoveUp.MoveWindow(rctMoveUp, TRUE);
        m_buttonMoveDown.MoveWindow(rctMoveDown, TRUE);
    }
    SetScaleToFitSize(CSize(cx, cy));
    CWnd *pWnd = this;
    while (pWnd != NULL)
    {
        pWnd = pWnd->GetParent();
    }
}

void CBonusesPane::UpdateFonts()
{
    m_listBonuses.SetFont(&afxGlobalData.fontRegular);
}

BOOL CBonusesPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_BONUSES_LIST,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CBonusesPane::OnAddBreakdown()
{
    // build a menu of the available breakdowns to show
    CMenu cMenu;
    cMenu.CreatePopupMenu();
    cMenu.AppendMenu(
            MF_STRING | MF_GRAYED,
            0,
            "Select Breakdown to Show");
    cMenu.AppendMenu(MF_SEPARATOR);

    Life* pLife = m_pCharacter->ActiveLife();
    const std::list<std::string>& bonuses = pLife->MonitoredBonuses();
    const XmlLib::enumMapEntry<BreakdownType>* p = breakdownNameMap;
    int index = 0;
    while (p->name != NULL)
    {
        // only add if this item is not already in the list
        XmlLib::SaxString entry = p->name;
        std::string name = (std::string)entry;
        bool bAlreadyPresent = false;
        for (auto&& it: bonuses)
        {
            if (it == name)
            {
                bAlreadyPresent = true;
                break;
            }
        }
        if (!bAlreadyPresent)
        {
            CString popupMenuName = EnumEntryText(p->value, breakdownMenuMap);
            if (popupMenuName.IsEmpty())
            {
                cMenu.AppendMenu(
                    MF_STRING,
                    IDC_BREAKDOWN_IDS_START_HERE + index,
                    name.c_str());
            }
            else
            {
                CString fullName = popupMenuName;
                popupMenuName += "\\";
                popupMenuName += name.c_str();
                AddMenuItem(cMenu.GetSafeHmenu(), popupMenuName, IDC_BREAKDOWN_IDS_START_HERE + index, true);
            }
        }
        ++p;
        ++index;
    }
    CPoint point;
    GetCursorPos(&point);
    CWinAppEx* pApp = dynamic_cast<CWinAppEx*>(AfxGetApp());
    UINT sel = pApp->GetContextMenuManager()->TrackPopupMenu(
            cMenu.GetSafeHmenu(),
            point.x,
            point.y,
            this);
    if (sel >= IDC_BREAKDOWN_IDS_START_HERE)
    {
        sel = (InventorySlotType)(sel - IDC_BREAKDOWN_IDS_START_HERE);
        p = breakdownNameMap;
        std::advance(p, sel);
        XmlLib::SaxString entry = p->name;
        std::string name = (std::string)entry;
        pLife->AddMonitoredBreakdown(name);
        ClearItems();
        PopulateItems();
    }
}

void CBonusesPane::OnRemoveBreakdown()
{
    int sel = m_listBonuses.GetSelectionMark();
    if (sel >= 0)
    {
        DWORD itemData = m_listBonuses.GetItemData(sel);
        BreakdownType bt = static_cast<BreakdownType>(itemData);
        BreakdownItem* pBItem = FindBreakdown(bt);
        pBItem->DetachObserver(this);       // no longer observe it
        Life* pLife = m_pCharacter->ActiveLife();
        if (pLife != NULL)
        {
            std::string name = (LPCTSTR)EnumEntryText(bt, breakdownNameMap);
            pLife->RemoveMonitoredBreakdown(name);
            ClearItems();
            PopulateItems();
            int count = m_listBonuses.GetItemCount();
            if (sel == count && sel > 0)
            {
                sel--;
            }
            // keep selection
            m_listBonuses.SetSelectionMark(sel);
            m_listBonuses.SetItemState(sel, LVIS_SELECTED, LVIS_SELECTED);
            m_listBonuses.SetFocus();
        }
    }
}

void CBonusesPane::OnMoveUpBreakdown()
{
    int sel = m_listBonuses.GetSelectionMark();
    if (sel >= 0)
    {
        CString itemToMove = m_listBonuses.GetItemText(sel, CI_Name);
        Life* pLife = m_pCharacter->ActiveLife();
        if (pLife != NULL)
        {
            pLife->MoveUpMonitoredBreakdown((LPCTSTR)itemToMove);
            ClearItems();
            PopulateItems();
            m_listBonuses.SetSelectionMark(sel-1);  // keep it selected
            m_listBonuses.SetItemState(sel-1, LVIS_SELECTED, LVIS_SELECTED);
            m_listBonuses.SetFocus();
        }
    }
}

void CBonusesPane::OnMoveDownBreakdown()
{
    int sel = m_listBonuses.GetSelectionMark();
    if (sel >= 0)
    {
        CString itemToMove = m_listBonuses.GetItemText(sel, CI_Name);
        Life* pLife = m_pCharacter->ActiveLife();
        if (pLife != NULL)
        {
            pLife->MoveDownMonitoredBreakdown((LPCTSTR)itemToMove);
            ClearItems();
            PopulateItems();
            m_listBonuses.SetSelectionMark(sel+1);  // keep it selected
            m_listBonuses.SetItemState(sel+1, LVIS_SELECTED, LVIS_SELECTED);
            m_listBonuses.SetFocus();
        }
    }
}

void CBonusesPane::UpdateActiveBuildChanged(Character *)
{
    ClearItems();
    PopulateItems();
}

void CBonusesPane::UpdateTotalChanged(
    BreakdownItem* item,
    BreakdownType type)
{
    UNREFERENCED_PARAMETER(item);
    UNREFERENCED_PARAMETER(type);
    ClearItems();
    PopulateItems();
}

void CBonusesPane::ClearItems()
{
    // un-observe all items and clear the list
    int count = m_listBonuses.GetItemCount();
    for (int i = 0; i < count; ++i)
    {
        DWORD data = m_listBonuses.GetItemData(i);
        BreakdownType bt = static_cast<BreakdownType>(data);
        BreakdownItem* pBItem = FindBreakdown(bt);
        pBItem->DetachObserver(this);
    }
    m_listBonuses.DeleteAllItems();
}

void CBonusesPane::PopulateItems()
{
    if (NULL != m_pCharacter)
    {
        Life* pLife = m_pCharacter->ActiveLife();
        if (pLife != NULL)
        {
            const std::list<std::string>& bonuses = pLife->MonitoredBonuses();
            for (auto&& it: bonuses)
            {
                BreakdownType bt = TextToEnumEntry(it, breakdownNameMap);
                if (bt != Breakdown_Unknown)
                {
                    BreakdownItem* pBItem = FindBreakdown(bt);
                    pBItem->AttachObserver(this); // need to observe changes
                    // now add it to the list
                    int index = m_listBonuses.InsertItem(m_listBonuses.GetItemCount(), it.c_str(), 0);
                    m_listBonuses.SetItemData(index, bt);   // item data is the breakdown type
                    double value = 0;
                    for (size_t i = CI_Name + 1; i < CI_Columns; ++i)
                    {
                        switch (i)
                        {
                            case CI_Enhancement:
                                value = pBItem->GetEffectValue("Enhancement", true);
                                break;
                            case CI_Insightful:
                                value = pBItem->GetEffectValue("Insightful", true);
                                break;
                            case CI_Artifact:
                                value = pBItem->GetEffectValue("Artifact", true);
                                break;
                            case CI_Quality:
                                value = pBItem->GetEffectValue("Quality", true);
                                break;
                            case CI_Profane:
                                value = pBItem->GetEffectValue("Profane", true);
                                break;
                            case CI_Equipment:
                                value = pBItem->GetEffectValue("Equipment", true);
                                break;
                            case CI_Competence:
                                value = pBItem->GetEffectValue("Competence", true);
                                break;
                            case CI_Exceptional:
                                value = pBItem->GetEffectValue("Exceptional", true);
                                break;
                            case CI_Festive:
                                value = pBItem->GetEffectValue("Festive", true);
                                break;
                            case CI_Fortune:
                                value = pBItem->GetEffectValue("Fortune", true);
                                break;
                        }
                        // no value shown if total is 0
                        if (value != 0)
                        {
                            CString valueAsText;
                            valueAsText.Format("%+1d", (int)value);
                            m_listBonuses.SetItemText(index, i, (LPCTSTR)valueAsText);
                        }
                    }
                }
            }
        }
        m_buttonAdd.EnableWindow(TRUE);
        m_buttonRemove.EnableWindow(FALSE);
        m_buttonMoveUp.EnableWindow(FALSE);
        m_buttonMoveDown.EnableWindow(FALSE);
    }
}

void CBonusesPane::OnItemchangedBonusesList(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pResult);
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    // ignore invalid notifications
    if ((pNMListView->uChanged & LVIF_STATE) 
            && (pNMListView->uNewState & LVIS_SELECTED))
    {
        // enable the move up/down buttons based on index and item count
        int sel = pNMListView->iItem;
        int count = m_listBonuses.GetItemCount();
        m_buttonRemove.EnableWindow(true);      // can always remove this item
        m_buttonMoveUp.EnableWindow(sel > 0);   // room to move up?
        m_buttonMoveDown.EnableWindow(sel < (count - 1));   // room to move down?
    }
}
