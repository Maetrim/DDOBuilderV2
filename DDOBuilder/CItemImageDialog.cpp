// CItemImageDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "DDOBuilder.h"
#include "CItemImageDialog.h"
#include "afxdialogex.h"
#include "InventorySlotTypes.h"
#include "GlobalSupportFunctions.h"

// CItemImageDialog dialog

IMPLEMENT_DYNAMIC(CItemImageDialog, CDialogEx)

CItemImageDialog::CItemImageDialog(CWnd* pParent /*=nullptr*/) :
    CDialogEx(IDD_DIALOG_SETITEMIMAGES, pParent),
    m_nextItem(0),
    m_selectedItemType(Inventory_Unknown)
{

}

CItemImageDialog::~CItemImageDialog()
{
}

void CItemImageDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_ITEMTYPE, m_comboItemType);
    DDX_Control(pDX, IDC_LIST_ICONS, m_availableIcons);
}

BEGIN_MESSAGE_MAP(CItemImageDialog, CDialogEx)
    ON_CBN_SELCHANGE(IDC_COMBO_ITEMTYPE, &CItemImageDialog::OnCbnSelchangeComboItemtype)
    ON_BN_CLICKED(IDC_BUTTON_START, &CItemImageDialog::OnBnClickedButtonStart)
    ON_BN_CLICKED(IDC_BUTTON_SETITEMIMAGE, &CItemImageDialog::OnBnClickedButtonSetitemimage)
    ON_BN_CLICKED(IDC_BUTTON_SKIP, &CItemImageDialog::OnBnClickedButtonSkip)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH_IMAGES, &CItemImageDialog::OnBnClickedButtonRefreshImages)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ICONS, &CItemImageDialog::OnLvnItemchangedList1)
END_MESSAGE_MAP()

// CItemImageDialog message handlers

BOOL CItemImageDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    for (size_t i = Inventory_Belt; i < Inventory_Weapon1; ++i)
    {
        CString itemType = EnumEntryText((InventorySlotType)i, InventorySlotTypeMap);
        int index = m_comboItemType.AddString(itemType);
        m_comboItemType.SetItemData(index, i);
    }
    int index = m_comboItemType.AddString("Cloth Armor");
    m_comboItemType.SetItemData(index, Inventory_ArmorCloth);
    index = m_comboItemType.AddString("Light Armor");
    m_comboItemType.SetItemData(index, Inventory_ArmorLight);
    index = m_comboItemType.AddString("Medium Armor");
    m_comboItemType.SetItemData(index, Inventory_ArmorMedium);
    index = m_comboItemType.AddString("Heavy Armor");
    m_comboItemType.SetItemData(index, Inventory_ArmorHeavy);
    index = m_comboItemType.AddString("Docent");
    m_comboItemType.SetItemData(index, Inventory_ArmorDocent);
    index = m_comboItemType.AddString("Cosmetic Armor");
    m_comboItemType.SetItemData(index, Inventory_CosmeticArmor);

    m_comboItemType.SetCurSel(0);

    PopulateAvailableIcons();
    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SKIP)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CItemImageDialog::OnCbnSelchangeComboItemtype()
{
    PopulateAvailableIcons();
    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SKIP)->EnableWindow(FALSE);
}

void CItemImageDialog::OnBnClickedButtonStart()
{
    m_comboItemType.EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SKIP)->EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
    m_nextItem = 0;
    FindNextItem();
}

void CItemImageDialog::OnBnClickedButtonSetitemimage()
{
    // update the icon for this Item
    const std::list<Item>& items = Items();
    std::list<Item>::const_iterator it = items.begin();
    std::advance(it, m_nextItem);
    Item itemCopy = (*it);

    int sel = m_availableIcons.GetSelectionMark();
    CString iconFilename = m_availableIcons.GetItemText(sel, 0);
    itemCopy.Set_Icon((LPCTSTR)iconFilename);

    // save the updated item file
    try
    {
        XmlLib::SaxWriter writer;
        writer.Open(itemCopy.Filename());
        writer.StartDocument(L"Items");
        itemCopy.Write(&writer);
        writer.EndDocument();
    }
    catch (const std::exception& e)
    {
        std::string errorMessage = e.what();
        CString err;
        err.Format("...Failed to save item. Error \"%s\"", errorMessage.c_str());
        AfxMessageBox(err, MB_ICONERROR);
    }

    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(FALSE);
    m_availableIcons.SetSelectionMark(-1);
    ++m_nextItem;
    FindNextItem();
}

void CItemImageDialog::OnBnClickedButtonSkip()
{
    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(FALSE);
    m_availableIcons.SetSelectionMark(-1);
    ++m_nextItem;
    FindNextItem();
}

void CItemImageDialog::OnBnClickedButtonRefreshImages()
{
    PopulateAvailableIcons();
}

void CItemImageDialog::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    UNREFERENCED_PARAMETER(pNMLV);
    *pResult = 0;
    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(TRUE);
}

void CItemImageDialog::PopulateAvailableIcons()
{
    m_availableIcons.DeleteAllItems();
    // first load all the icons available associated with this item type
    int sel = m_comboItemType.GetCurSel();
    InventorySlotType ist = static_cast<InventorySlotType>(m_comboItemType.GetItemData(sel));
    std::string directory;
    switch (ist)
    {
    case Inventory_CosmeticArmor:directory = "Armor_Cosmetic\\"; break;
    case Inventory_ArmorCloth:   directory = "Armor_Cloth\\"; break;
    case Inventory_ArmorDocent:   directory = "Armor_Docent\\"; break;
    case Inventory_ArmorHeavy:   directory = "Armor_Heavy\\"; break;
    case Inventory_ArmorLight:   directory = "Armor_Light\\"; break;
    case Inventory_ArmorMedium:   directory = "Armor_Medium\\"; break;
    case Inventory_Belt:    directory = "Belts\\"; break;
    case Inventory_Boots:   directory = "Boots\\"; break;
    case Inventory_Bracers: directory = "Bracers\\"; break;
    case Inventory_CosmeticCloak:
    case Inventory_Cloak:   directory = "Cloaks\\"; break;
    case Inventory_Gloves:  directory = "Gloves\\"; break;
    case Inventory_Goggles: directory = "Goggles\\"; break;
    case Inventory_CosmeticHelm:
    case Inventory_Helmet:  directory = "Helmets\\"; break;
    case Inventory_Necklace:directory = "Necklace\\"; break;
    case Inventory_Quiver:  directory = "Quiver\\"; break;
    case Inventory_Ring1:
    case Inventory_Ring2:   directory = "Rings\\"; break;
    case Inventory_Trinket: directory = "Trinkets\\"; break;
    }
    m_images.DeleteImageList();
    m_imagesMap.clear();
    m_images.Create(32, 32, ILC_COLOR32, 100, 1000);
    std::string dataFolder = DataFolder();
    std::string fileFilter(dataFolder);
    fileFilter += "ItemImages\\";
    fileFilter += directory;
    fileFilter += "*.png";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(fileFilter.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        LoadImage(directory, findFileData.cFileName);
        while (FindNextFile(hFind, &findFileData))
        {
            LoadImage(directory, findFileData.cFileName);
        }
        FindClose(hFind);
    }
    // now populate the control
    m_availableIcons.SetImageList(&m_images, LVSIL_SMALL);
    m_availableIcons.SetImageList(&m_images, LVSIL_NORMAL);

    // add one entry for each image we have
    for (auto&& it : m_imagesMap)
    {
        int index = m_availableIcons.InsertItem(m_availableIcons.GetItemCount(), it.first.c_str(), it.second);
        m_availableIcons.SetItemData(index, it.second);
    }
    m_selectedItemType = ist;
}

void CItemImageDialog::LoadImage(const std::string& localPath, std::string filename)
{
    CImage image;
    std::string fullPath("DataFiles\\ItemImages\\");
    fullPath += localPath;
    size_t pos = filename.find(".png");
    if (pos != std::string::npos)
    {
        filename.replace(pos, 4, "");
    }
    if (S_OK == LoadImageFile(fullPath, filename, &image, CSize(32, 32), false))
    {
        CBitmap bitmap;
        bitmap.Attach(image.Detach());
        int imageIndex = m_images.Add(&bitmap, c_transparentColour);
        m_imagesMap[filename] = imageIndex;
    }
}

void CItemImageDialog::FindNextItem()
{
    bool bFound = false;
    const std::list<Item>& items = Items();
    while (!bFound && m_nextItem < items.size())
    {
        std::list<Item>::const_iterator it = items.begin();
        std::advance(it, m_nextItem);
        if ((*it).CanEquipToSlot(m_selectedItemType)
                && (*it).Icon() == "")
        {
            // found an item that equips to this slot type with no icon
            GetDlgItem(IDC_EDIT_ITEMNAME)->SetWindowText((*it).Name().c_str());
            bFound = true;
        }
        else
        {
            ++m_nextItem;
        }
    }
    if (!bFound)
    {
        m_comboItemType.EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_SKIP)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_ITEMNAME)->SetWindowText("");
        AfxMessageBox("All items of that type iterated");
    }
}
