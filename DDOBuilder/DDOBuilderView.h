// DDOBuilderView.h
//

#pragma once
#include "resource.h"
#include "Character.h"

class CDDOBuilderView :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    protected:
        CDDOBuilderView();
        virtual ~CDDOBuilderView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
        DECLARE_DYNCREATE(CDDOBuilderView)

    public:
        enum{ IDD = IDD_DDOBUILDER_FORM };
        CDDOBuilderDoc* GetDocument() const;

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        virtual void OnInitialUpdate(); // called first time after construct

        afx_msg void OnFilePrintPreview();
        afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
        afx_msg LRESULT OnNewDocument(WPARAM, LPARAM);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);

        afx_msg void OnButtonStrPlus();
        afx_msg void OnButtonStrMinus();
        afx_msg void OnButtonDexPlus();
        afx_msg void OnButtonDexMinus();
        afx_msg void OnButtonConPlus();
        afx_msg void OnButtonConMinus();
        afx_msg void OnButtonIntPlus();
        afx_msg void OnButtonIntMinus();
        afx_msg void OnButtonWisPlus();
        afx_msg void OnButtonWisMinus();
        afx_msg void OnButtonChaPlus();
        afx_msg void OnButtonChaMinus();
        afx_msg void OnSelendokComboRace();
        afx_msg void OnSelendokComboAlignment();
        afx_msg void OnSelendokComboTomeStr();
        afx_msg void OnSelendokComboTomeDex();
        afx_msg void OnSelendokComboTomeCon();
        afx_msg void OnSelendokComboTomeInt();
        afx_msg void OnSelendokComboTomeWis();
        afx_msg void OnSelendokComboTomeCha();
        afx_msg void OnChangeName();
        afx_msg void OnSelendokComboAbilityLevel4();
        afx_msg void OnSelendokComboAbilityLevel8();
        afx_msg void OnSelendokComboAbilityLevel12();
        afx_msg void OnSelendokComboAbilityLevel16();
        afx_msg void OnSelendokComboAbilityLevel20();
        afx_msg void OnSelendokComboAbilityLevel24();
        afx_msg void OnSelendokComboAbilityLevel28();
        afx_msg void OnSelendokComboAbilityLevel32();
        afx_msg void OnSelendokComboAbilityLevel36();
        afx_msg void OnSelendokComboAbilityLevel40();
        afx_msg void OnButtonGuildBuffs();
        afx_msg void OnKillFocusGuildLevel();
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
        afx_msg void OnBnClickedRadio28pt();
        afx_msg void OnBnClickedRadio32pt();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnEditResetbuild();
        afx_msg void OnEditFeatsUpdateEpicOnly(CCmdUI * pCmdUi);
        afx_msg void OnEditFeatsUpdateShowUnavailable(CCmdUI * pCmdUi);
        afx_msg void OnEditFeatsEpicOnly();
        afx_msg void OnEditFeatsShowUnavailable();
        afx_msg BOOL OnTtnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnDumpWeaponGroups();
        afx_msg void OnUpdateDumpWeaponGroups(CCmdUI * pCmdUi);
        DECLARE_MESSAGE_MAP()

        // Character overrides
        virtual void UpdateActiveLifeChanged(Character* pCharacter) override;
        virtual void UpdateActiveBuildChanged(Character * pCharacter) override;

        // Life overrides
        virtual void UpdateLifeFeatTrained(Life*, const std::string&) override;
        virtual void UpdateLifeFeatRevoked(Life*, const std::string&) override;

        // Build overrides
        virtual void UpdateBuildLevelChanged(Build * pBuild) override;
        virtual void UpdateClassChanged(
                Build * pBuild,
                const std::string& classFrom,
                const std::string& classTo,
                size_t level) override;
    private:
        void PopulateComboboxes();
        void RestoreControls();
        void EnableButtons();
        void DisplayAbilityValue(AbilityType ability, CEdit * control);
        void DisplaySpendCost(AbilityType ability, CEdit * control);
        void UpdateAvailableSpend();
        void UpdateBuildDescription();
        void UpdateRadioPoints();
        void DisableControls();
        void ShowHidControls(size_t level, CComboBox * pCombo, UINT idStatic);

        Character * m_pCharacter;
        CStatic m_staticBuildDescription;
        CButton m_button28Pt;
        CButton m_button32Pt;
        CButton m_button34Pt;
        CButton m_button36Pt;
        CEdit m_editName;
        CComboBox m_comboRace;
        CComboBox m_comboAlignment;
        CEdit m_editStr;
        CEdit m_editDex;
        CEdit m_editCon;
        CEdit m_editInt;
        CEdit m_editWis;
        CEdit m_editCha;
        CButton m_buttonStrPlus;
        CButton m_buttonStrMinus;
        CButton m_buttonDexPlus;
        CButton m_buttonDexMinus;
        CButton m_buttonConPlus;
        CButton m_buttonConMinus;
        CButton m_buttonIntPlus;
        CButton m_buttonIntMinus;
        CButton m_buttonWisPlus;
        CButton m_buttonWisMinus;
        CButton m_buttonChaPlus;
        CButton m_buttonChaMinus;
        CEdit m_editCostStr;
        CEdit m_editCostDex;
        CEdit m_editCostCon;
        CEdit m_editCostInt;
        CEdit m_editCostWis;
        CEdit m_editCostCha;
        CComboBox m_comboTomeStr;
        CComboBox m_comboTomeDex;
        CComboBox m_comboTomeCon;
        CComboBox m_comboTomeInt;
        CComboBox m_comboTomeWis;
        CComboBox m_comboTomeCha;
        CButton m_buttonGuildBuffs;
        CEdit m_editGuildLevel;
        CStatic m_staticAvailableSpend;
        CComboBox m_comboAILevel4;
        CComboBox m_comboAILevel8;
        CComboBox m_comboAILevel12;
        CComboBox m_comboAILevel16;
        CComboBox m_comboAILevel20;
        CComboBox m_comboAILevel24;
        CComboBox m_comboAILevel28;
        CComboBox m_comboAILevel32;
        CComboBox m_comboAILevel36;
        CComboBox m_comboAILevel40;

        bool m_bIgnoreFocus;
        CString m_tipText;
        bool m_bHadIntialUpdate;
};

#ifndef _DEBUG  // debug version in DDOBuilderView.cpp
inline CDDOBuilderDoc* CDDOBuilderView::GetDocument() const
   { return reinterpret_cast<CDDOBuilderDoc*>(m_pDocument); }
#endif

