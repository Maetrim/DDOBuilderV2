// StancesPane.h
//
#pragma once
#include "Resource.h"
#include <vector>
#include "Character.h"
#include "StanceGroup.h"
#include "InfoTip.h"

struct SliderItem
{
public:
    UINT m_sliderControlId;
    std::string m_name;
    CStatic * m_label;
    CSliderCtrl * m_slider;
    size_t m_creationCount;
    int m_position;
    int m_sliderMin;
    int m_sliderMax;
    SliderItem() :
            m_sliderControlId(0),
            m_creationCount(0),
            m_position(0),
            m_sliderMin(0),
            m_sliderMax(0),
            m_label(NULL),
            m_slider(NULL)
    {
    }
    void CreateControls()
    {
        m_label = new CStatic;
        m_slider = new CSliderCtrl;
    };
    ~SliderItem()
    {
        delete m_label;
        delete m_slider;
    };
};

class CStancesPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    public:
        enum { IDD = IDD_STANCES_PANE };
        DECLARE_DYNCREATE(CStancesPane)

#ifdef _DEBUG
        virtual void AssertValid() const;
    #ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif
        virtual void OnInitialUpdate();
        const std::vector<CStanceButton *> & UserStances() const;
        const std::vector<CStanceButton *> & AutoStances() const;
        StanceGroup* GetStanceGroup(CStanceButton* pButton);
        StanceGroup* GetStanceGroup(const std::string& name);
        const CStanceButton * GetStance(const std::string& stanceName);
        bool IsStanceActive(const std::string& name, WeaponType wt) const;
        const SliderItem * GetSlider(const std::string& name) const;
    protected:
        CStancesPane();           // protected constructor used by dynamic creation
        virtual ~CStancesPane();

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        afx_msg void OnHScroll(UINT sbCode, UINT nPos, CScrollBar * pScrollbar);
        DECLARE_MESSAGE_MAP()

    private:
        // CharacterObserver
        virtual void UpdateActiveBuildChanged(Character* pCharacter) override;

        // LifeObserver
        virtual void UpdateAlignmentChanged(Life*, AlignmentType) override;
        virtual void UpdateRaceChanged(Life*, const std::string&) override;

        // BuildObserver
        virtual void UpdateClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) override;
        virtual void UpdateNewStance(Build*, const Stance&) override;
        virtual void UpdateRevokeStance(Build*, const Stance&) override;
        virtual void UpdateStanceActivated(Build*, const std::string& stanceName) override;
        virtual void UpdateStanceDeactivated(Build*, const std::string& stanceName) override;
        virtual void UpdateStanceDisabled(Build*, const std::string& stanceName) override;
        virtual void UpdateEnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateFeatEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateFeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateItemEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateGearChanged(Build*, InventorySlotType) override;

        void CreateStanceWindows();
        void AddStance(const Stance & stance);
        void RevokeStance(const Stance & stance);
        void DestroyAllStances();
        void UpdateStanceStates();
        void DestroyStances(std::vector<CStanceButton*>& items);

        void PositionWindow(CWnd * pWnd, CRect * itemRect, int maxX);
        void ShowTip(const CStanceButton & item, CRect itemRect);
        void HideTip();
        void SetTooltipText(const CStanceButton & item, CPoint tipTopLeft, CPoint tipAlternate);
        void UpdateSliders(const Effect & effect, bool bApply);
        std::list<SliderItem>::iterator GetSlider(const Effect & effect, bool bCreateIfMissing);
        std::list<SliderItem>::iterator GetSlider(UINT controlId);

        StanceGroup* CreateStanceGroup(const std::string& strName, bool singleSelection);
        void PositionStanceGroup(StanceGroup& sg, CRect* pGroupRect, CRect* pItemRect, int maxX);

        CDocument * m_pDocument;
        Character * m_pCharacter;
        std::list<SliderItem> m_sliders;
        CStatic m_staticHiddenSizer;
        std::list<StanceGroup*> m_stanceGroups;
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        const CStanceButton * m_pTooltipItem;
        UINT m_nextStanceId;
        UINT m_nextSliderId;
        bool m_bHadInitialUpdate;
};
