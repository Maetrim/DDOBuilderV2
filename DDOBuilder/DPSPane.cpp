
#include "stdafx.h"

#include "DPSPane.h"
#include "GlobalSupportFunctions.h"
#include "EnhancementTree.h"
#include "AttackChainNameDialog.h"
#include "MouseHook.h"
#include "WeaponData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace
{
    enum Columns
    {
        CI_AttackName = 0,
        CI_DPSScore,
        CI_TimePoint,
        CI_Cooldown
    };
}

/////////////////////////////////////////////////////////////////////////////
// CDPSPane

CDPSPane::CDPSPane() :
    CFormView(CDPSPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL),
    m_bHadInitialUpdate(false),
    m_bIgnoreChange(true),
    m_hoverItem(-1),
    m_hoverHandle(0)
{
}

CDPSPane::~CDPSPane()
{
}

IMPLEMENT_DYNCREATE(CDPSPane, CFormView)
BEGIN_MESSAGE_MAP(CDPSPane, CFormView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, &CDPSPane::OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, &CDPSPane::OnLoadComplete)
    ON_BN_CLICKED(IDC_BUTTON_NEW, &CDPSPane::OnButtonNewAttackChain)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, &CDPSPane::OnButtonDeleteAttackChain)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CDPSPane::OnTtnNeedText)
    ON_NOTIFY(HDN_ITEMCHANGED, IDC_COMBO_ATTACKCHAINS, &CDPSPane::OnEndtrackListAttackChain)
    ON_NOTIFY(HDN_ITEMCHANGED, IDC_AVAILABLE_LIST, &CDPSPane::OnEndtrackListAvailableAttacks)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_ATTACK_LIST, &CDPSPane::OnAttackListSelectionChanged)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_AVAILABLE_LIST, &CDPSPane::OnAvailableSelectionChanged)
    ON_NOTIFY(NM_HOVER, IDC_ATTACK_LIST, &CDPSPane::OnHoverAttackList)
    ON_NOTIFY(NM_HOVER, IDC_AVAILABLE_LIST, &CDPSPane::OnHoverAvailableAttackList)
    ON_MESSAGE(WM_MOUSELEAVE, &CDPSPane::OnMouseLeave)
    ON_BN_CLICKED(IDC_BUTTON_ADD, &CDPSPane::OnAddAttack)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CDPSPane::OnRemoveAttack)
END_MESSAGE_MAP()

void CDPSPane::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    if (!m_bHadInitialUpdate)
    {
        m_tooltip.Create(this);
        m_tipCreated = true;
        m_bIgnoreChange = true;
        m_attackList.InsertColumn(0, "Attack Name", LVCFMT_LEFT, 300);
        m_attackList.InsertColumn(1, "DPS Score", LVCFMT_RIGHT, 40);
        m_attackList.InsertColumn(2, "Time Point", LVCFMT_RIGHT, 40);
        m_attackList.InsertColumn(3, "Cooldown", LVCFMT_RIGHT, 40);
        m_availableList.InsertColumn(0, "Available Attacks", LVCFMT_LEFT, 300);
        m_availableList.InsertColumn(1, "Cooldown", LVCFMT_RIGHT, 40);
        m_attackList.SetExtendedStyle(m_attackList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
        m_availableList.SetExtendedStyle(m_availableList.GetExtendedStyle()| LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
        // Images for the buttons
        m_buttonAddAttackChain.SetImage(IDB_BITMAP_ADD);
        m_buttonDeleteAttackChain.SetImage(IDB_BITMAP_REMOVE);
        m_buttonRemove.SetImage(IDB_BITMAP_REMOVE);
        m_buttonMoveUp.SetImage(IDB_BITMAP_MOVEUP);
        m_buttonMoveDown.SetImage(IDB_BITMAP_MOVEDOWN);
        m_buttonAdd.SetImage(IDB_BITMAP_ADD);
        m_bHadInitialUpdate = true;
        PopulateAttackChainCombo();
        PopulateAttackChain();
        EnableToolTips(TRUE);
        m_attackList.GetHeaderCtrl()->SetDlgCtrlID(IDC_COMBO_ATTACKCHAINS);
        m_availableList.GetHeaderCtrl()->SetDlgCtrlID(IDC_AVAILABLE_LIST);
        LoadColumnWidthsByName(&m_attackList, "AttackList_%s");
        LoadColumnWidthsByName(&m_availableList, "AvailableList_%s");
        m_bIgnoreChange = false;
    }
}

void CDPSPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_NEW, m_buttonAddAttackChain);
    DDX_Control(pDX, IDC_COMBO_ATTACKCHAINS, m_comboAttackChains);
    DDX_Control(pDX, IDC_BUTTON_DELETE, m_buttonDeleteAttackChain);
    DDX_Control(pDX, IDC_ATTACK_LIST, m_attackList);
    DDX_Control(pDX, IDC_AVAILABLE_LIST, m_availableList);
    DDX_Control(pDX, IDC_BUTTON_REMOVE, m_buttonRemove);
    DDX_Control(pDX, IDC_BUTTON_MOVEUP, m_buttonMoveUp);
    DDX_Control(pDX, IDC_BUTTON_MOVEDOWN, m_buttonMoveDown);
    DDX_Control(pDX, IDC_BUTTON_ADD, m_buttonAdd);
}

void CDPSPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_attackList.GetSafeHwnd())
            && IsWindowVisible())
    {
        // position all the windows
        // +-----------------------------------------------+
        // | [+][...............[]] [-]                    |
        // | +--------------------+ +--------------------+ |
        // | |                    | |                    | |
        // | | Attack List        | | Available List     | |
        // | |                    | |                    | |
        // | |                    | |                    | |
        // | |                    | |                    | |
        // | |                    | |                    | |
        // | |                    | |                    | |
        // | |                    | |                    | |
        // | |                    | |                    | |
        // | +--------------------+ +--------------------+ |
        // | [-][^][v]              [+]                    |
        // +-----------------------------------------------+
        CRect rctAdd;
        CRect rctCombo;
        CRect rctDelete;
        m_buttonAddAttackChain.GetWindowRect(&rctAdd);
        m_comboAttackChains.GetWindowRect(&rctCombo);
        m_buttonDeleteAttackChain.GetWindowRect(&rctDelete);
        rctAdd -= rctAdd.TopLeft();
        rctAdd += CPoint(c_controlSpacing, c_controlSpacing);
        rctCombo -= rctCombo.TopLeft();
        rctCombo += CPoint(rctAdd.right + c_controlSpacing, c_controlSpacing);
        rctDelete -= rctDelete.TopLeft();
        rctDelete += CPoint(rctCombo.right + c_controlSpacing, c_controlSpacing);
        CRect rctAttackList(c_controlSpacing, rctDelete.bottom + c_controlSpacing, (cx / 2) - c_controlSpacing, 0); // y max set later
        CRect rctAvailableList(rctAttackList.right + c_controlSpacing, rctDelete.bottom + c_controlSpacing, cx - c_controlSpacing, 0); // y max set later
        CRect rctRemove;
        CRect rctMoveUp;
        CRect rctMoveDown;
        CRect rctAdd2;
        m_buttonRemove.GetWindowRect(&rctRemove);
        m_buttonMoveUp.GetWindowRect(&rctMoveUp);
        m_buttonMoveDown.GetWindowRect(&rctMoveDown);
        m_buttonAdd.GetWindowRect(&rctAdd2);
        rctRemove -= rctRemove.TopLeft();
        rctMoveUp -= rctMoveUp.TopLeft();
        rctMoveDown -= rctMoveDown.TopLeft();
        rctAdd2 -= rctAdd2.TopLeft();
        rctRemove += CPoint(c_controlSpacing, cy - rctRemove.Height() - c_controlSpacing);
        rctMoveUp += CPoint(rctRemove.right + c_controlSpacing, cy - rctMoveUp.Height() - c_controlSpacing);
        rctMoveDown += CPoint(rctMoveUp.right + c_controlSpacing, cy - rctMoveDown.Height() - c_controlSpacing);
        rctAdd2 += CPoint(rctAvailableList.left, cy - rctAdd2.Height() - c_controlSpacing);
        rctAttackList.bottom = rctRemove.top - c_controlSpacing;
        rctAvailableList.bottom = rctRemove.top - c_controlSpacing;

        m_buttonAddAttackChain.MoveWindow(rctAdd, FALSE);
        m_comboAttackChains.MoveWindow(rctCombo, FALSE);
        m_buttonDeleteAttackChain.MoveWindow(rctDelete, FALSE);
        m_attackList.MoveWindow(rctAttackList, FALSE);
        m_availableList.MoveWindow(rctAvailableList, FALSE);
        m_buttonRemove.MoveWindow(rctRemove, FALSE);
        m_buttonMoveUp.MoveWindow(rctMoveUp, FALSE);
        m_buttonMoveDown.MoveWindow(rctMoveDown, FALSE);
        m_buttonAdd.MoveWindow(rctAdd2, FALSE);
    }
    SetScaleToFitSize(CSize(cx, cy));
    CWnd *pWnd = this;
    while (pWnd != NULL)
    {
        pWnd = pWnd->GetParent();
    }
}

BOOL CDPSPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_BUTTON_NEW,
        IDC_COMBO_ATTACKCHAINS,
        IDC_BUTTON_DELETE,
        IDC_ATTACK_LIST,
        IDC_AVAILABLE_LIST,
        IDC_BUTTON_REMOVE,
        IDC_BUTTON_MOVEUP,
        IDC_BUTTON_MOVEDOWN,
        IDC_BUTTON_ADD,
        0 // end marker
    };


    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

LRESULT CDPSPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDDOBuilderDoc* pDoc = (CDDOBuilderDoc*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        Life* pLife = m_pCharacter->ActiveLife();
        if (pLife != NULL)
        {
            pLife->AttachObserver(this);
        }
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->AttachObserver(this);
        }
    }
    return 0L;
}

LRESULT CDPSPane::OnLoadComplete(WPARAM, LPARAM)
{
    // once data load has completed, we iterate all items that could provide an "Attack"
    // object. We then pre-load and map all names vs images for these in advance
    int lastAttackCount = AfxGetApp()->GetProfileInt("ItemLoad", "LastAttackCount", 1);
    size_t imageCount = 0;
    // first iterate all the feats
    m_attackImages.Create(
        32,
        32,
        ILC_COLOR32,
        lastAttackCount,
        100);
    const std::map<std::string, Feat>& feats = StandardFeats();
    for (auto&& fit: feats)
    {
        for (auto&& ait: fit.second.Attacks())
        {
            std::string name = ait.Name();
            std::string icon = ait.Icon();
            CImage image;
            if (S_OK != LoadImageFile("DataFiles\\EnhancementImages\\", icon, &image, CSize(32, 32), false))
            {
                // see if its a feat icon we need to use
                if (S_OK != LoadImageFile("DataFiles\\FeatImages\\", icon, &image, CSize(32, 32), true))
                {
                }
            }
            CBitmap bitmap;
            bitmap.Attach(image.Detach());
            int imageIndex = m_attackImages.Add(&bitmap, c_transparentColour);
            m_imageMap[icon] = imageIndex;
            ++imageCount;
        }
    }
    // now check all enhancement tree items
    const std::list<EnhancementTree>& trees = EnhancementTrees();
    for (auto&& tit: trees)
    {
        for (auto&& etit: tit.Items())
        {
            for (auto eait: etit.Attacks())
            {
                std::string name = eait.Name();
                std::string icon = eait.Icon();
                CImage image;
                if (S_OK != LoadImageFile("DataFiles\\EnhancementImages\\", icon, &image, CSize(32, 32), false))
                {
                    // see if its a feat icon we need to use
                    if (S_OK != LoadImageFile("DataFiles\\FeatImages\\", icon, &image, CSize(32, 32), true))
                    {
                    }
                }
                CBitmap bitmap;
                bitmap.Attach(image.Detach());
                int imageIndex = m_attackImages.Add(&bitmap, c_transparentColour);
                m_imageMap[icon] = imageIndex;
                ++imageCount;
            }
            if (true == etit.HasSelections())
            {
                // check for Attack object in sub items also
                const Selector& selections = etit.Selections();
                for (auto&& stit: selections.Selections())
                {
                    for (auto&& astit : stit.Attacks())
                    {
                        std::string name = astit.Name();
                        std::string icon = astit.Icon();
                        CImage image;
                        if (S_OK != LoadImageFile("DataFiles\\EnhancementImages\\", icon, &image, CSize(32, 32), false))
                        {
                            // see if its a feat icon we need to use
                            if (S_OK != LoadImageFile("DataFiles\\FeatImages\\", icon, &image, CSize(32, 32), true))
                            {
                            }
                        }
                        CBitmap bitmap;
                        bitmap.Attach(image.Detach());
                        int imageIndex = m_attackImages.Add(&bitmap, c_transparentColour);
                        m_imageMap[icon] = imageIndex;
                        ++imageCount;
                    }
                }
            }
        }
    }
    AfxGetApp()->WriteProfileInt("ItemLoad", "LastAttackCount", imageCount);
    std::stringstream ss;
    ss << "Images loaded for DPS attacks " << imageCount;
    GetLog().AddLogEntry(ss.str().c_str());
    m_attackList.SetImageList(&m_attackImages, LVSIL_SMALL);
    m_attackList.SetImageList(&m_attackImages, LVSIL_NORMAL);
    m_availableList.SetImageList(&m_attackImages, LVSIL_SMALL);
    m_availableList.SetImageList(&m_attackImages, LVSIL_NORMAL);
    m_attackList.SetExtendedStyle(m_attackList.GetExtendedStyle()
                | LVS_EX_FULLROWSELECT
                | LVS_EX_TRACKSELECT
                | LVS_EX_ONECLICKACTIVATE
                //| LVS_EX_LABELTIP); // stop hover tooltips from working
                );
    m_availableList.SetExtendedStyle(m_availableList.GetExtendedStyle()
                | LVS_EX_FULLROWSELECT
                | LVS_EX_TRACKSELECT
                | LVS_EX_ONECLICKACTIVATE
                //| LVS_EX_LABELTIP); // stop hover tooltips from working
                );
    return 0;
}

void CDPSPane::UpdateActiveLifeChanged(Character*)
{
    Life *pLife = m_pCharacter->ActiveLife();
    Build *pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
            && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
    }
}

void CDPSPane::UpdateActiveBuildChanged(Character*)
{
    m_availableAttacksWithStacks.clear();
    if (m_pCharacter != NULL)
    {
        Life *pLife = m_pCharacter->ActiveLife();
        Build *pBuild = m_pCharacter->ActiveBuild();
        if (pLife != NULL
                && pBuild != NULL)
        {
            pLife->AttachObserver(this);
            pBuild->AttachObserver(this);
            PopulateAttackChainCombo();
            PopulateAttackChain();
        }
    }
}

void CDPSPane::UpdateNewAttack(Build* , const Attack& attack)
{
    // add a new attack or add a stack
    bool found = false;
    for (auto&& ait: m_availableAttacksWithStacks)
    {
        if (ait.Name() == attack.Name())
        {
            ait.AddStack();
            found = true;
            break;  // no need to check the rest
        }
    }
    if (!found)
    {
        m_availableAttacksWithStacks.push_back(attack);
        m_availableAttacksWithStacks.back().AddStack();
    }
    UpdateAvailableList();
}

void CDPSPane::UpdateRevokeAttack(Build* , const Attack& attack)
{
    auto it = m_availableAttacksWithStacks.begin();
    for (auto&& ait: m_availableAttacksWithStacks)
    {
        if (ait.Name() == attack.Name())
        {
            ait.RevokeStack();
            if (ait.Stacks() == 0)
            {
                // remove entry
                m_availableAttacksWithStacks.erase(it);
            }
            break;  // no need to check the rest
        }
        ++it;
    }
    UpdateAvailableList();
}

void CDPSPane::UpdateAvailableList()
{
    m_availableList.LockWindowUpdate();
    m_availableList.DeleteAllItems();
    size_t entryIndex = 0;
    for (auto&& it: m_availableAttacksWithStacks)
    {
        std::string name = it.Name();
        std::string icon = it.Icon();
        size_t iconIndex = 0;
        if (m_imageMap.find(icon) != m_imageMap.end())
        {
            iconIndex = m_imageMap[icon];   // get the index
        }
        int index = m_availableList.InsertItem(m_availableList.GetItemCount(), name.c_str(), iconIndex);
        m_availableList.SetItemData(index, entryIndex);
        ++entryIndex;
    }
    m_availableList.UnlockWindowUpdate();
}

void CDPSPane::PopulateAttackChainCombo()
{
    bool bDisableAll = true;
    m_comboAttackChains.ResetContent();
    if (m_pCharacter != NULL)
    {
        const Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            int sel = CB_ERR;
            const std::list<AttackChain>& attackChains = pBuild->AttackChains();
            for (auto&& acit: attackChains)
            {
                int index = m_comboAttackChains.AddString(acit.Name().c_str());
                if (acit.Name() == pBuild->ActiveAttackChain())
                {
                    sel = index;
                }
            }
            if (sel != CB_ERR)
            {
                m_comboAttackChains.SetCurSel(sel);
                m_buttonDeleteAttackChain.EnableWindow(TRUE);
                m_comboAttackChains.EnableWindow(TRUE);
                m_attackList.EnableWindow(TRUE);
                m_availableList.EnableWindow(TRUE);
                m_buttonRemove.EnableWindow(TRUE);
                PopulateAttackChain();
            }
            else
            {
                m_buttonDeleteAttackChain.EnableWindow(FALSE);
            }
            m_buttonMoveUp.EnableWindow(FALSE);     // enabled on selection change in control
            m_buttonMoveDown.EnableWindow(FALSE);   // enabled on selection change in control
            m_buttonAdd.EnableWindow(FALSE);        // enabled on selection change in control
            m_buttonAddAttackChain.EnableWindow(TRUE);  // always enabled if we have a build
            bDisableAll = false;
        }
    }
    if (bDisableAll)
    {
        m_buttonAddAttackChain.EnableWindow(FALSE);
        m_comboAttackChains.EnableWindow(FALSE);
        m_buttonDeleteAttackChain.EnableWindow(FALSE);
        m_attackList.EnableWindow(FALSE);
        m_availableList.EnableWindow(FALSE);
        m_buttonRemove.EnableWindow(FALSE);
        m_buttonMoveUp.EnableWindow(FALSE);
        m_buttonMoveDown.EnableWindow(FALSE);
        m_buttonAdd.EnableWindow(FALSE);
    }
}

void CDPSPane::OnButtonNewAttackChain()
{
    CAttackChainNameDialog dlg(this, m_pCharacter);
    if (dlg.DoModal() == IDOK)
    {
        AttackChain newAttackChain;
        newAttackChain.SetName(dlg.Name());
        // default to one entry "Basic Attack"
        std::list<std::string> defaultAttacks;
        defaultAttacks.push_back("Basic Attack");
        newAttackChain.SetAttacks(defaultAttacks);
        m_pCharacter->ActiveBuild()->AddAttackChain(newAttackChain);
        PopulateAttackChainCombo();
        PopulateAttackChain();
    }
}

void CDPSPane::OnButtonDeleteAttackChain()
{
    int sel = m_comboAttackChains.GetCurSel();
    if (CB_ERR != sel)
    {
        CString acName;
        m_comboAttackChains.GetLBText(sel, acName);
        m_pCharacter->ActiveBuild()->DeleteAttackChain((LPCTSTR)acName);
        PopulateAttackChainCombo();
        PopulateAttackChain();
    }
}

BOOL CDPSPane::OnTtnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(pResult);

    UINT_PTR nID = pNMHDR->idFrom;
    nID = ::GetDlgCtrlID((HWND)nID);

    switch (nID)
    {
    case IDC_BUTTON_NEW:
        m_tipText = "Create a new Attack Chain";
        break;
    case IDC_COMBO_ATTACKCHAINS:
        m_tipText = "Select the active Attack Chain";
        break;
    case IDC_BUTTON_DELETE:
        m_tipText = "Delete the active Attack Chain";
        break;
    case IDC_ATTACK_LIST:
        m_tipText = "The list of attacks in this Attack Chain";
        break;
    case IDC_AVAILABLE_LIST:
        m_tipText = "The list of attacks available for your Attack Chain";
        break;
    case IDC_BUTTON_REMOVE:
        m_tipText = "Remove the selected attack from your Attack Chain";
        break;
    case IDC_BUTTON_MOVEUP:
        m_tipText = "Move the selected attack up in the Attack Chain\n"
                    "This is disabled if doing so would violate the cooldown for this attack.";
        break;
    case IDC_BUTTON_MOVEDOWN:
        m_tipText = "Move the selected attack down in the Attack Chain\n"
                    "This is disabled if doing so would violate the cooldown for this attack.";
        break;
    case IDC_BUTTON_ADD:
        m_tipText = "Add the selected attack to the Attack Chain, under the selected item.\n"
                    "This is disabled if doing so would violate the cooldown for this attack.";
        break;
    default:
        m_tipText = "";
        break;
    }
    // ensure multi line tooltips
    ::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 800);
    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    pTTTA->lpszText = m_tipText.GetBuffer();
    return TRUE;
}

void CDPSPane::PopulateAttackChain()
{
    double baCooldown = SetBasicAttackCooldown();
    // try and preserve the selection if there is one
    int sel = m_attackList.GetSelectionMark();
    m_attackList.DeleteAllItems();
    if (m_pCharacter != NULL)
    {
        const Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            const AttackChain& ac = pBuild->GetActiveAttackChain();
            double timePoint = 0;
            CString strCooldown;
            CString strTimePoint;
            int index = 0;
            // this is the one we need to populate with
            for (auto&& acn: ac.Attacks())
            {
                // find the Attack object from the available list
                Attack attack = FindAttack(acn);
                size_t iconIndex = 0;
                if (m_imageMap.find(attack.Icon()) != m_imageMap.end())
                {
                    iconIndex = m_imageMap[attack.Icon()];   // get the index
                }
                index = m_attackList.InsertItem(m_attackList.GetItemCount(), acn.c_str(), iconIndex);
                if (attack.HasCooldown())
                {
                    strCooldown.Format("%.2f", attack.Cooldown()[attack.Stacks()-1]);
                    m_attackList.SetItemText(index, CI_Cooldown, strCooldown);
                }
                strTimePoint.Format("%.2f", timePoint);
                m_attackList.SetItemText(index, CI_TimePoint, strTimePoint);
                if (attack.HasExecutionTime())
                {
                    timePoint += attack.ExecutionTime();
                }
                else
                {
                    // if no execution time, it takes the same time as a "Basic Attack"
                    timePoint += baCooldown;
                }
            }
            // finally add an entry for the total duration of the attack chain
            index = m_attackList.InsertItem(m_attackList.GetItemCount(), "Total Attack Chain Duration", 0);
            strCooldown.Format("%.2f", timePoint);
            m_attackList.SetItemText(index, CI_Cooldown, strCooldown);
        }
        if (sel != -1)
        {
            m_attackList.SetSelectionMark(sel);
            m_attackList.SetItemState(sel, LVIS_SELECTED, LVIS_SELECTED);
            m_attackList.EnsureVisible(sel, FALSE);
        }
        CalculateAttackChainDPS();
    }
}

Attack CDPSPane::FindAttack(const std::string& name) const
{
    Attack attack("Not Found", "This Attack was not found", "Unknown");
    for (auto&& aait: m_availableAttacksWithStacks)
    {
        if (aait.Name() == name)
        {
            attack = aait;
        }
    }
    return attack;
}

void CDPSPane::OnEndtrackListAttackChain(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (!m_bIgnoreChange)
    {
        // just save the column widths to registry so restored next time we run
        UNREFERENCED_PARAMETER(pNMHDR);
        UNREFERENCED_PARAMETER(pResult);
        SaveColumnWidthsByName(&m_attackList, "AttackList_%s");
    }
}

void CDPSPane::OnEndtrackListAvailableAttacks(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (!m_bIgnoreChange)
    {
        // just save the column widths to registry so restored next time we run
        UNREFERENCED_PARAMETER(pNMHDR);
        UNREFERENCED_PARAMETER(pResult);
        SaveColumnWidthsByName(&m_availableList, "AvailableList_%s");
    }
}

double CDPSPane::SetBasicAttackCooldown()
{
    // The "Basic Attack" has a cooldown based on the attack style and BAB
    // of the build in question
    double attacksPerMinute = 100;          // TBD
    double cooldown = 60.0 / attacksPerMinute;

    for (auto&& aaIt: m_availableAttacksWithStacks)
    {
        if (aaIt.Name() == "Basic Attack")
        {
            aaIt.SetCooldown(cooldown);
        }
    }
    return cooldown;
}

void CDPSPane::OnAttackListSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    if ((pNMListView->uChanged & LVIF_STATE) 
            && (pNMListView->uNewState & LVIS_SELECTED))
    {
        // item selection has changed
        int sel = pNMListView->iItem;
        m_buttonRemove.EnableWindow(TRUE);
        BOOL enable = EvaluateMoveUp(sel);
        m_buttonMoveUp.EnableWindow(enable);
        enable = EvaluateMoveDown(sel);
        m_buttonMoveDown.EnableWindow(enable);
    }
    *pResult = 0;
}

void CDPSPane::OnAvailableSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    if ((pNMListView->uChanged & LVIF_STATE) 
            && (pNMListView->uNewState & LVIS_SELECTED))
    {
        // item selection has changed
        m_buttonAdd.EnableWindow(TRUE);
    }
    *pResult = 0;
}

BOOL CDPSPane::EvaluateMoveUp(size_t sel)
{
    return TRUE; // TBD
}

BOOL CDPSPane::EvaluateMoveDown(size_t sel)
{
    return TRUE;    //TBD
}

void CDPSPane::ShowTip(const Attack& attack, CRect itemRect)
{
    if (m_showingTip)
    {
        m_tooltip.Hide();
    }
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    CPoint tipAlternate(itemRect.left, itemRect.top - 2);
    SetTooltipText(attack, tipTopLeft, tipAlternate);
    m_showingTip = true;
}

void CDPSPane::HideTip()
{
    // tip not shown if not over a gear slot
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
    }
}

void CDPSPane::SetTooltipText(
        const Attack& attack,
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
    m_tooltip.SetAttack(attack);
    m_tooltip.Show();
    m_showingTip = true;
}

void CDPSPane::OnHoverAttackList(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
    // the user it hovering over a list control item. Identify it and display
    // the Attack tooltip for this item
    CPoint mousePosition;
    GetCursorPos(&mousePosition);
    m_attackList.ScreenToClient(&mousePosition);

    LVHITTESTINFO hitInfo;
    hitInfo.pt = mousePosition;
    if (m_attackList.SubItemHitTest(&hitInfo) >= 0)
    {
        if (m_hoverItem != hitInfo.iItem)
        {
            // the item under the hover has changed
            m_hoverItem = hitInfo.iItem;
            // mouse is over a valid item, get the items rectangle and
            // show the Attack tooltip
            CRect rect;
            m_attackList.GetItemRect(hitInfo.iItem, &rect, LVIR_BOUNDS);
            HideTip();
            // get the item index
            size_t itemIndex = m_attackList.GetItemData(hitInfo.iItem);
            m_attackList.ClientToScreen(&rect);
            CPoint tipTopLeft(rect.left, rect.bottom);
            CPoint tipAlternate(rect.left, rect.top);
            Build* pBuild = m_pCharacter->ActiveBuild();
            const AttackChain& ac = pBuild->GetActiveAttackChain();
            std::list<std::string>::const_iterator it = ac.Attacks().begin();
            std::advance(it, itemIndex);
            const Attack& attack = FindAttack((*it));
            SetTooltipText(attack, tipTopLeft, tipAlternate);
            m_showingTip = true;
            // make sure we don't stack multiple monitoring of the same rectangle
            if (m_hoverHandle == 0)
            {
                m_hoverHandle = GetMouseHook()->AddRectangleToMonitor(
                        this->GetSafeHwnd(),
                        rect,           // screen coordinates,
                        WM_MOUSEENTER,
                        WM_MOUSELEAVE,
                        true);
            }
        }
    }
    *pResult = 1;   // stop the hover selecting the item
}

void CDPSPane::OnHoverAvailableAttackList(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
    // the user it hovering over a list control item. Identify it and display
    // the Attack tooltip for this item
    CPoint mousePosition;
    GetCursorPos(&mousePosition);
    m_availableList.ScreenToClient(&mousePosition);

    LVHITTESTINFO hitInfo;
    hitInfo.pt = mousePosition;
    if (m_availableList.SubItemHitTest(&hitInfo) >= 0)
    {
        if (m_hoverItem != hitInfo.iItem)
        {
            // the item under the hover has changed
            m_hoverItem = hitInfo.iItem;
            // mouse is over a valid item, get the items rectangle and
            // show the Attack tooltip
            CRect rect;
            m_availableList.GetItemRect(hitInfo.iItem, &rect, LVIR_BOUNDS);
            HideTip();
            // get the item index
            size_t itemIndex = m_availableList.GetItemData(hitInfo.iItem);
            m_availableList.ClientToScreen(&rect);
            CPoint tipTopLeft(rect.left, rect.bottom);
            CPoint tipAlternate(rect.left, rect.top);
            std::list<Attack>::const_iterator it = m_availableAttacksWithStacks.begin();
            std::advance(it, itemIndex);
            SetTooltipText((*it), tipTopLeft, tipAlternate);
            m_showingTip = true;
            // make sure we don't stack multiple monitoring of the same rectangle
            if (m_hoverHandle == 0)
            {
                m_hoverHandle = GetMouseHook()->AddRectangleToMonitor(
                        this->GetSafeHwnd(),
                        rect,           // screen coordinates,
                        WM_MOUSEENTER,
                        WM_MOUSELEAVE,
                        true);
            }
        }
    }
    *pResult = 1;   // stop the hover selecting the item
}

LRESULT CDPSPane::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    // hide any tooltip when the mouse leaves the area its being shown for
    HideTip();
    if (wParam == m_hoverHandle)
    {
        // no longer monitoring this location
        GetMouseHook()->DeleteRectangleToMonitor(m_hoverHandle);
        m_hoverHandle = 0;
        m_hoverItem = -1;
    }
    return 0;
}

void CDPSPane::OnAddAttack()
{
    int selAttack = m_availableList.GetSelectionMark();
    int insertLoc = m_attackList.GetSelectionMark();
    if (insertLoc == -1)
    {
        // insert at he end of the list if no selection
        insertLoc = m_attackList.GetItemCount() - 1; // -1 due to total attack chain line
    }
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AttackChain ac = pBuild->GetActiveAttackChain();
        std::list<Attack>::const_iterator it = m_availableAttacksWithStacks.begin();
        std::advance(it, selAttack);
        std::string attackToAdd = (*it).Name();
        ac.AddAttack(attackToAdd, insertLoc);
        pBuild->UpdateAttackChain(pBuild->ActiveAttackChain(), ac);
        PopulateAttackChain();
    }
}

void CDPSPane::OnRemoveAttack()
{
    int delLoc = m_attackList.GetSelectionMark();
    if (delLoc >=0)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            AttackChain ac = pBuild->GetActiveAttackChain();
            ac.RemoveAttackAt(delLoc);
            pBuild->UpdateAttackChain(pBuild->ActiveAttackChain(), ac);
            PopulateAttackChain();
        }
    }
}

void CDPSPane::CalculateAttackChainDPS()
{
    if (m_pCharacter != NULL)
    {
        int ai = 0;
        CString text;
        double totalDPS = 0.0;
        const Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // extract basic weapon data from breakdowns
            WeaponData mainHand(Inventory_Weapon1);
            WeaponData offHand(Inventory_Weapon2);
            // determine the type of attacks of this attack chain
            AttackType at = AT_Unknown;
            if (pBuild->IsStanceActive("Two Weapon Fighting"))
            {
                at = AT_TWF;
            }
            else if (pBuild->IsStanceActive("Two Handed Fighting"))
            {
                at = AT_THF;
            }
            else if (pBuild->IsStanceActive("Single Weapon Fighting"))
            {
                at = AT_SWF;
            }
            else if (pBuild->IsStanceActive("Ranged Combat"))
            {
                at = AT_Ranged;
            }
            else if (pBuild->IsStanceActive("Unarmed"))
            {
                at = AT_Handwraps;
            }
            else if (pBuild->IsStanceActive("Sword and Board"))
            {
                at = AT_SwordAndBoard;
            }
            std::list<AttackBuff> buffs;
            const AttackChain& ac = pBuild->GetActiveAttackChain();
            double timePoint = 0.0;
            for (auto&& acit: ac.Attacks())
            {
                Attack attack = FindAttack(acit);
                double attackDPS = EvaluateAttack(at, attack, buffs, timePoint);
                text.Format("%d", (int)attackDPS);
                m_attackList.SetItemText(ai, CI_DPSScore, text);
                totalDPS += attackDPS;
                timePoint += attack.ExecutionTime();
                DropTimedOutBuffs(&buffs, timePoint);
                ai++;
            }
            text.Format("%d", (int)totalDPS);
            m_attackList.SetItemText(m_attackList.GetItemCount()-1, CI_DPSScore, text);
        }
    }
}

double CDPSPane::EvaluateAttack(
    AttackType at,
    const Attack& attack,
    const std::list<AttackBuff>& buffs,
    double timePoint)
{
    double dps = 0.0;
    switch (at)
    {
        case AT_Unknown:        break;
        case AT_TWF:            dps = EvaluateTWF(attack, buffs, timePoint); break;
        case AT_THF:            dps = EvaluateTHF(attack, buffs, timePoint); break;
        case AT_SWF:            dps = EvaluateSWF(attack, buffs, timePoint); break;
        case AT_Ranged:         dps = EvaluateRanged(attack, buffs, timePoint); break;
        case AT_Handwraps:      dps = EvaluateHandwraps(attack, buffs, timePoint); break;
        case AT_SwordAndBoard:  dps = EvaluateSwordAndBoard(attack, buffs, timePoint); break;
    }
    return dps;
}

double CDPSPane::EvaluateTWF(
    const Attack& attack,
    const std::list<AttackBuff>& buffs,
    double timePoint)
{
    double dps = 0.0;
    return dps;
}

double CDPSPane::EvaluateTHF(
    const Attack& attack,
    const std::list<AttackBuff>& buffs,
    double timePoint)
{
    double dps = 0.0;
    return dps;
}

double CDPSPane::EvaluateSWF(
    const Attack& attack,
    const std::list<AttackBuff>& buffs,
    double timePoint)
{
    double dps = 0.0;
    return dps;
}

double CDPSPane::EvaluateRanged(
    const Attack& attack,
    const std::list<AttackBuff>& buffs,
    double timePoint)
{
    double dps = 0.0;
    return dps;
}

double CDPSPane::EvaluateHandwraps(
    const Attack& attack,
    const std::list<AttackBuff>& buffs,
    double timePoint)
{
    double dps = 0.0;
    return dps;
}

double CDPSPane::EvaluateSwordAndBoard(
    const Attack& attack,
    const std::list<AttackBuff>& buffs,
    double timePoint)
{
    double dps = 0.0;
    return dps;
}

void CDPSPane::DropTimedOutBuffs(
    std::list<AttackBuff> * buffs,
    double timePoint)
{
    std::list<AttackBuff>::iterator it = buffs->begin();
    while (it != buffs->end())
    {
        if ((*it).Expired(timePoint))
        {
            it = buffs->erase(it);
        }
        else
        {
            ++it;
        }
    }
}
