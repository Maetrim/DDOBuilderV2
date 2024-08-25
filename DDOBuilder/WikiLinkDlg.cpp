// WikiLinkDlg.cpp
//
#include "stdafx.h"
#include "WikiLinkDlg.h"

CWikiLinkDlg::CWikiLinkDlg() :
    CDialogEx(CWikiLinkDlg::IDD)
{
}

CWikiLinkDlg::~CWikiLinkDlg()
{
}

void CWikiLinkDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_LINK, m_editLink);
}

BEGIN_MESSAGE_MAP(CWikiLinkDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CWikiLinkDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CWikiLinkDlg::OnOK()
{
    m_editLink.GetWindowText(m_linkText);
    CDialogEx::OnOK();
}

CString CWikiLinkDlg::GetLink()
{
    return m_linkText;
}
