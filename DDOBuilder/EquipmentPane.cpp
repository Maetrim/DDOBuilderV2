// EquipmentPane.cpp
//
#include "stdafx.h"
#include "EquipmentPane.h"

#include "DDOBuilderDoc.h"
#include "InventoryDialog.h"
#include "GlobalSupportFunctions.h"
#include "GearSetNameDialog.h"
#include "ItemSelectDialog.h"
//#include "FindGearDialog.h"
#include "MouseHook.h"
#include "XmlLib\SaxReader.h"
#include "MainFrm.h"
#include "LogPane.h"

namespace
{
    const int c_windowSizeX = 38;
    const int c_windowSizeY = 48;
}

IMPLEMENT_DYNCREATE(CEquipmentPane, CFormView)

CEquipmentPane::CEquipmentPane() :
    CFormView(CEquipmentPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL),
    m_inventoryView(NULL),
    m_tipCreated(false),
    m_pTooltipItem(NULL),
    m_showingTip(false),
    m_nextSetId(IDC_SPECIALFEAT_0),
    m_bLoadComplete(false)
{
}

CEquipmentPane::~CEquipmentPane()
{
    delete m_inventoryView;
    m_inventoryView = NULL;
    DestroyAllSets();
}

void CEquipmentPane::DestroyAllSets()
{
    for (size_t i = 0; i < m_setbuttons.size(); ++i)
    {
        m_setbuttons[i]->DestroyWindow();
        delete m_setbuttons[i];
        m_setbuttons[i] = NULL;
    }
    m_setbuttons.clear();
    m_nextSetId = IDC_SPECIALFEAT_0;
}

void CEquipmentPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_GEAR_NAME, m_comboGearSelections);
    DDX_Control(pDX, IDC_BUTTON_NEW, m_buttonNew);
    DDX_Control(pDX, IDC_BUTTON_COPY, m_buttonCopy);
    DDX_Control(pDX, IDC_BUTTON_PASTE, m_buttonPaste);
    DDX_Control(pDX, IDC_BUTTON_DELETE, m_buttonDelete);
    DDX_Control(pDX, IDC_BUTTON_IMPORT, m_buttonImport);
    DDX_Control(pDX, IDC_STATIC_NUM_FILIGREES, m_staticNumFiligrees);
    DDX_Control(pDX, IDC_COMBO_NUM_FILLIGREES, m_comboNumFiligrees);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CEquipmentPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_COMMAND(ID_GEAR_NEW, OnGearNew)
    ON_COMMAND(ID_GEAR_COPY, OnGearCopy)
    ON_COMMAND(ID_GEAR_PASTE, OnGearPaste)
    ON_COMMAND(ID_GEAR_DELETE, OnGearDelete)
    ON_COMMAND(ID_GEAR_IMPORT, OnGearImport)
    ON_UPDATE_COMMAND_UI(ID_GEAR_NEW, OnUpdateGearNew)
    ON_UPDATE_COMMAND_UI(ID_GEAR_COPY, OnUpdateGearCopy)
    ON_UPDATE_COMMAND_UI(ID_GEAR_PASTE, OnUpdateGearPaste)
    ON_UPDATE_COMMAND_UI(ID_GEAR_DELETE, OnUpdateGearDelete)
    ON_UPDATE_COMMAND_UI(ID_GEAR_IMPORT, OnUpdateGearImport)
    ON_BN_CLICKED(IDC_BUTTON_NEW, OnGearNew)
    ON_BN_CLICKED(IDC_BUTTON_COPY, OnGearCopy)
    ON_BN_CLICKED(IDC_BUTTON_PASTE, OnGearPaste)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, OnGearDelete)
    ON_BN_CLICKED(IDC_BUTTON_IMPORT, OnGearImport)
    ON_CBN_SELENDOK(IDC_COMBO_GEAR_NAME, OnGearSelectionSelEndOk)
    ON_CBN_SELENDOK(IDC_COMBO_NUM_FILLIGREES, OnGearNumFiligreesSelEndOk)
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CEquipmentPane::OnTtnNeedText)
END_MESSAGE_MAP()
#pragma warning(pop)

#ifdef _DEBUG
void CEquipmentPane::AssertValid() const
{
    CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEquipmentPane::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CEquipmentPane::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    if (!m_tipCreated)
    {
        m_tooltip.Create(this);
        m_tipCreated = true;
        CRect rctButton;
        CRect rctInventory(
                c_controlSpacing,
                rctButton.bottom + c_controlSpacing,
                c_controlSpacing + 223,
                rctButton.bottom + c_controlSpacing + 290);
        m_inventoryView = new CInventoryDialog(this);
        m_inventoryView->Create(CInventoryDialog::IDD, this);
        m_inventoryView->MoveWindow(&rctInventory);
        m_inventoryView->ShowWindow(SW_SHOW);
        m_inventoryView->AttachObserver(this);
        if (m_pCharacter != NULL)
        {
            Build* pBuild = m_pCharacter->ActiveBuild();
            if (pBuild != NULL)
            {
                m_inventoryView->SetGearSet(pBuild, pBuild->ActiveGearSet());
            }
        }
        // Images for new/copy/delete buttons
        m_buttonNew.SetImage(IDB_BITMAP_NEW);
        m_buttonCopy.SetImage(IDB_BITMAP_COPY);
        m_buttonPaste.SetImage(IDB_BITMAP_PASTE);
        m_buttonDelete.SetImage(IDB_BITMAP_DELETE);
        m_buttonImport.SetImage(IDB_BITMAP_IMPORT);

        EnableControls();
        EnableToolTips(TRUE);
    }
}

void CEquipmentPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (m_inventoryView != NULL)
    {
        // position all the windows
        // +------------------------------------------------------------------------+
        // | [Drop List Combo] [N][C][P][D][I] [Num Filigrees][List]     +-+ +-+ +-+|
        // | +-------------------------------+ +-----------------------+ +-+ +-+ +-+|
        // | |                               | |                       |(set icons) |
        // | | Inventory Bitmap              | | Filigrees Bitmap      | +-+ +-+ +-+|
        // | |                               | |                       | +-+ +-+ +-+|
        // | |                               | |                       |            |
        // | |                               | |                       | +-+ +-+ +-+|
        // | |                               | |                       | +-+ +-+ +-+|
        // | |                               | |                       |            |
        // | |                               | |                       |            |
        // | |                               | |                       |            |
        // | +-------------------------------+ +-----------------------+            |
        // +------------------------------------------------------------------------+
        CRect rctCombo;
        m_comboGearSelections.GetWindowRect(&rctCombo);
        rctCombo -= rctCombo.TopLeft();
        rctCombo += CPoint(c_controlSpacing, c_controlSpacing);
        CRect rctNew;
        CRect rctCopy;
        CRect rctPaste;
        CRect rctDelete;
        CRect rctImport;
        CRect rctNumFiligrees;
        CRect rctNumFiligreesCombo;
        m_buttonNew.GetWindowRect(&rctNew);
        m_buttonCopy.GetWindowRect(&rctCopy);
        m_buttonPaste.GetWindowRect(&rctPaste);
        m_buttonDelete.GetWindowRect(&rctDelete);
        m_buttonImport.GetWindowRect(&rctImport);

        rctImport -= rctImport.TopLeft();
        rctImport += CSize(223 + c_controlSpacing - rctImport.Width(), c_controlSpacing);
        rctDelete -= rctDelete.TopLeft();
        rctDelete += CSize(rctImport.left - c_controlSpacing - rctDelete.Width(), c_controlSpacing);
        rctPaste -= rctPaste.TopLeft();
        rctPaste += CSize(rctDelete.left - c_controlSpacing - rctPaste.Width(), c_controlSpacing);
        rctCopy -= rctCopy.TopLeft();
        rctCopy += CSize(rctPaste.left - c_controlSpacing - rctCopy.Width(), c_controlSpacing);
        rctNew -= rctNew.TopLeft();
        rctNew += CSize(rctCopy.left - c_controlSpacing - rctNew.Width(), c_controlSpacing);
        rctCombo.right = rctNew.left - c_controlSpacing;
        m_staticNumFiligrees.GetWindowRect(rctNumFiligrees);
        rctNumFiligrees -= rctNumFiligrees.TopLeft();
        rctNumFiligrees += CPoint(rctImport.right + c_controlSpacing, c_controlSpacing);
        m_comboNumFiligrees.GetWindowRect(rctNumFiligreesCombo);
        rctNumFiligreesCombo -= rctNumFiligreesCombo.TopLeft();
        rctNumFiligreesCombo += CPoint(rctNumFiligrees.right + c_controlSpacing, c_controlSpacing);
        rctNumFiligreesCombo.bottom = cy;   // drop list to bottom of view

        CRect rctInventory(
                c_controlSpacing,
                rctDelete.bottom + c_controlSpacing,
                c_controlSpacing + 418,
                rctDelete.bottom + c_controlSpacing + 385);
        m_comboGearSelections.MoveWindow(rctCombo);
        m_buttonNew.MoveWindow(rctNew);
        m_buttonCopy.MoveWindow(rctCopy);
        m_buttonPaste.MoveWindow(rctPaste);
        m_buttonDelete.MoveWindow(rctDelete);
        m_buttonImport.MoveWindow(rctImport);
        m_inventoryView->MoveWindow(rctInventory);
        m_staticNumFiligrees.MoveWindow(rctNumFiligrees);
        m_comboNumFiligrees.MoveWindow(rctNumFiligreesCombo);

        if (m_setbuttons.size() > 0)
        {
            CRect itemRect(rctInventory.right + c_controlSpacing, rctInventory.top,
                rctInventory.right + c_controlSpacing + c_windowSizeX, rctInventory.top + c_controlSpacing + c_windowSizeY);
            // Sets are always shown if they exist
            for (size_t i = 0; i < m_setbuttons.size(); ++i)
            {
                m_setbuttons[i]->MoveWindow(itemRect, TRUE);
                m_setbuttons[i]->ShowWindow(SW_SHOW);
                // move rectangle across for next control
                itemRect += CPoint(itemRect.Width() + c_controlSpacing, 0);
                if (itemRect.right > cx)
                {
                    // oops, not enough space in client area here
                    // move down and start the next row of controls
                    itemRect -= CPoint(itemRect.left, 0);
                    itemRect += CPoint(rctInventory.right + c_controlSpacing, itemRect.Height() + c_controlSpacing);
                }
            }
            SetScrollSizes(
                    MM_TEXT,
                    CSize(
                            rctInventory.right + c_controlSpacing,
                            itemRect.bottom + c_controlSpacing));
        }
        else
        {
            SetScrollSizes(
                    MM_TEXT,
                    CSize(
                            rctInventory.right + c_controlSpacing,
                            rctInventory.bottom + c_controlSpacing));
        }
    }
}

LRESULT CEquipmentPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
        DestroyAllSets();
    }

    // wParam is the document pointer
    CDDOBuilderDoc* pDoc = (CDDOBuilderDoc*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        if (IsWindow(GetSafeHwnd()))
        {
            SetScrollPos(SB_HORZ, 0, TRUE);
            SetScrollPos(SB_VERT, 0, TRUE);
        }
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
    PopulateCombobox();
    PopulateGear();
    EnableControls();
    return 0L;
}

LRESULT CEquipmentPane::OnLoadComplete(WPARAM, LPARAM)
{
    m_bLoadComplete = true;
    // buttons should update on the next OnIdle
    return 0;
}

BOOL CEquipmentPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_COMBO_GEAR_NAME,
        IDC_BUTTON_NEW,
        IDC_BUTTON_COPY,
        IDC_BUTTON_PASTE,
        IDC_BUTTON_DELETE,
        0 // end marker
    };

    if (m_inventoryView != NULL
            && ::IsWindow(m_inventoryView->GetSafeHwnd())
            && ::IsWindowVisible(m_inventoryView->GetSafeHwnd()))
    {
        CRect rctWnd;
        m_inventoryView->GetWindowRect(&rctWnd);
        ScreenToClient(&rctWnd);
        pDC->ExcludeClipRect(&rctWnd);
    }

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CEquipmentPane::PopulateCombobox()
{
    // show the list of selectable gear layouts
    m_comboGearSelections.LockWindowUpdate();
    m_comboGearSelections.ResetContent();
    bool selected = false;
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            const std::list<EquippedGear> & setups = pBuild->GearSetups();
            std::list<EquippedGear>::const_iterator it = setups.begin();
            size_t indexIntoList = 0;
            while (it != setups.end())
            {
                int index = m_comboGearSelections.AddString((*it).Name().c_str());
                if ((*it).Name() == pBuild->ActiveGear())
                {
                    m_comboGearSelections.SetCurSel(index);
                    selected = true;
                }
                ++indexIntoList;
                ++it;
            }
            if (!selected && setups.size() > 0)
            {
                // default to the first gear setup in the list
                m_comboGearSelections.SetCurSel(0);
            }
        }
    }
    m_comboGearSelections.UnlockWindowUpdate();

    // populate the Filigree combobox
    m_comboNumFiligrees.LockWindowUpdate();
    m_comboNumFiligrees.ResetContent();
    for (size_t i = 0; i <= MAX_FILIGREE; ++i)
    {
        CString text;
        text.Format("%d", i);
        m_comboNumFiligrees.AddString(text);
    }
    size_t count = 0;
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            count = pBuild->ActiveGearSet().NumFiligrees();
        }
    }
    m_comboNumFiligrees.SetCurSel(count);
    m_comboNumFiligrees.UnlockWindowUpdate();
}

std::string CEquipmentPane::SelectedGearSet() const
{
    std::string gearSet;
    int sel = m_comboGearSelections.GetCurSel();
    if (sel != CB_ERR)
    {
        CString name;
        m_comboGearSelections.GetLBText(sel, name);
        gearSet = (LPCTSTR)name;
    }
    return gearSet;
}

void CEquipmentPane::EnableControls()
{
    // controls disabled if no document
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    if (pBuild == NULL)
    {
        m_comboGearSelections.EnableWindow(FALSE);
        m_inventoryView->EnableWindow(FALSE);
        m_buttonNew.EnableWindow(FALSE);
        m_buttonCopy.EnableWindow(FALSE);
        m_buttonPaste.EnableWindow(FALSE);
        m_buttonDelete.EnableWindow(FALSE);
        m_buttonImport.EnableWindow(FALSE);
        m_staticNumFiligrees.EnableWindow(FALSE);
        m_comboNumFiligrees.EnableWindow(FALSE);
    }
    else
    {
        const std::list<EquippedGear> & setups = pBuild->GearSetups();
        m_comboGearSelections.EnableWindow(setups.size() > 1);
        m_inventoryView->EnableWindow(setups.size() > 0);
        m_buttonNew.EnableWindow(TRUE);     // always available
        m_buttonCopy.EnableWindow(setups.size() > 0);
        m_buttonPaste.EnableWindow(IsClipboardFormatAvailable(CF_PRIVATEFIRST));
        m_buttonDelete.EnableWindow(setups.size() > 0);
        m_buttonImport.EnableWindow(TRUE);
        m_staticNumFiligrees.EnableWindow(TRUE);
        m_comboNumFiligrees.EnableWindow(TRUE);
    }
}

void CEquipmentPane::PopulateGear()
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    if (m_inventoryView != NULL
            && pBuild != NULL)
    {
        m_inventoryView->SetGearSet(pBuild, pBuild->ActiveGearSet());
    }
    else
    {
        EquippedGear emptyGear;
        m_inventoryView->SetGearSet(NULL, emptyGear);
    }
}

// InventoryObserver overrides
void CEquipmentPane::UpdateSlotLeftClicked(
        CInventoryDialog*,
        InventorySlotType slot)
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    //if (slot == Inventory_FindItems)
    //{
    //    // no tooltips while a dialog is displayed
    //    GetMouseHook()->SaveState();
    //    CFindGearDialog dlg(this, m_pCharacter);
    //    dlg.DoModal();
    //    GetMouseHook()->RestoreState();
    //}
    //else
    {
        // determine the item selected in this slot already (if any)
        EquippedGear gear = pBuild->GetGearSet(SelectedGearSet());
        Item item;
        if (gear.HasItemInSlot(slot))
        {
            item = gear.ItemInSlot(slot);
        }
        if (gear.IsSlotRestricted(slot, pBuild))
        {
            // not allowed to equip in this due to item in weapon slot 1 or an item restricting this slot
            ::MessageBeep(MB_OK);
        }
        else
        {
            // no tooltips while a dialog is displayed
            GetMouseHook()->SaveState();
            CItemSelectDialog dlg(this, slot, item, pBuild);
            if (dlg.DoModal() == IDOK)
            {
                pBuild->SetGear(SelectedGearSet(), slot, dlg.SelectedItem());
                m_inventoryView->SetGearSet(pBuild, pBuild->ActiveGearSet());
            }
            GetMouseHook()->RestoreState();
        }
    }
}

void CEquipmentPane::UpdateSlotRightClicked(
        CInventoryDialog*,
        InventorySlotType slot)
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    if (slot == Inventory_FindItems)
    {
        // right click "Find Gear" does nothing
    }
    else
    {
        // option to clear the gear item from the selected slot
        EquippedGear gear = pBuild->GetGearSet(SelectedGearSet());
        if (gear.HasItemInSlot(slot))
        {
            int sel = AfxMessageBox("Clear the equipped item from this slot?", MB_YESNO);
            if (sel == IDYES)
            {
                pBuild->ClearGearInSlot(SelectedGearSet(), slot);
                m_inventoryView->SetGearSet(pBuild, pBuild->ActiveGearSet());
            }
        }
    }
}

//void CEquipmentPane::UpdateGearChanged(Character * charData, InventorySlotType slot)
//{
//    m_inventoryView->SetGearSet(m_pCharacter, m_pCharacter->ActiveGearSet());
//}

void CEquipmentPane::OnUpdateGearNew(CCmdUI * pCmdUi)
{
    // can always create a new gear set if a document is open and has a build
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    pCmdUi->Enable(m_bLoadComplete && pBuild != NULL);
}

void CEquipmentPane::OnUpdateGearCopy(CCmdUI * pCmdUi)
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        // can copy a gear set if we have an active one
        const std::list<EquippedGear> & setups = pBuild->GearSetups();
        pCmdUi->Enable(m_bLoadComplete && setups.size() > 0);
    }
    else
    {
        pCmdUi->Enable(FALSE);
    }
}

void CEquipmentPane::OnUpdateGearPaste(CCmdUI * pCmdUi)
{
    // can paste if we have data of the correct format available on the clipboard
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    bool enable = pBuild != NULL
            && ::IsClipboardFormatAvailable(CF_PRIVATEFIRST);
    pCmdUi->Enable(m_bLoadComplete && enable);
    m_buttonPaste.EnableWindow(m_bLoadComplete && enable);
}

void CEquipmentPane::OnUpdateGearDelete(CCmdUI * pCmdUi)
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        // can delete a gear set if we have an active one
        const std::list<EquippedGear> & setups = pBuild->GearSetups();
        pCmdUi->Enable(m_bLoadComplete && setups.size() > 0);
    }
    else
    {
        pCmdUi->Enable(FALSE);
    }
}

void CEquipmentPane::OnUpdateGearImport(CCmdUI* pCmdUi)
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    pCmdUi->Enable(m_bLoadComplete && pBuild != NULL);
    m_buttonImport.EnableWindow(m_bLoadComplete && pBuild != NULL);
}

void CEquipmentPane::OnGearNew()
{
    // no tooltips while a dialog is displayed
    GetMouseHook()->SaveState();
    // create a new gear set that they must name
    CGearSetNameDialog dlg(this, m_pCharacter);
    if (dlg.DoModal() == IDOK)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        // ensure the gear set name is unique
        if (!pBuild->DoesGearSetExist(dlg.Name()))
        {
            EquippedGear newGear(dlg.Name());
            pBuild->AddGearSet(newGear);
            // once added the new gear set automatically becomes active
            pBuild->SetActiveGearSet(dlg.Name());

            // add it to the combo box
            // index is the count of gear items
            int index = m_comboGearSelections.AddString(dlg.Name().c_str());
            // new entry starts selected
            m_comboGearSelections.SetCurSel(index);
            PopulateGear();
            // have the correct enable state
            EnableControls();
        }
        else
        {
            AfxMessageBox(
                    "Error: A gear set must have a unique name.\n"
                    "A gear set with the name you selected already exists.\n"
                    "Try again but use a different name.",
                    MB_ICONERROR);
        }
    }
    GetMouseHook()->RestoreState();
}

void CEquipmentPane::OnGearCopy()
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    // copy the current gear set to the clipboard as xml text using a custom
    // clipboard format to preserve any other data present on the clipboard
    HGLOBAL hData = NULL;
    if (pBuild!= NULL)
    {
        // get the current gear
        EquippedGear gear = pBuild->GetGearSet(SelectedGearSet());
        // write the gear as XML text
        XmlLib::SaxWriter writer;
        gear.Write(&writer);
        std::string xmlText = writer.Text();

        // read the file into a global memory handle
        hData = GlobalAlloc(GMEM_MOVEABLE, xmlText.size() + 1); // space for \0
        if (hData != NULL)
        {
            char * buffer = (char *)GlobalLock(hData);
            strcpy_s(buffer, xmlText.size() + 1, xmlText.data());
            GlobalUnlock(hData);
        }
        if (::OpenClipboard(NULL))
        {
            ::SetClipboardData(CF_PRIVATEFIRST, hData);
            ::CloseClipboard();
            // if we copied, paste is available
            m_buttonPaste.EnableWindow(true);
        }
        else
        {
            AfxMessageBox("Failed to open the clipboard.", MB_ICONERROR);
        }
    }
}

void CEquipmentPane::OnGearPaste()
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    if (pBuild != NULL
            && ::IsClipboardFormatAvailable(CF_PRIVATEFIRST))
    {
        if (::OpenClipboard(NULL))
        {
            // is the data in the right format
            HGLOBAL hGlobal = ::GetClipboardData(CF_PRIVATEFIRST);
            if (hGlobal != NULL)
            {
                // get the data as text from the clipboard
                std::string xmlText;
                char * buffer = (char *)::GlobalLock(hGlobal);
                xmlText = buffer;
                GlobalUnlock(hGlobal);
                ::CloseClipboard();

                // parse the XML text and read into a local object
                EquippedGear gear;
                XmlLib::SaxReader reader(&gear, gear.ElementName());
                bool ok = reader.ParseText(xmlText);
                if (ok)
                {
                    // looks like it read ok, we can progress
                    CGearSetNameDialog dlg(this, m_pCharacter);
                    if (dlg.DoModal() == IDOK)
                    {
                        // ensure the gear set name is unique
                        if (!m_pCharacter->ActiveBuild()->DoesGearSetExist(dlg.Name()))
                        {
                            gear.SetName(dlg.Name());
                            pBuild->AddGearSet(gear);
                            // once added the copy gear set automatically becomes active
                            pBuild->SetActiveGearSet(dlg.Name());

                            // add it to the combo box
                            // index is the count of gear items
                            int index = m_comboGearSelections.AddString(dlg.Name().c_str());
                            // new entry starts selected
                            m_comboGearSelections.SetCurSel(index);
                            PopulateGear();
                            // have the correct enable state
                            EnableControls();
                        }
                        else
                        {
                            // gear sets names must be unique, they can paste again
                            AfxMessageBox("A Gear Set with that name already exists.", MB_ICONERROR);
                        }
                    }
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
        AfxMessageBox("No Gear Set data available on the clipboard.", MB_ICONERROR);
    }
}

void CEquipmentPane::OnGearDelete()
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    // delete the current gear selection
    int sel = m_comboGearSelections.GetCurSel();
    if (sel != CB_ERR)
    {
        // get the name of the gear set to delete
        CString name;
        m_comboGearSelections.GetLBText(sel, name);
        // double check that is what they really want
        CString message;
        message.Format("Are you sure you want to delete the gear set called:\n"
            "\"%s\"?", (LPCTSTR)name);
        UINT ret = AfxMessageBox(message, MB_ICONQUESTION | MB_YESNO);
        if (ret == IDYES)
        {
            pBuild->DeleteGearSet((LPCTSTR)name);
            PopulateCombobox();
            // have the correct enable state
            EnableControls();
            // now show correct gear in inventory window
            PopulateGear();
        }
    }
}

void CEquipmentPane::OnGearImport()
{
    // display the file select dialog
    CFileDialog filedlg(
            TRUE,
            NULL,
            NULL,
            OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,
            "Gear Planner Files (*.gearset)|*.gearset|All files (*.*)|*.*||",
            this);
    if (filedlg.DoModal() == IDOK)
    {
        // update the filename
        CString name = filedlg.GetPathName();
        // we try and parse the data first before allowing the user to name the gear set
        EquippedGear newGear("");
        if (newGear.ImportFromFile(name))
        {
            // we successfully imported the item data
            // now allow the user to name the gear set
            // no tooltips while a dialog is displayed
            GetMouseHook()->SaveState();
            // create a new gear set that they must name
            CGearSetNameDialog dlg(this, m_pCharacter);
            if (dlg.DoModal() == IDOK)
            {
                // ensure the gear set name is unique
                Build* pBuild = m_pCharacter->ActiveBuild();
                if (!pBuild->DoesGearSetExist(dlg.Name()))
                {
                    newGear.Set_Name(dlg.Name());
                    pBuild->AddGearSet(newGear);
                    // once added the new gear set automatically becomes active
                    pBuild->SetActiveGearSet(dlg.Name());
                    // add it to the combo box
                    // index is the count of gear items
                    int index = m_comboGearSelections.AddString(dlg.Name().c_str());
                    // new entry starts selected
                    m_comboGearSelections.SetCurSel(index);
                    PopulateGear();
                    // have the correct enable state
                    EnableControls();
                    CString text;
                    text.Format("Gear set imported as \"%s\"", dlg.Name().c_str());
                    GetLog().AddLogEntry(text);
                }
                else
                {
                    AfxMessageBox(
                            "Error: A gear set must have a unique name.\n"
                            "A gear set with the name you selected already exists.\n"
                            "Try again but use a different name.",
                            MB_ICONERROR);
                }
            }
            GetMouseHook()->RestoreState();
        }
    }
}

void CEquipmentPane::OnGearSelectionSelEndOk()
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    int sel = m_comboGearSelections.GetCurSel();
    if (sel != CB_ERR)
    {
        CString name;
        m_comboGearSelections.GetLBText(sel, name);
        pBuild->SetActiveGearSet((LPCTSTR)name);
        PopulateGear();
    }
}

void CEquipmentPane::OnGearNumFiligreesSelEndOk()
{
    Build* pBuild = (m_pCharacter == NULL) ? NULL : m_pCharacter->ActiveBuild();
    int sel = m_comboNumFiligrees.GetCurSel();
    if (sel != CB_ERR)
    {
        // applies to active gear set
        pBuild->SetNumFiligrees(sel);
        PopulateGear();
    }
}

void CEquipmentPane::AddSetBonusStack(const SetBonus& setBonus)
{
    // only add the stance if it is not already present
    bool found = false;
    for (size_t i = 0; i < m_setbuttons.size(); ++i)
    {
        if (m_setbuttons[i]->IsYou(setBonus))
        {
            found = true;
            m_setbuttons[i]->AddStack();
        }
    }
    if (!found)
    {
        // position the created windows left to right until
        // they don't fit then move them down a row and start again
        // each stance window is c_windowSize * c_windowSize pixels
        CRect wndClient;
        GetClientRect(&wndClient);
        CRect itemRect(
                c_controlSpacing,
                c_controlSpacing,
                c_windowSizeX + c_controlSpacing,
                c_windowSizeY + c_controlSpacing);

        // now create the new auto stance control
        m_setbuttons.push_back(new CSetBonusButton(m_pCharacter, setBonus));
        // create a parent window that is c_windowSize by c_windowSize pixels in size
        m_setbuttons.back()->Create(
                "",
                WS_CHILD,       // default is not visible
                itemRect,
                this,
                m_nextSetId++);
        m_setbuttons.back()->AddStack();
        if (IsWindow(GetSafeHwnd()))
        {
            // now force an on size event to position everything
            CRect rctWnd;
            GetClientRect(&rctWnd);
            OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
        }
    }
}

void CEquipmentPane::RevokeSetBonusStack(const SetBonus& setBonus)
{
    // only revoke the stance if it is not already present and its the last stack of it
    bool found = false;
    size_t i;
    for (i = 0; i < m_setbuttons.size(); ++i)
    {
        if (m_setbuttons[i]->IsYou(setBonus))
        {
            found = true;
            m_setbuttons[i]->RevokeStack();
            break;      // keep the index
        }
    }
    if (found
            && m_setbuttons[i]->NumStacks() == 0)
    {
        // all instances of this stance are gone, remove the button
        m_setbuttons[i]->DestroyWindow();
        delete m_setbuttons[i];
        m_setbuttons[i] = NULL;
        // clear entries from the array
        std::vector<CSetBonusButton *>::iterator it = m_setbuttons.begin() + i;
        m_setbuttons.erase(it);
        // now force an on size event
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}

void CEquipmentPane::UpdateNewSetBonusStack(Build*, const SetBonus& setBonus)
{
    AddSetBonusStack(setBonus);
}

void CEquipmentPane::UpdateRevokeSetBonusStack(Build*, const SetBonus& setBonus)
{
    RevokeSetBonusStack(setBonus);
}

void CEquipmentPane::UpdateRaceChanged(Life*, const std::string&)
{
    // ensure gear re-draws on a race change
    Build* pBuild = m_pCharacter->ActiveBuild();
    m_inventoryView->SetGearSet(pBuild, pBuild->ActiveGearSet());
}

void CEquipmentPane::OnMouseMove(UINT, CPoint point)
{
    // determine which stance the mouse may be over
    CWnd * pWnd = ChildWindowFromPoint(point);
    CSetBonusButton * pSetBonus = dynamic_cast<CSetBonusButton*>(pWnd);
    if (pSetBonus != NULL
            && pSetBonus != m_pTooltipItem)
    {
        CRect itemRect;
        pSetBonus->GetWindowRect(&itemRect);
        ScreenToClient(itemRect);
        // over a new item or a different item
        m_pTooltipItem = pSetBonus;
        ShowTip(*pSetBonus, itemRect);
    }
    else
    {
        if (m_showingTip
                && pSetBonus != m_pTooltipItem)
        {
            // no longer over the same item
            HideTip();
        }
    }
}

LRESULT CEquipmentPane::OnMouseLeave(WPARAM, LPARAM)
{
    // hide any tooltip when the mouse leaves the area its being shown for
    HideTip();
    return 0;
}

void CEquipmentPane::ShowTip(const CSetBonusButton & item, CRect itemRect)
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

void CEquipmentPane::HideTip()
{
    // tip not shown if not over an assay
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
        m_pTooltipItem = NULL;
    }
}

void CEquipmentPane::SetTooltipText(
        const CSetBonusButton& item,
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
    m_tooltip.SetSetBonusItem(item.GetSetBonus(), item.NumStacks());
    m_tooltip.Show();
}

void CEquipmentPane::UpdateActiveLifeChanged(Character*)
{
    Life *pLife = m_pCharacter->ActiveLife();
    Build *pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
            && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
    }
    Invalidate();
    PopulateCombobox();
    EnableControls();
}

void CEquipmentPane::UpdateActiveBuildChanged(Character*)
{
    DestroyAllSets();
    if (m_pCharacter != NULL)
    {
        Life *pLife = m_pCharacter->ActiveLife();
        Build *pBuild = m_pCharacter->ActiveBuild();
        if (pLife != NULL
                && pBuild != NULL)
        {
            pLife->AttachObserver(this);
            pBuild->AttachObserver(this);
        }
        if (pBuild != NULL)
        {
            m_inventoryView->SetGearSet(pBuild, pBuild->ActiveGearSet());
        }
    }
    Invalidate();
    PopulateCombobox();
    EnableControls();
}

void CEquipmentPane::UpdateBuildLevelChanged(Build*)
{
    // gear may have been revoked due to level change
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        m_inventoryView->SetGearSet(pBuild, pBuild->ActiveGearSet());
    }
}

void CEquipmentPane::UpdateGearChanged(Build*, InventorySlotType)
{
    Invalidate();
    EnableControls();
}

BOOL CEquipmentPane::OnTtnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(pResult);

    UINT_PTR nID = pNMHDR->idFrom;
    nID = ::GetDlgCtrlID((HWND)nID);

    switch (nID)
    {
    case IDC_COMBO_GEAR_NAME:
        m_tipText = "This is the active gear set for this build\n"
                "Select another from the drop list if it is active.";
        break;
    case IDC_BUTTON_NEW:
        m_tipText = "Create a new named gear set for this build.";
        break;
    case IDC_BUTTON_COPY:
        m_tipText = "Copy the current  gear set to the clipboard.";
        break;
    case IDC_BUTTON_PASTE:
        m_tipText = "Paste the current  gear set from the clipboard to a new named gear set.";
        break;
    case IDC_BUTTON_DELETE:
        m_tipText = "Delete the active gear set from this build.";
        break;
    case IDC_BUTTON_IMPORT:
        m_tipText = "Import a .gearset file from the Gear Planner application.";
        break;
    case IDC_COMBO_NUM_FILLIGREES:
        m_tipText = "Set how many Weapon filigree slots you have for this gear set.";
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
