// CWeaponImageDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "DDOBuilder.h"
#include "CWeaponImageDialog.h"
#include "afxdialogex.h"
#include "InventorySlotTypes.h"
#include "GlobalSupportFunctions.h"

// CWeaponImageDialog dialog

IMPLEMENT_DYNAMIC(CWeaponImageDialog, CDialogEx)

CWeaponImageDialog::CWeaponImageDialog(CWnd* pParent /*=nullptr*/) :
    CDialogEx(IDD_DIALOG_SETITEMIMAGES, pParent),
    m_nextItem(0),
    m_selectedItemType(Weapon_Unknown)
{

}

CWeaponImageDialog::~CWeaponImageDialog()
{
}

void CWeaponImageDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_ITEMTYPE, m_comboItemType);
    DDX_Control(pDX, IDC_LIST_ICONS, m_availableIcons);
}

BEGIN_MESSAGE_MAP(CWeaponImageDialog, CDialogEx)
    ON_CBN_SELCHANGE(IDC_COMBO_ITEMTYPE, &CWeaponImageDialog::OnCbnSelchangeComboItemtype)
    ON_BN_CLICKED(IDC_BUTTON_START, &CWeaponImageDialog::OnBnClickedButtonStart)
    ON_BN_CLICKED(IDC_BUTTON_SETITEMIMAGE, &CWeaponImageDialog::OnBnClickedButtonSetitemimage)
    ON_BN_CLICKED(IDC_BUTTON_SKIP, &CWeaponImageDialog::OnBnClickedButtonSkip)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH_IMAGES, &CWeaponImageDialog::OnBnClickedButtonRefreshImages)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ICONS, &CWeaponImageDialog::OnLvnItemchangedList1)
END_MESSAGE_MAP()

// CWeaponImageDialog message handlers

BOOL CWeaponImageDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    for (size_t i = Weapon_StartOfList; i < Weapon_Count; ++i)
    {
        CString itemType = EnumEntryText((WeaponType)i, weaponTypeMap);
        int index = m_comboItemType.AddString(itemType);
        m_comboItemType.SetItemData(index, i);
    }
    m_comboItemType.SetCurSel(0);

    PopulateAvailableIcons();
    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SKIP)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CWeaponImageDialog::OnCbnSelchangeComboItemtype()
{
    PopulateAvailableIcons();
    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SKIP)->EnableWindow(FALSE);
}

void CWeaponImageDialog::OnBnClickedButtonStart()
{
    m_comboItemType.EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SKIP)->EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
    m_nextItem = 0;
    FindNextItem();
}

void CWeaponImageDialog::OnBnClickedButtonSetitemimage()
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

void CWeaponImageDialog::OnBnClickedButtonSkip()
{
    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(FALSE);
    m_availableIcons.SetSelectionMark(-1);
    ++m_nextItem;
    FindNextItem();
}

void CWeaponImageDialog::OnBnClickedButtonRefreshImages()
{
    PopulateAvailableIcons();
}

void CWeaponImageDialog::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    UNREFERENCED_PARAMETER(pNMLV);
    *pResult = 0;
    GetDlgItem(IDC_BUTTON_SETITEMIMAGE)->EnableWindow(TRUE);
}

void CWeaponImageDialog::PopulateAvailableIcons()
{
    m_availableIcons.DeleteAllItems();
    // first load all the icons available associated with this item type
    int sel = m_comboItemType.GetCurSel();
    WeaponType wt = static_cast<WeaponType>(m_comboItemType.GetItemData(sel));
    std::string directory;
    switch (wt)
    {
    case Weapon_BastardSword:           directory = "Weapon_BastardSword\\"; break;
    case Weapon_BattleAxe:              directory = "Weapon_BattleAxe\\"; break;
    case Weapon_Club:                   directory = "Weapon_Club\\"; break;
    case Weapon_Dagger:                 directory = "Weapon_Dagger\\"; break;
    case Weapon_Dart:                   directory = "Weapon_Dart\\"; break;
    case Weapon_DwarvenAxe:             directory = "Weapon_DwarvenAxe\\"; break;
    case Weapon_Falchion:               directory = "Weapon_Falchion\\"; break;
    case Weapon_GreatCrossbow:          directory = "Weapon_GreatCrossbow\\"; break;
    case Weapon_GreatAxe:               directory = "Weapon_GreatAxe\\"; break;
    case Weapon_GreatClub:              directory = "Weapon_GreatClub\\"; break;
    case Weapon_GreatSword:             directory = "Weapon_GreatSword\\"; break;
    case Weapon_HandAxe:                directory = "Weapon_HandAxe\\"; break;
    case Weapon_Handwraps:              directory = "Weapon_Handwraps\\"; break;
    case Weapon_HeavyCrossbow:          directory = "Weapon_HeavyCrossbow\\"; break;
    case Weapon_HeavyMace:              directory = "Weapon_HeavyMace\\"; break;
    case Weapon_HeavyPick:              directory = "Weapon_HeavyPick\\"; break;
    case Weapon_Kama:                   directory = "Weapon_Kama\\"; break;
    case Weapon_Khopesh:                directory = "Weapon_Khopesh\\"; break;
    case Weapon_Kukri:                  directory = "Weapon_Kukri\\"; break;
    case Weapon_LightCrossbow:          directory = "Weapon_LightCrossbow\\"; break;
    case Weapon_LightHammer:            directory = "Weapon_LightHammer\\"; break;
    case Weapon_LightMace:              directory = "Weapon_LightMace\\"; break;
    case Weapon_LightPick:              directory = "Weapon_LightPick\\"; break;
    case Weapon_Longbow:                directory = "Weapon_Longbow\\"; break;
    case Weapon_Longsword:              directory = "Weapon_Longsword\\"; break;
    case Weapon_Maul:                   directory = "Weapon_Maul\\"; break;
    case Weapon_Morningstar:            directory = "Weapon_Morningstar\\"; break;
    case Weapon_Quarterstaff:           directory = "Weapon_Quarterstaff\\"; break;
    case Weapon_Rapier:                 directory = "Weapon_Rapier\\"; break;
    case Weapon_RepeatingHeavyCrossbow: directory = "Weapon_RepeatingHeavyCrossbow\\"; break;
    case Weapon_RepeatingLightCrossbow: directory = "Weapon_RepeatingLightCrossbow\\"; break;
    case Weapon_Scimitar:               directory = "Weapon_Scimitar\\"; break;
    case Weapon_Shortbow:               directory = "Weapon_Shortbow\\"; break;
    case Weapon_Shortsword:             directory = "Weapon_Shortsword\\"; break;
    case Weapon_Shuriken:               directory = "Weapon_Shuriken\\"; break;
    case Weapon_Sickle:                 directory = "Weapon_Sickle\\"; break;
    case Weapon_ThrowingAxe:            directory = "Weapon_ThrowingAxe\\"; break;
    case Weapon_ThrowingDagger:         directory = "Weapon_ThrowingDagger\\"; break;
    case Weapon_ThrowingHammer:         directory = "Weapon_ThrowingHammer\\"; break;
    case Weapon_Unarmed:                directory = "Weapon_Unarmed\\"; break;
    case Weapon_Warhammer:              directory = "Weapon_Warhammer\\"; break;
    case Weapon_ShieldBuckler:          directory = "Shields\\"; break;
    case Weapon_ShieldSmall:            directory = "Shields\\"; break;
    case Weapon_ShieldLarge:            directory = "Shields\\"; break;
    case Weapon_ShieldTower:            directory = "Shields\\"; break;
    case Weapon_Orb:                    directory = "Orbs\\"; break;
    case Weapon_RuneArm:                directory = "Weapon_RuneArm\\"; break;
    case Weapon_Collar:                 directory = "Collar\\"; break;
    case Weapon_CosmeticShield:         directory = "Shields\\"; break;
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
    m_selectedItemType = wt;
}

void CWeaponImageDialog::LoadImage(const std::string& localPath, std::string filename)
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

void CWeaponImageDialog::FindNextItem()
{
    bool bFound = false;
    const std::list<Item>& items = Items();
    while (!bFound && m_nextItem < items.size())
    {
        std::list<Item>::const_iterator it = items.begin();
        std::advance(it, m_nextItem);
        if ((*it).HasWeapon()
                && (*it).Weapon() == m_selectedItemType
                && (!(*it).HasIcon() || (*it).Icon() == ""))
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
