// AttackChainNameDialog.h
//
#pragma once
#include "Resource.h"
#include <string>

class Character;

class CAttackChainNameDialog :
        public CDialog
{
    DECLARE_DYNAMIC(CAttackChainNameDialog)

    public:
        CAttackChainNameDialog(
                CWnd* pParent,
                const Character* pCharacter,
                const std::string& defaultName = "");
        virtual ~CAttackChainNameDialog();

        std::string Name() const;

    // Dialog Data
        enum { IDD = IDD_NAME_ATTACKCHAIN };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
        virtual BOOL OnInitDialog() override;
        virtual void OnOK() override;

        DECLARE_MESSAGE_MAP()
    private:
        const Character * m_pCharacter;
        CEdit m_editAttackChainName;

        std::string m_name;
};
