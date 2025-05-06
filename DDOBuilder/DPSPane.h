#pragma once
#include "Resource.h"
#include "Character.h"
#include "Attack.h"
#include "AttackBuff.h"
#include "InfoTip.h"

enum AttackType
{
    AT_Unknown = 0,
    AT_TWF,
    AT_THF,
    AT_SWF,
    AT_Ranged,
    AT_Handwraps,
    AT_SwordAndBoard
};

class CDPSPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    // Construction
    public:
        CDPSPane();
        virtual ~CDPSPane();
        enum { IDD = IDD_DPS_PANE };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX) override;
        virtual void OnInitialUpdate() override;
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
        afx_msg void OnButtonNewAttackChain();
        afx_msg void OnButtonDeleteAttackChain();
        afx_msg BOOL OnTtnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnEndtrackListAttackChain(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnEndtrackListAvailableAttacks(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnAttackListSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnAvailableSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnHoverAttackList(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnHoverAvailableAttackList(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        afx_msg void OnAddAttack();
        afx_msg void OnRemoveAttack();

        DECLARE_DYNCREATE(CDPSPane)
        DECLARE_MESSAGE_MAP()

        // CharacterObserver overrides
        virtual void UpdateActiveLifeChanged(Character*) override;
        virtual void UpdateActiveBuildChanged(Character*) override;

        virtual void UpdateNewAttack(Build*, const Attack&) override;
        virtual void UpdateRevokeAttack(Build*, const Attack&) override;
    private:
        void UpdateAvailableList();
        void PopulateAttackChainCombo();
        void PopulateAttackChain();
        Attack FindAttack(const std::string& name) const;
        double SetBasicAttackCooldown();
        BOOL EvaluateMoveUp(size_t sel);
        BOOL EvaluateMoveDown(size_t sel);
        void ShowTip(const Attack& attack, CRect itemRect);
        void HideTip();
        void SetTooltipText(const Attack& attack, CPoint tipTopLeft, CPoint tipAlternate);

        void CalculateAttackChainDPS();
        double EvaluateAttack(AttackType at, const Attack& attack, const std::list<AttackBuff>& buffs, double timePoint);
        double EvaluateTWF(const Attack& attack, const std::list<AttackBuff>& buffs, double timePoint);
        double EvaluateTHF(const Attack& attack, const std::list<AttackBuff>& buffs, double timePoint);
        double EvaluateSWF(const Attack& attack, const std::list<AttackBuff>& buffs, double timePoint);
        double EvaluateRanged(const Attack& attack, const std::list<AttackBuff>& buffs, double timePoint);
        double EvaluateHandwraps(const Attack& attack, const std::list<AttackBuff>& buffs, double timePoint);
        double EvaluateSwordAndBoard(const Attack& attack, const std::list<AttackBuff>& buffs, double timePoint);
        void DropTimedOutBuffs(std::list<AttackBuff> * buffs, double timePoint);

        CMFCButton m_buttonAddAttackChain;
        CComboBox m_comboAttackChains;
        CMFCButton m_buttonDeleteAttackChain;
        CListCtrl m_attackList;
        CListCtrl m_availableList;
        CMFCButton m_buttonRemove;
        CMFCButton m_buttonMoveUp;
        CMFCButton m_buttonMoveDown;
        CMFCButton m_buttonAdd;

        CDDOBuilderDoc* m_pDocument;
        Character* m_pCharacter;
        bool m_bHadInitialUpdate;
        CImageList m_attackImages;
        std::map<std::string, int> m_imageMap;
        CString m_tipText;
        bool m_bIgnoreChange;

        std::list<Attack> m_availableAttacksWithStacks;
        // tooltips for Attacks
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        int m_hoverItem;
        UINT m_hoverHandle;
};

