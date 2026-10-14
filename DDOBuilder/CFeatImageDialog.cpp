// CFeatImageDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "DDOBuilder.h"
#include "CFeatImageDialog.h"
#include "afxdialogex.h"
#include "GlobalSupportFunctions.h"

// CFeatImageDialog dialog

IMPLEMENT_DYNAMIC(CFeatImageDialog, CDialogEx)

CFeatImageDialog::CFeatImageDialog(CWnd* pParent /*=nullptr*/) :
    CDialogEx(IDD_DIALOG_FEATIMAGES, pParent)
{

}

CFeatImageDialog::~CFeatImageDialog()
{
}

void CFeatImageDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_ICONS, m_availableIcons);
}

BEGIN_MESSAGE_MAP(CFeatImageDialog, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_MAKETRANSPARENT, &CFeatImageDialog::OnBnClickedButtonMakeTransparent)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH_IMAGES, &CFeatImageDialog::OnBnClickedButtonRefreshImages)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ICONS, &CFeatImageDialog::OnLvnItemchangedList1)
END_MESSAGE_MAP()

// CFeatImageDialog message handlers

BOOL CFeatImageDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    PopulateAvailableIcons();
    GetDlgItem(IDC_BUTTON_MAKETRANSPARENT)->EnableWindow(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CFeatImageDialog::OnBnClickedButtonMakeTransparent()
{
    int sel = m_availableIcons.GetSelectionMark();
    CString iconFilename = m_availableIcons.GetItemText(sel, 0);
    CString folder = "FeatImages\\";
    folder += iconFilename;
    MakeIconTransparent(folder, false);
    GetDlgItem(IDC_BUTTON_MAKETRANSPARENT)->EnableWindow(FALSE);
}

void CFeatImageDialog::OnBnClickedButtonRefreshImages()
{
    PopulateAvailableIcons();
}

void CFeatImageDialog::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    UNREFERENCED_PARAMETER(pNMLV);
    *pResult = 0;
    GetDlgItem(IDC_BUTTON_MAKETRANSPARENT)->EnableWindow(TRUE);
}

void CFeatImageDialog::PopulateAvailableIcons()
{
    m_availableIcons.DeleteAllItems();
    // first load all the feat icons
    m_images.DeleteImageList();
    m_imagesMap.clear();
    m_images.Create(32, 32, ILC_COLOR32, 100, 1000);
    std::string dataFolder = DataFolder();
    std::string fileFilter(dataFolder);
    fileFilter += "FeatImages\\*.png";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(fileFilter.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        LoadImage(findFileData.cFileName);
        while (FindNextFile(hFind, &findFileData))
        {
            LoadImage(findFileData.cFileName);
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
}

void CFeatImageDialog::LoadImage(std::string filename)
{
    CImage image;
    std::string fullPath("DataFiles\\FeatImages\\");
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
