// SpecialFeatsPane.cpp
//
#include "stdafx.h"
#include "SpecialFeatsPane.h"

#include "FeatSelectionDialog.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const int c_windowSizeX = 39;
    const int c_windowSizeY = 55;
    const int c_windowSpacing = 0;
}

IMPLEMENT_DYNCREATE(CSpecialFeatPane, CFormView)
#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CSpecialFeatPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_BN_CLICKED(IDC_STATIC_HEROIC, OnHeroic)
    ON_BN_CLICKED(IDC_STATIC_RACIAL, OnRacial)
    ON_BN_CLICKED(IDC_STATIC_ICONIC, OnIconic)
    ON_BN_CLICKED(IDC_STATIC_EPIC, OnEpic)
    ON_BN_CLICKED(IDC_STATIC_SPECIAL, OnSpecial)
    ON_BN_CLICKED(IDC_STATIC_FAVOR, OnFavor)
    ON_MESSAGE(WM_CONTEXTMENU, OnRevokeFeats)
END_MESSAGE_MAP()
#pragma warning(pop)


CSpecialFeatPane::CSpecialFeatPane() :
    CFormView(CSpecialFeatPane::IDD),
    m_pDoc(NULL),
    m_pCharacter(NULL)
{
    // fixed sized font in use for static controls
    LOGFONT lf;
    ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
    NONCLIENTMETRICS nm;
    nm.cbSize = sizeof(NONCLIENTMETRICS);
    VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0));
    lf = nm.lfMenuFont;
    lf.lfHeight = -12;
    m_staticFont.CreateFontIndirect(&lf);
}

CSpecialFeatPane::~CSpecialFeatPane()
{
    CleanUp(m_heroicSelectionViews);
    CleanUp(m_racialSelectionViews);
    CleanUp(m_iconicSelectionViews);
    CleanUp(m_epicSelectionViews);
    CleanUp(m_specialSelectionViews);
    CleanUp(m_favorSelectionViews);
}

void CSpecialFeatPane::CleanUp(std::vector<CFeatSelectionDialog*> & vWindows)
{
    for (size_t vi = 0; vi < vWindows.size(); ++vi)
    {
        delete vWindows[vi];
    }
    vWindows.clear();
}

void CSpecialFeatPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    // DDX the number of controls that we have feats for
    DDX_Control(pDX, IDC_STATIC_HEROIC, m_staticHeroic);
    DDX_Control(pDX, IDC_STATIC_RACIAL, m_staticRacial);
    DDX_Control(pDX, IDC_STATIC_ICONIC, m_staticIconic);
    DDX_Control(pDX, IDC_STATIC_EPIC, m_staticEpic);
    DDX_Control(pDX, IDC_STATIC_SPECIAL, m_staticSpecial);
    DDX_Control(pDX, IDC_STATIC_FAVOR, m_staticFavor);

    m_staticHeroic.SetFont(&m_staticFont);
    m_staticRacial.SetFont(&m_staticFont);
    m_staticIconic.SetFont(&m_staticFont);
    m_staticEpic.SetFont(&m_staticFont);
    m_staticSpecial.SetFont(&m_staticFont);
    m_staticFavor.SetFont(&m_staticFont);
}

LRESULT CSpecialFeatPane::OnLoadComplete(WPARAM, LPARAM)
{
    // create the dynamic controls used to add feats
    const std::list<Feat> & heroicPastLifeFeats = HeroicPastLifeFeats();
    const std::list<Feat> & racialPastLifeFeats = RacialPastLifeFeats();
    const std::list<Feat> & iconicPastLifeFeats = IconicPastLifeFeats();
    const std::list<Feat> & epicPastLifeFeats = EpicPastLifeFeats();
    const std::list<Feat> & specialFeats = SpecialFeats();
    const std::list<Feat> & favorFeats = FavorFeats();
    m_heroicSelectionViews.reserve(heroicPastLifeFeats.size());
    m_racialSelectionViews.reserve(racialPastLifeFeats.size());
    m_iconicSelectionViews.reserve(iconicPastLifeFeats.size());
    m_epicSelectionViews.reserve(epicPastLifeFeats.size());
    m_specialSelectionViews.reserve(specialFeats.size());
    m_favorSelectionViews.reserve(favorFeats.size());

    CreateFeatWindows(&m_staticHeroic, heroicPastLifeFeats, &m_heroicSelectionViews);
    CreateFeatWindows(&m_staticRacial, racialPastLifeFeats, &m_racialSelectionViews);
    CreateFeatWindows(&m_staticIconic, iconicPastLifeFeats, &m_iconicSelectionViews);
    CreateFeatWindows(&m_staticEpic, epicPastLifeFeats, &m_epicSelectionViews);
    CreateFeatWindows(&m_staticSpecial, specialFeats, &m_specialSelectionViews);
    CreateFeatWindows(&m_staticFavor, favorFeats, &m_favorSelectionViews);

    UpdateDocumentPointers(m_heroicSelectionViews);
    UpdateDocumentPointers(m_racialSelectionViews);
    UpdateDocumentPointers(m_iconicSelectionViews);
    UpdateDocumentPointers(m_epicSelectionViews);
    UpdateDocumentPointers(m_specialSelectionViews);
    UpdateDocumentPointers(m_favorSelectionViews);

    UpdateFeatTrainedCounts(FeatAcquisition_HeroicPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_RacialPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_IconicPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_EpicPastLife);

    // cause a resize after all sub windows have updated
    CRect rect;
    GetClientRect(&rect);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rect.Width(), rect.Height()));
    return 0;
}

void CSpecialFeatPane::CreateFeatWindows(
        CStatic * groupWindow,
        const std::list<Feat> & featList,
        std::vector<CFeatSelectionDialog *> * dialogs)
{
    // position the created windows left to right under the static control until
    // they don't fit then move them down a row and start again
    CRect wndRect;
    groupWindow->GetWindowRect(wndRect);
    ScreenToClient(&wndRect);

    double dScaleFactor = GetDPIMultiplier(GetSafeHwnd());
    CRect itemRect(
            c_controlSpacing,
            c_controlSpacing,
            static_cast<LONG>(c_windowSizeX * dScaleFactor) + c_controlSpacing,
            static_cast<LONG>(c_windowSizeY * dScaleFactor) + c_controlSpacing);
    itemRect += CPoint(0, wndRect.bottom);
    std::list<Feat>::const_iterator li = featList.begin();
    for (size_t fi = 0; fi < featList.size(); ++fi)
    {
        // show a feat selection dialog
        CFeatSelectionDialog * dlg = new CFeatSelectionDialog(this, *li);
        dlg->Create(CFeatSelectionDialog::IDD, this);
        dlg->MoveWindow(&itemRect);
        dlg->ShowWindow(SW_SHOW);
        dialogs->push_back(dlg);
        li++;
        // move rectangle across for next set of controls
        itemRect += CPoint(itemRect.Width() + c_windowSpacing, 0);
    }
}

void CSpecialFeatPane::OnInitialUpdate()
{
    __super::OnInitialUpdate();
}

void CSpecialFeatPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    // move all our feat windows to be as many across as we can, then start the next row
    if (IsWindow(m_staticHeroic.GetSafeHwnd())
            && IsWindowVisible())
    {
        // position the created windows left to right until they don't fit
        // then move them down a row and start again
        // do this for each set of controls
        size_t fi = 0;
        int maxx = c_controlSpacing;
        int maxy = c_controlSpacing;
        fi = PositionWindows(&m_staticHeroic, fi, m_heroicSelectionViews, &maxx, &maxy);
        fi = PositionWindows(&m_staticRacial, fi, m_racialSelectionViews, &maxx, &maxy);
        fi = PositionWindows(&m_staticIconic, fi, m_iconicSelectionViews, &maxx, &maxy);
        fi = PositionWindows(&m_staticEpic, fi, m_epicSelectionViews, &maxx, &maxy);
        fi = PositionWindows(&m_staticSpecial, fi, m_specialSelectionViews, &maxx, &maxy);
        fi = PositionWindows(&m_staticFavor, fi, m_favorSelectionViews, &maxx, &maxy);
        // set scale based on area used by the windows.
        // This will introduce scroll bars if required
        SetScrollSizes(MM_TEXT, CSize(maxx, maxy));
    }
}

size_t CSpecialFeatPane::PositionWindows(
        CStatic * groupWindow,
        size_t startIndex,
        const std::vector<CFeatSelectionDialog *> & dialogs,
        int * maxX,
        int * yPos)
{
    double dScaleFactor = GetDPIMultiplier(GetSafeHwnd());
    // first position the group control
    CRect rctGroup;
    groupWindow->GetWindowRect(&rctGroup);
    rctGroup -= rctGroup.TopLeft();
    rctGroup.bottom = rctGroup.top + static_cast<LONG>(c_windowSizeY * dScaleFactor);
    rctGroup += CPoint(c_controlSpacing, *yPos);
    // position the group control
    groupWindow->MoveWindow(rctGroup);

    // now position the group windows immediately to the right
    CRect itemRect(c_controlSpacing, *yPos, static_cast<LONG>(c_windowSizeX * dScaleFactor) , static_cast<LONG>(c_windowSizeY * dScaleFactor) + *yPos);
    itemRect += CPoint(rctGroup.Width() + c_windowSpacing + c_windowSpacing, 0);
    for (size_t fi = 0; fi < dialogs.size(); ++fi)
    {
        dialogs[fi]->MoveWindow(itemRect);
        // move rectangle across for next set of controls
        itemRect += CPoint(itemRect.Width() + c_windowSpacing, 0);
    }
    // always 1 line of icons
    *yPos += itemRect.Height() + c_controlSpacing;
    // keep track of the longest line of icons
    *maxX = max(*maxX, itemRect.left);
    return startIndex + dialogs.size();
}

LRESULT CSpecialFeatPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    m_staticHeroic.SetWindowText("Heroic");
    m_staticRacial.SetWindowText("Racial");
    m_staticIconic.SetWindowText("Iconic");
    m_staticEpic.SetWindowText("Epic");
    // wParam is the document pointer
    CDDOBuilderDoc * pDoc = (CDDOBuilderDoc*)(wParam);
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pDoc = pDoc;
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        UpdateDocumentPointers(m_heroicSelectionViews);
        UpdateDocumentPointers(m_racialSelectionViews);
        UpdateDocumentPointers(m_iconicSelectionViews);
        UpdateDocumentPointers(m_epicSelectionViews);
        UpdateDocumentPointers(m_specialSelectionViews);
        UpdateDocumentPointers(m_favorSelectionViews);
    }
    UpdateFeatTrainedCounts(FeatAcquisition_HeroicPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_RacialPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_IconicPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_EpicPastLife);
    return 0L;
}

BOOL CSpecialFeatPane::OnEraseBkgnd(CDC* pDC)
{
    std::vector<int> controlsNotToBeErased;
    //static int controlsNotToBeErased[] =
    //{
        controlsNotToBeErased.push_back(IDC_STATIC_HEROIC);
        controlsNotToBeErased.push_back(IDC_STATIC_RACIAL);
        controlsNotToBeErased.push_back(IDC_STATIC_ICONIC);
        controlsNotToBeErased.push_back(IDC_STATIC_EPIC);
        controlsNotToBeErased.push_back(IDC_STATIC_SPECIAL);
        controlsNotToBeErased.push_back(IDC_STATIC_FAVOR);
        //0 // end marker
    //};

    // now handle the dynamic controls also
    for (size_t i = 0; i < m_heroicSelectionViews.size(); ++i)
    {
        CWnd * pControl = m_heroicSelectionViews[i];
        if (pControl)
        {
            controlsNotToBeErased.push_back(pControl->GetDlgCtrlID());
        }
    }
    for (size_t i = 0; i < m_racialSelectionViews.size(); ++i)
    {
        CWnd * pControl = m_racialSelectionViews[i];
        if (pControl)
        {
            controlsNotToBeErased.push_back(pControl->GetDlgCtrlID());
        }
    }
    for (size_t i = 0; i < m_iconicSelectionViews.size(); ++i)
    {
        CWnd * pControl = m_iconicSelectionViews[i];
        if (pControl)
        {
            controlsNotToBeErased.push_back(pControl->GetDlgCtrlID());
        }
    }
    for (size_t i = 0; i < m_epicSelectionViews.size(); ++i)
    {
        CWnd * pControl = m_epicSelectionViews[i];
        if (pControl)
        {
            controlsNotToBeErased.push_back(pControl->GetDlgCtrlID());
        }
    }
    for (size_t i = 0; i < m_specialSelectionViews.size(); ++i)
    {
        CWnd * pControl = m_specialSelectionViews[i];
        if (pControl)
        {
            controlsNotToBeErased.push_back(pControl->GetDlgCtrlID());
        }
    }
    for (size_t i = 0; i < m_favorSelectionViews.size(); ++i)
    {
        CWnd * pControl = m_favorSelectionViews[i];
        if (pControl)
        {
            controlsNotToBeErased.push_back(pControl->GetDlgCtrlID());
        }
    }

    controlsNotToBeErased.push_back(0); // end marker
    BOOL ret = OnEraseBackground(this, pDC, &controlsNotToBeErased[0]);
    return ret;
}

void CSpecialFeatPane::UpdateDocumentPointers(
    std::vector<CFeatSelectionDialog*> & vWindows)
{
    for (size_t vi = 0; vi < vWindows.size(); ++vi)
    {
        if (IsWindow(vWindows[vi]->GetSafeHwnd()))
        {
            vWindows[vi]->SendMessage(
                    UWM_NEW_DOCUMENT,
                    (WPARAM)m_pDoc,
                    (LPARAM)m_pCharacter);
        }
    }
}

void CSpecialFeatPane::OnHeroic()
{
    TrainAllFeats(m_heroicSelectionViews);
}

void CSpecialFeatPane::OnRacial()
{
    TrainAllFeats(m_racialSelectionViews);
}

void CSpecialFeatPane::OnIconic()
{
    TrainAllFeats(m_iconicSelectionViews);
}

void CSpecialFeatPane::OnEpic()
{
    TrainAllFeats(m_epicSelectionViews);
}

void CSpecialFeatPane::OnSpecial()
{
    TrainAllFeats(m_specialSelectionViews);
}

void CSpecialFeatPane::OnFavor()
{
    TrainAllFeats(m_favorSelectionViews);
}

void CSpecialFeatPane::TrainAllFeats(std::vector<CFeatSelectionDialog *>& feats)
{
    // try and train one of each feat (only if it can be trained)
    for (size_t i = 0; i < feats.size(); ++i)
    {
        feats[i]->DoLeftClickAction();
    }
}

LRESULT CSpecialFeatPane::OnRevokeFeats(WPARAM, LPARAM)
{
    CPoint p;
    GetCursorPos(&p);
    CWnd* pWnd = WindowFromPoint(p);
    if (pWnd != NULL)
    {
        UINT nID = pWnd->GetDlgCtrlID();
        switch (nID)
        {
        case IDC_STATIC_HEROIC: RevokeAllFeats(m_heroicSelectionViews); break;
        case IDC_STATIC_RACIAL: RevokeAllFeats(m_racialSelectionViews); break;
        case IDC_STATIC_ICONIC: RevokeAllFeats(m_iconicSelectionViews); break;
        case IDC_STATIC_EPIC:   RevokeAllFeats(m_epicSelectionViews); break;
        case IDC_STATIC_SPECIAL:    RevokeAllFeats(m_specialSelectionViews); break;
        case IDC_STATIC_FAVOR:  RevokeAllFeats(m_favorSelectionViews); break;
        }
    }
    return 0;
}

void CSpecialFeatPane::RevokeAllFeats(std::vector<CFeatSelectionDialog*>& feats)
{
    // try and revoke one of each feat (only if it has been trained)
    for (size_t i = 0; i < feats.size(); ++i)
    {
        feats[i]->DoRightClickAction();
    }
}


void CSpecialFeatPane::UpdateActiveLifeChanged(Character*)
{
    UpdateDocumentPointers(m_heroicSelectionViews);
    UpdateDocumentPointers(m_racialSelectionViews);
    UpdateDocumentPointers(m_iconicSelectionViews);
    UpdateDocumentPointers(m_epicSelectionViews);
    UpdateDocumentPointers(m_specialSelectionViews);
    UpdateDocumentPointers(m_favorSelectionViews);
}

void CSpecialFeatPane::UpdateActiveBuildChanged(Character *)
{
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->AttachObserver(this);
    }
    UpdateDocumentPointers(m_heroicSelectionViews);
    UpdateDocumentPointers(m_racialSelectionViews);
    UpdateDocumentPointers(m_iconicSelectionViews);
    UpdateDocumentPointers(m_epicSelectionViews);
    UpdateDocumentPointers(m_specialSelectionViews);
    UpdateDocumentPointers(m_favorSelectionViews);
    UpdateFeatTrainedCounts(FeatAcquisition_HeroicPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_RacialPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_IconicPastLife);
    UpdateFeatTrainedCounts(FeatAcquisition_EpicPastLife);
}

void CSpecialFeatPane::UpdateFeatTrained(
    Build*,
    const std::string& featName)
{
    const Feat& feat = FindFeat(featName);
    UpdateFeatTrainedCounts(feat.Acquire());
}

void CSpecialFeatPane::UpdateFeatRevoked(
    Build*,
    const std::string& featName)
{
    const Feat& feat = FindFeat(featName);
    UpdateFeatTrainedCounts(feat.Acquire());
}

void CSpecialFeatPane::UpdateFeatTrainedCounts(
    FeatAcquisitionType fat)
{
    if (m_pCharacter != NULL
        && m_pCharacter->ActiveBuild() != NULL)
    {
        switch (fat)
        {
            case FeatAcquisition_HeroicPastLife:
            case FeatAcquisition_RacialPastLife:
            case FeatAcquisition_IconicPastLife:
            case FeatAcquisition_EpicPastLife:
                break;
            default:
                return; // no changes for other feat types
        }
        // count the number of feats trained of this type
        size_t numPastLifeFeats = 0;
        const std::list<TrainedFeat>& feats = m_pCharacter->ActiveBuild()->SpecialFeats();
        std::list<TrainedFeat>::const_iterator it = feats.begin();
        while (it != feats.end())
        {
            const Feat& trainedFeat = FindFeat((*it).FeatName());
            if (trainedFeat.Acquire() == fat)
            {
                // this is a match for this type, count it
                ++numPastLifeFeats;
            }
            ++it;
        }
        int maxCount = 0;
        CString newText;
        switch (fat)
        {
            case FeatAcquisition_HeroicPastLife:
                maxCount = HeroicPastLifeFeats().size() * 3;
                newText.Format("Heroic\r\n%d/%d", numPastLifeFeats, maxCount);
                m_staticHeroic.SetWindowText(newText);
                break;
            case FeatAcquisition_RacialPastLife:
                maxCount = RacialPastLifeFeats().size() * 3;
                newText.Format("Racial\r\n%d/%d", numPastLifeFeats, maxCount);
                m_staticRacial.SetWindowText(newText);
                break;
            case FeatAcquisition_IconicPastLife:
                maxCount = IconicPastLifeFeats().size() * 3;
                newText.Format("Iconic\r\n%d/%d", numPastLifeFeats, maxCount);
                m_staticIconic.SetWindowText(newText);
                break;
            case FeatAcquisition_EpicPastLife:
                maxCount = EpicPastLifeFeats().size() * 3;
                newText.Format("Epic\r\n%d/%d", numPastLifeFeats, maxCount);
                m_staticEpic.SetWindowText(newText);
                break;
        }
    }
}

