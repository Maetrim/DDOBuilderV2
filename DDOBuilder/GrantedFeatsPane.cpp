// GrantedFeatsPane.cpp
//
#include "stdafx.h"
#include "GrantedFeatsPane.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"
#include "MainFrm.h"

IMPLEMENT_DYNCREATE(CGrantedFeatsPane, CFormView)

CGrantedFeatsPane::CGrantedFeatsPane() :
    CFormView(CGrantedFeatsPane::IDD),
    m_pCharacter(NULL),
    m_bHadInitialUpdate(false),
    m_grantedHandle(0)
{
}

CGrantedFeatsPane::~CGrantedFeatsPane()
{
}

void CGrantedFeatsPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_GRANTEDFEATS, m_listGrantedFeats);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CGrantedFeatsPane, CFormView)
    ON_WM_SIZE()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()
#pragma warning(pop)

LRESULT CGrantedFeatsPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
        m_pCharacter = NULL;
    }
    // wParam is the document pointer
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    if (pCharacter != m_pCharacter)
    {
        m_pCharacter = pCharacter;
        if (pCharacter != NULL)
        {
            // need to know about gear changes
            pCharacter->AttachObserver(this);
        }
        m_listGrantedFeats.SetCharacter(m_pCharacter);
    }
    m_grantedNotifyState.clear();
    m_grantedFeats.clear();
    PopulateGrantedFeatsList();
    return 0L;
}

void CGrantedFeatsPane::UpdateActiveLifeChanged(Character*)
{
    Life* pLife = m_pCharacter->ActiveLife();
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
        m_grantedNotifyState.clear();
        m_grantedFeats.clear();
        PopulateGrantedFeatsList();
    }
}

void CGrantedFeatsPane::UpdateActiveBuildChanged(Character*)
{
    Life* pLife = m_pCharacter->ActiveLife();
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
        m_grantedNotifyState.clear();
        m_grantedFeats.clear();
        PopulateGrantedFeatsList();
    }
}

void CGrantedFeatsPane::OnInitialUpdate()
{
    if (!m_bHadInitialUpdate)
    {
        m_bHadInitialUpdate = true;
        CFormView::OnInitialUpdate();

        CWnd* pWnd = AfxGetApp()->m_pMainWnd;
        CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
        CBreakdownsPane* pBreakdownsPane = dynamic_cast<CBreakdownsPane*>(
                pMainWnd->GetPaneView(RUNTIME_CLASS(CBreakdownsPane)));
        pBreakdownsPane->RegisterBuildCallbackEffect(Effect_GrantFeat, this);

        m_listGrantedFeats.SetupControl();
    }
}

bool CGrantedFeatsPane::IsGrantedFeat(const std::string& featName) const
{
    bool isGranted = false;
    if (m_pCharacter != NULL)
    {
        const Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            for (auto&& gfit : m_grantedFeats)
            {
                Feat feat = FindFeat(gfit.Item().front());
                if (gfit.HasRequirementsToBeActive())
                {
                    feat.SetRequirements(gfit.RequirementsToBeActive());
                }
                else
                {
                    Requirements empty;
                    feat.SetRequirements(empty);
                }
                if (feat.Name() == featName
                        && feat.RequirementsToTrain().Met(*pBuild, pBuild->Level()-1, true, Inventory_Unknown, Weapon_Unknown, Weapon_Unknown))
                {
                    isGranted = true;
                }
            }
        }
    }
    return isGranted;
}

const std::list<Effect>& CGrantedFeatsPane::GrantedFeats() const
{
    return m_grantedFeats;
}

LRESULT CGrantedFeatsPane::OnLoadComplete(WPARAM, LPARAM)
{
    return 0;
}

void CGrantedFeatsPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (IsWindow(m_listGrantedFeats.GetSafeHwnd())
            && IsWindowVisible())
    {
        CSize requiredSize = m_listGrantedFeats.RequiredSize();
        if (requiredSize.cy > cy)
        {
            // make room for the vertical scroll bar
            cx -= (GetSystemMetrics(SM_CXVSCROLL) + 1);
        }
        int scrollY = GetScrollPos(SB_VERT);
        CRect rctControl(0, 0, cx, requiredSize.cy);
        rctControl -= CPoint(0, scrollY);
        m_listGrantedFeats.MoveWindow(rctControl);
        // show scroll bars if required
        SetScrollSizes(
                MM_TEXT,
                CSize(cx, requiredSize.cy));
    }
}

// EffectCallbackItem
void CGrantedFeatsPane::BuildLevelChanged(Build*)
{
    PopulateGrantedFeatsList();
}

void CGrantedFeatsPane::ClassChanged(Build*, const std::string&, const std::string&, size_t)
{
    PopulateGrantedFeatsList();
}

void CGrantedFeatsPane::FeatTrained(Build*, const std::string&)
{
    PopulateGrantedFeatsList();
}

void CGrantedFeatsPane::FeatRevoked(Build*, const std::string&)
{
    PopulateGrantedFeatsList();
}

void CGrantedFeatsPane::RaceChanged(Life*, const std::string&)
{
    PopulateGrantedFeatsList();
}

void CGrantedFeatsPane::FeatEffectApplied(Build*, const Effect& effect)
{
    AddGrantedFeatEffect(effect);
}

void CGrantedFeatsPane::FeatEffectRevoked(Build*, const Effect& effect)
{
    RevokeGrantedFeatEffect(effect);
}

void CGrantedFeatsPane::ItemEffectApplied(Build*, const Effect& effect)
{
    AddGrantedFeatEffect(effect);
}

void CGrantedFeatsPane::ItemEffectRevoked(Build*, const Effect& effect)
{
    RevokeGrantedFeatEffect(effect);
}

void CGrantedFeatsPane::ItemEffectApplied(Build*, const Effect& effect, WeaponType wt)
{
    UNREFERENCED_PARAMETER(wt);
    AddGrantedFeatEffect(effect);
}

void CGrantedFeatsPane::ItemEffectRevoked(Build*, const Effect& effect, WeaponType wt)
{
    UNREFERENCED_PARAMETER(wt);
    RevokeGrantedFeatEffect(effect);
}

void CGrantedFeatsPane::EnhancementTrained(Build*, const EnhancementItemParams& item)
{
    UNREFERENCED_PARAMETER(item);
}

void CGrantedFeatsPane::EnhancementRevoked(Build*, const EnhancementItemParams& item)
{
    UNREFERENCED_PARAMETER(item);
}

void CGrantedFeatsPane::EnhancementEffectApplied(Build*, const Effect& effect)
{
    AddGrantedFeatEffect(effect);
}

void CGrantedFeatsPane::EnhancementEffectRevoked(Build*, const Effect& effect)
{
    RevokeGrantedFeatEffect(effect);
}

void CGrantedFeatsPane::StanceActivated(Build* charData, const std::string& stanceName)
{
    UNREFERENCED_PARAMETER(charData);
    UNREFERENCED_PARAMETER(stanceName);
    PopulateGrantedFeatsList(); // granted feats may be stance dependent
}

void CGrantedFeatsPane::StanceDeactivated(Build* charData, const std::string& stanceName)
{
    UNREFERENCED_PARAMETER(charData);
    UNREFERENCED_PARAMETER(stanceName);
    PopulateGrantedFeatsList(); // granted feats may be stance dependent
}

void CGrantedFeatsPane::SliderChanged(Build*, const std::string&, int)
{
}

void CGrantedFeatsPane::GearChanged(Build*, InventorySlotType)
{
}

void CGrantedFeatsPane::PopulateGrantedFeatsList()
{
    // build the list of granted and inactive feats
    std::list<Feat> activeGrantedFeats;
    std::list<Feat> inactiveGrantedFeats;
    if (m_pCharacter != NULL)
    {
        const Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            for (auto&& gfit : m_grantedFeats)
            {
                Feat feat = FindFeat(gfit.Item().front());
                // make sure the feat has the requirements for its granted state
                if (gfit.HasRequirementsToBeActive())
                {
                    feat.SetRequirements(gfit.RequirementsToBeActive());
                }
                else
                {
                    Requirements empty;
                    feat.SetRequirements(empty);
                }
                if (feat.RequirementsToTrain().Met(*pBuild, pBuild->Level()-1, true, Inventory_Unknown, Weapon_Unknown, Weapon_Unknown))
                {
                    // its active
                    activeGrantedFeats.push_back(feat);
                }
                else
                {
                    // its inactive
                    inactiveGrantedFeats.push_back(feat);
                }
            }
        }
    }
    m_listGrantedFeats.LockWindowUpdate();
    m_listGrantedFeats.Clear();
    if (activeGrantedFeats.size() > 0)
    {
        m_listGrantedFeats.AddSection("Granted Feats", activeGrantedFeats);
    }
    if (inactiveGrantedFeats.size() > 0)
    {
        m_listGrantedFeats.AddSection("Inactive Granted Feats", inactiveGrantedFeats);
    }
    m_listGrantedFeats.UnlockWindowUpdate();
    CRect rctWindow;
    GetClientRect(&rctWindow);
    OnSize(SIZE_RESTORED, rctWindow.Width(), rctWindow.Height());
}

BOOL CGrantedFeatsPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_GRANTEDFEATS,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CGrantedFeatsPane::AddGrantedFeatEffect(const Effect& effect)
{
    // add to the list, or add another stack
    bool bFound = false;
    for (auto&& gfit : m_grantedFeats)
    {
        if (effect == gfit)
        {
            bFound = true;
            gfit.AddEffectStack();
        }
    }
    if (!bFound)
    {
        // its a new granted feat
        m_grantedFeats.push_back(effect); // has 1 stack by default
        Build* pBuild = m_pCharacter->ActiveBuild();
        // now determine whether this granted feat is already trained
        // if it is, we do not notify its effects. If its not, we do
        // we need to record the notify state for this item so that
        // if the user trains this feat in some other way we have to remove
        // the effects for the granted feat
        std::string featName = effect.Item().front();
        if (effect.HasRequirementsToBeActive()
                && !effect.RequirementsToBeActive().Met(*pBuild, pBuild->Level()-1, true, Inventory_Unknown, Weapon_Unknown, Weapon_Unknown))
        {
            // the feat although granted is inactive as its requirements were not met
            m_grantedNotifyState.push_back(false);
        }
        else if (!pBuild->IsFeatTrained(featName))
        {
            // this feat is not currently trained, we need to notify about its
            // effects
            const Feat & feat = FindFeat(featName);
            pBuild->NotifyFeatTrained(featName);
            pBuild->ApplyFeatEffects(feat);
            m_grantedNotifyState.push_back(true);
        }
        else
        {
            // we have not notified about this feats effects as it
            // is already trained through some other mechanism (either selected
            // or an automatic feat)
            m_grantedNotifyState.push_back(false);
        }
    }
    PopulateGrantedFeatsList();
}

void CGrantedFeatsPane::RevokeGrantedFeatEffect(const Effect& effect)
{
    // remove from the list, or lose another stack
    ASSERT(m_grantedFeats.size() == m_grantedNotifyState.size());
    bool bRemoved = false;
    std::list<bool>::iterator bit = m_grantedNotifyState.begin();
    std::list<Effect>::iterator gfit = m_grantedFeats.begin();
    while (gfit != m_grantedFeats.end())
    {
        if (effect == *gfit)
        {
            if ((*gfit).RevokeEffectStack())
            {
                // was the last stack, remove it from th list
                gfit = m_grantedFeats.erase(gfit);
                bit = m_grantedNotifyState.erase(bit);
                bRemoved = true;
            }
            else
            {
                gfit++;
                bit++;
            }
        }
        else
        {
            gfit++;
            bit++;
        }
    }
    if (bRemoved)
    {
        // granted feat was removed, remove its effects if it was active
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (!effect.HasRequirementsToBeActive()
                || effect.RequirementsToBeActive().Met(*pBuild, pBuild->Level()-1, true, Inventory_Unknown, Weapon_Unknown, Weapon_Unknown))
        {
            std::string featName = effect.Item().front();
            if (!pBuild->IsFeatTrained(featName))
            {
                // this feat is not currently trained, we need to notify about its
                // effects
                const Feat & feat = FindFeat(featName);
                pBuild->NotifyFeatRevoked(featName);
                pBuild->RevokeFeatEffects(feat);
            }
        }
    }
    PopulateGrantedFeatsList();
}

