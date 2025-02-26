// AttackChainNameDialog.cpp
//
#include "stdafx.h"
#include "AttackChainNameDialog.h"

#include "Character.h"

// CAttackChainNameDialog dialog

IMPLEMENT_DYNAMIC(CAttackChainNameDialog, CDialog)

CAttackChainNameDialog::CAttackChainNameDialog(
        CWnd* pParent,
        const Character* pCharacter,
        const std::string& defaultName) :
    CDialog(CAttackChainNameDialog::IDD, pParent),
    m_pCharacter(pCharacter),
    m_name(defaultName)
{
}

CAttackChainNameDialog::~CAttackChainNameDialog()
{
}

void CAttackChainNameDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_GEAR_SET_NAME, m_editAttackChainName);
}

BEGIN_MESSAGE_MAP(CAttackChainNameDialog, CDialog)
END_MESSAGE_MAP()

// CAttackChainNameDialog message handlers
BOOL CAttackChainNameDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_editAttackChainName.SetWindowText(m_name.c_str());

    return TRUE;
}

void CAttackChainNameDialog::OnOK()
{
    // check that the name entered is unique and does not already exist
    CString name;
    m_editAttackChainName.GetWindowText(name);
    if (name == "")
    {
        AfxMessageBox("Attack Chain names cannot be blank.", MB_ICONEXCLAMATION);
    }
    else
    {
        const std::list<AttackChain> & setups = m_pCharacter->ActiveBuild()->AttackChains();
        bool unique = true;
        std::list<AttackChain>::const_iterator it = setups.begin();
        while (unique && it != setups.end())
        {
            unique = ((*it).Name() != (LPCSTR)name);
            ++it;
        }
        if (!unique)
        {
            // tell user name is not unique and get them to re-enter
            AfxMessageBox("The Attack Chain name you entered already exists.\n"
                    "All Attack Chain names must be unique. Please try again with a different name.",
                    MB_ICONEXCLAMATION);
        }
        else
        {
            // all ok, return success
            m_name = (LPCSTR)name;
            CDialog::OnOK();
        }
    }
}

std::string CAttackChainNameDialog::Name() const
{
    return m_name;
}

