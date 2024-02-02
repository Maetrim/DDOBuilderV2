// FeatSelectionDialog.cpp
//

#include "stdafx.h"
#include "FeatSelectionDialog.h"
#include "GlobalSupportFunctions.h"
#include "MainFrm.h"

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CFeatSelectionDialog, CDialog)
    //{{AFX_MSG_MAP(CFeatSelectionDialog)
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_BN_CLICKED(IDC_FEAT_BUTTON, OnFeatButtonLeftClick)
    ON_NOTIFY(NM_RCLICK, IDC_FEAT_BUTTON, OnFeatButtonRightClick)
    ON_NOTIFY(NM_THEMECHANGED, IDC_FEAT_BUTTON, OnFeatButtonMiddleClick)
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#pragma warning(pop)

CFeatSelectionDialog::CFeatSelectionDialog(
        CWnd* pParent,
        const Feat & feat) :
    CDialog(CFeatSelectionDialog::IDD, pParent),
    m_feat(feat),
    m_pCharacter(NULL),
    m_showingTip(false),
    m_tipCreated(false)
{
    //{{AFX_DATA_INIT(CFeatSelectionDialog)
    //}}AFX_DATA_INIT
    // create the font used
    // fixed sized font in use for feat buttons
    LOGFONT lf;
    ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
    NONCLIENTMETRICS nm;
    nm.cbSize = sizeof(NONCLIENTMETRICS);
    VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0));
    lf = nm.lfMenuFont;
    lf.lfHeight = -12;
    m_buttonFont.CreateFontIndirect(&lf);
}

void CFeatSelectionDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFeatSelectionDialog)
    DDX_Control(pDX, IDC_FEAT_BUTTON, m_featButton);
    //}}AFX_DATA_MAP
}

void CFeatSelectionDialog::DoLeftClickAction()
{
    OnFeatButtonLeftClick();
}

void CFeatSelectionDialog::DoRightClickAction()
{
    OnFeatButtonRightClick(NULL, NULL);
}

BOOL CFeatSelectionDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_tooltip.Create(this);
    m_tipCreated = true;
    LoadFeatBitmap();
    SetupControls();
    m_featButton.SetFont(&m_buttonFont);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CFeatSelectionDialog::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    // wParam is the document pointer
    CDocument * pDoc = (CDocument*)(wParam);
    UNREFERENCED_PARAMETER(pDoc);
    //m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    if (pCharacter != m_pCharacter)
    {
        if (m_pCharacter != NULL)
        {
            m_pCharacter->DetachObserver(this);
        }
        m_pCharacter = pCharacter;
        if (m_pCharacter != NULL)
        {
            m_pCharacter->AttachObserver(this);
        }
    }
    SetupControls();
    return 0L;
}

void CFeatSelectionDialog::SetupControls()
{
    if (m_pCharacter != NULL
            && m_pCharacter->ActiveBuild() != NULL)
    {
        size_t trainedCount = m_pCharacter->ActiveBuild()->GetSpecialFeatTrainedCount(m_feat.Name());
        CString text;
        text.Format("%d", trainedCount);
        m_featButton.m_bTopImage = true;
        m_featButton.SetTransparent();
        m_featButton.SetWindowText(text);
        m_featButton.EnableWindow(true);
    }
    else
    {
        // no document selected
        m_featButton.EnableWindow(false);
        m_featButton.m_bTopImage = true;
        m_featButton.SetTransparent();
        m_featButton.SetWindowText("-");
    }
}

void CFeatSelectionDialog::LoadFeatBitmap()
{
    CImage image;
    HRESULT result = LoadImageFile(
            "DataFiles\\FeatImages\\",
            m_feat.Icon(),
            &image,
            CSize(32, 32));

    if (result != S_OK)
    {
        result = LoadImageFile(
            "DataFiles\\FeatImages\\",
            "Unknown",
            &image,
            CSize(32, 32));
    }
    m_featButton.SetBitmap(image.Detach());
}

void CFeatSelectionDialog::OnFeatButtonLeftClick()
{
    if (m_pCharacter != NULL
            && m_pCharacter->ActiveBuild() != NULL)
    {
        size_t trainedCount = m_pCharacter->ActiveBuild()->GetSpecialFeatTrainedCount(m_feat.Name());
        bool canIncrease = (trainedCount < m_feat.MaxTimesAcquire());
        if (canIncrease)
        {
            m_pCharacter->ActiveBuild()->TrainSpecialFeat(m_feat.Name());
            SetupControls();
        }
    }
    // always hide the tip on a button click as interferes with mouse leave events
    if (m_showingTip)
    {
        HideTip();
    }
}

void CFeatSelectionDialog::OnFeatButtonRightClick(
        NMHDR * pNotifyStruct,
        LRESULT * result)
{
    UNREFERENCED_PARAMETER(pNotifyStruct);
    UNREFERENCED_PARAMETER(result);
    if (m_pCharacter != NULL
            && m_pCharacter->ActiveBuild() != NULL)
    {
        size_t trainedCount = m_pCharacter->ActiveBuild()->GetSpecialFeatTrainedCount(m_feat.Name());
        bool canDecrease = (trainedCount > 0);
        if (canDecrease)
        {
            m_pCharacter->ActiveBuild()->RevokeSpecialFeat(m_feat.Name());
            SetupControls();
        }
    }
    // always hide the tip on a button click as interferes with mouse leave events
    if (m_showingTip)
    {
        HideTip();
    }
}

BOOL CFeatSelectionDialog::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_FEAT_BUTTON,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CFeatSelectionDialog::OnMouseMove(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    // determine whether the mouse is over the feat icon
    CWnd * pWnd = ChildWindowFromPoint(point);
    if (pWnd == &m_featButton)
    {
        if (!m_showingTip)
        {
            CRect itemRect;
            m_featButton.GetWindowRect(&itemRect);
            ScreenToClient(&itemRect);
            ShowTip(itemRect);
        }
        // track the mouse so we know when it leaves our window
        // (us when disabled, our button when enabled)
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = GetSafeHwnd(); // (m_pCharacter != NULL) ? m_featButton.GetSafeHwnd() : m_hWnd;
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = 1;
        _TrackMouseEvent(&tme);
    }
    else
    {
        if (m_showingTip)
        {
            HideTip();
        }
    }
}

LRESULT CFeatSelectionDialog::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    CPoint point;
    GetCursorPos(&point);
    CWnd* pWnd = WindowFromPoint(point);
    if (pWnd != &m_featButton)
    {
        // hide any tooltip when the mouse leaves the area its being shown for
        HideTip();
    }
    return 0;
}

void CFeatSelectionDialog::ShowTip(CRect itemRect)
{
    if (m_showingTip)
    {
        m_tooltip.Hide();
    }
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    CPoint tipAlternate(itemRect.left, itemRect.top - 2);
    SetTooltipText(tipTopLeft, tipAlternate);
    m_showingTip = true;
}

void CFeatSelectionDialog::HideTip()
{
    // tip not shown if not over an assay
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
    }
}

void CFeatSelectionDialog::SetTooltipText(
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    if (m_pCharacter != NULL
            && m_pCharacter->ActiveBuild() != NULL)
    {
        m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
        m_tooltip.SetFeatItem(
                *m_pCharacter->ActiveBuild(),
                &m_feat,
                false,
                MAX_GAME_LEVEL,
                false);
        m_tooltip.Show();
    }
}

void CFeatSelectionDialog::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    // move all our feat windows to be as many across as we can, then start the next row
    if (IsWindow(m_featButton.GetSafeHwnd()))
    {
        // feat button takes entire space
        m_featButton.MoveWindow(0, 0, cx, cy);
    }
}

void CFeatSelectionDialog::OnCancel()
{
    // do nothing to stop dialog being dismissed
}

void CFeatSelectionDialog::UpdateActiveBuildChanged(Character *)
{
    // if the build selection has changed, then the life selection could also
    // have changed. This may change how many of this feat we have
    SetupControls();
}

void CFeatSelectionDialog::OnFeatButtonMiddleClick(NMHDR * pNotifyStruct, LRESULT * result)
{
    UNREFERENCED_PARAMETER(pNotifyStruct);
    UNREFERENCED_PARAMETER(result);
    if (GetKeyState(VK_SHIFT) < 0)
    {
        // may be trying to set for a sub item (if trained)
        CString iconName = m_feat.Icon().c_str();
        CString prompt;
        prompt.Format("Make the Enhancement Image \"%s\" transparent?\r\n"
            "Yes - Normal Transparency for a passive icon\r\n"
            "No  - Transparency for a passive icon with top left +\r\n"
            "Cancel - Cancel the action\r\n", (LPCTSTR)iconName);
        int ret = AfxMessageBox(prompt, MB_YESNOCANCEL);
        CString fullIconLocation("FeatImages\\");
        fullIconLocation += iconName;
        if (IDYES == ret)
        {
            MakeIconTransparent(fullIconLocation, false);
        }
        else if (IDNO == ret)
        {
            MakeIconTransparent(fullIconLocation, true);
        }
    }
}
