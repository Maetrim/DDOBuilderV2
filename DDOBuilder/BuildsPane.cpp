// BuildsPane.cpp : implementation file
//

#include "stdafx.h"
#include "BuildsPane.h"
#include "GlobalSupportFunctions.h"
#include "DDOBuilderDoc.h"
#include "LogPane.h"
#include "XmlLib\SaxReader.h"

// CBuildsPane
namespace
{
    DWORD MakeItemData(TreeEntryItem type, size_t lifeIndex, size_t buildIndex)
    {
        return MAKELONG(type, MAKEWORD(lifeIndex, buildIndex));
    }
    TreeEntryItem ExtractType(DWORD itemData)
    {
        TreeEntryItem type = static_cast<TreeEntryItem>(LOWORD(itemData));
        return type;
    }
    size_t ExtractLifeIndex(DWORD itemData)
    {
        size_t lifeIndex = LOBYTE(HIWORD(itemData));
        return lifeIndex;
    }
    size_t ExtractBuildIndex(DWORD itemData)
    {
        size_t buildIndex = HIBYTE(HIWORD(itemData));
        return buildIndex;
    }
}

IMPLEMENT_DYNCREATE(CBuildsPane, CFormView)
BEGIN_MESSAGE_MAP(CBuildsPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_BN_CLICKED(IDC_BUTTON_NEW_LIFE, OnButtonNewLife)
    ON_BN_CLICKED(IDC_BUTTON_DELETE_LIFE, OnButtonDeleteLife)
    ON_BN_CLICKED(IDC_BUTTON_ADD_BUILD, OnButtonAddBuild)
    ON_BN_CLICKED(IDC_BUTTON_DELETE_BUILD, OnButtonDeleteBuild)
    ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_BUILDS, &CBuildsPane::OnSelchangedTreeBuilds)
    ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE_BUILDS, &CBuildsPane::OnBeginlabeleditTreeBuilds)
    ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_BUILDS, &CBuildsPane::OnEndlabeleditTreeBuilds)
    ON_NOTIFY(NM_DBLCLK, IDC_TREE_BUILDS, &CBuildsPane::OnDblclkTreeBuilds)
    ON_NOTIFY(NM_CLICK, IDC_TREE_BUILDS, &CBuildsPane::OnClickTreeBuilds)
    ON_NOTIFY(NM_RCLICK, IDC_TREE_BUILDS, &CBuildsPane::OnRButtonDownTreeBuilds)
    ON_UPDATE_COMMAND_UI_RANGE(ID_LEVELSELECT_1, ID_LEVELSELECT_40, &CBuildsPane::OnUpdateBuildLevel)
    ON_MESSAGE(WM_USER, OnStartLabelEdit)
    ON_UPDATE_COMMAND_UI(ID_LIFE_COPYTOCLIPBOARD, &CBuildsPane::OnUpdateCopyLifeToClipboard)
    ON_UPDATE_COMMAND_UI(ID_LIFE_PASTEFROMCLIPBOARD, &CBuildsPane::OnUpdatePasteLife)
    ON_COMMAND(ID_LIFE_PASTEFROMCLIPBOARD, &CBuildsPane::OnPasteLife)
    ON_COMMAND_RANGE(ID_LEVELSELECT_1, ID_LEVELSELECT_40, OnSetBuildLevel)
END_MESSAGE_MAP()

CBuildsPane::CBuildsPane() :
    CFormView(CBuildsPane::IDD),
    m_pDoc(NULL),
    m_pCharacter(NULL),
    m_hPopupMenuItem(NULL),
    m_bEscape(false),
    m_bLoadComplete(false),
    m_bEditInProgress(false)
{
}

CBuildsPane::~CBuildsPane()
{
}

LRESULT CBuildsPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
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
        // only add life starts enabled, overridden if we get a
        // selected item
        m_buttonDeleteLife.EnableWindow(false);
        m_buttonAddBuild.EnableWindow(false);
        m_buttonDeleteBuild.EnableWindow(false);
        // need to clear any selection for a load to restore to the previous selection
        // on start up
        m_treeBuilds.SelectItem(NULL);
        PopulateBuildsList();
    }
    return 0L;
}

LRESULT CBuildsPane::OnLoadComplete(WPARAM, LPARAM)
{
    m_bLoadComplete = true;
    PopulateBuildsList();
    return 0;
}

void CBuildsPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_NEW_LIFE, m_buttonNewLife);
    DDX_Control(pDX, IDC_BUTTON_DELETE_LIFE, m_buttonDeleteLife);
    DDX_Control(pDX, IDC_BUTTON_ADD_BUILD, m_buttonAddBuild);
    DDX_Control(pDX, IDC_BUTTON_DELETE_BUILD, m_buttonDeleteBuild);
    DDX_Control(pDX, IDC_TREE_BUILDS, m_treeBuilds);
}

void CBuildsPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_treeBuilds.GetSafeHwnd()))
    {
        // [Add][Del][Add][Del]
        // +------------------+
        // |Tree              |
        // |                  |
        // |                  |
        // |                  |
        // +------------------+
        CRect rctAddLife;
        CRect rctDelLife;
        CRect rctAddBuild;
        CRect rctDelBuild;
        CRect rctTree;
        m_buttonNewLife.GetWindowRect(rctAddLife);
        m_buttonDeleteLife.GetWindowRect(rctDelLife);
        m_buttonAddBuild.GetWindowRect(rctAddBuild);
        m_buttonDeleteBuild.GetWindowRect(rctDelBuild);
        rctAddLife -= rctAddLife.TopLeft();
        rctAddLife += CPoint(c_controlSpacing, c_controlSpacing);
        // note that following button bump down a line if not enough room
        // to be viewed fully
        rctDelLife -= rctDelLife.TopLeft();
        rctDelLife += CPoint(c_controlSpacing + rctAddLife.right, rctAddLife.top);
        if (rctDelLife.right > cx)
        {
            rctDelLife -= rctDelLife.TopLeft();
            rctDelLife += CPoint(c_controlSpacing, c_controlSpacing + rctAddLife.bottom);
        }

        rctAddBuild -= rctAddBuild.TopLeft();
        rctAddBuild += CPoint(c_controlSpacing + rctDelLife.right, rctDelLife.top);
        if (rctAddBuild.right > cx)
        {
            rctAddBuild -= rctAddBuild.TopLeft();
            rctAddBuild += CPoint(c_controlSpacing, c_controlSpacing + rctDelLife.bottom);
        }

        rctDelBuild -= rctDelBuild.TopLeft();
        rctDelBuild += CPoint(c_controlSpacing + rctAddBuild.right, rctAddBuild.top);
        if (rctDelBuild.right > cx)
        {
            rctDelBuild -= rctDelBuild.TopLeft();
            rctDelBuild += CPoint(c_controlSpacing, c_controlSpacing + rctAddBuild.bottom);
        }

        rctTree = CRect(
                c_controlSpacing,
                rctDelBuild.bottom + c_controlSpacing,
                cx - c_controlSpacing,
                cy - c_controlSpacing);
        // now we know where they need to be moved to, move them
        m_buttonNewLife.MoveWindow(rctAddLife, FALSE);
        m_buttonDeleteLife.MoveWindow(rctDelLife, FALSE);
        m_buttonAddBuild.MoveWindow(rctAddBuild, FALSE);
        m_buttonDeleteBuild.MoveWindow(rctDelBuild, FALSE);
        m_treeBuilds.MoveWindow(rctTree, FALSE);
    }
    SetScaleToFitSize(CSize(cx, cy));
}

BOOL CBuildsPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_BUTTON_NEW_LIFE,
        IDC_BUTTON_DELETE_LIFE,
        IDC_BUTTON_ADD_BUILD,
        IDC_BUTTON_DELETE_BUILD,
        IDC_TREE_BUILDS,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CBuildsPane::OnInitialUpdate()
{
    __super::OnInitialUpdate();
    PopulateBuildsList();
}

BOOL CBuildsPane::PreTranslateMessage(MSG* pMsg)
{
    // If edit control is visible in tree view control, when you send a
    // WM_KEYDOWN message to the edit control it will dismiss the edit
    // control. When the ENTER key was sent to the edit control, the
    // parent window of the tree view control is responsible for updating
    // the item's label in TVN_ENDLABELEDIT notification code.
    if (pMsg->message == WM_KEYDOWN &&
        (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
    {
        CEdit* edit = m_treeBuilds.GetEditControl();
        if (edit != NULL)
        {
            m_bEscape = (pMsg->wParam == VK_ESCAPE);
            edit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
            return TRUE;
        }
    }
    return CFormView::PreTranslateMessage(pMsg);
}

void CBuildsPane::UpdateActiveBuildPositionChanged(Character*)
{
    DWORD selItemData = MakeItemData(
            TEI_Build,
            m_pCharacter->ActiveLifeIndex(),
            m_pCharacter->ActiveBuildIndex());
    SelectTreeItem(selItemData);
    PopulateBuildsList();
}

void CBuildsPane::UpdateRaceChanged(
    Life*,
    const std::string&)
{
    PopulateBuildsList();
}

void CBuildsPane::UpdateBuildLevelChanged(Build*)
{
    PopulateBuildsList();
}

void CBuildsPane::UpdateClassChanged(
    Build*,
    const std::string&,
    const std::string&,
    size_t)
{
    PopulateBuildsList();
}

void CBuildsPane::PopulateBuildsList()
{
    if (m_pCharacter != NULL && !m_bEditInProgress)
    {
        // keep the same item selected if there was one
        HTREEITEM hSelItem = m_treeBuilds.GetSelectedItem();
        DWORD selItemData = MakeItemData(
                TEI_Build,
                m_pCharacter->ActiveLifeIndex(),
                m_pCharacter->ActiveBuildIndex());
        if (hSelItem != NULL)
        {
            selItemData = m_treeBuilds.GetItemData(hSelItem);
        }
        // the item data for each element in the tree is the life:build index
        m_treeBuilds.LockWindowUpdate();
        m_treeBuilds.DeleteAllItems();
        // add each life as a root element in the tree
        size_t lifeIndex = 0;
        const std::list<Life> & lives = m_pCharacter->Lives();
        std::list<Life>::const_iterator lit = lives.begin();
        while (lit != lives.end())
        {
            CString name = (*lit).UIDescription(lifeIndex);
            HTREEITEM hItem = m_treeBuilds.InsertItem(
                    name,
                    -1,
                    -1,
                    TVI_ROOT,
                    TVI_LAST);
            // the life is the first item in the build list
            DWORD itemId = MakeItemData(TEI_Life, lifeIndex, 0);
            m_treeBuilds.SetItemData(hItem, itemId);
            // now add each of the builds under this Life
            size_t buildIndex = 0;
            const std::list<Build> & builds = (*lit).Builds();
            std::list<Build>::const_iterator bit = builds.begin();
            while (bit != builds.end())
            {
                // each build in a life has a maximum level
                name = (*bit).UIDescription(buildIndex);
                HTREEITEM hSubItem = m_treeBuilds.InsertItem(
                        name,
                        -1,
                        -1,
                        hItem,
                        TVI_LAST);
                itemId = MakeItemData(TEI_Build, lifeIndex, buildIndex);
                m_treeBuilds.SetItemData(hSubItem, itemId);
                ++bit;
                ++buildIndex;
            }
            if (!(*lit).HasTreeCollapsed())
            {
                m_treeBuilds.Expand(hItem, TVE_EXPAND);
            }
            ++lit;
            ++lifeIndex;
        }
        // restore the selection if there was one
        SelectTreeItem(selItemData);
        m_treeBuilds.UnlockWindowUpdate();
    }
    NMHDR hdrNotUsed;
    LRESULT lNotUsed;
    OnSelchangedTreeBuilds(&hdrNotUsed, &lNotUsed);
}

void CBuildsPane::OnButtonNewLife()
{
    OnNewLife();
}

Build* CBuildsPane::OnNewLife()
{
    Build* pBuild = NULL;
    GetLog().AddLogEntry("New life added");
    size_t lifeIndex = m_pCharacter->AddLife();
    PopulateBuildsList();
    // new life, 1st build starts selected
    SelectTreeItem(TEI_Build, lifeIndex, 0);
    if (lifeIndex == 0)
    {
        m_pCharacter->SetModifiedFlag(FALSE);
    }
    pBuild = m_pCharacter->ActiveBuild();
    return pBuild;
}

Build* CBuildsPane::OnNewImportLife()
{
    Build* pBuild = NULL;
    GetLog().AddLogEntry("New life added");
    size_t lifeIndex = m_pCharacter->AddImportLife();
    PopulateBuildsList();
    // new life, 1st build starts selected
    SelectTreeItem(TEI_Build, lifeIndex, 0);
    if (lifeIndex == 0)
    {
        m_pCharacter->SetModifiedFlag(FALSE);
    }
    pBuild = m_pCharacter->ActiveBuild();
    return pBuild;
}

void CBuildsPane::OnButtonDeleteLife()
{
    GetLog().AddLogEntry("Life deleted");
    HTREEITEM hItem = m_treeBuilds.GetSelectedItem();
    DWORD itemData = m_treeBuilds.GetItemData(hItem);
    size_t lifeIndex = ExtractLifeIndex(itemData);
    m_pCharacter->DeleteLife(lifeIndex);
    PopulateBuildsList();
    if (m_pCharacter->Lives().size() > 0)
    {
        if (m_pCharacter->Lives().size() > lifeIndex)
        {
            // just select the one that was next in the list
            SelectTreeItem(TEI_Life, lifeIndex, 0);
        }
        else
        {
            if (lifeIndex > 0)
            {
                // go to the previous life in the list
                SelectTreeItem(TEI_Life, lifeIndex-1, 0);
            }
        }
    }
    else
    {
        // no lives in character, no selection
        // ensure buttons are correct for no selection
        LRESULT result;
        OnSelchangedTreeBuilds(NULL, &result);
    }
}

void CBuildsPane::OnButtonAddBuild()
{
    GetLog().AddLogEntry("Build added to life");
    HTREEITEM hItem = m_treeBuilds.GetSelectedItem();
    DWORD itemData = m_treeBuilds.GetItemData(hItem);
    size_t lifeIndex = ExtractLifeIndex(itemData);
    size_t buildIndex = m_pCharacter->AddBuild(lifeIndex);
    PopulateBuildsList();
    // new build starts selected
    SelectTreeItem(TEI_Build, lifeIndex, buildIndex);
}

void CBuildsPane::OnButtonDeleteBuild()
{
    std::stringstream ss;
    ss << "Build \"" << m_pCharacter->ActiveBuild()->ComplexUIDescription() << "\" deleted from life.";
    GetLog().AddLogEntry(ss.str().c_str());
    HTREEITEM hItem = m_treeBuilds.GetSelectedItem();
    DWORD itemData = m_treeBuilds.GetItemData(hItem);
    size_t lifeIndex = ExtractLifeIndex(itemData);
    size_t buildIndex = ExtractBuildIndex(itemData);
    m_pCharacter->DeleteBuild(lifeIndex, buildIndex);
    PopulateBuildsList();
    const Life & life = m_pCharacter->GetLife(lifeIndex);
    if (life.Builds().size() > 0)
    {
        if (life.Builds().size() > buildIndex)
        {
            // just select the one that was next in the list
            SelectTreeItem(TEI_Build, lifeIndex, buildIndex);
        }
        else
        {
            if (buildIndex > 0)
            {
                // go to the previous Build in the list
                SelectTreeItem(TEI_Build, lifeIndex, buildIndex-1);
            }
        }
    }
    else
    {
        // no builds left in this life, drop back to the Life element being selected
        SelectTreeItem(TEI_Life, lifeIndex, 0);
    }
}

void CBuildsPane::OnSelchangedTreeBuilds(NMHDR *, LRESULT *pResult)
{
    // new life is always available
    m_buttonNewLife.EnableWindow(m_bLoadComplete);
    // enable the button states based on what item is selected in the tree
    HTREEITEM hSelItem = m_treeBuilds.GetSelectedItem();
    if (hSelItem != NULL)
    {
        // we have a valid selection
        DWORD itemData = m_treeBuilds.GetItemData(hSelItem);
        TreeEntryItem type = ExtractType(itemData);
        size_t lifeIndex = ExtractLifeIndex(itemData);
        size_t buildIndex = ExtractBuildIndex(itemData);
        switch (type)
        {
            case TEI_Life:
                m_buttonDeleteLife.EnableWindow(m_bLoadComplete);
                m_buttonAddBuild.EnableWindow(m_bLoadComplete);
                m_buttonDeleteBuild.EnableWindow(false);
                break;
            case TEI_Build:
                m_buttonDeleteLife.EnableWindow(false);
                m_buttonAddBuild.EnableWindow(m_bLoadComplete);
                m_buttonDeleteBuild.EnableWindow(m_bLoadComplete);
                break;
            default:
                // fail!
                m_buttonDeleteLife.EnableWindow(false);
                m_buttonAddBuild.EnableWindow(false);
                m_buttonDeleteBuild.EnableWindow(false);
                break;
        }
        m_pCharacter->SetActiveBuild(lifeIndex, buildIndex);
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->AttachObserver(this);
        }
        Life* pLife = m_pCharacter->ActiveLife();
        if (pLife != NULL)
        {
            pLife->AttachObserver(this);
        }
    }
    else
    {
        // only New life available
        m_buttonDeleteLife.EnableWindow(false);
        m_buttonAddBuild.EnableWindow(false);
        m_buttonDeleteBuild.EnableWindow(false);
        if (m_pCharacter != NULL)
        {
            m_pCharacter->SetActiveBuild(m_pCharacter->ActiveLifeIndex(), m_pCharacter->ActiveBuildIndex());
            Build* pBuild = m_pCharacter->ActiveBuild();
            if (pBuild != NULL)
            {
                pBuild->AttachObserver(this);
            }
            Life* pLife = m_pCharacter->ActiveLife();
            if (pLife != NULL)
            {
                pLife->AttachObserver(this);
            }
        }
    }
    *pResult = 0;
}

void CBuildsPane::OnBeginlabeleditTreeBuilds(NMHDR *pNMHDR, LRESULT *pResult)
{
    // the user can only edit life names
    LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
    DWORD itemData = m_treeBuilds.GetItemData(pTVDispInfo->item.hItem);
    TreeEntryItem type = ExtractType(itemData);
    *pResult = (type == TEI_Life)
            ? FALSE     // allow edit of life tree elements
            : TRUE;     // cancel edit of non-life tree elements
}

void CBuildsPane::OnEndlabeleditTreeBuilds(NMHDR *pNMHDR, LRESULT *pResult)
{
    if (!m_bEscape)
    {
        m_bEscape = true;   // stop a double edit on a click away
        // the user has renamed a life or build
        LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
        DWORD itemData = m_treeBuilds.GetItemData(pTVDispInfo->item.hItem);
        CString strText = pTVDispInfo->item.pszText;
        CString strOldName = m_treeBuilds.GetItemText(pTVDispInfo->item.hItem);
        m_treeBuilds.SetItemText(pTVDispInfo->item.hItem, strText);
        // update the item
        TreeEntryItem type = ExtractType(itemData);
        size_t lifeIndex = ExtractLifeIndex(itemData);
        CString strLogText;
        switch (type)
        {
            case TEI_Life:
                m_pCharacter->SetLifeName(lifeIndex, strText);
                strLogText.Format("Life renamed from \"%s\" to \"%s\"", (LPCSTR)strOldName, (LPCSTR)strText);
                GetLog().AddLogEntry(strLogText);
                PopulateBuildsList();
                break;
            default:
                // fail!
                break;
        }
    }
    else
    {
        m_bEscape = false;
    }
    *pResult = 0;
}

void CBuildsPane::SelectTreeItem(
        TreeEntryItem type,
        size_t lifeIndex,
        size_t buildIndex)
{
    SelectTreeItem(MakeItemData(type, lifeIndex, buildIndex));
}

void CBuildsPane::SelectTreeItem(DWORD itemData)
{
    HTREEITEM hCurrent = m_treeBuilds.GetSelectedItem();
    // iterate the tree and find the item with this item data
    // if not found, select the default item
    bool found = false;
    HTREEITEM hItem = m_treeBuilds.GetChildItem(TVI_ROOT);
    while (!found && hItem != NULL)
    {
        DWORD dwItem = m_treeBuilds.GetItemData(hItem);
        CString strText = m_treeBuilds.GetItemText(hItem);
        if (dwItem == itemData)
        {
            // this is the one we want
            found = true;
        }
        else
        {
            hItem = GetNextTreeItem(m_treeBuilds, hItem);
        }
    }
    if (hItem != NULL
            && hItem != hCurrent)
    {
        m_treeBuilds.Select(hItem, TVGN_CARET);
        m_treeBuilds.SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
        m_treeBuilds.EnsureVisible(hItem);
    }
}

void CBuildsPane::OnClickTreeBuilds(NMHDR* pNMHDR, LRESULT* pResult)
{
    CPoint mouse;
    GetCursorPos(&mouse);
    m_treeBuilds.ScreenToClient(&mouse);
    UINT uFlags = 0;
    HTREEITEM hItem = m_treeBuilds.HitTest(mouse, &uFlags);
    if (hItem == m_treeBuilds.GetSelectedItem())
    {
        OnDblclkTreeBuilds(pNMHDR, pResult);
    }
}

void CBuildsPane::OnRButtonDownTreeBuilds(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
    CPoint mouse;
    GetCursorPos(&mouse);
    m_treeBuilds.ScreenToClient(&mouse);
    UINT uFlags = 0;
    HTREEITEM hItem = m_treeBuilds.HitTest(mouse, &uFlags);
    if (hItem != NULL)
    {
        DWORD itemData = m_treeBuilds.GetItemData(hItem);
        TreeEntryItem type = ExtractType(itemData);
        if (type == TEI_Build || type == TEI_Life)
        {
            // display a pop-up menu to allow the user to select action
            // this build to be at
            CMenu menuLevelSelect;
            menuLevelSelect.LoadMenu(IDR_POPUP_MENUS);
            CMenu* pMenu = menuLevelSelect.GetSubMenu(0); // lives pop-up actions
            // find where to display it
            CRect rectItem;
            m_treeBuilds.GetItemRect(hItem, &rectItem, FALSE);
            m_treeBuilds.ClientToScreen(&rectItem);
            int x = rectItem.left;
            int y = rectItem.bottom + 1;
            CWinAppEx* pApp = dynamic_cast<CWinAppEx*>(AfxGetApp());
            m_hPopupMenuItem = hItem;
            UINT sel = pApp->GetContextMenuManager()->TrackPopupMenu(
                pMenu->GetSafeHmenu(),
                x,
                y,
                this);
            if (sel != 0)
            {
                switch (sel)
                {
                    case ID_LIFE_EXPORTTONEWFILE:
                        SaveLifeToNewFile(itemData);
                        break;
                    case ID_LIFE_IMPORTLIVESFROMOTHER:
                        ImportLivesFromOtherFile();
                        break;
                    case ID_LIFE_COPYTOCLIPBOARD:
                        CopyLifeToClipboard(itemData);
                        break;
                    case ID_LIFE_PASTEFROMCLIPBOARD:
                        OnPasteLife();
                        break;
                }
            }
            m_hPopupMenuItem = NULL;
        }
    }
    *pResult = 0;
}

void CBuildsPane::OnDblclkTreeBuilds(NMHDR *pNMHDR, LRESULT *pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
    // identify the item that was double clicked, and if its a build item
    // allow them to set the level via a drop menu
    CPoint mouse;
    GetCursorPos(&mouse);
    m_treeBuilds.ScreenToClient(&mouse);
    UINT uFlags = 0;
    HTREEITEM hItem = m_treeBuilds.HitTest(mouse, &uFlags);
    if (hItem != NULL)
    {
        DWORD itemData = m_treeBuilds.GetItemData(hItem);
        TreeEntryItem type = ExtractType(itemData);
        if (type == TEI_Build)
        {
            size_t lifeIndex = ExtractLifeIndex(itemData);
            size_t buildIndex = ExtractBuildIndex(itemData);
            // display a pop-up menu to allow the user to select the level they want
            // this build to be at
            CMenu menuLevelSelect;
            menuLevelSelect.LoadMenu(IDR_LEVEL_SELECT_MENU);
            CMenu * pMenu = menuLevelSelect.GetSubMenu(0);
            // find where to display it
            CRect rectItem;
            m_treeBuilds.GetItemRect(hItem, &rectItem, FALSE);
            m_treeBuilds.ClientToScreen(&rectItem);
            int x = rectItem.left;
            int y = rectItem.bottom + 1;
            CWinAppEx * pApp = dynamic_cast<CWinAppEx*>(AfxGetApp());
            m_hPopupMenuItem = hItem;
            UINT sel = pApp->GetContextMenuManager()->TrackPopupMenu(
                    pMenu->GetSafeHmenu(),
                    x,
                    y,
                    this);
            if (sel != 0)
            {
                // only update if the level selected is different
                int selectedLevel = (sel - ID_LEVELSELECT_1) + 1; // 1 based
                int buildLevel = m_pCharacter->GetBuildLevel(lifeIndex, buildIndex);
                if (selectedLevel != buildLevel)
                {
                    // update the build
                    m_pCharacter->SetBuildLevel(lifeIndex, buildIndex, selectedLevel);
                    // update our UI element
                    PopulateBuildsList();
                }
            }
            m_hPopupMenuItem = NULL;
        }
        else
        {
            // post a message to ourself to cause label edit start
            PostMessage(WM_USER, 0, 0L);
        }
        *pResult = 0;
    }
}

LRESULT CBuildsPane::OnStartLabelEdit(WPARAM, LPARAM)
{
    // start a label edit
    HTREEITEM hItem = m_treeBuilds.GetSelectedItem();
    if (hItem != NULL)
    {
        m_bEditInProgress = true;
        m_treeBuilds.EditLabel(hItem);
        m_bEditInProgress = false;
    }
    return 0;
}

void CBuildsPane::OnUpdateBuildLevel(CCmdUI* pCmdUI)
{
    // show a check mark against the builds current level selection
    if (m_hPopupMenuItem != NULL)
    {
        DWORD itemData = m_treeBuilds.GetItemData(m_hPopupMenuItem);
        size_t lifeIndex = ExtractLifeIndex(itemData);
        size_t buildIndex = ExtractBuildIndex(itemData);
        int buildLevel = m_pCharacter->GetBuildLevel(lifeIndex, buildIndex);
        int menuItemLevel = (pCmdUI->m_nID - ID_LEVELSELECT_1) + 1;
        pCmdUI->SetCheck(menuItemLevel == buildLevel);
        // disable item for levels not yet supported in game
        pCmdUI->Enable(menuItemLevel <= MAX_GAME_LEVEL);
    }
    else
    {
        int buildLevel = m_pCharacter->ActiveBuild()->Level();
        int menuItemLevel = (pCmdUI->m_nID - ID_LEVELSELECT_1) + 1;
        pCmdUI->SetCheck(menuItemLevel == buildLevel);
        // disable item for levels not yet supported in game
        pCmdUI->Enable(menuItemLevel <= MAX_GAME_LEVEL);
    }
}

void CBuildsPane::OnSetBuildLevel(UINT nID)
{
    int selectedLevel = (nID - ID_LEVELSELECT_1) + 1; // 1 based
    int buildLevel = m_pCharacter->ActiveBuild()->Level();
    if (selectedLevel != buildLevel)
    {
        // update the build
        m_pCharacter->ActiveBuild()->SetLevel(selectedLevel);
        // update our UI element
        PopulateBuildsList();
    }
}

void CBuildsPane::OnUpdatePasteLife(CCmdUI* pCmdUi)
{
    // can paste if we have data of the correct format available on the clipboard
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    bool enable = pBuild != NULL
        && ::IsClipboardFormatAvailable(CF_CUSTOM_LIFE);
    pCmdUi->Enable(m_bLoadComplete && enable);
}

void CBuildsPane::OnUpdateCopyLifeToClipboard(CCmdUI* pCmdUi)
{
    pCmdUi->Enable(m_bLoadComplete);
}

void CBuildsPane::CopyLifeToClipboard(DWORD itemData)
{
    size_t lifeIndex = ExtractLifeIndex(itemData);
    const Life& life = m_pCharacter->GetLife(lifeIndex);
    // copy the current gear set to the clipboard as xml text using a custom
    // clipboard format to preserve any other data present on the clipboard
    HGLOBAL hData = NULL;
    // write the life as XML text
    XmlLib::SaxWriter writer;
    life.Write(&writer);
    std::string xmlText = writer.Text();

    // read the file into a global memory handle
    hData = GlobalAlloc(GMEM_MOVEABLE, xmlText.size() + 1); // space for \0
    if (hData != NULL)
    {
        char* buffer = (char*)GlobalLock(hData);
        strcpy_s(buffer, xmlText.size() + 1, xmlText.data());
        GlobalUnlock(hData);
    }
    if (::OpenClipboard(NULL))
    {
        ::EmptyClipboard();
        ::SetClipboardData(CF_CUSTOM_LIFE, hData);
        ::CloseClipboard();
    }
    else
    {
        AfxMessageBox("Failed to open the clipboard.", MB_ICONERROR);
    }
}

void CBuildsPane::OnPasteLife()
{
    if (::IsClipboardFormatAvailable(CF_CUSTOM_LIFE))
    {
        if (::OpenClipboard(NULL))
        {
            // is the data in the right format
            HGLOBAL hGlobal = ::GetClipboardData(CF_CUSTOM_LIFE);
            if (hGlobal != NULL)
            {
                // get the data as text from the clipboard
                std::string xmlText;
                char* buffer = (char*)::GlobalLock(hGlobal);
                xmlText = buffer;
                GlobalUnlock(hGlobal);
                ::CloseClipboard();

                // parse the XML text and read into a local object
                Life life(m_pCharacter);
                XmlLib::SaxReader reader(&life, life.ElementName());
                bool ok = reader.ParseText(xmlText);
                if (ok)
                {
                    GetLog().AddLogEntry("Life pasted from clipboard");
                    m_pCharacter->AppendLife(life);
                    PopulateBuildsList();
                    // when pasted we do not change the selection
                }
                else
                {
                    // something is wrong with the data on the clipboard
                    AfxMessageBox("Failed to read data from clipboard.", MB_ICONERROR);
                }
            }
        }
        else
        {
            AfxMessageBox("Failed to open the clipboard.", MB_ICONERROR);
        }
    }
    else
    {
        // no data of correct format on the clipboard
        AfxMessageBox("No Life available on the clipboard.", MB_ICONERROR);
    }
}

void CBuildsPane::SaveLifeToNewFile(DWORD itemData)
{
    CFileDialog filedlg(
        FALSE,
        NULL,
        NULL,
        OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "DDOBuilder Files (*.DDOBuild)|*.DDOBuild||",
        this);
    if (filedlg.DoModal() == IDOK)
    {
        std::string filename = (LPCTSTR)filedlg.GetPathName();
        if (filename.find(".DDOBuild") == std::string::npos)
        {
            filename += ".DDOBuild";
        }
        size_t lifeIndex = ExtractLifeIndex(itemData);
        const Life& life = m_pCharacter->GetLife(lifeIndex);
        CDDOBuilderDoc cTempDoc;
        Character cNew(&cTempDoc);
        cNew.AppendLife(life);
        try
        {
            const XmlLib::SaxString f_saxElementName = L"DDOBuilderCharacterData"; // root element name to look for
            XmlLib::SaxWriter writer;
            writer.Open(filename);
            writer.StartDocument(f_saxElementName);
            cNew.Write(&writer);
            writer.EndDocument();
            std::stringstream ss;
            ss << "Life exported to file \"" << filename << "\"";
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

void CBuildsPane::ImportLivesFromOtherFile()
{
    CFileDialog filedlg(
        TRUE,
        NULL,
        NULL,
        OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "DDOBuilder Files (*.DDOBuild)|*.DDOBuild||",
        this);
    if (filedlg.DoModal() == IDOK)
    {
        std::string filename = (LPCTSTR)filedlg.GetPathName();
        CDDOBuilderDoc cTempDoc;
        if (TRUE == cTempDoc.LoadIndirect(filename.c_str()))
        {
            CString text;
            text.Format("Loaded document \"%s\".", filename.c_str());
            GetLog().AddLogEntry(text);
            const std::list<Life>& lives = cTempDoc.GetCharacter()->Lives();
            if (lives.size() > 0)
            {
                size_t li = 1;
                for (auto&& lit: lives)
                {
                    m_pCharacter->AppendLife(lit);
                    text.Format("...Imported life %d, with %d builds.", li, lit.Builds().size());
                    GetLog().AddLogEntry(text);
                    ++li;
                }
                PopulateBuildsList();
                m_pCharacter->SetModifiedFlag(TRUE);
            }
            else
            {
                GetLog().AddLogEntry("...No Lives present in file.");
            }
        }
        else
        {
            CString text;
            text.Format("The document \"%s\" failed to load.", filename.c_str());
            GetLog().AddLogEntry(text);
        }
    }
}

void CBuildsPane::ReselectCurrentItem()
{
    m_pCharacter->SetActiveBuild(
        m_pCharacter->ActiveLifeIndex(), m_pCharacter->ActiveBuildIndex(), true);
}

