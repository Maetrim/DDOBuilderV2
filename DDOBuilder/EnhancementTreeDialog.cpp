// EnhancementTreeDialog.cpp
//

#include "stdafx.h"
#include "EnhancementTreeDialog.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "SelectionSelectDialog.h"
#include "MouseHook.h"
#include "Character.h"

namespace
{
    // enhancement window size
    const size_t c_sizeX = 299;
    const size_t c_sizeY = 466;
    enum ImageBackgroundEnum
    {
        IBE_CoreEnhancementOverlay = 0,
        IBE_CoreInnateAbilityOverlay,
        IBE_CoreClickie,
        IBE_CorePassive,
        IBE_ItemClickie,
        IBE_ItemPassive,
        IBE_CoreClickieDisabled,
        IBE_CorePassiveDisabled,
        IBE_ItemClickieDisabled,
        IBE_ItemPassiveDisabled,
        IBE_ArrowLeft,
        IBE_ArrowRight,
        IBE_ArrowUp,
        IBE_LongArrowUp,
        IBE_ExtraLongArrowUp,
        IBE_PassiveAvailable,
        IBE_ActiveAvailable,
        IBE_PassiveNotAvailable,
        IBE_PassiveNotAllowed,
        IBE_ActiveNotAvailable,
        IBE_ActiveNotAllowed,
        IBE_CorePassiveAvailable,
        IBE_CoreActiveAvailable,
        IBE_ResetTreeDisabled,
        IBE_ResetTreeEnabled,
        IBE_CoreRevokeOverlay,
        IBE_StandardRevokeOverlay,
        IBE_count
    };
    const int c_leftOffsetCore = 16;
    const int c_leftOffsetItem = 11;
    const int c_topCore = 364;
    const int c_topItem1 = 362;
    const int c_xItemSpacingCore = 46;
    const int c_xItemSpacingItem = 59;
    const int c_yItemSpacing = 72;
    const int c_iconLeft = 10;
    const int c_iconTop = 424;
}
// global image data used for drawing all enhancements trees.
// this is only initialised once
bool s_imagesInitiliased = false;
CImage s_imageBorders[IBE_count];

void CEnhancementTreeDialog::InitialiseStaticImages()
{
    if (!s_imagesInitiliased)
    {
        // load all the standard static background images used when rendering
        // an enhancement tree
        std::string folderPath = "DataFiles\\UIImages\\";
        LoadImageFile(folderPath, "CoreEnhancementOverlay", &s_imageBorders[IBE_CoreEnhancementOverlay], CSize(299, 71));
        LoadImageFile(folderPath, "CoreInnateAbilityOverlay", &s_imageBorders[IBE_CoreInnateAbilityOverlay], CSize(299, 71));
        LoadImageFile(folderPath, "CoreClickie", &s_imageBorders[IBE_CoreClickie], CSize(38, 38));
        LoadImageFile(folderPath, "CorePassive", &s_imageBorders[IBE_CorePassive], CSize(38, 38));
        LoadImageFile(folderPath, "ItemClickie", &s_imageBorders[IBE_ItemClickie], CSize(43, 53));
        LoadImageFile(folderPath, "ItemPassive", &s_imageBorders[IBE_ItemPassive], CSize(43, 53));
        LoadImageFile(folderPath, "CoreClickie", &s_imageBorders[IBE_CoreClickieDisabled], CSize(38, 38));
        LoadImageFile(folderPath, "CorePassive", &s_imageBorders[IBE_CorePassiveDisabled], CSize(38, 38));
        LoadImageFile(folderPath, "ItemClickie", &s_imageBorders[IBE_ItemClickieDisabled], CSize(43, 53));
        LoadImageFile(folderPath, "ItemPassive", &s_imageBorders[IBE_ItemPassiveDisabled], CSize(43, 53));
        LoadImageFile(folderPath, "EnhancementArrowLeft", &s_imageBorders[IBE_ArrowLeft], CSize(13, 14));
        LoadImageFile(folderPath, "EnhancementArrowRight", &s_imageBorders[IBE_ArrowRight], CSize(13, 14));
        LoadImageFile(folderPath, "EnhancementArrowUp", &s_imageBorders[IBE_ArrowUp], CSize(14, 15));
        LoadImageFile(folderPath, "EnhancementLongArrowUp", &s_imageBorders[IBE_LongArrowUp], CSize(14, 86));
        LoadImageFile(folderPath, "EnhancementExtraLongArrowUp", &s_imageBorders[IBE_ExtraLongArrowUp], CSize(14, 157));
        LoadImageFile(folderPath, "PassiveAvailable", &s_imageBorders[IBE_PassiveAvailable], CSize(51, 61));
        LoadImageFile(folderPath, "ActiveAvailable", &s_imageBorders[IBE_ActiveAvailable], CSize(51, 61));
        LoadImageFile(folderPath, "PassiveNotAvailable", &s_imageBorders[IBE_PassiveNotAvailable], CSize(43, 53));
        LoadImageFile(folderPath, "PassiveNotAllowed", &s_imageBorders[IBE_PassiveNotAllowed], CSize(43, 53));
        LoadImageFile(folderPath, "ActiveNotAvailable", &s_imageBorders[IBE_ActiveNotAvailable], CSize(43, 53));
        LoadImageFile(folderPath, "ActiveNotAllowed", &s_imageBorders[IBE_ActiveNotAllowed], CSize(43, 53));
        LoadImageFile(folderPath, "CorePassiveAvailable", &s_imageBorders[IBE_CorePassiveAvailable], CSize(50, 50));
        LoadImageFile(folderPath, "CoreActiveAvailable", &s_imageBorders[IBE_CoreActiveAvailable], CSize(48, 48));
        LoadImageFile(folderPath, "ResetTreeEnabled", &s_imageBorders[IBE_ResetTreeEnabled], CSize(79, 24));
        LoadImageFile(folderPath, "ResetTreeEnabled", &s_imageBorders[IBE_ResetTreeDisabled], CSize(79, 24));
        LoadImageFile(folderPath, "CoreRevokeOverlay", &s_imageBorders[IBE_CoreRevokeOverlay], CSize(38, 38));
        LoadImageFile(folderPath, "StandardRevokeOverlay", &s_imageBorders[IBE_StandardRevokeOverlay], CSize(51, 61));
        for (size_t i = 0; i < IBE_count; ++i)
        {
            s_imageBorders[i].SetTransparentColor(c_transparentColour);
        }
        MakeGrayScale(&s_imageBorders[IBE_CoreClickieDisabled], c_transparentColour);
        MakeGrayScale(&s_imageBorders[IBE_CorePassiveDisabled], c_transparentColour);
        MakeGrayScale(&s_imageBorders[IBE_ItemClickieDisabled], c_transparentColour);
        MakeGrayScale(&s_imageBorders[IBE_ItemPassiveDisabled], c_transparentColour);
        MakeGrayScale(&s_imageBorders[IBE_ResetTreeDisabled], c_transparentColour);
        s_imagesInitiliased = true;
    }
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CEnhancementTreeDialog, CDialog)
    //{{AFX_MSG_MAP(CEnhancementTreeDialog)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_SETCURSOR()
    ON_WM_CAPTURECHANGED()
    ON_WM_MBUTTONDOWN()
END_MESSAGE_MAP()
#pragma warning(pop)

CEnhancementTreeDialog::CEnhancementTreeDialog(
        CWnd* pParent,
        Character* pCharacter,
        const EnhancementTree & tree,
        TreeType type) :
    CDialog(CEnhancementTreeDialog::IDD, pParent),
    m_pTree(&tree),
    m_type(type),
    m_pCharacter(pCharacter),
    m_bitmapSize(CSize(0, 0)),
    m_bCreateHitBoxes(false),
    m_tipCreated(false),
    m_pTooltipItem(NULL),
    m_bDraggingTree(false)
{
    InitialiseStaticImages();
    //{{AFX_DATA_INIT(CEnhancementTreeDialog)
    //}}AFX_DATA_INIT
    std::string folderPath = "DataFiles\\UIImages\\";
    LoadImageFile(folderPath, m_pTree->Background(), &m_imageBackground, CSize(c_sizeX, c_sizeY));
    LoadImageFile(folderPath, m_pTree->Icon(), &m_treeIcon, CSize(32, 32));
    m_treeIcon.SetTransparentColor(c_transparentColour);
    Life* pLife = pCharacter->ActiveLife();
    if (pLife != NULL)
    {
        pLife->AttachObserver(this);
    }
    Build* pBuild = pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->AttachObserver(this);
    }
}

const std::string& CEnhancementTreeDialog::CurrentTree() const
{
    return m_pTree->Name();
}

void CEnhancementTreeDialog::ChangeTree(const EnhancementTree& tree)
{
    m_pTree = &tree;
    m_imageBackground.Destroy();
    m_treeIcon.Destroy();
    std::string folderPath = "DataFiles\\UIImages\\";
    LoadImageFile(folderPath, m_pTree->Background(), &m_imageBackground, CSize(c_sizeX, c_sizeY));
    LoadImageFile(folderPath, m_pTree->Icon(), &m_treeIcon, CSize(32, 32));
    m_treeIcon.SetTransparentColor(c_transparentColour);
    m_hitBoxes.clear();
    m_bCreateHitBoxes = true;
    // on a tree change, make sure we update to the correct tree type
    // so purchases will work correctly
    m_type = TT_enhancement;        // assume
    if (tree.HasIsRacialTree()) m_type = TT_racial;
    if (tree.HasIsUniversalTree()) m_type = TT_universal;
    if (tree.HasIsEpicDestiny()) m_type = TT_epicDestiny;
    if (IsWindow(GetSafeHwnd()))
    {
        Invalidate(TRUE);
    }
}

void CEnhancementTreeDialog::SetTreeType(TreeType tt)
{
    // only called when being changed to a preview tree
    m_type = tt;
}

void CEnhancementTreeDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CEnhancementTreeDialog)
    //}}AFX_DATA_MAP
}

BOOL CEnhancementTreeDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_tooltip.Create(this);
    m_tipCreated = true;

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CEnhancementTreeDialog::OnEraseBkgnd(CDC* pDC)
{
    UNREFERENCED_PARAMETER(pDC);
    return 0;
}

void CEnhancementTreeDialog::OnPaint()
{
    CPaintDC pdc(this); // validates the client area on destruction
    int dcSaveIndex = pdc.SaveDC();
    ASSERT(dcSaveIndex != 0);
    // ensure we have a background bitmap
    if (m_bitmapSize == CSize(0, 0))
    {
        if (m_cachedDisplay.m_hObject != NULL)
        {
            VERIFY(m_cachedDisplay.DeleteObject() != 0); // ensure
        }
        // create the bitmap we will render to
        VERIFY(m_cachedDisplay.CreateCompatibleBitmap(
                &pdc,
                c_sizeX,
                c_sizeY) != 0);
        m_bitmapSize = CSize(c_sizeX, c_sizeY);
    }
    Build* pBuild = m_pCharacter->ActiveBuild();
    // draw to a compatible device context and then splat to screen
    CDC memoryDc;
    VERIFY(memoryDc.CreateCompatibleDC(&pdc) != 0);
    int memDcSaveIndex = memoryDc.SaveDC();
    ASSERT(memDcSaveIndex != 0);
    VERIFY(memoryDc.SelectObject(&m_cachedDisplay) != NULL);
    VERIFY(memoryDc.SelectStockObject(DEFAULT_GUI_FONT) != NULL);

    // first render the tree background
    VERIFY(m_imageBackground.BitBlt(
            memoryDc.GetSafeHdc(),
            CRect(0, 0, c_sizeX, c_sizeY),
            CPoint(0, 0),
            SRCCOPY) != 0);
    // the rest of the stuff only gets added
    // if this tree has enhancement tree items. The tree could be an empty tree
    if (pBuild != NULL && m_pTree->Items().size() > 0)
    {
        // add the core enhancements overlay
        VERIFY(s_imageBorders[IBE_CoreEnhancementOverlay].TransparentBlt(
                memoryDc.GetSafeHdc(),
                0,
                356,
                c_sizeX,
                71) != 0);

        // add the tree icon and name
        VERIFY(m_treeIcon.TransparentBlt(
                memoryDc.GetSafeHdc(),
                c_iconLeft,
                c_iconTop,
                32,
                32) != 0);
        CSize textSize = memoryDc.GetTextExtent(m_pTree->Name().c_str());
        memoryDc.SetBkMode(TRANSPARENT);
        memoryDc.SetTextColor(RGB(255, 255, 255));  // white
        memoryDc.TextOut(
                c_iconLeft + 32 + c_controlSpacing,
                c_iconTop + (32 - textSize.cy) / 2,
                m_pTree->Name().c_str());

        // now number of AP spent in tree
        size_t spentInTree = pBuild->APSpentInTree(m_pTree->Name());
        CString text;
        text.Format("%d AP Spent", spentInTree);
        textSize = memoryDc.GetTextExtent(text);
        memoryDc.TextOut(
                c_sizeX - textSize.cx - c_controlSpacing,
                c_iconTop - 3,
                text);

        // show the Reset Tree button in the correct state
        if (spentInTree > 0)
        {
            // reset tree is available if the user has spent points within it
            VERIFY(s_imageBorders[IBE_ResetTreeEnabled].TransparentBlt(
                    memoryDc.GetSafeHdc(),
                    c_sizeX - 79 - c_controlSpacing,
                    c_sizeY - 24 - c_controlSpacing,
                    79,
                    24) != 0);
        }
        else
        {
            // reset not available
            VERIFY(s_imageBorders[IBE_ResetTreeDisabled].TransparentBlt(
                    memoryDc.GetSafeHdc(),
                    c_sizeX - 79 - c_controlSpacing,
                    c_sizeY - 24 - c_controlSpacing,
                    79,
                    24) != 0);
        }
        // use a smaller font for drawing any x/y when items are rendered
        LOGFONT lf;
        ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
        strcpy_s(lf.lfFaceName, "Consolas");
        lf.lfHeight = 11;
        CFont smallFont;
        VERIFY(smallFont.CreateFontIndirect(&lf) != 0);
        CFont * pOldFont = memoryDc.SelectObject(&smallFont);

        m_bCreateHitBoxes = (m_hitBoxes.size() == 0);
        // now add all the tree enhancements and their states
        const std::list<EnhancementTreeItem> & items = m_pTree->Items();
        std::list<EnhancementTreeItem>::const_iterator it = items.begin();
        while (it != items.end())
        {
            RenderTreeItem((*it), &memoryDc);
            ++it;
        }
        VERIFY(memoryDc.SelectObject(pOldFont) == &smallFont);
    }
    // now draw to display
    VERIFY(pdc.BitBlt(
            0,
            0,
            m_bitmapSize.cx,
            m_bitmapSize.cy,
            &memoryDc,
            0,
            0,
            SRCCOPY) != 0);
    VERIFY(memoryDc.RestoreDC(memDcSaveIndex) != 0);
    VERIFY(memoryDc.DeleteDC() != 0);
    VERIFY(pdc.RestoreDC(dcSaveIndex) != 0);
}

void CEnhancementTreeDialog::RenderTreeItem(
        const EnhancementTreeItem & item,
        CDC * pDC)
{
    bool isCore = (item.YPosition() == 0);

    if (isCore)
    {
        RenderItemCore(item, pDC);
    }
    else
    {
        // first render the items border image
        CRect itemRect(0, 0, 43, 53);
        // now apply the item position to the rectangle
        itemRect += CPoint(
                c_leftOffsetItem + c_xItemSpacingItem * item.XPosition(),
                c_topItem1 - c_yItemSpacing * item.YPosition());

        // clickie item state may be overridden by item selection
        Build* pBuild = m_pCharacter->ActiveBuild();
        const TrainedEnhancement * te = pBuild->IsTrained(item.InternalName(), "");//, m_type);
        bool clickie = item.HasClickie();
        std::string selection;
        if (te != NULL)
        {
            if (te->HasSelection())
            {
                selection = te->Selection();
                clickie = item.IsSelectionClickie(selection);
            }
        }
        if (clickie)
        {
            RenderItemClickie(item, pDC, itemRect);
        }
        else
        {
            RenderItemPassive(item, pDC, itemRect);
        }
        // show how many have been acquired of max ranks
        bool isTier5Blocked = item.IsTier5Blocked(*pBuild, m_pTree->Name());
        if (!isTier5Blocked)
        {
            // only show trained x/y if item is allowed
            te = pBuild->IsTrained(item.InternalName(), "");//, m_type);
            CString text;
            text.Format(
                    "%d/%d",
                    (te == NULL) ? 0: te->Ranks(),
                    item.Ranks(selection));
            CSize textSize = pDC->GetTextExtent(text);
            pDC->TextOut(
                    itemRect.left + (itemRect.Width() - textSize.cx) / 2,
                    itemRect.bottom - textSize.cy - 2,
                    text);
        }
        bool canRevoke = item.CanRevoke(pBuild->FindSpendInTree(m_pTree->Name()));
        if (canRevoke)
        {
            VERIFY(s_imageBorders[IBE_StandardRevokeOverlay].TransparentBlt(
                    pDC->GetSafeHdc(),
                    itemRect.left,
                    itemRect.top,
                    itemRect.Width(),
                    itemRect.Height()) != 0);
        }
        if (item.HasArrowLeft())
        {
            VERIFY(s_imageBorders[IBE_ArrowLeft].TransparentBlt(
                    pDC->GetSafeHdc(),
                    itemRect.left - 15,
                    itemRect.top + (itemRect.Height() - 14) / 2,
                    13,
                    14) != 0);
        }
        if (item.HasArrowRight())
        {
            VERIFY(s_imageBorders[IBE_ArrowRight].TransparentBlt(
                    pDC->GetSafeHdc(),
                    itemRect.right + 2,
                    itemRect.top + (itemRect.Height() - 14) / 2,
                    13,
                    14) != 0);
        }
        if (item.HasArrowUp())
        {
            VERIFY(s_imageBorders[IBE_ArrowUp].TransparentBlt(
                    pDC->GetSafeHdc(),
                    itemRect.left + (itemRect.Width() - 14) / 2,
                    itemRect.top - 17,
                    14,
                    15) != 0);
        }
        if (item.HasLongArrowUp())
        {
            VERIFY(s_imageBorders[IBE_LongArrowUp].TransparentBlt(
                    pDC->GetSafeHdc(),
                    itemRect.left + (itemRect.Width() - 14) / 2,
                    itemRect.top - 88,
                    14,
                    86) != 0);
        }
        if (item.HasExtraLongArrowUp())
        {
            VERIFY(s_imageBorders[IBE_ExtraLongArrowUp].TransparentBlt(
                    pDC->GetSafeHdc(),
                    itemRect.left + (itemRect.Width() - 14) / 2,
                    itemRect.top - 161,
                    14,
                    157) != 0);
       }
    }
}

void CEnhancementTreeDialog::RenderItemCore(
        const EnhancementTreeItem & item,
        CDC * pDC)
{
    // core items are rendered across the bottom section of the bitmap
    CRect itemRect(0, 0, 38, 38);
    Build* pBuild = m_pCharacter->ActiveBuild();
    size_t spentInTree = pBuild->APSpentInTree(m_pTree->Name());
    bool isAllowed = item.IsAllowed(*pBuild, "", pBuild->Level()-1, spentInTree);
    bool isTrainable = item.CanTrain(*pBuild, spentInTree, m_type, pBuild->Level()-1);
    bool canRevoke = item.CanRevoke(pBuild->FindSpendInTree(m_pTree->Name()));
    // now apply the item position to the rectangle
    itemRect += CPoint(c_leftOffsetCore + c_xItemSpacingCore * item.XPosition(), c_topCore);
    // clickie item state may be overridden by item selection
    const TrainedEnhancement * te = pBuild->IsTrained(item.InternalName(), "");//, m_type);
    bool clickie = item.HasClickie();
    std::string selection;
    if (te != NULL)
    {
        if (te->HasSelection())
        {
            selection = te->Selection();
            clickie = item.IsSelectionClickie(selection);
        }
    }
    if (clickie)
    {
        VERIFY(s_imageBorders[IBE_CoreClickie].TransparentBlt(
                pDC->GetSafeHdc(),
                itemRect.left,
                itemRect.top,
                itemRect.Width(),
                itemRect.Height()) != 0);
    }
    else
    {
        VERIFY(s_imageBorders[IBE_CorePassive].TransparentBlt(
                pDC->GetSafeHdc(),
                itemRect.left,
                itemRect.top,
                itemRect.Width(),
                itemRect.Height()) != 0);
    }
    // render the cores icon, this may depend on a selector sub item
    CRect iconRect(itemRect);
    iconRect.DeflateRect(3, 3, 3, 3);
    ASSERT(iconRect.Width() == 32);
    ASSERT(iconRect.Height() == 32);
    RenderItemSelection(item, pDC, iconRect);

    // always show trained x/y
    CString text;
    text.Format(
            "%d/%d",
            (te == NULL) ? 0: te->Ranks(),
            item.Ranks(selection));
    CSize textSize = pDC->GetTextExtent(text);
    pDC->TextOut(
            itemRect.left + (itemRect.Width() - textSize.cx) / 2,
            itemRect.bottom + 2,
            text);

    if (m_bCreateHitBoxes)
    {
        EnhancementHitBox hitBox(item, itemRect);
        m_hitBoxes.push_back(hitBox);
    }

    if (canRevoke)
    {
        VERIFY(s_imageBorders[IBE_CoreRevokeOverlay].TransparentBlt(
                pDC->GetSafeHdc(),
                itemRect.left,
                itemRect.top,
                itemRect.Width(),
                itemRect.Height()) != 0);
    }
    if (isTrainable && isAllowed)
    {
        // available outline is larger
        itemRect.InflateRect(5, 5, 5, 5);
        VERIFY(s_imageBorders[item.HasClickie() ? IBE_CoreActiveAvailable : IBE_CorePassiveAvailable].TransparentBlt(
                pDC->GetSafeHdc(),
                itemRect.left,
                itemRect.top,
                itemRect.Width(),
                itemRect.Height()) != 0);
    }
}

void CEnhancementTreeDialog::RenderItemClickie(
        const EnhancementTreeItem & item,
        CDC * pDC,
        CRect itemRect)
{
    // it has a square active border
    VERIFY(s_imageBorders[IBE_ItemClickie].TransparentBlt(
            pDC->GetSafeHdc(),
            itemRect.left,
            itemRect.top,
            itemRect.Width(),
            itemRect.Height()) != 0);
    // show whether it can be trained or not
    RenderItemState(item, pDC, itemRect);
    // render the cores icon, this may depend on a selector sub item
    CRect iconRect(itemRect);
    iconRect.DeflateRect(6, 7, 5, 14);
    ASSERT(iconRect.Width() == 32);
    ASSERT(iconRect.Height() == 32);
    RenderItemSelection(item, pDC, iconRect);

    if (m_bCreateHitBoxes)
    {
        EnhancementHitBox hitBox(item, itemRect);
        m_hitBoxes.push_back(hitBox);
    }
}

void CEnhancementTreeDialog::RenderItemPassive(
        const EnhancementTreeItem & item,
        CDC * pDC,
        CRect itemRect)
{
    // it has a passive hexagonal border
    // it has a square active border
    VERIFY(s_imageBorders[IBE_ItemPassive].TransparentBlt(
            pDC->GetSafeHdc(),
            itemRect.left,
            itemRect.top,
            itemRect.Width(),
            itemRect.Height()) != 0);
    // show whether it can be trained or not
    RenderItemState(item, pDC, itemRect);
    // render the cores icon, this may depend on a selector sub item
    CRect iconRect(itemRect);
    iconRect.DeflateRect(6, 6, 5, 15);
    ASSERT(iconRect.Width() == 32);
    ASSERT(iconRect.Height() == 32);
    RenderItemSelection(item, pDC, iconRect);

    if (m_bCreateHitBoxes)
    {
        EnhancementHitBox hitBox(item, itemRect);
        m_hitBoxes.push_back(hitBox);
    }
}

void CEnhancementTreeDialog::RenderItemState(
        const EnhancementTreeItem & item,
        CDC * pDC,
        CRect itemRect)
{
    Build* pBuild = m_pCharacter->ActiveBuild();
    size_t spentInTree = pBuild->APSpentInTree(m_pTree->Name());
    bool isAllowed = item.IsAllowed(*pBuild, "", pBuild->Level()-1, spentInTree);
    bool isTier5Blocked = item.IsTier5Blocked(*pBuild, m_pTree->Name());
    bool isTrainable = item.CanTrain(*pBuild, spentInTree, m_type, pBuild->Level()-1);
    if (isTier5Blocked)
    {
        VERIFY(s_imageBorders[item.HasClickie() ? IBE_ActiveNotAllowed : IBE_PassiveNotAllowed].TransparentBlt(
                pDC->GetSafeHdc(),
                itemRect.left,
                itemRect.top,
                itemRect.Width(),
                itemRect.Height()) != 0);
    }
    else if (!isAllowed)
    {
        VERIFY(s_imageBorders[item.HasClickie() ? IBE_ActiveNotAvailable : IBE_PassiveNotAvailable].TransparentBlt(
                pDC->GetSafeHdc(),
                itemRect.left,
                itemRect.top,
                itemRect.Width(),
                itemRect.Height()) != 0);
    }
    else if (isTrainable)
    {
        // available outline is larger
        itemRect.InflateRect(4, 4, 4, 4);
        VERIFY(s_imageBorders[item.HasClickie() ? IBE_ActiveAvailable : IBE_PassiveAvailable].TransparentBlt(
                pDC->GetSafeHdc(),
                itemRect.left,
                itemRect.top,
                itemRect.Width(),
                itemRect.Height()) != 0);
    }
}

void CEnhancementTreeDialog::RenderItemSelection(
        const EnhancementTreeItem & item,
        CDC * pDC,
        const CRect & itemRect)
{
    // first determine the items icon
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        item.RenderIcon(*pBuild, pDC, itemRect);
    }
}

void CEnhancementTreeDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnLButtonDown(nFlags, point);
    if (m_pTree->Items().size() > 0)
    {
        if (GetKeyState(VK_SHIFT) < 0
            && GetKeyState(VK_CONTROL) < 0)
        {
            ApplyArrowToItem();
        }
        else
        {
            // determine which enhancement has been clicked on
            const EnhancementTreeItem * item = FindByPoint();
            if (item != NULL)
            {
                // an item has been clicked, see if we can train a rank in it
                Build* pBuild = m_pCharacter->ActiveBuild();
                size_t spentInTree = pBuild->APSpentInTree(m_pTree->Name());
                const TrainedEnhancement * te = pBuild->IsTrained(item->InternalName(), "");//, m_type);
                bool isAllowed = item->MeetRequirements(*pBuild, "", m_pTree->Name(), pBuild->Level()-1, spentInTree);
                bool isTrainable = item->CanTrain(*pBuild, spentInTree, m_type, pBuild->Level()-1);
                if (isAllowed && isTrainable)
                {
                    // if its a selector that needs selecting do selection, else
                    // just train it
                    if (item->HasSelections()
                            && te == NULL)
                    {
                        // need to show the selection dialog
                        CSelectionSelectDialog dlg(
                                this,
                                *pBuild,
                                *item,
                                m_pTree->Name(),
                                m_type);
                        // no tooltips while a dialog is displayed
                        GetMouseHook()->SaveState();
                        if (dlg.DoModal() == IDOK)
                        {
                            // they made a valid selection
                            switch (m_type)
                            {
                            case TT_universal:
                            case TT_racial:
                            case TT_enhancement:
                                pBuild->Enhancement_TrainEnhancement(
                                        m_pTree->Name(),
                                        item->InternalName(),
                                        dlg.Selection(),
                                        item->ItemCosts(dlg.Selection()));
                                break;
                            case TT_epicDestiny:
                                pBuild->Destiny_TrainEnhancement(
                                        m_pTree->Name(),
                                        item->InternalName(),
                                        dlg.Selection(),
                                        item->ItemCosts(dlg.Selection()));
                                break;
                            case TT_reaper:
                                pBuild->Reaper_TrainEnhancement(
                                        m_pTree->Name(),
                                        item->InternalName(),
                                        dlg.Selection(),
                                        item->ItemCosts(dlg.Selection()));
                                break;
                            }
                            Invalidate();
                        }
                        GetMouseHook()->RestoreState();
                    }
                    else
                    {
                        std::string selection = (te != NULL && te->HasSelection()) ? te->Selection() : "";
                        if (te == NULL          // if its not trained
                                || te->Ranks() < item->Ranks(selection)) // or has ranks still to be trained
                        {
                            switch (m_type)
                            {
                            case TT_universal:
                            case TT_racial:
                            case TT_enhancement:
                                pBuild->Enhancement_TrainEnhancement(
                                        m_pTree->Name(),
                                        item->InternalName(),
                                        selection,
                                        item->ItemCosts(selection));
                                break;
                            case TT_epicDestiny:
                                pBuild->Destiny_TrainEnhancement(
                                        m_pTree->Name(),
                                        item->InternalName(),
                                        selection,
                                        item->ItemCosts(selection));
                                break;
                            case TT_reaper:
                                pBuild->Reaper_TrainEnhancement(
                                        m_pTree->Name(),
                                        item->InternalName(),
                                        selection,
                                        item->ItemCosts(selection));
                                break;
                            }
                            Invalidate();
                        }
                    }
                }
                else
                {
                    // although this item cannot be trained, still allow the selection
                    // dialog to be displayed for item if it has one.
                    if (item->HasSelections()
                            && te == NULL)
                    {
                        // need to show the selection dialog
                        CSelectionSelectDialog dlg(
                                AfxGetApp()->m_pMainWnd->GetActiveWindow(),
                                *pBuild,
                                *item,
                                m_pTree->Name(),
                                m_type);
                        // no tooltips while a dialog is displayed
                        GetMouseHook()->SaveState();
                        dlg.DoModal();
                        GetMouseHook()->RestoreState();
                    }
                }
            }
            // check for a click of the reset tree button
            CRect rctResetButton(
                    c_sizeX - 79 - c_controlSpacing,
                    c_sizeY - 24 - c_controlSpacing,
                    c_sizeX - c_controlSpacing,
                    c_sizeY - c_controlSpacing);
            if (rctResetButton.PtInRect(point))
            {
                Build* pBuild = m_pCharacter->ActiveBuild();
                size_t spentInTree = pBuild->APSpentInTree(m_pTree->Name());
                if (spentInTree > 0)
                {
                    UINT ret = AfxMessageBox("Are you sure you want to reset this tree?", MB_YESNO | MB_ICONWARNING);
                    if (ret == IDYES)
                    {
                        // lets go for a tree reset action
                        switch (m_type)
                        {
                        case TT_universal:
                        case TT_racial:
                        case TT_enhancement:
                            pBuild->Enhancement_ResetEnhancementTree(m_pTree->Name());
                            break;
                        case TT_epicDestiny:
                            pBuild->Destiny_ResetEnhancementTree(m_pTree->Name());
                            break;
                        case TT_reaper:
                            pBuild->Reaper_ResetEnhancementTree(m_pTree->Name());
                            break;
                        }
                    }
                }
            }
            // optional tree drag and reposition option
            if (!m_pTree->HasIsRacialTree()
                    && !m_pTree->HasIsReaperTree()
                    && !m_pTree->HasIsEpicDestiny()
                    && m_pTree->Items().size() > 0)
            {
                CPoint mouse;
                GetCursorPos(&mouse);
                ScreenToClient(&mouse);
                if (mouse.x >= c_iconLeft && mouse.x < c_iconLeft + 32)
                {
                    if (mouse.y >= c_iconTop && mouse.y < c_iconTop + 32)
                    {
                        // start the tree drag operation
                        m_bDraggingTree = true;
                        SetCapture();
                    }
                }
            }
        }
    }
}

void CEnhancementTreeDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);
    if (m_bDraggingTree)
    {
        ReleaseCapture();
        m_bDraggingTree = false;
        // get the tree drag info
        CPoint mouse;
        GetCursorPos(&mouse);
        CWnd * pWnd = WindowFromPoint(mouse);
        // see if it can be converted to a CEnhancementTreeDialog *
        CEnhancementTreeDialog * pTarget = dynamic_cast<CEnhancementTreeDialog*>(pWnd);
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL
                && pTarget != NULL
                && pTarget != this)
        {
            if (pTarget->CanSwapTree())
            {
                // get the names of the two trees to swap
                std::string tree1 = m_pTree->Name();
                std::string tree2 = pTarget->m_pTree->Name();
                pBuild->Enhancement_SwapTrees(tree1, tree2);
            }
        }
    }
}

void CEnhancementTreeDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnRButtonDown(nFlags, point);
    // revoke the last enhancement trained in this tree
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        // determine which enhancement has been clicked on
        const EnhancementTreeItem * item = FindByPoint();
        if (item != NULL
                && item->CanRevoke(pBuild->FindSpendInTree(m_pTree->Name())))
        {
            switch (m_type)
            {
            case TT_universal:
            case TT_racial:
            case TT_enhancement:
                pBuild->Enhancement_RevokeEnhancement(m_pTree->Name(), item->InternalName());
                break;
            case TT_epicDestiny:
                pBuild->Destiny_RevokeEnhancement(m_pTree->Name(), item->InternalName());
                break;
            case TT_reaper:
                pBuild->Reaper_RevokeEnhancement(m_pTree->Name(), item->InternalName());
                break;
            }
            Invalidate();   // redraw
        }
    }
}

const EnhancementTreeItem * CEnhancementTreeDialog::FindByPoint(CRect * pRect) const
{
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    // identify the item under the cursor
    const EnhancementTreeItem * item = NULL;
    std::list<EnhancementHitBox>::const_iterator it = m_hitBoxes.begin();
    while (item == NULL && it != m_hitBoxes.end())
    {
        if ((*it).IsInRect(point))
        {
            // mouse is over this item
            item = &(*it).Item();
            if (pRect != NULL)
            {
                *pRect = (*it).Rect();
            }
        }
        ++it;
    }
    return item;
}

void CEnhancementTreeDialog::OnMouseMove(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);
    if (!m_bDraggingTree)
    {
        // determine which enhancement the mouse may be over
        CRect itemRect;
        const EnhancementTreeItem * item = FindByPoint(&itemRect);
        if (item != NULL
                && item != m_pTooltipItem)
        {
            // over a new item or a different item
            m_pTooltipItem = item;
            ShowTip(*item, itemRect);
        }
        else
        {
            if (m_showingTip
                    && item != m_pTooltipItem)
            {
                // no longer over the same item
                HideTip();
            }
        }
    }
    else
    {
        // ask the target window if it can accept this drop action
        CPoint mouse;
        GetCursorPos(&mouse);
        CWnd * pWnd = WindowFromPoint(mouse);
        // see if it can be converted to a CEnhancementTreeDialog *
        CEnhancementTreeDialog * pTarget = dynamic_cast<CEnhancementTreeDialog*>(pWnd);
        if (pTarget != NULL
                && pTarget != this)
        {
            // has to be a valid target tree that is not us
            if (pTarget->CanSwapTree())
            {
                SetCursor(LoadCursor(NULL, IDC_UPARROW));
            }
            else
            {
                SetCursor(LoadCursor(NULL, IDC_NO));
            }
        }
        else
        {
            // drag is in progress, but no valid target, override the mouse cu
            SetCursor(LoadCursor(NULL, IDC_SIZEWE));
        }
    }
}

LRESULT CEnhancementTreeDialog::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    // hide any tooltip when the mouse leaves the area its being shown for
    HideTip();
    return 0;
}

void CEnhancementTreeDialog::ShowTip(const EnhancementTreeItem & item, CRect itemRect)
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

void CEnhancementTreeDialog::HideTip()
{
    // tip not shown if not over an assay
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
        m_pTooltipItem = NULL;
    }
}

void CEnhancementTreeDialog::SetTooltipText(
        const EnhancementTreeItem & item,
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    UNREFERENCED_PARAMETER(item);
    UNREFERENCED_PARAMETER(tipTopLeft);
    UNREFERENCED_PARAMETER(tipAlternate);
    // different tooltip if the item is trained or not
    // if its trained, we need to show the selected sub-item tooltip
    Build* pBuild = m_pCharacter->ActiveBuild();
    const TrainedEnhancement * te = pBuild->IsTrained(item.InternalName(), "");//, m_type);
    const EnhancementSelection * es = NULL;
    std::string selection;
    m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
    if (te != NULL)
    {
        // this item is trained, we may need to show the selected sub-item tooltip text
        if (te->HasSelection())
        {
            selection = te->Selection();
            const Selector & selector = item.Selections();
            const std::list<EnhancementSelection> & selections = selector.Selections();
            // find the selected item
            std::list<EnhancementSelection>::const_iterator it = selections.begin();
            while (it != selections.end())
            {
                if ((*it).Name() == te->Selection())
                {
                    es = &(*it);
                    break;
                }
                ++it;
            }
        }
    }
    if (es != NULL)
    {
        m_tooltip.SetEnhancementSelectionItem(
                *pBuild,
                &item,
                es,
                pBuild->APSpentInTree(m_pTree->Name()),
                te->Ranks());
    }
    else
    {
        // its a top level item without sub-options
        m_tooltip.SetEnhancementTreeItem(
                *pBuild,
                &item,
                selection,
                pBuild->APSpentInTree(m_pTree->Name()));
    }
    m_tooltip.Show();
}

// LifeObserver overrides
void CEnhancementTreeDialog::UpdateActionPointsChanged(Life*)
{
    // if the number of available APs have changed, then some items buy
    // states may have changed if we have 4 or less APs available
    // (Note that 4 AP is the maximum cost of any specific enhancement - arcane archer tree)
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild->AvailableActionPoints(pBuild->Level(), m_type) <= 4)
    {
        // avoid unnecessary redraws when there will be no visual change
        Invalidate();
    }
}

// BuildObserver overrides
void CEnhancementTreeDialog::UpdateBuildLevelChanged(Build*)
{
    // selectable enhancements may have changed
    ValidateTreeSelections();
    Invalidate();
}

void CEnhancementTreeDialog::UpdateFeatTrained(
        Build* pBuild,
        const std::string& featName)
{
    UNREFERENCED_PARAMETER(pBuild);
    UNREFERENCED_PARAMETER(featName);
    // selectable enhancements may have changed
    Invalidate();
}

void CEnhancementTreeDialog::UpdateFeatRevoked(
        Build * pBuild,
        const std::string& featName)
{
    UNREFERENCED_PARAMETER(pBuild);
    UNREFERENCED_PARAMETER(featName);
    ValidateTreeSelections();
}

void CEnhancementTreeDialog::ValidateTreeSelections()
{
    // review all trained enhancements in this tree
    // if they are no longer valid, revoke enhancements in the tree
    // 
    //  we try an in place revoke first, else we revoke the last item
    //  trained in the tree until we can do an in place revoke and
    // all items are valid.
    bool bTreeHasInvalidItems = false;
    std::string treeName(m_pTree->Name());
    do 
    {
        bTreeHasInvalidItems = false;
        Build* pBuild = m_pCharacter->ActiveBuild();
        SpendInTree * esit = pBuild->FindSpendInTree(treeName);
        size_t spentInTree = pBuild->APSpentInTree(treeName);
        const EnhancementTreeItem* pBadItem = NULL;
        if (esit != NULL)
        {
            std::string revokedSelection;
            const std::list<TrainedEnhancement>& enhancements = esit->Enhancements();
            std::list<TrainedEnhancement>::const_iterator it = enhancements.begin();
            while (!bTreeHasInvalidItems && it != enhancements.end())
            {
                const EnhancementTreeItem* pItem = FindEnhancement((*it).EnhancementName());
                revokedSelection = (*it).HasSelection() ? (*it).Selection() : "";
                bool isAllowed = pItem->MeetRequirements(
                    *pBuild,
                    revokedSelection,
                    treeName,
                    pBuild->Level()-1,
                    spentInTree);
                if (!isAllowed)
                {
                    bTreeHasInvalidItems = true;
                    pBadItem = pItem;
                    break;
                }
                ++it;
            }
            if (bTreeHasInvalidItems)
            {
                // get the name of the tree item about to be revoked and add it to the
                // reported revoked message.
                std::string revokedEnhancement;
                // 1: can we revoke the bad enhancement directly?
                if (pBadItem->CanRevoke(esit))
                {
                    revokedEnhancement = pBadItem->InternalName();
                }
                else
                {
                    // revoke the last trained enhancement until we can revoke the bad one
                    revokedEnhancement = enhancements.back().EnhancementName();
                }
                // the tree does have invalid items in it.
                // revoke an item in this tree and try again to see if the tree is now valid
                pBuild->Enhancement_RevokeEnhancement(treeName, revokedEnhancement);
            }
        }
    } while (bTreeHasInvalidItems);
}

void CEnhancementTreeDialog::UpdateEnhancementTrained(
        Build*,
        const EnhancementItemParams&)
{
    Invalidate();
}

void CEnhancementTreeDialog::UpdateEnhancementRevoked(
        Build*,
        const EnhancementItemParams&)
{
    Invalidate();
}

void CEnhancementTreeDialog::UpdateActionPointsChanged(Build* pBuild)
{
    // if the number of available APs have changed, then some items buy
    // states may have changed if we have 4 or less APs available
    // (Note that 4 AP is the maximum cost of any specific enhancement - arcane archer tree)
    if (pBuild->AvailableActionPoints(pBuild->Level(), m_type) <= 4)
    {
        // avoid unnecessary redraws when there will be no visual change
        Invalidate();
    }
}

BOOL CEnhancementTreeDialog::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    // to be able to do a tree drag event it cannot be the racial, reaper or epic
    // destiny or an empty tree
    BOOL retVal = FALSE;

    if (!m_pTree->HasIsRacialTree()
            && !m_pTree->HasIsReaperTree()
            && !m_pTree->HasIsEpicDestiny()
            && m_pTree->Items().size() > 0)
    {
        CPoint mouse;
        GetCursorPos(&mouse);
        ScreenToClient(&mouse);
        if (mouse.x >= c_iconLeft && mouse.x < c_iconLeft + 32)
        {
            if (mouse.y >= c_iconTop && mouse.y < c_iconTop + 32)
            {
                // it is over the tree icon, show WE cursor
                SetCursor(LoadCursor(NULL, IDC_SIZEWE));
                retVal = TRUE;
            }
        }
    }

    if (retVal == FALSE)
    {
        // call base class if we didn't handle special case
        retVal = CDialog::OnSetCursor(pWnd, nHitTest, message);
    }
    return retVal;
}

bool CEnhancementTreeDialog::CanSwapTree() const
{
    bool canSwap = false;
    // can we accept this drop request?
    if (!m_pTree->HasIsRacialTree()
            && !m_pTree->HasIsReaperTree()
            && !m_pTree->HasIsEpicDestiny()
            && m_pTree->Items().size() > 0)
    {
        canSwap = true;
    }
    return canSwap;
}

void CEnhancementTreeDialog::OnCaptureChanged(CWnd* pWnd)
{
    UNREFERENCED_PARAMETER(pWnd);
    // something else has captured the mouse, abort drag operation
    if (m_bDraggingTree)
    {
        m_bDraggingTree = false;
    }
}

void CEnhancementTreeDialog::OnMButtonDown(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);
    if (GetKeyState(VK_SHIFT) < 0)
    {
        const EnhancementTreeItem * item = FindByPoint();
        if (item != NULL)
        {
            // may be trying to set for a sub item (if trained)
            CString iconName = item->Icon().c_str();
            const TrainedEnhancement* te = m_pCharacter->ActiveBuild()->IsTrained(item->InternalName(), "");
            if (te != NULL)
            {
                if (te->HasSelection())
                {
                    iconName = item->SelectionIcon(te->Selection()).c_str();
                }
            }
            CString prompt;
            prompt.Format("Make the Enhancement Image \"%s\" transparent?\r\n"
                "Yes - Normal Transparency for a passive icon\r\n"
                "No  - Transparency for a passive icon with top left +\r\n"
                "Cancel - Cancel the action\r\n", (LPCTSTR)iconName);
            int ret = AfxMessageBox(prompt, MB_YESNOCANCEL);
            CString fullIconLocation("EnhancementImages\\");
            fullIconLocation += iconName;
            if (IDYES == ret)
            {
                MakeIconTransparent(fullIconLocation, false);
            }
            else if (IDNO == ret)
            {
                MakeIconTransparent(fullIconLocation, true);
            }
        }
    }
    else
    {
        // copy the bitmap content to the clipboard
        CDC screenDC;
        screenDC.Attach(::GetDC(NULL));
        CDC clipboardDC;
        CDC bitmapDC;
        CBitmap clipboardBitmap;

        bitmapDC.CreateCompatibleDC(&screenDC);
        bitmapDC.SaveDC();
        bitmapDC.SelectObject(&m_cachedDisplay);
        // draw to a compatible device context and then copy to clipboard
        clipboardDC.CreateCompatibleDC(&screenDC);
        clipboardDC.SaveDC();
        clipboardBitmap.CreateCompatibleBitmap(
                &screenDC,
                c_sizeX,
                c_sizeY);
        clipboardDC.SelectObject(&clipboardBitmap);
        clipboardDC.BitBlt(0, 0, c_sizeX, c_sizeY, &bitmapDC, 0, 0, SRCCOPY);
        clipboardDC.RestoreDC(-1);
        clipboardDC.DeleteDC();
        ::ReleaseDC(NULL, screenDC.Detach());

        bitmapDC.RestoreDC(-1);
        bitmapDC.DeleteDC();

        // Open the clipboard
        if (::OpenClipboard(NULL))
        {
            ::EmptyClipboard();
            SetClipboardData(CF_BITMAP, clipboardBitmap.Detach());     // clipboard now owns the object
            ::CloseClipboard();
        }
        AfxMessageBox("Enhancement tree image copied to clipboard as a bitmap.", MB_ICONEXCLAMATION);
    }
}

void CEnhancementTreeDialog::ApplyArrowToItem()
{
    const EnhancementTreeItem* pConstItem = FindByPoint();
    if (pConstItem != NULL)
    {
        EnhancementTreeItem *item = const_cast<EnhancementTreeItem*>(pConstItem);
        // can only set arrows on non core items
        if (item->YPosition() > 0)
        {
            int ret = AfxMessageBox("What type of arrows to add?\r\n"
                "\r\n"
                "YES    <- Small Arrow Up\r\n"
                "NO     <-- Medium Arrow Up\r\n"
                "CANCEL <--- Long Arrow Up\r\n", MB_YESNOCANCEL);
            size_t offset = 0;
            switch (ret)
            {
                case IDYES:     offset = 1; break;
                case IDNO:      offset = 2; break;
                case IDCANCEL:  offset = 3; break;
            }
            EnhancementTreeItem* pTarget = m_pTree->FindItemByPosition(item->XPosition(), item->YPosition() + offset);
            if (pTarget != NULL)
            {
                CString text;
                text.Format("Are you sure you want to add an arrow to item:\r\n"
                        "%s\r\n"
                        "and a dependency on that item to item:\r\n"
                        "%s?", item->Name().c_str(), pTarget->Name().c_str());
                ret = AfxMessageBox(text, MB_YESNO);
                if (ret == IDYES)
                {
                    switch (offset)
                    {
                        case 1: item->Set_ArrowUp(); break;
                        case 2: item->Set_LongArrowUp(); break;
                        case 3: item->Set_ExtraLongArrowUp(); break;
                    }
                    // add a requirement to target item
                    Requirements requirements = pTarget->HasRequirementsToTrain() ? pTarget->RequirementsToTrain() : Requirements();
                    std::list<Requirement> reqs = requirements.Requires();
                    Requirement r(Requirement_Enhancement, item->InternalName());
                    reqs.push_back(r);
                    requirements.AddRequirement(r);
                    pTarget->Set_RequirementsToTrain(requirements);
                    m_pTree->Save();
                    // visually show changes
                    Invalidate();
                }
            }
            else
            {
                AfxMessageBox("No arrow applied as no target item", MB_ICONERROR);
            }
        }
    }
}
