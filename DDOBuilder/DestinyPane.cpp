// DestinyPane.cpp
//
#include "stdafx.h"
#include "DestinyPane.h"

#include "Build.h"
#include "Character.h"
#include "Feat.h"
#include "GlobalSupportFunctions.h"

#include "EnhancementTreeDialog.h"

namespace
{
    // destiny window size
    const size_t c_sizeX = 300;
    const size_t c_sizeY = 466;
}

IMPLEMENT_DYNCREATE(CDestinyPane, CFormView)

CDestinyPane::CDestinyPane() :
    CFormView(CDestinyPane::IDD),
    m_pDoc(NULL),
    m_pCharacter(NULL),
    m_numDestinyButtons(0),
    m_tipCreated(false),
    m_showingTip(false),
    m_pTooltipItem(NULL),
    m_bHadinitialUpdate(false)
{
}

CDestinyPane::~CDestinyPane()
{
    for (size_t vi = 0; vi < m_treeViews.size(); ++vi)
    {
        delete m_treeViews[vi];
    }
}

void CDestinyPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TREE_SELECT1, m_comboTreeSelect[0]);
    DDX_Control(pDX, IDC_TREE_SELECT2, m_comboTreeSelect[1]);
    DDX_Control(pDX, IDC_TREE_SELECT3, m_comboTreeSelect[2]);
    DDX_Control(pDX, IDC_TREE_SELECT4, m_comboTreeSelect[3]);
    DDX_Control(pDX, IDC_STATIC_PREVIEW, m_staticPreview);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE1, m_destinyTrees[0]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE2, m_destinyTrees[1]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE3, m_destinyTrees[2]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE4, m_destinyTrees[3]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE5, m_destinyTrees[4]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE6, m_destinyTrees[5]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE7, m_destinyTrees[6]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE8, m_destinyTrees[7]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE9, m_destinyTrees[8]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE10, m_destinyTrees[9]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE11, m_destinyTrees[10]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE12, m_destinyTrees[11]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE13, m_destinyTrees[12]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE14, m_destinyTrees[13]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE15, m_destinyTrees[14]);
    DDX_Control(pDX, IDC_BUTTON_DESTINY_TREE16, m_destinyTrees[15]);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CDestinyPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_REGISTERED_MESSAGE(UWM_UPDATE_TREES, OnUpdateTrees)
    ON_CONTROL_RANGE(CBN_SELENDOK, IDC_TREE_SELECT1, IDC_TREE_SELECT4, OnTreeSelect)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_DESTINY_TREE1, IDC_BUTTON_DESTINY_TREE16, OnClaimTree)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()
#pragma warning(pop)

void CDestinyPane::OnInitialUpdate()
{
    if (!m_bHadinitialUpdate)
    {
        m_bHadinitialUpdate = true;
        CFormView::OnInitialUpdate();
        m_tooltip.Create(this);
        m_tipCreated = true;
    }
}

LRESULT CDestinyPane::OnLoadComplete(WPARAM, LPARAM)
{
    UpdateWindowTitle();
    AddCustomButtons();

    // cause a resize after all sub windows have updated
    CRect rect;
    GetClientRect(&rect);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rect.Width(), rect.Height()));
    return 0;
}

void CDestinyPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (m_treeViews.size() > 0
            && IsWindow(m_treeViews[0]->GetSafeHwnd()))
    {
        // we can position and show all the visible enhancement windows
        // with scrolls bars as required

        // default location of first enhancement tree
        CRect itemRect(
                c_controlSpacing,
                c_controlSpacing,
                c_sizeX + c_controlSpacing,
                c_sizeY + c_controlSpacing);
        int scrollX = GetScrollPos(SB_HORZ);
        int scrollY = GetScrollPos(SB_VERT);
        itemRect -= CPoint(scrollX, scrollY);

        CRect rctStaticPreview;
        m_staticPreview.GetWindowRect(&rctStaticPreview);
        rctStaticPreview -= rctStaticPreview.TopLeft();

        ASSERT(m_treeViews.size() == MST_Number);
        for (size_t ti = 0; ti < m_visibleTrees.size(); ++ti)
        {
            size_t index = m_visibleTrees[ti];
            ASSERT(index >= 0 && index < MST_Number);
            // move the window to the correct location
            m_treeViews[index]->MoveWindow(itemRect, false);
            m_treeViews[index]->ShowWindow(SW_SHOW);        // ensure visible
            CRect rctCombo(itemRect.left, itemRect.bottom, itemRect.right, itemRect.bottom + 300);
            m_comboTreeSelect[ti].MoveWindow(rctCombo);
            m_comboTreeSelect[ti].ShowWindow(SW_SHOW);
            // if this is the preview tree, place the preview text under the combo box
            if (ti == MST_Number - 1)
            {
                rctStaticPreview += rctCombo.TopLeft();
                rctStaticPreview += CPoint(0, rctStaticPreview.Height() + c_controlSpacing);
                rctStaticPreview.right = rctCombo.right;
                m_staticPreview.MoveWindow(rctStaticPreview);
            }
            // now move the rectangle to the next tree location
            itemRect += CPoint(itemRect.Width() + c_controlSpacing, 0);
        }
        // position the destiny claimed buttons under the trees and combo boxes
        CRect rect(0, 0, 36, 36);
        rect -= CPoint(scrollX, 0);   // take scroll location into account (scrollY from itemRect)
        rect += CPoint(c_controlSpacing, itemRect.bottom + c_controlSpacing + GetSystemMetrics(SM_CYVSCROLL) + c_controlSpacing);
        for (size_t i = 0; i < m_numDestinyButtons; ++i)
        {
            m_destinyTrees[i].MoveWindow(rect, true);
            m_destinyTrees[i].ShowWindow(SW_SHOW);
            rect += CPoint(rect.Width() + c_controlSpacing, 0);
        }
        SetScrollSizes(
                MM_TEXT,
                CSize(
                        itemRect.left + scrollX,
                        rect.bottom + scrollY + c_controlSpacing));
    }
}

LRESULT CDestinyPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDDOBuilderDoc* pDoc = (CDDOBuilderDoc*)(wParam);
    m_pDoc = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);

    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        // trees definitely change if the character has changed
        m_availableTrees = DetermineTrees();
        DestroyEnhancementWindows();
        CreateEnhancementWindows();
    }
    else
    {
        DestroyEnhancementWindows();
    }
    EnableDisableComboboxes();
    UpdateWindowTitle();
    return 0L;
}

BOOL CDestinyPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_TREE_SELECT1,
        IDC_TREE_SELECT2,
        IDC_TREE_SELECT3,
        IDC_TREE_SELECT4,
        IDC_STATIC_PREVIEW,
        IDC_BUTTON_DESTINY_TREE1,
        IDC_BUTTON_DESTINY_TREE2,
        IDC_BUTTON_DESTINY_TREE3,
        IDC_BUTTON_DESTINY_TREE4,
        IDC_BUTTON_DESTINY_TREE5,
        IDC_BUTTON_DESTINY_TREE6,
        IDC_BUTTON_DESTINY_TREE7,
        IDC_BUTTON_DESTINY_TREE8,
        IDC_BUTTON_DESTINY_TREE9,
        IDC_BUTTON_DESTINY_TREE10,
        IDC_BUTTON_DESTINY_TREE11,
        IDC_BUTTON_DESTINY_TREE12,
        IDC_BUTTON_DESTINY_TREE13,
        IDC_BUTTON_DESTINY_TREE14,
        IDC_BUTTON_DESTINY_TREE15,
        IDC_BUTTON_DESTINY_TREE16,
        0 // end marker
    };

    // exclude any visible enhancement view windows
    for (size_t i = 0; i < m_treeViews.size(); ++i)
    {
        if (::IsWindow(m_treeViews[i]->GetSafeHwnd())
                && ::IsWindowVisible(m_treeViews[i]->GetSafeHwnd()))
        {
            CRect rctWnd;
            m_treeViews[i]->GetWindowRect(&rctWnd);
            ScreenToClient(&rctWnd);
            pDC->ExcludeClipRect(&rctWnd);
        }
    }

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CDestinyPane::UpdateActiveLifeChanged(Character*)
{
    Life *pLife = m_pCharacter->ActiveLife();
    Build *pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
    }
    m_availableTrees = DetermineTrees();
    DestroyEnhancementWindows();
    CreateEnhancementWindows();
    EnableDisableComboboxes();
    UpdateWindowTitle();
    // ensure we know about destiny point changes
    BreakdownItem* pBD = FindBreakdown(Breakdown_DestinyPoints);
    if (pBD != NULL)
    {
        pBD->AttachObserver(this); // need to know about changes
    }
}

void CDestinyPane::UpdateActiveBuildChanged(Character*)
{
    Life *pLife = m_pCharacter->ActiveLife();
    Build *pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
    }
    m_availableTrees = DetermineTrees();
    DestroyEnhancementWindows();
    CreateEnhancementWindows();
    EnableDisableComboboxes();
    UpdateWindowTitle();
}

std::list<EnhancementTree> CDestinyPane::DetermineTrees()
{
    std::list<EnhancementTree> trees;
    Build *pBuild = m_pCharacter->ActiveBuild();
    // see which destiny trees are available
    if (pBuild != NULL)
    {
        // just find all the available epic destiny trees
        const std::list<EnhancementTree> & allTrees = EnhancementTrees();
        std::list<EnhancementTree>::const_iterator it = allTrees.begin();
        while (it != allTrees.end())
        {
            if ((*it).HasIsEpicDestiny()
                    && (*it).MeetRequirements(*pBuild, pBuild->Level()-1)  // 0 based for level needed
                    )
            {
                // yes this is one of our tree's add it
                trees.push_back((*it));
            }
            ++it;
        }

        // first remove any trees present which are no longer compatible
        // we also revoke those trees AP spends if required
        Destiny_SelectedTrees selTrees = pBuild->DestinySelectedTrees(); // take a copy
        for (size_t ti = 0; ti < MST_Number; ++ti)
        {
            std::string treeName = selTrees.Tree(ti);
            if (!Destiny_SelectedTrees::IsNotSelected(treeName))
            {
                // we have a tree selected here, is it in the new list of trees available?
                bool found = false;
                std::list<EnhancementTree>::iterator tit = trees.begin();
                while (!found && tit != trees.end())
                {
                    if ((*tit).Name() == treeName)
                    {
                        // ok, it's still in the list
                        found = true;
                    }
                    ++tit;
                }
                if (!found)
                {
                    // the tree is no longer valid for this race/class setup
                    // revoke any points spent in it
                    if (pBuild->APSpentInTree(treeName) > 0)
                    {
                        // no user confirmation for this as they have already changed
                        // the base requirement that included the tree. All
                        // APs spent in this tree have to be returned to the pool of
                        // those available.
                        pBuild->Destiny_ResetEnhancementTree(treeName);   // revokes any trained enhancements also
                    }
                    // now remove it from the selected list
                    selTrees.SetNotSelected(ti);
                }
            }
        }

        // now that we have the tree list, assign them to unused tree selections
        // if there are any left
        std::list<EnhancementTree>::iterator tit = trees.begin();
        while (tit != trees.end())
        {
            if (!selTrees.IsTreePresent((*tit).Name()))
            {
                // tree not present, see if it can be assigned
                for (size_t ti = 0; ti < MST_Number; ++ti)
                {
                    std::string treeName = selTrees.Tree(ti);
                    if (Destiny_SelectedTrees::IsNotSelected(treeName)
                            || ((ti == 0) && treeName == c_noTreeSelection))    // special case for racial tree
                    {
                        // no assignment yet for this tree, assign this tree to it
                        selTrees.SetTree(ti, (*tit).Name());
                        break;  // done
                    }
                }
            }
            // try the next tree
            ++tit;
        }
        pBuild->Destiny_SetSelectedTrees(selTrees);
    }
    return trees;
}

void CDestinyPane::CreateEnhancementWindows()
{
    LockWindowUpdate();
    // all enhancement windows are a set size and max of MST_Number available to the user
    CRect itemRect(
        c_controlSpacing,
        c_controlSpacing,
        c_sizeX + c_controlSpacing,
        c_sizeY + c_controlSpacing);
    // show the trees selected by the user
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        const Destiny_SelectedTrees& selTrees = pBuild->DestinySelectedTrees(); // take a copy
        for (size_t i = 0; i < MST_Number; ++i)
        {
            std::string treeName = selTrees.Tree(i);
            if (!Destiny_SelectedTrees::IsNotSelected(treeName))
            {
                // we have a selected tree here
                PopulateTreeCombo(&m_comboTreeSelect[i], treeName);
                // create the destiny dialog
                TreeType tt = (i != MST_Number - 1) ? TT_epicDestiny : TT_preview;
                // show an enhancement dialog
                CEnhancementTreeDialog* dlg = new CEnhancementTreeDialog(
                        this,
                        m_pCharacter,
                        EnhancementTree::GetTree(treeName),
                        tt);
                dlg->Create(CEnhancementTreeDialog::IDD, this);
                dlg->MoveWindow(&itemRect);
                dlg->ShowWindow(SW_SHOW);
                m_treeViews.push_back(dlg);
            }
            else
            {
                // show a blank tree and populate the selection combo
                PopulateTreeCombo(&m_comboTreeSelect[i], "");
                // create the blank tree dialog
                CEnhancementTreeDialog* dlg = new CEnhancementTreeDialog(
                        this,
                        m_pCharacter,
                        EnhancementTree::GetTree(c_noTreeSelection),
                        (i != MST_Number - 1) ? TT_epicDestiny : TT_preview);
                dlg->Create(CEnhancementTreeDialog::IDD, this);
                dlg->MoveWindow(&itemRect);
                dlg->ShowWindow(SW_SHOW);
                m_treeViews.push_back(dlg);
            }
            // TBD this is just so they all get shown for now
            m_visibleTrees.push_back(m_visibleTrees.size());
        }
        UnlockWindowUpdate();
        // reposition and show the windows (handled in OnSize)
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}

void CDestinyPane::DestroyEnhancementWindows()
{
    LockWindowUpdate();
    // the user has changed either a race or class and the available enhancement trees
    // has changed
    for (size_t i = 0; i < m_treeViews.size(); ++i)
    {
        m_treeViews[i]->DestroyWindow();
        delete m_treeViews[i];
        m_treeViews[i] = NULL;
    }
    m_treeViews.clear();
    m_visibleTrees.clear();
    UnlockWindowUpdate();
}

void CDestinyPane::UpdateEnhancementWindows()
{
    LockWindowUpdate();
    // show the trees selected by the user
    const Destiny_SelectedTrees & selTrees = m_pCharacter->ActiveBuild()->DestinySelectedTrees(); // take a copy
    for (size_t i = 0; i < MST_Number; ++i)
    {
        // if the tree name in this location is different update the window
        std::string treeName = selTrees.Tree(i);
        CEnhancementTreeDialog * pDlg = m_treeViews[i];
        if (pDlg != NULL)
        {
            if (pDlg->CurrentTree() != treeName)
            {
                pDlg->ChangeTree(GetEnhancementTree(treeName));
                if (i == MST_Number - 1)
                {
                    // this is the preview tree, change the tree type so items from
                    // it cannot be changed, or the tree can not be dragged dropped from/too
                    pDlg->SetTreeType(TT_preview);
                }
            }
            // always update the list of available trees
            PopulateTreeCombo(&m_comboTreeSelect[i], treeName);
        }
    }
    UnlockWindowUpdate();
    // reposition and show the windows (handled in OnSize)
    CRect rctWnd;
    GetClientRect(&rctWnd);
    OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
}

void CDestinyPane::PopulateTreeCombo(
        CComboBox * combo,
        const std::string& selectedTree)
{
    combo->LockWindowUpdate();
    combo->ResetContent();
    // always add a "No selection" item
    combo->AddString(c_noTreeSelection.c_str());
    // now add any trees which are not already selected
    int sel = 0;        // assume "No selection"
    Build* pBuild = m_pCharacter->ActiveBuild();
    const Destiny_SelectedTrees & selTrees = pBuild->DestinySelectedTrees();
    std::list<EnhancementTree>::iterator tit = m_availableTrees.begin();
    while (tit != m_availableTrees.end())
    {
        if (!selTrees.IsTreePresent((*tit).Name())
                || selectedTree == (*tit).Name())       // include the name of the selected tree in the combo
        {
            // tree not present, list it in the control
            int index = combo->AddString((*tit).Name().c_str());
            if ((*tit).Name() == selectedTree)
            {
                sel = index;        // this is the item that should be selected
            }
        }
        // Move to the next tree
        ++tit;
    }
    // select the active tree
    combo->SetCurSel(sel);
    // disable the control is any points are spent in this tree
    if (pBuild->APSpentInTree(selectedTree) > 0)
    {
        // can't change trees if any points spent in the tree
        combo->EnableWindow(FALSE);
    }
    else
    {
        // if no points spent or "No selection" then enable the control
        combo->EnableWindow(TRUE);
    }
    // done
    combo->UnlockWindowUpdate();
}

// LifeObserver overrides
// BuildObserver overrides
void CDestinyPane::UpdateEnhancementTrained(
    Build*,
    const EnhancementItemParams&)
{
    UpdateWindowTitle();
}

void CDestinyPane::UpdateEnhancementRevoked(
    Build*,
    const EnhancementItemParams&)
{
    UpdateWindowTitle();
}

void CDestinyPane::UpdateFeatEffectApplied(
    Build*,
    const Effect & effect)
{
    if (effect.IsType(Effect_EnhancementTree))
    {
        // do a delayed tree update as we do not want a current tree object
        // to be destroyed while a const reference to it is being used during
        // the notify
        PostMessage(UWM_UPDATE_TREES, 0, 0L);
    }
}

void CDestinyPane::UpdateFeatEffectRevoked(
    Build*,
    const Effect& effect)
{
    if (effect.IsType(Effect_EnhancementTree))
    {
        // do a delayed tree update as we do not want a current tree object
        // to be destroyed while a const reference to it is being used during
        // the notify
        PostMessage(UWM_UPDATE_TREES, 0, 0L);
    }
}

void CDestinyPane::UpdateEnhancementEffectApplied(
    Build *,
    const Effect & effect)
{
    if (effect.IsType(Effect_EnhancementTree))
    {
        // do a delayed tree update as we do not want a current tree object
        // to be destroyed while a const reference to it is being used during
        // the notify
        PostMessage(UWM_UPDATE_TREES, 0, 0L);
    }
}

void CDestinyPane::UpdateEnhancementEffectRevoked(
    Build*,
    const Effect & effect)
{
    if (effect.IsType(Effect_EnhancementTree))
    {
        // do a delayed tree update as we do not want a current tree object
        // to be destroyed while a const reference to it is being used during
        // the notify
        PostMessage(UWM_UPDATE_TREES, 0, 0L);
    }
}

void CDestinyPane::UpdateEnhancementTreeOrderChanged(Build*)
{
    DestroyEnhancementWindows();
    CreateEnhancementWindows();
}

LRESULT CDestinyPane::OnUpdateTrees(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    // received a delayed tree update message, do the work
    UpdateTrees();
    UpdateWindowTitle();
    EnableDisableComboboxes();
    return 0;
}

void CDestinyPane::UpdateTrees()
{
    // only update if there is a change in actual trees
    std::list<EnhancementTree> trees = DetermineTrees();
    if (trees != m_availableTrees)
    {
        // yup, they have changed
        m_availableTrees = trees;
        UpdateEnhancementWindows();
    }
}

void CDestinyPane::UpdateActionPointsChanged(Life*)
{
    UpdateWindowTitle();
}

void CDestinyPane::UpdateWindowTitle()
{
    CString title("Epic Destinies");
    if (m_pCharacter != NULL)
    {
        Life* pLife = m_pCharacter->ActiveLife();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL
            && pLife != NULL)
        {
            title.Format("Epic Destinies - %d points available to spend",
                    pBuild->AvailableActionPoints(pBuild->Level(), TT_epicDestiny));
            BreakdownItem* pBD = FindBreakdown(Breakdown_DestinyPoints);
            if (pBD != NULL)
            {
                int destinyPoints = static_cast<int>(pBD->Total());
                CString additional;
                additional.Format(", Permanent DP %d", destinyPoints);
                title += additional;
            }
        }
    }
    GetParent()->SetWindowText(title);
    EnableDisableComboboxes();
}

void CDestinyPane::OnTreeSelect(UINT nID)
{
    // the user has selected a new destiny tree from a drop list under one of
    // the destiny view windows. Switch the selected tree to the one wanted
    size_t treeIndex = nID - IDC_TREE_SELECT1;
    ASSERT(treeIndex > 0 && treeIndex < MST_Number);
    int sel = m_comboTreeSelect[treeIndex].GetCurSel();
    if (sel != CB_ERR)
    {
        CString entry;
        m_comboTreeSelect[treeIndex].GetLBText(sel, entry);
        std::string treeName = (LPCSTR)entry;
        // select the tree wanted!
        Build* pBuild = m_pCharacter->ActiveBuild();
        Destiny_SelectedTrees selTrees = pBuild->DestinySelectedTrees(); // take a copy
        selTrees.SetTree(treeIndex, treeName);      // modify
        pBuild->Destiny_SetSelectedTrees(selTrees);   // update
        // update our state
        UpdateEnhancementWindows();
    }
}

void CDestinyPane::EnableDisableComboboxes()
{
    bool bDisable = true;
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            const Destiny_SelectedTrees& selTrees = pBuild->DestinySelectedTrees();
            for (size_t i = 0; i < MST_Number; ++i)
            {
                std::string treeName = selTrees.Tree(i);
                // can only select a different tree if no points spent in this one
                bool enable = (pBuild->APSpentInTree(treeName) == 0);
                m_comboTreeSelect[i].EnableWindow(enable);
            }
            // also set the states of the universal tree buttons
            for (size_t i = 0; i < m_numDestinyButtons; ++i)
            {
                m_destinyTrees[i].EnableWindow(true);
                m_destinyTrees[i].ShowWindow(SW_SHOW);
                // show the correct checked state
                CString name;
                m_destinyTrees[i].GetWindowText(name);
                bool trained = (pBuild->GetSpecialFeatTrainedCount((LPCTSTR)name) > 0);
                m_destinyTrees[i].SetSelected(trained);
            }
            m_staticPreview.ShowWindow(SW_SHOW);
            bDisable = false;
        }
    }
    if (bDisable)
    {
        // all combo boxes should be hidden and disabled
        for (size_t i = 0; i < MST_Number; ++i)
        {
            m_comboTreeSelect[i].EnableWindow(false);
            m_comboTreeSelect[i].ShowWindow(SW_HIDE);
        }
        // also set the states of the universal tree buttons
        for (size_t i = 0; i < m_numDestinyButtons; ++i)
        {
            m_destinyTrees[i].EnableWindow(false);
            m_destinyTrees[i].ShowWindow(SW_HIDE);
        }
        m_staticPreview.ShowWindow(SW_HIDE);
        Invalidate(TRUE);
        GetParent()->Invalidate(TRUE);
    }
}

void CDestinyPane::AddCustomButtons()
{
    // add buttons under the tree to allow quick access to the
    // destiny enhancement trees
    const std::list<Feat> & destinyTreeFeats = DestinyTreeFeats();
    std::list<Feat>::const_iterator it = destinyTreeFeats.begin();
    size_t buttonIndex = 0;
    while (it != destinyTreeFeats.end())
    {
        m_destinyTrees[buttonIndex].Initialise(
                (*it).Name().c_str(),
                "DataFiles\\UIImages\\",
                (*it).Icon());
        ++buttonIndex;
        ++m_numDestinyButtons;
        ++it;
    }
}

void CDestinyPane::OnClaimTree(UINT nID)
{
    UINT id = nID - IDC_BUTTON_DESTINY_TREE1;
    CString name;
    m_destinyTrees[id].GetWindowText(name);
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        bool trained = (pBuild->GetSpecialFeatTrainedCount((LPCTSTR)name) > 0);
        if (trained)
        {
            pBuild->RevokeSpecialFeat((LPCTSTR)name);
        }
        else
        {
            pBuild->TrainSpecialFeat((LPCTSTR)name);
        }
        m_destinyTrees[id].SetSelected(!trained);
        // ensure displayed trees are shown
        std::list<EnhancementTree> trees = DetermineTrees();
        if (trees != m_availableTrees)
        {
            // yup, they have changed
            m_availableTrees = trees;
            UpdateEnhancementWindows();
        }
        UpdateWindowTitle();
    }
}

void CDestinyPane::OnLButtonDown(UINT, CPoint point)
{
    // determine which universal tree selector the mouse may be over
    CWnd* pWnd = ChildWindowFromPoint(point);
    CIconButton* pIconButton = dynamic_cast<CIconButton*>(pWnd);
    if (pIconButton != NULL)
    {
        OnClaimTree(pIconButton->GetDlgCtrlID());
    }
}

void CDestinyPane::OnMouseMove(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    // determine which universal tree selector the mouse may be over
    CWnd * pWnd = ChildWindowFromPoint(point);
    CIconButton * pIconButton = dynamic_cast<CIconButton*>(pWnd);
    if (m_pCharacter == NULL)
    {
        // ensure buttons do not produce tooltips while no character active
        pIconButton = NULL;
    }
    if (pIconButton != NULL
            && pIconButton != m_pTooltipItem)
    {
        CRect itemRect;
        pIconButton->GetWindowRect(&itemRect);
        ScreenToClient(itemRect);
        // over a new item or a different item
        m_pTooltipItem = pIconButton;
        ShowTip(*pIconButton, itemRect);
    }
    else
    {
        if (m_showingTip
                && pIconButton != m_pTooltipItem)
        {
            // no longer over the same item
            HideTip();
        }
    }
}

LRESULT CDestinyPane::OnMouseLeave(WPARAM, LPARAM)
{
    // hide any tooltip when the mouse leaves the area its being shown for
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    CWnd* pWnd = ChildWindowFromPoint(point);
    CIconButton* pIconButton = dynamic_cast<CIconButton*>(pWnd);
    if (pIconButton == NULL)
    {
        if (m_showingTip)
        {
            HideTip();
        }
    }
    else
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = 1;
        _TrackMouseEvent(&tme);
    }
    return 0;
}

void CDestinyPane::ShowTip(const CIconButton & item, CRect itemRect)
{
    if (m_showingTip)
    {
        m_tooltip.Hide();
    }
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    CPoint tipAlternate(itemRect.left, itemRect.top - 2);
    SetTooltipText(item, tipTopLeft, tipAlternate);
    m_showingTip = true;
    // track the mouse so we know when it leaves our window
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;
    _TrackMouseEvent(&tme);
}

void CDestinyPane::HideTip()
{
    // tip not shown if not over an assay
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
        m_pTooltipItem = NULL;
    }
}

void CDestinyPane::SetTooltipText(
        const CIconButton & item,
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    CString text;
    item.GetWindowText(text);
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        const Feat& feat = FindFeat((LPCTSTR)text);
        m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
        m_tooltip.SetFeatItem(*pBuild, &feat, false, pBuild->Level(), false);
        m_tooltip.Show();
    }
}

void CDestinyPane::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    UNREFERENCED_PARAMETER(item);
    UNREFERENCED_PARAMETER(type);
    UpdateWindowTitle();
}
