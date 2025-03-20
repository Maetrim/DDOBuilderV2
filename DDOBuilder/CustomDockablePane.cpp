// CustomDockablePane.cpp
//
#include "stdafx.h"
#include "CustomDockablePane.h"
#include "CustomTabbedPane.h"
#include "CustomMultiPaneFrameWnd.h"
#include "CustomContextMenuManager.h"

//---------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CCustomDockablePane, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_CONTEXTMENU()
    ON_WM_MOUSEACTIVATE()
    ON_MESSAGE(WM_HELPHITTEST, &CCustomDockablePane::OnHelpHitTest)
END_MESSAGE_MAP()
#pragma warning(pop)

//---------------------------------------------------------------------------
CCustomDockablePane::CCustomDockablePane(UINT uViewId) :
    CDockablePane(),
    m_view(NULL),
    m_resizeViewWithPane(false),
    m_pCharacter(NULL),
    m_document(NULL),
    m_viewId(uViewId),
    m_hIcon(0)
{
    m_hIcon = (HICON) ::LoadImage(
            ::AfxGetResourceHandle(),
            MAKEINTRESOURCE(m_viewId),
            IMAGE_ICON,
            ::GetSystemMetrics(SM_CXSMICON),
            ::GetSystemMetrics(SM_CYSMICON), 0);
    m_pTabbedControlBarRTC = RUNTIME_CLASS(CCustomTabbedPane);
    //m_pMiniFrameRTC = RUNTIME_CLASS(CCustomMultiPaneFrameWnd);
}

CCustomDockablePane::~CCustomDockablePane()
{
    DestroyIcon(m_hIcon);
}

//---------------------------------------------------------------------------
int CCustomDockablePane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    m_pTabbedControlBarRTC = RUNTIME_CLASS(CCustomTabbedPane);
    //m_pMiniFrameRTC = RUNTIME_CLASS(CCustomMultiPaneFrameWnd);
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    CCreateContext* createContext = (CCreateContext*)(lpCreateStruct->lpCreateParams);
    CRuntimeClass* viewRuntimeClass = createContext->m_pNewViewClass;

    CView* theView = (CView*)(viewRuntimeClass->CreateObject());

    CRect r;
    GetClientRect(r);
    theView->Create(
            0,
            0,
            WS_CHILD | WS_VISIBLE,
            r,
            this,
            0,
            createContext);
    SetView(theView, true);
    theView->SendMessage(WM_INITIALUPDATE, 0, 0);

    SetMinSize(CSize(30, 30));
    SetIcon(m_hIcon, FALSE);

    return 0;
}

//---------------------------------------------------------------------------
void CCustomDockablePane::SetView(CView* v, bool r)
{
    m_view = v;
    m_resizeViewWithPane = r;
}

void CCustomDockablePane::SetDocumentAndCharacter(
        CDocument* pDoc,
        Character* pCharacter)
{
    m_document = pDoc;
    m_pCharacter = pCharacter;
    m_view->SendMessage(UWM_NEW_DOCUMENT, (WPARAM)pDoc, (LPARAM)pCharacter);
}

//---------------------------------------------------------------------------
void CCustomDockablePane::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}

//---------------------------------------------------------------------------
void CCustomDockablePane::AdjustLayout()
{
    if (GetSafeHwnd())
    {
        // If we have a contained view, resize it as well
        if (m_view && m_resizeViewWithPane)
        {
            CRect r;
            GetClientRect(r);

            if (m_view->GetSafeHwnd()
                    && r.Height() > 0
                    && r.Width() > 0)
            {
                m_view->SetWindowPos(
                        this,
                        r.left,
                        r.top,
                        r.Width(),
                        r.Height(),
                        SWP_NOACTIVATE | SWP_NOZORDER);
            }
        }
    }
}

BOOL CCustomDockablePane::OnCmdMsg(
        UINT nID,
        int nCode,
        void* pExtra,
        AFX_CMDHANDLERINFO* pHandlerInfo)
{
    BOOL handled = false;

    if (IsWindow(m_view->GetSafeHwnd()))
    {
        handled = m_view->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    if (!handled)
    {
        handled =  CDockablePane::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    return handled;
}

void CCustomDockablePane::DrawCaption(CDC* pDC, CRect rectCaption)
{
    ::DrawIconEx(
            pDC->GetSafeHdc(),
            rectCaption.left+2,
            rectCaption.top+2,
            m_hIcon,
            ::GetSystemMetrics(SM_CXSMICON),
            ::GetSystemMetrics(SM_CYSMICON),
            0,
            NULL,
            DI_NORMAL);
    CRect rctIcon(rectCaption);
    rctIcon.bottom += 2;
    rctIcon.right = rctIcon.bottom;
    pDC->Draw3dRect(rctIcon,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
    // make sure we don't over draw the icon we just drew
    rectCaption.left += ::GetSystemMetrics(SM_CXSMICON) + 4;
    // let the base class draw the rest
    CDockablePane::DrawCaption(pDC, rectCaption);
}

CTabbedPane* CCustomDockablePane::CreateTabbedPane()
{
    m_pTabbedControlBarRTC = RUNTIME_CLASS(CCustomTabbedPane);
    CTabbedPane* pPane = CDockablePane::CreateTabbedPane();
    CCustomTabbedPane* pCustomPane = dynamic_cast<CCustomTabbedPane*>(pPane);
    if (pCustomPane != NULL)
    {
        pCustomPane->m_hIcon = m_hIcon;
    }
    return pPane;
}

void CCustomDockablePane::OnAfterChangeParent(CWnd* pWndOldParent)
{
    CDockablePane::OnAfterChangeParent(pWndOldParent);
}

void CCustomDockablePane::OnContextMenu(CWnd* pWnd, CPoint point)
{
    UNREFERENCED_PARAMETER(pWnd);
    UNREFERENCED_PARAMETER(point);
}

int CCustomDockablePane::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    // this is required to stop an assert in CView::OnMouseActivate when a dockable
    // pane is set to floating mode.
    CFrameWnd* pParentFrame = GetParentFrame();

    if( (pParentFrame == pDesktopWnd) || 
        (pDesktopWnd->IsChild(pParentFrame)))
    {
        return CDockablePane::OnMouseActivate(pDesktopWnd, nHitTest, message);
    }

    return MA_NOACTIVATE;
}

void CCustomDockablePane::OnWindowPosChanging(WINDOWPOS* pos)
{
    // ensure tooltip locations are correct on window move
    CDockablePane::OnWindowPosChanging(pos);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(pos->cx, pos->cy));
}

LRESULT CCustomDockablePane::OnHelpHitTest(WPARAM, LPARAM)
{
    return 0x10000 + m_viewId;
}

