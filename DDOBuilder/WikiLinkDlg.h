// WikiLinkDlg.h
//
#include "Resource.h"

class CWikiLinkDlg : public CDialogEx
{
    public:
        CWikiLinkDlg();
        ~CWikiLinkDlg();

        CString GetLink();

        //{{AFX_DATA(CWikiLinkDlg)
        enum { IDD = IDD_DIALOG_WIKILINK };
        //}}AFX_DATA

    protected:
        //{{AFX_VIRTUAL(CWikiLinkDlg)
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CWikiLinkDlg)
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
    private:
        CEdit m_editLink;
        CString m_linkText;
};

