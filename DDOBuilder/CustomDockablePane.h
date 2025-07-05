// CustomDockablePane.h
//
#pragma once

class Character;
//---------------------------------------------------------------------------
class CCustomDockablePane :
        public CDockablePane
{
    public:
        CCustomDockablePane(UINT uViewId);
        virtual ~CCustomDockablePane();

        //MFC
        DECLARE_MESSAGE_MAP()

        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg void OnWindowPosChanging(WINDOWPOS* pos);
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
        afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
        virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) override;
        virtual void DrawCaption(CDC* pDC, CRect rectCaption) override;
        virtual CTabbedPane* CreateTabbedPane() override;
        virtual void OnAfterChangeParent(CWnd* pWndOldParent) override;
        afx_msg LRESULT OnHelpHitTest(WPARAM, LPARAM);
        afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

    public:
        // If derived classes have a contained view they can get free auto resize
        void SetView(CView*, bool resizeWithPane = true);
        CView* GetView() {return m_view;}

        void SetDocumentAndCharacter(CDocument* pDoc, Character* pCharacter);

    protected:
        CDocument* m_document;
        Character* m_pCharacter;

    private:
        void AdjustLayout();

        // Members
        CView* m_view;
        bool m_resizeViewWithPane;
        UINT m_viewId;
        HICON m_hIcon;
};
