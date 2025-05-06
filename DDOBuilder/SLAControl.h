// SLAControl.h
//

#pragma once

#include "InfoTip.h"
#include "Character.h"

class SLAHitBox
{
    public:
        SLAHitBox(int slaIndex, const CRect & rect) :
                m_slaIndex(slaIndex), m_rect(rect)
        {
        };
        ~SLAHitBox() {};

        bool IsInRect(CPoint point) const
        {
            return (m_rect.PtInRect(point) != 0);
        };
        int SLAIndex() const
        {
            return m_slaIndex;
        };
        CRect Rect() const
        {
            return m_rect;
        };
    private:
        int m_slaIndex;
        CRect m_rect;
};

class SLA
{
    public:
        SLA(const std::string & name, size_t count, const std::vector<double> & amountVector) :
                m_slaName(name),
                m_count(count),
                m_vector(amountVector)
        {
        };
        ~SLA() {};

        const std::string & Name() const
        {
            return m_slaName;
        }

        bool operator==(const SLA & other) const
        {
            return (m_slaName == other.m_slaName);
        }

        size_t Count() const
        {
            return m_count;
        }
        void IncrementCount()
        {
            m_count++;
        }
        void DecrementCount()
        {
            m_count--;
        }
        int Cost() const
        {
            return static_cast<int>(m_vector[(m_count - 1) * 4 + 1]);
        }
        int MCL() const
        {
            return static_cast<int>(m_vector[(m_count - 1) * 4 + 2]);
        }
        double Cooldown() const
        {
            return m_vector[(m_count - 1) * 4 + 3];
        }
    private:
        std::string m_slaName;
        size_t m_count;
        std::vector<double> m_vector;
};

class CSLAControl :
    public CStatic
{
    public:
        CSLAControl();
        virtual ~CSLAControl();

        void ClearSLAs();

        void SetCharacter(Character * pCharacter);
        void AddSLA(const std::string & slaName, size_t stacks, const std::vector<double> & amountVector);
        void RevokeSLA(const std::string & slaName);
        const std::list<SLA> & SLAs() const;

    protected:
        //{{AFX_VIRTUAL(CSLAControl)
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CSLAControl)
        afx_msg void OnPaint();
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam) ;
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        static void InitialiseStaticImages();
        const SLAHitBox * FindByPoint(CRect * pRect = NULL) const;
        void ShowTip(const SLAHitBox & item, CRect itemRect);
        void HideTip();
        void SetTooltipText(const SLAHitBox & item, CPoint tipTopLeft, CPoint tipAlternate);

        Character * m_pCharacter;
        CSize m_bitmapSize;
        CBitmap m_cachedDisplay;
        std::list<SLAHitBox> m_hitBoxes;
        bool m_bCreateHitBoxes;
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        const SLAHitBox * m_pTooltipItem;
        std::list<SLA> m_SLAs;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
