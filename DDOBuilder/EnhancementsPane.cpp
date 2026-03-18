// EnhancementsView.cpp
//
#include "stdafx.h"
#include "EnhancementsPane.h"

#include "Build.h"
#include "Character.h"
#include "Feat.h"
#include "GlobalSupportFunctions.h"

#include "EnhancementTreeDialog.h"
#include "XmlLib\SaxReader.h"

namespace
{
    // enhancement window size
    const size_t c_sizeX = 300;
    const size_t c_sizeY = 466;
}

IMPLEMENT_DYNCREATE(CEnhancementsPane, CFormView)

CEnhancementsPane::CEnhancementsPane() :
    CFormView(CEnhancementsPane::IDD),
    m_pDoc(NULL),
    m_pCharacter(NULL),
    m_numUniversalButtons(0),
    m_tipCreated(false),
    m_pTooltipItem(NULL),
    m_showingTip(false),
    m_bHadinitialUpdate(false)
{
}

CEnhancementsPane::~CEnhancementsPane()
{
    for (size_t vi = 0; vi < m_treeViews.size(); ++vi)
    {
        delete m_treeViews[vi];
    }
}

void CEnhancementsPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_LOADTREE, m_buttonLoad);
    DDX_Control(pDX, IDC_BUTTON_SAVETREE1, m_buttonSave[0]);
    DDX_Control(pDX, IDC_BUTTON_SAVETREE2, m_buttonSave[1]);
    DDX_Control(pDX, IDC_BUTTON_SAVETREE3, m_buttonSave[2]);
    DDX_Control(pDX, IDC_BUTTON_SAVETREE4, m_buttonSave[3]);
    DDX_Control(pDX, IDC_BUTTON_SAVETREE5, m_buttonSave[4]);
    DDX_Control(pDX, IDC_BUTTON_SAVETREE6, m_buttonSave[5]);
    DDX_Control(pDX, IDC_BUTTON_SAVETREE7, m_buttonSave[6]);
    // no IDC_TREE_SELECT1 as that is the racial tree which cannot be swapped out
    DDX_Control(pDX, IDC_TREE_SELECT2, m_comboTreeSelect[0]);
    DDX_Control(pDX, IDC_TREE_SELECT3, m_comboTreeSelect[1]);
    DDX_Control(pDX, IDC_TREE_SELECT4, m_comboTreeSelect[2]);
    DDX_Control(pDX, IDC_TREE_SELECT5, m_comboTreeSelect[3]);
    DDX_Control(pDX, IDC_TREE_SELECT6, m_comboTreeSelect[4]);
    DDX_Control(pDX, IDC_TREE_SELECT7, m_comboTreeSelect[5]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE1, m_universalTrees[0]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE2, m_universalTrees[1]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE3, m_universalTrees[2]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE4, m_universalTrees[3]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE5, m_universalTrees[4]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE6, m_universalTrees[5]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE7, m_universalTrees[6]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE8, m_universalTrees[7]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE9, m_universalTrees[8]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE10, m_universalTrees[9]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE11, m_universalTrees[10]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE12, m_universalTrees[11]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE13, m_universalTrees[12]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE14, m_universalTrees[13]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE15, m_universalTrees[14]);
    DDX_Control(pDX, IDC_BUTTON_UNIVERSAL_TREE16, m_universalTrees[15]);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CEnhancementsPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_REGISTERED_MESSAGE(UWM_UPDATE_TREES, OnUpdateTrees)
    ON_CONTROL_RANGE(CBN_SELENDOK, IDC_TREE_SELECT2, IDC_TREE_SELECT7, OnTreeSelect)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_UNIVERSAL_TREE1, IDC_BUTTON_UNIVERSAL_TREE16, OnUniversalTree)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_SAVETREE1, IDC_BUTTON_SAVETREE7, OnSaveTree)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CEnhancementsPane::OnTtnNeedText)
    ON_COMMAND(IDC_BUTTON_LOADTREE, OnLoadTree)
END_MESSAGE_MAP()
#pragma warning(pop)

void CEnhancementsPane::OnInitialUpdate()
{
    if (!m_bHadinitialUpdate)
    {
        m_bHadinitialUpdate = true;
        CFormView::OnInitialUpdate();
        m_tooltip.Create(this);
        m_tipCreated = true;
        m_buttonLoad.SetImage(IDB_BITMAP_LOAD);
        for (size_t i = 0; i < MST_Number; ++i)
        {
            m_buttonSave[i].SetImage(IDB_BITMAP_SAVE);
            m_buttonSave[i].EnableWindow(FALSE);
        }
        EnableToolTips(TRUE);
    }
}

LRESULT CEnhancementsPane::OnLoadComplete(WPARAM, LPARAM)
{
    UpdateWindowTitle();
    AddCustomButtons();

    // cause a resize after all sub windows have updated
    CRect rect;
    GetClientRect(&rect);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rect.Width(), rect.Height()));
    return 0;
}

void CEnhancementsPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (m_treeViews.size() > 0
            && IsWindow(m_treeViews[0]->GetSafeHwnd())
            && IsWindowVisible())
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

        CRect rctButton;
        ASSERT(m_treeViews.size() == MST_Number);
        for (size_t ti = 0; ti < m_visibleTrees.size(); ++ti)
        {
            size_t index = m_visibleTrees[ti];
            ASSERT(index >= 0 && index < MST_Number);
            m_buttonSave[ti].GetWindowRect(&rctButton);
            rctButton -= rctButton.TopLeft();
            rctButton += CPoint(itemRect.left, itemRect.bottom);
            m_buttonSave[ti].MoveWindow(rctButton);
            m_buttonSave[ti].ShowWindow(SW_SHOW);        // ensure visible
            if (ti == 0)
            {
                m_buttonLoad.GetWindowRect(&rctButton);
                rctButton -= rctButton.TopLeft();
                rctButton += CPoint(itemRect.left + rctButton.Width() + c_controlSpacing, itemRect.bottom);
                m_buttonLoad.MoveWindow(rctButton);
                m_buttonLoad.ShowWindow(SW_SHOW);        // ensure visible
            }
            // move the window to the correct location
            m_treeViews[index]->MoveWindow(itemRect, false);
            m_treeViews[index]->ShowWindow(SW_SHOW);        // ensure visible
            if (ti > 0 && ti < MST_Number)
            {
                CRect rctCombo(rctButton.right + c_controlSpacing, itemRect.bottom, itemRect.right, itemRect.bottom + 300);
                m_comboTreeSelect[ti-1].MoveWindow(rctCombo);
                m_comboTreeSelect[ti-1].ShowWindow(SW_SHOW);
            }
            // now move the rectangle to the next tree location
            itemRect += CPoint(itemRect.Width() + c_controlSpacing, 0);
        }
        // position the universal tree buttons under the racial tree
        CRect rect(0, 0, 36, 36);
        rect -= CPoint(scrollX, 0);   // take scroll location into account (scrollY from itemRect)
        rect += CPoint(c_controlSpacing, itemRect.bottom + c_controlSpacing + GetSystemMetrics(SM_CYVSCROLL) + c_controlSpacing);
        for (size_t i = 0; i < MST_UniversalTrees; ++i)
        {
            m_universalTrees[i].MoveWindow(rect, true);
            m_universalTrees[i].ShowWindow((i < m_numUniversalButtons) ? SW_SHOW : SW_HIDE);
            rect += CPoint(rect.Width() + c_controlSpacing, 0);
        }
        SetScrollSizes(
                MM_TEXT,
                CSize(
                        itemRect.left + scrollX,
                        rect.bottom + scrollY + c_controlSpacing));
    }
}

LRESULT CEnhancementsPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDDOBuilderDoc* pDoc = (CDDOBuilderDoc*)(wParam);
    m_pDoc = pDoc;
    // lParam is the character pointer
    Character* pCharacter = (Character*)(lParam);

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

BOOL CEnhancementsPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_BUTTON_LOADTREE,
        IDC_BUTTON_SAVETREE1,
        IDC_BUTTON_SAVETREE2,
        IDC_BUTTON_SAVETREE3,
        IDC_BUTTON_SAVETREE4,
        IDC_BUTTON_SAVETREE5,
        IDC_BUTTON_SAVETREE6,
        IDC_BUTTON_SAVETREE7,
        IDC_TREE_SELECT2,
        IDC_TREE_SELECT3,
        IDC_TREE_SELECT4,
        IDC_TREE_SELECT5,
        IDC_TREE_SELECT6,
        IDC_TREE_SELECT7,
        IDC_BUTTON_UNIVERSAL_TREE1,
        IDC_BUTTON_UNIVERSAL_TREE2,
        IDC_BUTTON_UNIVERSAL_TREE3,
        IDC_BUTTON_UNIVERSAL_TREE4,
        IDC_BUTTON_UNIVERSAL_TREE5,
        IDC_BUTTON_UNIVERSAL_TREE6,
        IDC_BUTTON_UNIVERSAL_TREE7,
        IDC_BUTTON_UNIVERSAL_TREE8,
        IDC_BUTTON_UNIVERSAL_TREE9,
        IDC_BUTTON_UNIVERSAL_TREE10,
        IDC_BUTTON_UNIVERSAL_TREE11,
        IDC_BUTTON_UNIVERSAL_TREE12,
        IDC_BUTTON_UNIVERSAL_TREE13,
        IDC_BUTTON_UNIVERSAL_TREE14,
        IDC_BUTTON_UNIVERSAL_TREE15,
        IDC_BUTTON_UNIVERSAL_TREE16,
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

void CEnhancementsPane::UpdateActiveLifeChanged(Character*)
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

void CEnhancementsPane::UpdateActiveBuildChanged(Character*)
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

std::list<EnhancementTree> CEnhancementsPane::DetermineTrees()
{
    std::list<EnhancementTree> trees;
    Build *pBuild = m_pCharacter->ActiveBuild();
    // see which classes we have and then make our race and class trees available
    if (pBuild != NULL)
    {
        // to determine which tree's we need to know how many class levels we have in each class
        const std::list<EnhancementTree> & allTrees = EnhancementTrees();
        std::list<EnhancementTree>::const_iterator it = allTrees.begin();
        while (it != allTrees.end())
        {
            // get all the trees that are compatible with the race/class setup
            if ((*it).MeetRequirements(*pBuild, pBuild->Level()-1)  // 0 based for level needed
                    && !(*it).HasIsReaperTree()         // no reaper trees in enhancements please!
                    && !(*it).HasIsEpicDestiny()        // no epic destiny trees in enhancements please!
                    && (!(*it).HasLegacy() || m_pCharacter->SupportLegacyTrees())) // no legacy trees unless requested by user
            {
                // yes this is one of our tree's add it
                trees.push_back((*it));
            }
            ++it;
        }
        // first remove any trees present which are no longer compatible
        // we also revoke those trees AP spends if required
        Enhancement_SelectedTrees selTrees = pBuild->EnhancementSelectedTrees(); // take a copy
        for (size_t ti = 0; ti < MST_Number; ++ti)
        {
            std::string treeName = selTrees.Tree(ti);
            if (!Enhancement_SelectedTrees::IsNotSelected(treeName))
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
                        pBuild->Enhancement_ResetEnhancementTree(treeName);   // revokes any trained enhancements also
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
                // never auto assign universal trees
                if (!(*tit).HasIsUniversalTree())
                {
                    size_t start = (*tit).HasIsRacialTree() ? 0 : 1;
                    // tree not present, see if it can be assigned
                    for (size_t ti = start; ti < MST_Number; ++ti)
                    {
                        std::string treeName = selTrees.Tree(ti);
                        if (Enhancement_SelectedTrees::IsNotSelected(treeName)
                                || ((ti == 0) && treeName == c_noTreeSelection))    // special case for racial tree
                        {
                            // no assignment yet for this tree, assign this tree to it
                            selTrees.SetTree(ti, (*tit).Name());
                            break;  // done
                        }
                    }
                }
            }
            // try the next tree
            ++tit;
        }
        pBuild->Enhancement_SetSelectedTrees(selTrees);
    }
    return trees;
}

void CEnhancementsPane::CreateEnhancementWindows()
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
        m_treeViews.reserve(MST_Number);
        const Enhancement_SelectedTrees & selTrees = pBuild->EnhancementSelectedTrees(); // take a copy
        for (size_t i = 0; i < MST_Number; ++i)
        {
            std::string treeName = selTrees.Tree(i);
            if (!Enhancement_SelectedTrees::IsNotSelected(treeName))
            {
                // we have a selected tree here
                if (i > 0)
                {
                    // no combo box selector for racial tree
                    PopulateTreeCombo(&m_comboTreeSelect[i-1], treeName);
                }
                // create the tree dialog
                TreeType tt = TT_racial;
                if (i > 0)  // first tree is always racial
                {
                    tt = EnhancementTree::GetTree(treeName).HasIsUniversalTree()
                            ? TT_universal
                            : TT_enhancement;
                }
                // show an enhancement dialog
                CEnhancementTreeDialog * dlg = new CEnhancementTreeDialog(
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
                PopulateTreeCombo(&m_comboTreeSelect[i-1], "");
                // create the blank tree dialog
                CEnhancementTreeDialog * dlg = new CEnhancementTreeDialog(
                        this,
                        m_pCharacter,
                        EnhancementTree::GetTree(c_noTreeSelection),
                        TT_enhancement);
                dlg->Create(CEnhancementTreeDialog::IDD, this);
                dlg->MoveWindow(&itemRect);
                dlg->ShowWindow(SW_SHOW);
                m_treeViews.push_back(dlg);
            }
            m_visibleTrees.push_back(m_visibleTrees.size());
        }
        UnlockWindowUpdate();
        // reposition and show the windows (handled in OnSize)
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}

void CEnhancementsPane::DestroyEnhancementWindows()
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

void CEnhancementsPane::UpdateEnhancementWindows()
{
    LockWindowUpdate();
    // show the trees selected by the user
    const Enhancement_SelectedTrees & selTrees = m_pCharacter->ActiveBuild()->EnhancementSelectedTrees(); // take a copy
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
            }
            // always update the list of available trees
            if (i > 0)
            {
                // no combo box selector for racial tree
                PopulateTreeCombo(&m_comboTreeSelect[i-1], treeName);
            }
        }
    }
    UnlockWindowUpdate();
    // reposition and show the windows (handled in OnSize)
    CRect rctWnd;
    GetClientRect(&rctWnd);
    OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
}

void CEnhancementsPane::PopulateTreeCombo(
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
    const Enhancement_SelectedTrees & selTrees = pBuild->EnhancementSelectedTrees();
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
void CEnhancementsPane::UpdateAlignmentChanged(Life*, AlignmentType)
{
    // if alignment has changed, then classes may have changed also
    // we need to update our windows
    // (Same trees may still be available)
    std::list<EnhancementTree> trees = DetermineTrees();
    if (trees != m_availableTrees)
    {
        // yup, they have changed
        m_availableTrees = trees;
        UpdateEnhancementWindows();
    }
}

// BuildObserver overrides
void CEnhancementsPane::UpdateBuildLevelChanged(Build*)
{
    UpdateEnhancementWindows();
    UpdateWindowTitle();
    UpdateClassChanged(NULL, "", "", 0);
}

void CEnhancementsPane::UpdateClassChanged(
        Build *,
        const std::string&,
        const std::string&,
        size_t)
{
    // if a class has changed so whether we need to update our windows
    // (Same trees may still be available)
    std::list<EnhancementTree> trees = DetermineTrees();
    if (trees != m_availableTrees)
    {
        // yup, they have changed
        m_availableTrees = trees;
        UpdateEnhancementWindows();
    }
}

void CEnhancementsPane::UpdateEnhancementTrained(
    Build*,
    const EnhancementItemParams&)
{
    UpdateWindowTitle();
}

void CEnhancementsPane::UpdateEnhancementRevoked(
    Build*,
    const EnhancementItemParams&)
{
    UpdateWindowTitle();
}

void CEnhancementsPane::UpdateFeatEffectApplied(
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

void CEnhancementsPane::UpdateFeatEffectRevoked(
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

void CEnhancementsPane::UpdateRaceChanged(
        Life*,
        const std::string&)
{
    // if the race has changed, we definitely need to update the available
    // enhancement trees
    m_availableTrees = DetermineTrees();
    UpdateEnhancementWindows();
}

void CEnhancementsPane::UpdateEnhancementEffectApplied(
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

void CEnhancementsPane::UpdateEnhancementEffectRevoked(
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

void CEnhancementsPane::UpdateEnhancementTreeOrderChanged(Build*, enum TreeType tt)
{
    if (tt == TT_enhancement || tt == TT_universal)
    {
        DestroyEnhancementWindows();
        CreateEnhancementWindows();
    }
}

void CEnhancementsPane::UpdateActionPointsChanged(Build*)
{
    UpdateWindowTitle();
    EnableDisableTreeSaveLoad();
    EnableDisableComboboxes();
}

LRESULT CEnhancementsPane::OnUpdateTrees(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    // received a delayed tree update message, do the work
    UpdateTrees();
    UpdateWindowTitle();
    EnableDisableTreeSaveLoad();
    EnableDisableComboboxes();
    return 0;
}

void CEnhancementsPane::UpdateTrees()
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

void CEnhancementsPane::UpdateActionPointsChanged(Life*)
{
    UpdateWindowTitle();
}

void CEnhancementsPane::UpdateWindowTitle()
{
    CString text("Enhancements");
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            CString additional;
            text.Format("Enhancements - Total points %d",
                    pBuild->AvailableActionPoints(pBuild->Level(), TT_allEnhancement));
            size_t bonusRacial = pBuild->BonusRacialActionPoints();
            if (bonusRacial > 0)
            {
                additional.Format(", Racial %d", bonusRacial);
                text += additional;
            }
            size_t bonusUniversal = pBuild->BonusUniversalActionPoints();
            if (bonusUniversal > 0)
            {
                additional.Format(", Universal %d", bonusUniversal);
                text += additional;
            }
            additional.Format(", To Spend %d", pBuild->AvailableActionPoints(pBuild->Level(), TT_allEnhancementMinusSpent));
            text += additional;
        }
    }
    GetParent()->SetWindowText(text);
    EnableDisableComboboxes();
}

void CEnhancementsPane::OnTreeSelect(UINT nID)
{
    // the user has selected a new enhancement tree from a drop list under one of
    // the enhancement view windows. Switch the selected tree to the one wanted
    size_t treeIndex = (nID - IDC_TREE_SELECT2) + 1;    // tree 0 is the racial tree and cant be changed (no control for it)
    ASSERT(treeIndex > 0 && treeIndex < MST_Number);
    int sel = m_comboTreeSelect[treeIndex-1].GetCurSel();
    if (sel != CB_ERR)
    {
        CString entry;
        m_comboTreeSelect[treeIndex-1].GetLBText(sel, entry);
        std::string treeName = (LPCSTR)entry;
        // select the tree wanted!
        Build* pBuild = m_pCharacter->ActiveBuild();
        Enhancement_SelectedTrees selTrees = pBuild->EnhancementSelectedTrees(); // take a copy
        selTrees.SetTree(treeIndex, treeName);      // modify
        pBuild->Enhancement_SetSelectedTrees(selTrees);   // update
        // update our state
        UpdateEnhancementWindows();
    }
}

void CEnhancementsPane::EnableDisableTreeSaveLoad()
{
    for (size_t i = 0; i < MST_Number; ++i)
    {
        m_buttonSave[i].EnableWindow(FALSE);
    }
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            const Destiny_SelectedTrees& selTrees = pBuild->DestinySelectedTrees();
            for (size_t i = 0; i < MST_Number; ++i)
            {
                bool bEnableSave = false;
                if (selTrees.TreeName().size() > i)
                {
                    const std::string& treeName = selTrees.Tree(i);
                    // can only save this tree if points spent in this one
                    bEnableSave = (pBuild->APSpentInTree(treeName) > 0);
                }
                m_buttonSave[i].EnableWindow(bEnableSave);
            }
        }
    }
}

void CEnhancementsPane::EnableDisableComboboxes()
{
    bool bDisable = true;
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            const Enhancement_SelectedTrees& selTrees = pBuild->EnhancementSelectedTrees();
            for (size_t i = 1; i < MST_Number; ++i)
            {
                bool enable = false;
                if (selTrees.TreeName().size() > i)
                {
                    std::string treeName = selTrees.Tree(i);
                    // can only select a different tree if no points spent in this one
                    enable = (pBuild->APSpentInTree(treeName) == 0);
                }
                m_comboTreeSelect[i-1].EnableWindow(enable);
            }
            // also set the states of the universal tree buttons
            for (size_t i = 0; i < m_numUniversalButtons; ++i)
            {
                m_universalTrees[i].EnableWindow(true);
                m_universalTrees[i].ShowWindow(SW_SHOW);
                // show the correct checked state
                CString name;
                m_universalTrees[i].GetWindowText(name);
                bool trained = (pBuild->GetSpecialFeatTrainedCount((LPCTSTR)name) > 0);
                m_universalTrees[i].SetSelected(trained);
            }
            bDisable = false;
            m_buttonLoad.EnableWindow(TRUE);
        }
    }
    if (bDisable)
    {
        // all combo boxes should be hidden and disabled
        for (size_t i = 1; i < MST_Number; ++i)
        {
            m_comboTreeSelect[i-1].EnableWindow(false);
            m_comboTreeSelect[i-1].ShowWindow(SW_HIDE);
        }
        // also set the states of the universal tree buttons
        for (size_t i = 0; i < m_numUniversalButtons; ++i)
        {
            m_universalTrees[i].EnableWindow(false);
            m_universalTrees[i].ShowWindow(SW_HIDE);
        }
        Invalidate(TRUE);
        GetParent()->Invalidate(TRUE);
        m_buttonLoad.EnableWindow(FALSE);
    }
}

void CEnhancementsPane::AddCustomButtons()
{
    // add buttons under the Racial tree to allow quick access to the
    // universal enhancement trees
    const std::list<Feat> & universalTreeFeats = UniversalTreeFeats();
    std::list<Feat>::const_iterator it = universalTreeFeats.begin();
    size_t buttonIndex = 0;
    while (it != universalTreeFeats.end())
    {
        m_universalTrees[buttonIndex].Initialise(
                (*it).Name().c_str(),
                "DataFiles\\FeatImages\\",
                (*it).Icon());
        ++buttonIndex;
        ++m_numUniversalButtons;
        ++it;
    }
}

void CEnhancementsPane::OnUniversalTree(UINT nID)
{
    UINT id = nID - IDC_BUTTON_UNIVERSAL_TREE1;
    CString name;
    if (m_universalTrees[id].IsWindowVisible())
    {
        m_universalTrees[id].GetWindowText(name);
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            bool doit = false;
            bool trained = (pBuild->GetSpecialFeatTrainedCount((LPCTSTR)name) > 0);
            if (trained)
            {
                int ret = AfxMessageBox("Are you sure you want to revoke access to this tree?\r\n"
                    "Any points spent in this tree will be revoked.\r\n"
                    "\r\n"
                    "(Note you may still have access via Favor reward ranks)", MB_ICONQUESTION | MB_YESNO);
                if (ret == IDYES)
                {
                    pBuild->RevokeSpecialFeat((LPCTSTR)name);
                    doit = true;
                }
            }
            else
            {
                pBuild->TrainSpecialFeat((LPCTSTR)name);
                doit = true;
            }
            if (doit)
            {
                m_universalTrees[id].SetSelected(!trained);
                // ensure displayed trees are shown
                std::list<EnhancementTree> trees = DetermineTrees();
                if (trees != m_availableTrees)
                {
                    // yup, they have changed
                    m_availableTrees = trees;
                    UpdateEnhancementWindows();
                }
            }
        }
    }
}

void CEnhancementsPane::OnSaveTree(UINT nID)
{
    UINT id = nID - IDC_BUTTON_SAVETREE1;
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            const Enhancement_SelectedTrees& selTrees = pBuild->EnhancementSelectedTrees();
            CString treeName = selTrees.Tree(id).c_str();
            CFileDialog filedlg(
                FALSE,
                NULL,
                (LPCTSTR)treeName,
                OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                "DDOBuilder Tree Files (*.DDOTree)|*.DDOTree||",
                this);
            if (filedlg.DoModal() == IDOK)
            {
                const std::list<EnhancementSpendInTree>& treeSpends = pBuild->EnhancementTreeSpend();
                EnhancementSpendInTree spend;
                for (auto&& tsit : treeSpends)
                {
                    if (tsit.TreeName() == (LPCTSTR)treeName)
                    {
                        spend = tsit;
                    }
                }

                std::string filename = (LPCTSTR)filedlg.GetPathName();
                if (filename.find(".DDOETree") == std::string::npos)
                {
                    filename += ".DDOETree";
                }
                try
                {
                    const XmlLib::SaxString f_saxElementName = L"DDOBuilderTree"; // root element name to look for
                    XmlLib::SaxWriter writer;
                    writer.Open(filename);
                    writer.StartDocument(f_saxElementName);
                    spend.Write(&writer);
                    writer.EndDocument();
                    std::stringstream ss;
                    ss << "Enhancement Tree " << treeName << " exported to file \"" << filename << "\"";
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                catch (const std::exception& e)
                {
                    std::string errorMessage = e.what();
                    // document has failed to save. Tell the user what we can about it
                    CString text;
                    text.Format("The document %s\n"
                        "failed to save. The XML parser reported the following problem:\n"
                        "\n", filename.c_str());
                    text += errorMessage.c_str();
                    AfxMessageBox(text, MB_ICONERROR);
                }
            }
        }
    }
}

void CEnhancementsPane::OnMouseMove(UINT nFlags, CPoint point)
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
    if (pWnd == NULL || !pWnd->IsWindowVisible())
    {
        // ensure none-visible windows do not show tooltips
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

LRESULT CEnhancementsPane::OnMouseLeave(WPARAM, LPARAM)
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

void CEnhancementsPane::OnLButtonDown(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    CWnd* pWnd = ChildWindowFromPoint(point);
    CIconButton* pIconButton = dynamic_cast<CIconButton*>(pWnd);
    if (m_pCharacter == NULL)
    {
        // ensure buttons do not produce tooltips while no character active
        pIconButton = NULL;
    }
    if (pIconButton != NULL)
    {
        UINT nID = pIconButton->GetDlgCtrlID();
        OnUniversalTree(nID);
    }
}

void CEnhancementsPane::ShowTip(const CIconButton & item, CRect itemRect)
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

void CEnhancementsPane::HideTip()
{
    // tip not shown if not over an assay
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
        m_pTooltipItem = NULL;
    }
}

void CEnhancementsPane::SetTooltipText(
        const CIconButton & item,
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    CString text;
    item.GetWindowText(text);
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        const Feat & feat = FindFeat((LPCTSTR)text);
        m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
        m_tooltip.SetFeatItem(*pBuild, &feat, false, pBuild->Level(), false);
        m_tooltip.Show();
    }
}

BOOL CEnhancementsPane::OnTtnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(pResult);

    UINT_PTR nID = pNMHDR->idFrom;
    nID = ::GetDlgCtrlID((HWND)nID);

    switch (nID)
    {
        case IDC_BUTTON_UNIVERSAL_TREE1:
        case IDC_BUTTON_UNIVERSAL_TREE2:
        case IDC_BUTTON_UNIVERSAL_TREE3:
        case IDC_BUTTON_UNIVERSAL_TREE4:
        case IDC_BUTTON_UNIVERSAL_TREE5:
        case IDC_BUTTON_UNIVERSAL_TREE6:
        case IDC_BUTTON_UNIVERSAL_TREE7:
        case IDC_BUTTON_UNIVERSAL_TREE8:
        case IDC_BUTTON_UNIVERSAL_TREE9:
        case IDC_BUTTON_UNIVERSAL_TREE10:
        case IDC_BUTTON_UNIVERSAL_TREE11:
        case IDC_BUTTON_UNIVERSAL_TREE12:
        case IDC_BUTTON_UNIVERSAL_TREE13:
        case IDC_BUTTON_UNIVERSAL_TREE14:
        case IDC_BUTTON_UNIVERSAL_TREE15:
        case IDC_BUTTON_UNIVERSAL_TREE16:
            m_tipText = "You have access to this universal tree";
            break;
        case IDC_BUTTON_SAVETREE1:
        case IDC_BUTTON_SAVETREE2:
        case IDC_BUTTON_SAVETREE3:
        case IDC_BUTTON_SAVETREE4:
        case IDC_BUTTON_SAVETREE5:
        case IDC_BUTTON_SAVETREE6:
        case IDC_BUTTON_SAVETREE7:
            m_tipText = "Save this enhancement tree layout to a file";
            break;
        case IDC_BUTTON_LOADTREE:
            m_tipText = "Load in a previously saved enhancement tree layout";
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

void CEnhancementsPane::OnLoadTree()
{
    // display the file select dialog
    CFileDialog filedlg(
            TRUE,
            NULL,
            NULL,
            OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,
            "DDOBuilder Tree Files (*.DDOTree)|*.DDOTree||",
            this);
    if (filedlg.DoModal() == IDOK)
    {
        bool bSuccess = false;
        EnhancementSpendInTree loadedTree;
        CString filename = filedlg.GetPathName();
        try
        {
            XmlLib::SaxReader reader(&loadedTree, L"DDOBuilderTree");
            // read in the xml from a file (fully qualified path)
            bool ok = reader.Open((LPCTSTR)filename);
            if (ok)
            {
                std::stringstream ss;
                ss << "DDOBuilder enhancement tree Document \"" << (LPCTSTR)filename << "\" loaded.";
                GetLog().AddLogEntry(ss.str().c_str());
                bSuccess = true;
            }
            else
            {
                std::string errorMessage = reader.ErrorMessage();
                // document has failed to load. Tell the user what we can about it
                CString text;
                text.Format("The document %s\n"
                    "failed to load. The XML parser reported the following problem:\n"
                    "\n", (LPCTSTR)filename);
                text += errorMessage.c_str();
                AfxMessageBox(text, MB_ICONERROR);
            }
        }
        catch (const std::exception& e)
        {
            std::string errorMessage = e.what();
            // document has failed to save. Tell the user what we can about it
            CString text;
            text.Format("The document %s\n"
                "failed to save. The XML parser reported the following problem:\n"
                "\n", (LPCTSTR)filename);
            text += errorMessage.c_str();
            AfxMessageBox(text, MB_ICONERROR);
        }
        if (bSuccess)
        {
            // things to check first for the loaded tree
            // 1: It matches one of the available trees for the current build setup
            // 2: It is the same version number as the tree
            // 3: The tree is already selected or can go in an unselected tree slot
            if (m_pCharacter != NULL)
            {
                Build* pBuild = m_pCharacter->ActiveBuild();
                if (pBuild != NULL)
                {
                    Enhancement_SelectedTrees selTrees = pBuild->EnhancementSelectedTrees();
                    // is it an available tree?
                    bool bAvailable = false;
                    for (auto&& atit: m_availableTrees)
                    {
                        if (atit.Name() == loadedTree.TreeName())
                        {
                            bAvailable = true;
                            break;
                        }
                    }
                    if (bAvailable)
                    {
                        bool bHas = false;
                        // if the tree has already been spent in then it needs to be revoked and replaced by the imported tree
                        bool bTreeHasSpent = (pBuild->APSpentInTree(loadedTree.TreeName()) > 0);
                        if (bTreeHasSpent)
                        {
                            pBuild->Enhancement_ResetEnhancementTree(loadedTree.TreeName());
                            bHas = true;
                        }
                        else
                        {
                            // need to find a slot of this tree, does it already have one?
                            for (auto&& stit: selTrees.TreeName())
                            {
                                if (stit == loadedTree.TreeName())
                                {
                                    bHas = true;
                                }
                            }
                            if (!bHas)
                            {
                                // if we can find an empty tree slot (No Selection) assign it to this tree
                                size_t index = 0;
                                for (auto&& stit: selTrees.TreeName())
                                {
                                    if (stit == c_noTreeSelection)
                                    {
                                        // we can use this tree slot
                                        selTrees.SetTree(index, loadedTree.TreeName());   // modify
                                        pBuild->Enhancement_SetSelectedTrees(selTrees);   // update
                                        // update our state
                                        UpdateEnhancementWindows();
                                        bHas = true;
                                        break;
                                    }
                                    ++index;
                                }
                            }
                        }
                        if (!bHas)
                        {
                            // the tree was not already present or we could not assign a tree slot to it
                            AfxMessageBox("No available tree slot available to apply this tree. Clear an existing tree slot and try a reload.", MB_ICONERROR);
                        }
                        else
                        {
                            // now apply each trained enhancement in order
                            for (auto&& ltit: loadedTree.Enhancements())
                            {
                                const EnhancementTreeItem* pItem = FindEnhancement(loadedTree.TreeName(), ltit.EnhancementName());
                                if (pItem != NULL)
                                {
                                    for (size_t rank = 0; rank < ltit.Ranks(); ++rank)
                                    {
                                        pBuild->Enhancement_TrainEnhancement(
                                            loadedTree.TreeName(),
                                            ltit.EnhancementName(),
                                            ltit.HasSelection() ? ltit.Selection() : "",
                                            pItem->CostPerRank());
                                    }
                                }
                                else
                                {
                                    AfxMessageBox("Enhancement item not found during apply action. Aborting", MB_ICONERROR);
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        CString text;
                        text.Format("The tree data loaded for tree type \"%s\" is not a tree that can be selected by this build.", loadedTree.TreeName().c_str());
                        AfxMessageBox(text, MB_ICONERROR);
                    }
                }
            }
        }
    }
}
