#pragma once
// CWeaponImageDialog dialog

class CWeaponImageDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CWeaponImageDialog)

public:
    CWeaponImageDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CWeaponImageDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_SETITEMIMAGES };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    afx_msg void OnCbnSelchangeComboItemtype();
    afx_msg void OnBnClickedButtonStart();
    afx_msg void OnBnClickedButtonSetitemimage();
    afx_msg void OnBnClickedButtonSkip();
    afx_msg void OnBnClickedButtonRefreshImages();
    afx_msg void OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

private:
    void PopulateAvailableIcons();
    void LoadImage(const std::string& localPath, std::string filename);
    void FindNextItem();

    CComboBox m_comboItemType;
    CListCtrl m_availableIcons;
    CImageList m_images;
    std::map<std::string, int> m_imagesMap;
    size_t m_nextItem;
    WeaponType m_selectedItemType;
};
