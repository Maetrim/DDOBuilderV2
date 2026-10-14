#pragma once
// CFeatImageDialog dialog

class CFeatImageDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CFeatImageDialog)

public:
    CFeatImageDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CFeatImageDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_SETITEMIMAGES };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    afx_msg void OnBnClickedButtonMakeTransparent();
    afx_msg void OnBnClickedButtonRefreshImages();
    afx_msg void OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

private:
    void PopulateAvailableIcons();
    void LoadImage(std::string filename);

    CImageList m_images;
    CListCtrl m_availableIcons;
    std::map<std::string, int> m_imagesMap;
};
