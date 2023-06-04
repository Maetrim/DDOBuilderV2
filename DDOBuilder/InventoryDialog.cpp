// InventoryDialog.cpp
//

#include "stdafx.h"
#include "InventoryDialog.h"

#include "Augment.h"
#include "GlobalSupportFunctions.h"
#include "SelectionSelectDialog.h"
#include "MainFrm.h"
#include "Filigree.h"
#include "Gem.h"
#include <algorithm>

namespace
{
    // inventory window size
    const int c_sizeX = 418;
    const int c_sizeY = 385;
    const int c_jewelX = 287;
    const int c_jewelY = 15;
    const int c_filigreeX = 232;
    const int c_filigreeY = 59;
    const int c_filigreeArtifactY = 275;
    const int c_filigreeXOffset = 36;
    const int c_filigreeYOffset = 51;
    const int c_sentientGem = -1;
    const int c_noSelection = -2;
    const int c_noMenuSelection = 0;
    const int c_clearMenuOption = 1;
    const int c_trueItemIndexOffset = 2;
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CInventoryDialog, CDialog)
    //{{AFX_MSG_MAP(CInventoryDialog)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()
#pragma warning(pop)

CInventoryDialog::CInventoryDialog(CWnd* pParent) :
    CDialog(CInventoryDialog::IDD, pParent),
    m_pBuild(NULL),
    m_bitmapSize(CSize(0, 0)),
    m_showingItemTip(false),
    m_showingFiligreeTip(false),
    m_showingAugmentTip(false),
    m_tipCreated(false),
    m_tooltipItem(Inventory_Unknown),
    m_filigreeIndex(0),
    m_bIsArtifactFiligree(false),
    m_tooltipFiligree(c_noSelection)
{
    //{{AFX_DATA_INIT(CInventoryDialog)
    //}}AFX_DATA_INIT
    // there are a fixed list of hit boxes which are hard coded here
    // order is important as used for drawing items (declare in same order as enum InventorySlotType)
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Arrows, CRect(164, 237, 196, 269)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Armor, CRect(30, 78, 62, 110)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Belt, CRect(164, 124, 196, 156)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Boots, CRect(72, 180, 104, 212)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Bracers, CRect(30, 124, 62, 156)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Cloak, CRect(164, 78, 196, 110)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Gloves, CRect(117, 180, 149, 212)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Goggles, CRect(30, 35, 62, 67)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Helmet, CRect(72, 24, 104, 56)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Necklace, CRect(117, 24, 149, 56)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Quiver, CRect(117, 237, 149, 269)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Ring1, CRect(30, 169, 62, 200)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Ring2, CRect(164, 168, 196, 200)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Trinket, CRect(164, 35, 196, 67)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_CosmeticArmor, CRect(93, 294, 125, 326)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_CosmeticCloak, CRect(133, 294, 155, 326)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_CosmeticHelm, CRect(53, 294, 85, 326)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_CosmeticWeapon1, CRect(73, 332, 105, 354)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_CosmeticWeapon2, CRect(113, 332, 145, 354)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Weapon1, CRect(30, 237, 62, 269)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_Weapon2, CRect(72, 237, 104, 269)));
    m_hitBoxesInventory.push_back(InventoryHitBox(Inventory_FindItems, CRect(99, 96, 128, 124)));

    // fixed hit boxes for Filigree items
    m_hitBoxesFiligrees.push_back(FiligreeHitBox(c_sentientGem, CRect(c_jewelX, c_jewelY, c_jewelX + 34, c_jewelY + 34)));
    CRect filigreeLocation(c_filigreeX, c_filigreeY, c_filigreeX + 34, c_filigreeY + 48);
    for (size_t i = 0; i < MAX_FILIGREE; ++i)
    {
        m_hitBoxesFiligrees.push_back(FiligreeHitBox(i, filigreeLocation));
        // move location for next filigree
        if ((i % 5) == 4)
        {
            // start of next row
            filigreeLocation += CPoint(c_filigreeXOffset * -4, c_filigreeYOffset);
        }
        else
        {
            // move across one
            filigreeLocation += CPoint(c_filigreeXOffset, 0);
        }
    }
    // now add the artifact filigree hit locations
    filigreeLocation = CRect(c_filigreeX, c_filigreeArtifactY, c_filigreeX + 34, c_filigreeArtifactY + 48);
    for (size_t i = 0; i < MAX_ARTIFACT_FILIGREE; ++i)
    {
        m_hitBoxesArtifactFiligrees.push_back(FiligreeHitBox(i, filigreeLocation));
        // move location for next filigree
        if ((i % 5) == 4)
        {
            // start of next row
            filigreeLocation += CPoint(c_filigreeXOffset * -4, c_filigreeYOffset);
        }
        else
        {
            // move across one
            filigreeLocation += CPoint(c_filigreeXOffset, 0);
        }
    }
    // load images used
    LoadImageFile("DataFiles\\UIImages\\", "Inventory", &m_imageBackground, CSize(418, 385), true);
    LoadImageFile("DataFiles\\UIImages\\", "Inventory", &m_imageBackgroundDisabled, CSize(418, 385), true);
    LoadImageFile("DataFiles\\UIImages\\", "CannotEquip", &m_imagesCannotEquip, CSize(32, 32), true);
    LoadImageFile("DataFiles\\UIImages\\", "SpellSlotTrainable", &m_imagesJewel, CSize(34, 34), true);
    LoadImageFile("DataFiles\\UIImages\\", "RareUnchecked", &m_imagesFiligree, CSize(34, 48), true);
    LoadImageFile("DataFiles\\UIImages\\", "RareChecked", &m_imagesFiligreeRare, CSize(34, 48), true);
    MakeGrayScale(&m_imageBackgroundDisabled, c_transparentColour);
}

void CInventoryDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CInventoryDialog)
    //}}AFX_DATA_MAP
}

void CInventoryDialog::SetGearSet(
        Build* pBuild,
        const EquippedGear & gear)
{
    m_pBuild = pBuild;
    m_gearSet = gear;
    Invalidate();
}

BOOL CInventoryDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_tooltip.Create(this);
    m_tipCreated = true;

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CInventoryDialog::OnEraseBkgnd(CDC* pDC)
{
    UNREFERENCED_PARAMETER(pDC);
    return 0;
}

void CInventoryDialog::OnPaint()
{
    CPaintDC pdc(this); // validates the client area on destruction
    pdc.SaveDC();
    // ensure we have a background bitmap
    if (m_bitmapSize == CSize(0, 0))
    {
        m_cachedDisplay.DeleteObject(); // ensure
        // create the bitmap we will render to
        m_cachedDisplay.CreateCompatibleBitmap(
                &pdc,
                c_sizeX,
                c_sizeY);
        m_bitmapSize = CSize(c_sizeX, c_sizeY);
    }
    // draw to a compatible device context and then splat to screen
    CDC memoryDc;
    memoryDc.CreateCompatibleDC(&pdc);
    memoryDc.SaveDC();
    memoryDc.SelectObject(&m_cachedDisplay);
    memoryDc.SelectStockObject(DEFAULT_GUI_FONT);

    // first render the inventory background
    if (IsWindowEnabled())
    {
        m_imageBackground.BitBlt(
                memoryDc.GetSafeHdc(),
                CRect(0, 0, c_sizeX, c_sizeY),
                CPoint(0, 0),
                SRCCOPY);
    }
    else
    {
        m_imageBackgroundDisabled.BitBlt(
                memoryDc.GetSafeHdc(),
                CRect(0, 0, c_sizeX, c_sizeY),
                CPoint(0, 0),
                SRCCOPY);
    }

    // now iterate the current inventory and draw the item icons
    for (size_t i = Inventory_Unknown + 1; i < Inventory_Count; ++i)
    {
        if (m_gearSet.IsSlotRestricted((InventorySlotType)i, m_pBuild))
        {
            CRect itemRect = m_hitBoxesInventory[i - 1].Rect();
            m_imagesCannotEquip.TransparentBlt(
                    memoryDc.GetSafeHdc(),
                    itemRect.left,
                    itemRect.top,
                    32,
                    32);
        }
        else if (m_gearSet.HasItemInSlot((InventorySlotType)i))
        {
            CRect itemRect = m_hitBoxesInventory[i - 1].Rect();
            POINT p;
            p.x = itemRect.left;
            p.y = itemRect.top;
            Item item = m_gearSet.ItemInSlot((InventorySlotType)i);
            ItemsImages().Draw(&memoryDc, item.IconIndex(), p, ILD_NORMAL);
            // show the augment slots this item has
            const std::vector<ItemAugment>& augments = item.Augments();
            CRect augmentRect = CRect(itemRect.left, itemRect.bottom + 1, itemRect.left + 6, itemRect.bottom + 7);
            for (auto&& ait: augments)
            {
                std::string colour = ait.Type();
                // no augment icons for mythic and reaper options
                if (colour != "Mythic" && colour.find("Reaper") == std::string::npos)
                {
                    memoryDc.SaveDC();
                    CPen augmentBorder(
                        PS_SOLID,
                        1,
                        ait.HasSelectedAugment()
                            ? RGB(0, 0, 0)              // black border if we have an augment
                            : RGB(255, 255, 255));      // white border if the no augment
                    memoryDc.SelectObject(augmentBorder);
                    COLORREF augmentColour = RGB(255, 255, 255);
                    if (colour == "Yellow") augmentColour = RGB(255, 255, 0);
                    else if (colour == "Red") augmentColour = RGB(255, 0, 0);
                    else if (colour == "Green") augmentColour = RGB(0, 255, 0);
                    else if (colour == "Blue") augmentColour = RGB(0, 0, 255);
                    else if (colour == "Purple") augmentColour = RGB(147, 112, 219);
                    else if (colour == "Orange") augmentColour = RGB(255, 165, 0);
                    else if (colour == "Colorless") augmentColour = RGB(255, 255, 255);
                    else augmentColour = RGB(128, 128, 128);            // all other augments are shown as gray
                    CBrush augmentFillBrush(augmentColour);
                    memoryDc.SelectObject(augmentFillBrush);
                    memoryDc.Rectangle(augmentRect);
                    memoryDc.RestoreDC(-1);
                    augmentRect += CPoint(augmentRect.Width(), 0);
                }
            }
        }
    }
    // now paint the Jewel and Filigrees
    if (IsWindowEnabled())
    {
        m_imagesJewel.TransparentBlt(
                memoryDc.GetSafeHdc(),
                c_jewelX,
                c_jewelY,
                34,
                34);
        if (m_gearSet.HasPersonality())
        {
            std::string personality = m_gearSet.Personality();
            const Gem& gem = FindSentientGemByName(personality);
            CImage image;
            LoadImageFile("DataFiles\\SentientGemImages\\", gem.Icon(), &image, CSize(32, 32));
            image.TransparentBlt(
                    memoryDc.GetSafeHdc(),
                    c_jewelX+1,
                    c_jewelY+1,
                    32,
                    32);
        }

        bool bHasArtifact = m_gearSet.HasMinorArtifact();
        // now draw all the weapon filigrees
        CSize filigreeLocation(c_filigreeX, c_filigreeY);
        size_t count = m_gearSet.NumFiligrees();
        for (size_t i = 0; i < count; ++i)
        {
            // and draw the selected filigree icon if there is one
            Filigree filigree = FindFiligreeByName(m_gearSet.GetFiligree(i));
            if (filigree.Name() != "")     // may not have a filigree selection
            {
                bool isRare = m_gearSet.IsRareFiligree(i);
                if (isRare)
                {
                    m_imagesFiligreeRare.TransparentBlt(
                            memoryDc.GetSafeHdc(),
                            filigreeLocation.cx,
                            filigreeLocation.cy,
                            34,
                            48);
                }
                else
                {
                    m_imagesFiligree.TransparentBlt(
                            memoryDc.GetSafeHdc(),
                            filigreeLocation.cx,
                            filigreeLocation.cy,
                            34,
                            48);
                }
                CImage image;
                LoadImageFile("DataFiles\\FiligreeImages\\", filigree.Icon(), &image, CSize(32, 32));
                image.TransparentBlt(
                        memoryDc.GetSafeHdc(),
                        filigreeLocation.cx+1,
                        filigreeLocation.cy+1,
                        32,
                        32);
            }
            else
            {
                // no rare option shown if no filigree selected
                m_imagesJewel.TransparentBlt(
                        memoryDc.GetSafeHdc(),
                        filigreeLocation.cx+1,
                        filigreeLocation.cy+1,
                        34,
                        34);
            }
            // move draw location for next filigree
            if ((i % 5) == 4)
            {
                // start of next row
                filigreeLocation.cx = c_filigreeX;
                filigreeLocation.cy += c_filigreeYOffset;
            }
            else
            {
                // move across one
                filigreeLocation.cx += c_filigreeXOffset;
            }
        }
        if (bHasArtifact)
        {
            filigreeLocation = CSize(c_filigreeX, c_filigreeArtifactY);
            for (size_t i = 0; i < MAX_ARTIFACT_FILIGREE; ++i)
            {
                // and draw the selected filigree icon if there is one
                Filigree filigree = FindFiligreeByName(m_gearSet.GetArtifactFiligree(i));
                if (filigree.Name() != "")     // may not have a filigree selection
                {
                    bool isRare = m_gearSet.IsRareArtifactFiligree(i);
                    if (isRare)
                    {
                        m_imagesFiligreeRare.TransparentBlt(
                                memoryDc.GetSafeHdc(),
                                filigreeLocation.cx,
                                filigreeLocation.cy,
                                34,
                                48);
                    }
                    else
                    {
                        m_imagesFiligree.TransparentBlt(
                                memoryDc.GetSafeHdc(),
                                filigreeLocation.cx,
                                filigreeLocation.cy,
                                34,
                                48);
                    }
                    CImage image;
                    LoadImageFile("DataFiles\\FiligreeImages\\", filigree.Icon(), &image, CSize(32, 32));
                    image.TransparentBlt(
                            memoryDc.GetSafeHdc(),
                            filigreeLocation.cx+1,
                            filigreeLocation.cy+1,
                            32,
                            32);
                }
                else
                {
                    // no rare option shown if no filigree selected
                    m_imagesJewel.TransparentBlt(
                            memoryDc.GetSafeHdc(),
                            filigreeLocation.cx+1,
                            filigreeLocation.cy+1,
                            34,
                            34);
                }
                // move draw location for next filigree
                if ((i % 5) == 4)
                {
                    // start of next row
                    filigreeLocation.cx = c_filigreeX;
                    filigreeLocation.cy += c_filigreeYOffset;
                }
                else
                {
                    // move across one
                    filigreeLocation.cx += c_filigreeXOffset;
                }
            }
        }
    }
    // now draw to display
    pdc.BitBlt(
            0,
            0,
            m_bitmapSize.cx,
            m_bitmapSize.cy,
            &memoryDc,
            0,
            0,
            SRCCOPY);
    memoryDc.RestoreDC(-1);
    memoryDc.DeleteDC();
    pdc.RestoreDC(-1);
}

void CInventoryDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnLButtonDown(nFlags, point);
    InventorySlotType slotClicked = FindItemByPoint(NULL);
    if (slotClicked != Inventory_Unknown)
    {
        NotifySlotLeftClicked(slotClicked);
    }
    else
    {
        // also check if they clicked on a filigree
        if (IsWindowEnabled())
        {
            int filigreeIndex = 0;
            bool isRareSection = false;
            bool isArtifactFiligree = false;
            CRect itemRect;
            bool bFiligree = FindFiligreeByPoint(&filigreeIndex, &isRareSection, &isArtifactFiligree, &itemRect);
            if (bFiligree)
            {
                if (filigreeIndex < 0)
                {
                    EditSentientGem(filigreeIndex, itemRect);   // m_gearSet personality
                }
                else
                {
                    // if we have a filigree in this location allow it to be changed or
                    // toggle the rare state of this filigree
                    std::string clickedFiligree;
                    if (isArtifactFiligree)
                    {
                        clickedFiligree = m_gearSet.GetArtifactFiligree(filigreeIndex);
                    }
                    else
                    {
                        clickedFiligree = m_gearSet.GetFiligree(filigreeIndex);
                    }
                    if (clickedFiligree.empty()
                            || !isRareSection)
                    {
                        CreateFiligreeMenu(filigreeIndex, isArtifactFiligree, clickedFiligree.empty());
                        ClientToScreen(&itemRect);
                        CWinAppEx * pApp = dynamic_cast<CWinAppEx*>(AfxGetApp());
                        UINT sel = pApp->GetContextMenuManager()->TrackPopupMenu(
                                m_filigreeMenu.GetSafeHmenu(),
                                itemRect.left,
                                itemRect.top,
                                this);
                        if (sel > c_noMenuSelection)    // sel is 0 if menu is cancelled
                        {
                            if (sel == c_clearMenuOption)   // clear augment option
                            {
                                if (isArtifactFiligree)
                                {
                                    m_gearSet.SetArtifactFiligreeRare(filigreeIndex, false);
                                    m_gearSet.SetArtifactFiligree(filigreeIndex, "");
                                }
                                else
                                {
                                    m_gearSet.SetFiligreeRare(filigreeIndex, false);
                                    m_gearSet.SetFiligree(filigreeIndex, "");
                                }
                                m_pBuild->UpdateActiveGearSet(m_gearSet);
                                Invalidate();
                            }
                            else
                            {
                                const std::list<Filigree>& filigrees = Filigrees();
                                std::list<Filigree>::const_iterator cit = filigrees.begin();
                                std::advance(cit, sel - c_trueItemIndexOffset);
                                if (cit != filigrees.end())
                                {
                                    std::string selectedItem = (*cit).Name();
                                    if (isArtifactFiligree)
                                    {
                                        m_gearSet.SetArtifactFiligree(filigreeIndex, selectedItem);
                                        m_pBuild->UpdateActiveGearSet(m_gearSet);
                                        Invalidate();
                                    }
                                    else
                                    {
                                        m_gearSet.SetFiligree(filigreeIndex, selectedItem);
                                        m_pBuild->UpdateActiveGearSet(m_gearSet);
                                        Invalidate();
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        ToggleRareState(filigreeIndex, isArtifactFiligree);
                    }
                }
            }
        }
    }
}

void CInventoryDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnRButtonDown(nFlags, point);
    InventorySlotType slotClicked = FindItemByPoint(NULL);
    if (slotClicked != Inventory_Unknown)
    {
        NotifySlotRightClicked(slotClicked);
    }
    else
    {
        // also check if they clicked on a filigree
        if (IsWindowEnabled())
        {
            int filigree = 0;
            bool isRareSection = false;
            bool isArtifactFiligree = false;
            CRect itemRect;
            bool bFiligree = FindFiligreeByPoint(&filigree, &isRareSection, &isArtifactFiligree, &itemRect);
            if (bFiligree)
            {
                // clear this Filigree/m_gearSet
                if (filigree >= 0)
                {
                    if (isArtifactFiligree)
                    {
                        m_gearSet.SetArtifactFiligreeRare(filigree, false);
                        m_gearSet.SetArtifactFiligree(filigree, "");
                    }
                    else
                    {
                        m_gearSet.SetFiligreeRare(filigree, false);
                        m_gearSet.SetFiligree(filigree, "");
                    }
                }
                else
                {
                    m_gearSet.SetPersonality("");
                }
                m_pBuild->UpdateActiveGearSet(m_gearSet);
                Invalidate();
            }
        }
    }
}

InventorySlotType CInventoryDialog::FindItemByPoint(CRect * pRect) const
{
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    // see if we need to highlight the item under the cursor
    InventorySlotType slot = Inventory_Unknown;
    std::vector<InventoryHitBox>::const_iterator it = m_hitBoxesInventory.begin();
    while (slot == Inventory_Unknown && it != m_hitBoxesInventory.end())
    {
        if ((*it).IsInRect(point))
        {
            // mouse is over this item
            slot = (*it).Slot();
            if (pRect != NULL)
            {
                *pRect = (*it).Rect();
            }
        }
        ++it;
    }
    return slot;
}

bool CInventoryDialog::FindFiligreeByPoint(
        int * filigree,
        bool * isRareSection,
        bool * bArtifactFiligree,
        CRect * pRect) const
{
    bool found = false;
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    size_t count = m_gearSet.NumFiligrees();
    bool bMinorArtifact = m_gearSet.HasMinorArtifact();
    // see if we need to highlight the item under the cursor
    std::vector<FiligreeHitBox>::const_iterator it = m_hitBoxesFiligrees.begin();
    while (!found && it != m_hitBoxesFiligrees.end())
    {
        if ((*it).IsInRect(point))
        {
            // mouse is over this item
            // but the item must exist (slot always exists for m_gearSet (c_sentientGem))
            if ((*it).Slot() < (int)count)
            {
                int yPos = point.y - (*it).Rect().top;
                *isRareSection =  (yPos > 34);  // true if click rare part
                *filigree = (*it).Slot();
                if (pRect != NULL)
                {
                    *pRect = (*it).Rect();
                }
                found = true;
            }
        }
        ++it;
    }
    if (bMinorArtifact)
    {
        it = m_hitBoxesArtifactFiligrees.begin();
        while (!found && it != m_hitBoxesArtifactFiligrees.end())
        {
            if ((*it).IsInRect(point))
            {
                // mouse is over this item
                if ((*it).Slot() < MAX_ARTIFACT_FILIGREE)
                {
                    int yPos = point.y - (*it).Rect().top;
                    *isRareSection =  (yPos > 34);  // true if click rare part
                    *filigree = (*it).Slot();
                    *bArtifactFiligree = true;
                    if (pRect != NULL)
                    {
                        *pRect = (*it).Rect();
                    }
                    found = true;
                }
            }
            ++it;
        }
    }
    return found;
}

void CInventoryDialog::OnMouseMove(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);
    // determine which item the mouse may be over
    CRect itemRect;
    InventorySlotType slot = FindItemByPoint(&itemRect);
    int filigreeIndex = 0;
    bool isRareSection = false;
    bool isArtifactFiligree = false;
    bool bFiligree = FindFiligreeByPoint(&filigreeIndex, &isRareSection, &isArtifactFiligree, &itemRect);
    if (slot != Inventory_Unknown)
            //&& slot != Inventory_FindItems)
    {
        if (slot != m_tooltipItem)
        {
            // over a new item or a different item
            if (m_gearSet.HasItemInSlot(slot))
            {
                m_tooltipItem = slot;
                ShowTip(m_gearSet.ItemInSlot(slot), itemRect);
            }
        }
    }
    else if (bFiligree)
    {
        // over a sentient gem/filigree item
        if (filigreeIndex == c_sentientGem)
        {
            if (m_tooltipFiligree != filigreeIndex)
            {
                if (m_gearSet.HasPersonality())
                {
                    m_tooltipFiligree = filigreeIndex;
                    std::string personality = m_gearSet.Personality();
                    Gem gem = FindSentientGemByName(personality);
                    ShowTip(gem, itemRect);
                }
                else
                {
                    HideTip();
                }
            }
        }
        else    // filigree
        {
            if (isArtifactFiligree)
            {
                if (m_tooltipFiligree != filigreeIndex)
                {
                    std::string filigreeName = m_gearSet.GetArtifactFiligree(filigreeIndex);
                    if (filigreeName.size() != 0)
                    {
                        m_tooltipFiligree = filigreeIndex;
                        const Filigree& filigree = FindFiligreeByName(filigreeName);
                        ShowTip(filigree, itemRect);
                    }
                    else
                    {
                        HideTip();
                    }
                }
            }
            else
            {
                if (m_tooltipFiligree != filigreeIndex)
                {
                    std::string filigreeName = m_gearSet.GetFiligree(filigreeIndex);
                    if (filigreeName.size() != 0)
                    {
                        m_tooltipFiligree = filigreeIndex;
                        const Filigree& filigree = FindFiligreeByName(filigreeName);
                        ShowTip(filigree, itemRect);
                    }
                    else
                    {
                        HideTip();
                    }
                }
            }
        }
    }
    else
    {
        HideTip();
    }
}

LRESULT CInventoryDialog::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    // hide any tooltip when the mouse leaves the area its being shown for
    HideTip();
    return 0;
}

void CInventoryDialog::ShowTip(const Item & item, CRect itemRect)
{
    HideTip();
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    CPoint tipAlternate(itemRect.left, itemRect.top - 2);
    SetTooltipText(item, tipTopLeft, tipAlternate);
    m_showingItemTip = true;
    // track the mouse so we know when it leaves our window
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;
    _TrackMouseEvent(&tme);
}

void CInventoryDialog::ShowTip(const Filigree& filigree, CRect itemRect)
{
    HideTip();
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    CPoint tipAlternate(itemRect.left, itemRect.top - 2);
    SetTooltipText(filigree, tipTopLeft, tipAlternate, false);
    m_showingFiligreeTip = true;
    // track the mouse so we know when it leaves our window
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;
    _TrackMouseEvent(&tme);
}

void CInventoryDialog::ShowTip(const Gem& gem, CRect itemRect)
{
    HideTip();
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    CPoint tipAlternate(itemRect.left, itemRect.top - 2);
    SetTooltipText(gem, tipTopLeft, tipAlternate, false);
    m_showingAugmentTip = true;
    // track the mouse so we know when it leaves our window
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;
    _TrackMouseEvent(&tme);
}

void CInventoryDialog::HideTip()
{
    // tip not shown if not over a gear slot
    if (m_tipCreated && (m_showingItemTip || m_showingFiligreeTip || m_showingAugmentTip))
    {
        m_tooltip.Hide();
        m_showingItemTip = false;
        m_showingFiligreeTip = false;
        m_showingAugmentTip = false;
        m_tooltipItem = Inventory_Unknown;
        m_tooltipFiligree = c_noSelection;
    }
}

void CInventoryDialog::SetTooltipText(
        const Item & item,
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
    m_tooltip.SetItem(&item);
    m_tooltip.Show();
}

void CInventoryDialog::SetTooltipText(
        const Filigree& filigree,
        CPoint tipTopLeft,
        CPoint tipAlternate,
        bool rightAlign)
{
    m_tooltip.SetOrigin(tipTopLeft, tipAlternate, rightAlign);
    m_tooltip.SetFiligree(&filigree);
    m_tooltip.Show();
}

void CInventoryDialog::SetTooltipText(
        const Gem& gem,
        CPoint tipTopLeft,
        CPoint tipAlternate,
        bool rightAlign)
{
    m_tooltip.SetOrigin(tipTopLeft, tipAlternate, rightAlign);
    m_tooltip.SetSentientGem(&gem);
    m_tooltip.Show();
}

CRect CInventoryDialog::GetItemRect(InventorySlotType slot) const
{
    // iterate the list of hit boxes looking for the item that matches
    CRect itemRect(0, 0, 0, 0);
    bool found = false;
    std::vector<InventoryHitBox>::const_iterator it = m_hitBoxesInventory.begin();
    while (!found && it != m_hitBoxesInventory.end())
    {
        if ((*it).Slot() == slot)
        {
            found = true;
            itemRect = (*it).Rect();
            break;
        }
        ++it;
    }
    ASSERT(found);
    return itemRect;
}

void CInventoryDialog::NotifySlotLeftClicked(InventorySlotType slot)
{
    NotifyAll(&InventoryObserver::UpdateSlotLeftClicked, this, slot);
}

void CInventoryDialog::NotifySlotRightClicked(InventorySlotType slot)
{
    NotifyAll(&InventoryObserver::UpdateSlotRightClicked, this, slot);
}

void CInventoryDialog::EditSentientGem(int filigreeIndex, CRect itemRect)
{
    UNREFERENCED_PARAMETER(itemRect);
    // hide the current tip
    HideTip();

    if (filigreeIndex < 0)
    {
        CreatePersonalityMenu();
        ClientToScreen(&itemRect);
        CWinAppEx * pApp = dynamic_cast<CWinAppEx*>(AfxGetApp());
        UINT sel = pApp->GetContextMenuManager()->TrackPopupMenu(
                m_personalityMenu.GetSafeHmenu(),
                itemRect.left,
                itemRect.top,
                this);
        if (sel > c_noMenuSelection)        // sel is 0 if menu is cancelled
        {
            if (sel == c_clearMenuOption)
            {
                // first option is always clear Personality
                m_gearSet.ClearPersonality();
            }
            else
            {
                sel -= c_trueItemIndexOffset;
                const std::list<Gem>& personalities = SentientGems();
                std::list<Gem>::const_iterator sit = personalities.begin();
                std::advance(sit, sel);
                std::string name = (*sit).Name();
                m_gearSet.SetPersonality(name);
            }
            m_pBuild->UpdateActiveGearSet(m_gearSet);
            Invalidate(FALSE);  // ensure display updates
        }
    }
}

void CInventoryDialog::OnCancel()
{
    // do nothing to stop dialog being dismissed
}

void CInventoryDialog::ToggleRareState(int filigree, bool isArtifactFiligree)
{
    if (isArtifactFiligree)
    {
        m_gearSet.SetArtifactFiligreeRare(filigree, !m_gearSet.IsRareArtifactFiligree(filigree));
    }
    else
    {
        m_gearSet.SetFiligreeRare(filigree, !m_gearSet.IsRareFiligree(filigree));
    }
    m_pBuild->UpdateActiveGearSet(m_gearSet);
    Invalidate();
}

void CInventoryDialog::CreatePersonalityMenu()
{
    m_personalityMenu.DestroyMenu();
    m_personalityMenu.CreatePopupMenu();
    // always have a clear personality option
    AddMenuItem(m_personalityMenu.GetSafeHmenu(), "Clear Personality", c_clearMenuOption, m_gearSet.HasPersonality());
    // create a menu for item for each personality
    const std::list<Gem>& personalities = SentientGems();
    int iItemIndex = c_trueItemIndexOffset;
    for (auto&& pit: personalities)
    {
        CString fullName = pit.Name().c_str();
        AddMenuItem(m_personalityMenu.GetSafeHmenu(), fullName, iItemIndex, true);
        iItemIndex++;
    }
}

void CInventoryDialog::CreateFiligreeMenu(int filigreeIndex, bool bArtifact, bool bEmptyFiligreeSlot)
{
    m_filigreeMenu.DestroyMenu();
    m_filigreeMenu.CreatePopupMenu();
    // always have a clear filigree option
    AddMenuItem(m_filigreeMenu.GetSafeHmenu(), "Clear Filigree", c_clearMenuOption, !bEmptyFiligreeSlot);
    // create a new sub menu for each filigree group
    int iItemIndex = c_trueItemIndexOffset;
    const std::list<Filigree>& filigrees = Filigrees();
    for (auto&& fit: filigrees)
    {
        CString fullName = fit.Menu().c_str();
        fullName += "\\";
        fullName += fit.Name().c_str();
        fullName.Replace(": ", "\\");
        bool bEnabled = true;
        if (bArtifact)
        {
            for (size_t fi = 0; fi < MAX_ARTIFACT_FILIGREE; ++fi)
            {
                // do not remove any selection from current slot
                if (fi != static_cast<size_t>(filigreeIndex))
                {
                    std::string filigree = m_gearSet.GetArtifactFiligree(fi);
                    if (fit.Name() == filigree)
                    {
                        bEnabled = false;   // its already present
                    }
                }
            }
        }
        else
        {
            for (size_t fi = 0; fi < MAX_FILIGREE; ++fi)
            {
                // do not remove any selection from current slot
                if (fi != static_cast<size_t>(filigreeIndex))
                {
                    // get current selection (if any)
                    std::string filigree = m_gearSet.GetFiligree(fi);
                    if (fit.Name() == filigree)
                    {
                        bEnabled = false;   // its already present
                    }
                }
            }
        }
        AddMenuItem(m_filigreeMenu.GetSafeHmenu(), fullName, iItemIndex, bEnabled);
        iItemIndex++;
    }
}
