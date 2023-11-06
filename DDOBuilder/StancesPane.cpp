// StancesPane.cpp
//
#include "stdafx.h"
#include "StancesPane.h"
#include "GlobalSupportFunctions.h"
#include "MainFrm.h"
#include "Race.h"

namespace
{
    const int c_windowSize = 38;
}

IMPLEMENT_DYNCREATE(CStancesPane, CFormView)

CStancesPane::CStancesPane() :
    CFormView(CStancesPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL),
    m_showingTip(false),
    m_tipCreated(false),
    m_pTooltipItem(NULL),
    m_nextStanceId(IDC_SPECIALFEAT_0),
    m_nextSliderId(50),
    m_bHadInitialUpdate(false)
{
}

CStancesPane::~CStancesPane()
{
    DestroyAllStances();
}

void CStancesPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_HIDDEN_SIZER, m_staticHiddenSizer);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CStancesPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_LBUTTONDOWN()
    ON_WM_HSCROLL()
END_MESSAGE_MAP()
#pragma warning(pop)

#ifdef _DEBUG
void CStancesPane::AssertValid() const
{
    CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CStancesPane::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CStancesPane::OnInitialUpdate()
{
    if (!m_bHadInitialUpdate)
    {
        m_bHadInitialUpdate = true;
        CFormView::OnInitialUpdate();
        m_tooltip.Create(this);
        m_tipCreated = true;
    }
}

void CStancesPane::PositionWindow(
        CWnd * pWnd,
        CRect * itemRect,
        int maxX)
{
    pWnd->MoveWindow(itemRect, TRUE);
    *itemRect += CPoint(itemRect->Width() + c_controlSpacing, 0);
    if (itemRect->right > maxX)
    {
        // oops, not enough space in client area here
        // move down and start the next row of controls
        *itemRect -= CPoint(itemRect->left, 0);
        *itemRect += CPoint(c_controlSpacing, itemRect->Height() + c_controlSpacing);
    }
    pWnd->Invalidate(TRUE);
}

void CStancesPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (m_stanceGroups.size() > 0)
    {
        // [Slider Label][Slider Control.........]
        // [User] [][][][][][][][][][][][][][][][]
        // [Auto] [][][][][][][][][][][][][][][][]
        CRect wndClient;
        GetClientRect(&wndClient);
        int sliderBottom = c_controlSpacing;
        // work out the right hand end of the slider controls
        int sliderEnd = c_windowSize + c_controlSpacing;
        while (sliderEnd < cx - (c_windowSize + c_controlSpacing))
        {
            sliderEnd += (c_windowSize + c_controlSpacing);
        }
        // do all the sliders
        std::list<SliderItem>::iterator si = m_sliders.begin();
        CRect rctSizer;
        m_staticHiddenSizer.GetWindowRect(rctSizer);
        rctSizer -= rctSizer.TopLeft();
        while (si != m_sliders.end())
        {
            CRect rctLabel(c_controlSpacing, sliderBottom, c_controlSpacing + 100, sliderBottom + rctSizer.Height());
            CRect rctSlider(rctLabel.right + c_controlSpacing, sliderBottom, sliderEnd, sliderBottom + rctSizer.Height());
            // now position the controls
            (*si).m_label->MoveWindow(rctLabel, TRUE);
            (*si).m_slider->MoveWindow(rctSlider, TRUE);
            (*si).m_label->ShowWindow(SW_SHOW);
            (*si).m_slider->ShowWindow(SW_SHOW);
            sliderBottom = rctSlider.bottom + c_controlSpacing;
            ++si;
        }
        CRect itemRect(
            c_controlSpacing,
            sliderBottom,
            c_windowSize + c_controlSpacing,
            sliderBottom + c_windowSize);
        for (auto&& sgi : m_stanceGroups)
        {
            PositionStanceGroup(*sgi, &itemRect, cx);
        }
        // show scroll bars if required
        SetScrollSizes(
            MM_TEXT,
            CSize(cx, itemRect.bottom + c_controlSpacing));
    }
}

void CStancesPane::PositionStanceGroup(
    StanceGroup& sg,
    CRect* pRect,
    int maxX)
{
    // user stance header first
    PositionWindow(sg.GroupLabel(), pRect, maxX);
    // move each stance button
    size_t numButtons = sg.NumButtons();
    bool bAutoGroup = sg.GroupName() == "Auto";
    for (size_t i = 0; i < numButtons; ++i)
    {
        if (bAutoGroup)
        {
            CWnd* pButton = sg.StanceButton(i);
            if (sg.GetStance(i)->IsSelected())
            {
                PositionWindow(pButton, pRect, maxX);
                pButton->ShowWindow(SW_SHOW);
            }
            else
            {
                CRect notVisibleRect(-100, -100, -68, -68);
                pButton->MoveWindow(notVisibleRect, FALSE);
                pButton->ShowWindow(SW_HIDE);
            }
        }
        else
        {
            // all non-auto group items are always shown
            PositionWindow(sg.StanceButton(i), pRect, maxX);
        }
    }
}

StanceGroup* CStancesPane::GetStanceGroup(CStanceButton* pButton)
{
    StanceGroup* pGroup = nullptr;
    for (auto&& sgit : m_stanceGroups)
    {
        if (sgit->HasStance(pButton))
        {
            pGroup = sgit;
        }
    }
    return pGroup;
}

StanceGroup* CStancesPane::GetStanceGroup(const std::string& name)
{
    StanceGroup* pGroup = nullptr;
    for (auto&& sgit : m_stanceGroups)
    {
        if (sgit->GroupName() == name)
        {
            pGroup = sgit;
        }
    }
    return pGroup;
}

LRESULT CStancesPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            m_pCharacter->ActiveLife()->DetachObserver(this);
            pBuild->DetachObserver(this);
        }
        m_pCharacter->DetachObserver(this);
        DestroyAllStances();
    }

    // wParam is the document pointer
    CDocument * pDoc = (CDocument*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        CreateStanceGroup("User", false);
        CreateStanceGroup("Auto", false);

        m_pCharacter->AttachObserver(this);
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            m_pCharacter->ActiveLife()->AttachObserver(this);
            pBuild->AttachObserver(this);
        }
        CreateStanceWindows();
        UpdateStanceStates();
    }
    return 0L;
}

BOOL CStancesPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_HIDDEN_SIZER,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CStancesPane::CreateStanceWindows()
{
    m_nextStanceId = IDC_SPECIALFEAT_0;
    // create all the stances defined in the Stances.xml file
    const std::list<Stance>& stances = Stances();
    for (auto&& sit : stances)
    {
        AddStance(sit);
    }

    // add the auto controlled stances for each weapon type. These are added dynamically
    for (size_t wt = Weapon_BastardSword; wt < Weapon_Count; ++wt)
    {
        if (wt < Weapon_ShieldBuckler || wt > Weapon_ShieldTower)
        {
            CString name = (LPCTSTR)EnumEntryText(static_cast<WeaponType>(wt), weaponTypeMap);
            CString prompt;
            if (IsVowel(name.GetAt(0)))
            {
                prompt.Format("You are wielding an %s", (LPCTSTR)name);
            }
            else
            {
                prompt.Format("You are wielding a %s", (LPCTSTR)name);
            }
            Stance weapon(
                    (LPCTSTR)name,
                    (LPCTSTR)name,
                    (LPCTSTR)prompt,
                    "Auto",
                    true);
            // the requirement depends of what type of "weapon" this is
            if (IsOffHandWeapon(static_cast<WeaponType>(wt)))
            {
                Requirement r(Requirement_ItemTypeInSlot, "Weapon2", (LPCTSTR)name);
                weapon.AddRequirement(r);
            }
            else
            {
                Requirement r(Requirement_ItemTypeInSlot, "Weapon1", (LPCTSTR)name);
                weapon.AddRequirement(r);
            }
            AddStance(weapon);
        }
    }
    // add the auto controlled stances for each race type. These are added dynamically
    const std::list<Race> & races = Races();
    for (auto&& rit: races)
    {
        CString name = rit.Name().c_str();
        CString icon = rit.Name().c_str();
        CString prompt;
        icon.Replace(" ", "");
        if (IsVowel(name.GetAt(0)))
        {
            prompt.Format("You are an %s", (LPCTSTR)name);
        }
        else
        {
            prompt.Format("You are a %s", (LPCTSTR)name);
        }
        Stance race(
                (LPCTSTR)name,
                (LPCTSTR)icon,
                (LPCTSTR)prompt,
                "Auto",
                true);
        Requirement r(Requirement_Race, rit.Name());
        race.AddRequirement(r);
        AddStance(race);
    }
}

void CStancesPane::AddStance(const Stance & stance)
{
    std::string group = "User";     // assume in user group
    if (stance.HasGroup())
    {
        group = stance.Group();
    }
    // only add the stance if it is not already present. Check all stance groups
    bool found = false;
    for (auto&& sgi : m_stanceGroups)
    {
        if (sgi->GroupName() == group)
        {
            sgi->AddStance(m_pCharacter, stance, group, this, m_nextStanceId);
            found = true;
            break;
        }
    }
    if (!found)
    {
        StanceGroup* pSG = CreateStanceGroup(group, group != "User");
        pSG->AddStance(m_pCharacter, stance, group, this, m_nextStanceId);
    }
    if (IsWindow(GetSafeHwnd()))
    {
        // now force an on size event to position everything
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}

void CStancesPane::RevokeStance(const Stance & stance)
{
    std::string group = "User";     // assume in user group
    if (stance.HasGroup())
    {
        group = stance.Group();
    }
    // only add the stance if it is not already present. Check all stance groups
    bool found = false;
    auto sgit = m_stanceGroups.begin();
    while (sgit != m_stanceGroups.end())
    {
        found = (*sgit)->RevokeStance(stance, group);
        if (found)
        {
            if ((*sgit)->NumButtons() == 0)
            {
                // if the group is now empty, we need to remove it
                // unless it is the "User" or "Auto" group
                if (group != "User"
                        && group != "Auto")
                {
                    (*sgit)->DestroyAll();
                    delete (*sgit);
                    m_stanceGroups.erase(sgit);
                }
            }
            break;
        }
        ++sgit;
    }
    if (IsWindow(GetSafeHwnd()))
    {
        // now force an on size event to position everything
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}

void CStancesPane::UpdateClassChanged(
        Build*,
        const std::string& /*classFrom*/,
        const std::string& /*classTo*/,
        size_t /*level*/)
{
    UpdateStanceStates();
}

void CStancesPane::UpdateNewStance(Build*, const Stance& stance)
{
    AddStance(stance);
}

void CStancesPane::UpdateRevokeStance(Build*, const Stance& stance)
{
    RevokeStance(stance);
}

void CStancesPane::UpdateStanceActivated(
        Build*,
        const std::string& stanceName)
{
    for (auto&& sgi : m_stanceGroups)
    {
        bool found = sgi->ActivateStance(stanceName);
        if (found)
        {
            break;
        }
    }
}

void CStancesPane::UpdateStanceDeactivated(
        Build*,
        const std::string& stanceName)
{
    for (auto&& sgi : m_stanceGroups)
    {
        bool found = sgi->DeactivateStance(stanceName);
        if (found)
        {
            break;
        }
    }
}

void CStancesPane::UpdateStanceDisabled(
        Build*,
        const std::string&)
{
    if (IsWindow(GetSafeHwnd()))
    {
        // now force an on size event to position everything
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}

void CStancesPane::UpdateFeatEffectApplied(
        Build* pBuild,
        const Effect & effect)
{
    // all handled the same way
    UpdateItemEffectApplied(pBuild, effect);
}

void CStancesPane::UpdateFeatEffectRevoked(
        Build*,
        const Effect & effect)
{
    UpdateSliders(effect, false);
}

void CStancesPane::UpdateEnhancementEffectApplied(
        Build* pBuild,
        const Effect & effect)
{
    // all handled the same way
    UpdateItemEffectApplied(pBuild, effect);
    UpdateStanceStates();
}

void CStancesPane::UpdateEnhancementEffectRevoked(
        Build*,
        const Effect & effect)
{
    UpdateSliders(effect, false);
    UpdateStanceStates();
}

void CStancesPane::UpdateItemEffectApplied(
        Build*,
        const Effect& effect)
{
    // see if this is an activate stance effect
    if (effect.IsType(Effect_ActivateStance))
    {
        {
            for (auto&& sgi : m_stanceGroups)
            {
                sgi->ActivateStance(effect.Item().front());
            }
        }
    }
    UpdateSliders(effect, true);
}

void CStancesPane::UpdateItemEffectRevoked(
        Build*,
        const Effect& effect)
{
    UpdateSliders(effect, false);
}

void CStancesPane::UpdateGearChanged(Build*, InventorySlotType)
{
    UpdateStanceStates();
}

void CStancesPane::OnLButtonDown(UINT nFlags, CPoint point)
{
    // determine which stance, if any was clicked on.
    // if they did click on a stance either activate or deactivate it
    CFormView::OnLButtonDown(nFlags, point);
    CWnd* pWnd = ChildWindowFromPoint(point);
    CStanceButton* pStance = dynamic_cast<CStanceButton*>(pWnd);
    if (pStance != NULL)
    {
        // must be a "user" controlled stance
        if (!pStance->GetStance().HasAutoControlled())
        {
            StanceGroup* pStanceGroup = GetStanceGroup(pStance);
            // yup, they clicked on a stance, now change its activation state
            if (pStance->IsDisabled())
            {
                // show action is not available
                ::MessageBeep(0xFFFFFFFF);
            }
            else if (pStance->IsSelected())
            {
                m_pCharacter->ActiveBuild()->DeactivateStance(pStance->GetStance(), pStanceGroup);
                pStance->SetSelected(false);
                m_pDocument->SetModifiedFlag(TRUE);
            }
            else
            {
                m_pCharacter->ActiveBuild()->ActivateStance(pStance->GetStance(), pStanceGroup);
                pStance->SetSelected(true);
                if (pStanceGroup->IsSingleSelection())
                {
                    // iterate the stance and deactivate any other stance that is active
                    size_t count = pStanceGroup->NumButtons();
                    for (size_t si = 0; si < count; ++si)
                    {
                        CStanceButton* pOtherStance = pStanceGroup->GetStance(si);
                        if (pOtherStance != pStance
                            && pOtherStance->IsSelected())
                        {
                            m_pCharacter->ActiveBuild()->DeactivateStance(pOtherStance->GetStance(), pStanceGroup);
                            pOtherStance->SetSelected(false);
                        }
                    }
                }
                m_pDocument->SetModifiedFlag(TRUE);
            }
        }
        else
        {
            // show action is not available
            ::MessageBeep(0xFFFFFFFF);
        }
    }
}

void CStancesPane::OnMouseMove(UINT, CPoint point)
{
    // determine which stance the mouse may be over
    CWnd * pWnd = ChildWindowFromPoint(point);
    CStanceButton * pStance = dynamic_cast<CStanceButton*>(pWnd);
    if (pStance != NULL
            && pStance != m_pTooltipItem)
    {
        CRect itemRect;
        pStance->GetWindowRect(&itemRect);
        ScreenToClient(itemRect);
        // over a new item or a different item
        m_pTooltipItem = pStance;
        ShowTip(*pStance, itemRect);
    }
    else
    {
        if (m_showingTip
                && pStance != m_pTooltipItem)
        {
            // no longer over the same item
            HideTip();
        }
    }
}

LRESULT CStancesPane::OnMouseLeave(WPARAM, LPARAM)
{
    // hide any tooltip when the mouse leaves the area its being shown for
    HideTip();
    return 0;
}

void CStancesPane::ShowTip(const CStanceButton & item, CRect itemRect)
{
    if (m_showingTip)
    {
        m_tooltip.Hide();
    }
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    CPoint tipAlternate(itemRect.left, itemRect.top - 2);
    SetTooltipText(item, tipTopLeft, tipAlternate);
    m_showingTip = true;
    // track the mouse so we know when it leaves our window
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;
    _TrackMouseEvent(&tme);
}

void CStancesPane::HideTip()
{
    // tip not shown if not over an assay
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
        m_pTooltipItem = NULL;
    }
}

void CStancesPane::SetTooltipText(
        const CStanceButton & item,
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
    m_tooltip.SetStanceItem(item.GetStance());
    m_tooltip.Show();
}

void CStancesPane::DestroyStances(std::vector<CStanceButton *> & items)
{
    for (size_t i = 0; i < items.size(); ++i)
    {
        items[i]->DestroyWindow();
        delete items[i];
        items[i] = NULL;
    }
    items.clear();
}

void CStancesPane::DestroyAllStances()
{
    for (auto&& sgi : m_stanceGroups)
    {
        sgi->DestroyAll();
        delete sgi;
    }
    m_stanceGroups.clear();
    m_nextStanceId = IDC_SPECIALFEAT_0;

    std::list<SliderItem>::iterator it = m_sliders.begin();
    while (it != m_sliders.end())
    {
        // time for this slider to disappear
        (*it).m_label->DestroyWindow();
        (*it).m_slider->DestroyWindow();
        it = m_sliders.erase(it);
    }
    if (IsWindow(GetSafeHwnd()))
    {
        // now force an on size event to position everything
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}

const std::vector<CStanceButton *> & CStancesPane::UserStances() const
{
    static std::vector<CStanceButton *> activeStances;
    activeStances.clear();
    //for (auto&& sgi : m_stanceGroups)
    //{
        //sgi.AddActiveStances(&activeStances);
    //}
    return activeStances;
}

const std::vector<CStanceButton *> & CStancesPane::AutoStances() const
{
    static std::vector<CStanceButton *> activeStances;
    activeStances.clear();
    //for (auto&& sgi : m_stanceGroups)
    //{
        //sgi.AddActiveStances(&activeStances);
    //}
    return activeStances;
}

const CStanceButton * CStancesPane::GetStance(const std::string& stanceName)
{
    const CStanceButton * pButton = NULL;
    for (auto&& sgi : m_stanceGroups)
    {
        pButton = sgi->GetStance(stanceName);
        if (pButton != NULL)
        {
            break;
        }
    }
    return pButton;
}

void CStancesPane::OnHScroll(UINT, UINT nPos, CScrollBar * pScrollbar)
{
    if (m_pCharacter != NULL
            && pScrollbar != NULL)  // must not be the window scroll bar
    {
        // find which control has changed and update if required
        UINT id = pScrollbar->GetDlgCtrlID();
        std::list<SliderItem>::iterator it = GetSlider(id);
        if (it != m_sliders.end())
        {
            nPos = (*it).m_slider->GetPos();
            if ((*it).m_position != static_cast<int>(nPos))
            {
                (*it).m_position = nPos;
                CString windowText;
                windowText.Format("%s: %d", (*it).m_name.c_str(), (*it).m_position);
                (*it).m_label->SetWindowText(windowText);
                m_pCharacter->ActiveBuild()->StanceSliderChanged((*it).m_name, nPos);
            }
        }
    }
}

void CStancesPane::UpdateSliders(const Effect & effect, bool bApply)
{
    if (effect.IsType(Effect_CreateSlider))
    {
        if (bApply)
        {
            // need to create or increment slider
            std::list<SliderItem>::iterator it = GetSlider(effect, true);
            (*it).m_creationCount++;    // count a reference
        }
        else
        {
            // need to decrement or removal slider
            std::list<SliderItem>::iterator it = GetSlider(effect, false);
            if (it != m_sliders.end())
            {
                // does exist, we can revoke a layer
                (*it).m_creationCount--;    // lose a reference
                if ((*it).m_creationCount == 0)
                {
                    // time for this slider to disappear
                    (*it).m_label->DestroyWindow();
                    (*it).m_slider->DestroyWindow();
                    m_sliders.erase(it);
                    // make sure controls are positioned
                    CRect rctWindow;
                    GetWindowRect(&rctWindow);
                    OnSize(SIZE_RESTORED, rctWindow.Width(), rctWindow.Height());
                }
            }
        }
    }
}

std::list<SliderItem>::iterator CStancesPane::GetSlider(
        const Effect & effect,
        bool bCreateIfMissing)
{
    bool bFound = false;
    std::list<SliderItem>::iterator it = m_sliders.begin();
    while (!bFound && it != m_sliders.end())
    {
        if ((*it).m_name == effect.Item().front())
        {
            bFound = true;  // this is the droid we're looking for
        }
        else
        {
            ++it;
        }
    }
    if (!bFound && bCreateIfMissing)
    {
        // add a new entry to the end
        SliderItem slider;
        m_sliders.push_front(slider);
        it = m_sliders.begin();
        (*it).CreateControls();
        // now initialise it fully
        (*it).m_sliderControlId = m_nextSliderId;
        (*it).m_name = effect.Item().front();
        (*it).m_creationCount = 0;  // increment outside
        (*it).m_position = (int)effect.Amount().at(0);  // default value
        (*it).m_sliderMin = (int)effect.Amount().at(1); // min value
        (*it).m_sliderMax = (int)effect.Amount().at(2); // max value
        // and create the windows controls
        CString windowText;
        windowText.Format("%s: %d", (*it).m_name.c_str(), (*it).m_position);
        (*it).m_label->Create(
                windowText,
                SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD,
                CRect(0, 0, 10, 10),        // updates on OnSize
                this,
                (*it).m_sliderControlId+1);
        (*it).m_slider->Create(
                WS_VISIBLE | WS_CHILD | TBS_AUTOTICKS | TBS_HORZ | TBS_BOTTOM,
                CRect(0, 0, 10, 10),        // updates on OnSize
                this,
                (*it).m_sliderControlId);
        CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
        (*it).m_label->SetFont(pDefaultGUIFont, TRUE);
        (*it).m_slider->SetFont(pDefaultGUIFont, TRUE);
        (*it).m_slider->SetRange((*it).m_sliderMin, (*it).m_sliderMax);
        (*it).m_slider->SetPos((*it).m_position);
        m_nextSliderId += 2;    // skip 2 control ids (label, slider)
        // make sure controls are positioned
        CRect rctWindow;
        GetClientRect(&rctWindow);
        OnSize(SIZE_RESTORED, rctWindow.Width(), rctWindow.Height());
    }
    return it;
}

std::list<SliderItem>::iterator CStancesPane::GetSlider(UINT controlId)
{
    bool bFound = false;
    std::list<SliderItem>::iterator it = m_sliders.begin();
    while (!bFound && it != m_sliders.end())
    {
        if ((*it).m_sliderControlId == controlId)
        {
            bFound = true;  // this is the droid we're looking for
        }
        else
        {
            ++it;
        }
    }
    ASSERT(bFound);
    return it;
}

const SliderItem * CStancesPane::GetSlider(
        const std::string& name) const
{
    const SliderItem * pSlider = NULL;
    bool bFound = false;
    std::list<SliderItem>::const_iterator it = m_sliders.begin();
    while (!bFound && it != m_sliders.end())
    {
        if ((*it).m_name == name)
        {
            bFound = true;  // this is the droid we're looking for
            pSlider = &(*it);
        }
        else
        {
            ++it;
        }
    }
    return pSlider;
}

bool CStancesPane::IsStanceActive(const std::string& name, WeaponType wt) const
{
    UNREFERENCED_PARAMETER(wt);
    bool bEnabled = false;
    // some special stances are based on a slider position
    // all these stances start with a numeric with the format of
    // "<number>% <stance name>"
    size_t percentPos = name.find("%");
    if (percentPos != std::string::npos
            && percentPos + 2 <= name.length()) // don't crash on badly formatted stance name
    {
        // this is a special stance
        int value = atoi(name.c_str());
        // identify the slider stance in question
        std::string stanceName = name.substr(name.find('%') + 2, 50);
        const SliderItem * pSlider = GetSlider(stanceName);
        if (pSlider != NULL)
        {
            // if the value is negative its an under comparison else its an over
            if (value < 0)
            {
                value = -value;
                bEnabled = (pSlider->m_position < value);
            }
            else
            {
                bEnabled = (pSlider->m_position >= value);
            }
        }
    }
    return bEnabled;
}

void CStancesPane::UpdateActiveBuildChanged(Character*)
{
    DestroyAllStances();
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        m_pCharacter->ActiveLife()->AttachObserver(this);
        pBuild->AttachObserver(this);
        CreateStanceWindows();
        UpdateStanceStates();
    }
}

void CStancesPane::UpdateAlignmentChanged(Life*, AlignmentType)
{
    UpdateStanceStates();
}

void CStancesPane::UpdateRaceChanged(Life*, const std::string&)
{
    UpdateStanceStates();
}

void CStancesPane::UpdateStanceStates()
{
    bool bChanged = false;
    // update auto controlled stances first
    for (auto&& sgi : m_stanceGroups)
    {
        if (sgi->GroupName() == "Auto")
        {
            size_t stanceCount = sgi->NumButtons();
            for (size_t index = 0; index < stanceCount; ++index)
            {
                CStanceButton* pStanceButton = sgi->GetStance(index);
                bChanged |= pStanceButton->Evaluate(m_pCharacter);
            }
        }
    }
    // now update all the other stances
    for (auto&& sgi : m_stanceGroups)
    {
        if (sgi->GroupName() != "Auto")
        {
            size_t stanceCount = sgi->NumButtons();
            for (size_t index = 0; index < stanceCount; ++index)
            {
                CStanceButton* pStanceButton = sgi->GetStance(index);
                bChanged |= pStanceButton->Evaluate(m_pCharacter);
            }
        }
    }
    if (bChanged)
    {
        // now force an on size event to position everything
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}


StanceGroup* CStancesPane::CreateStanceGroup(
        const std::string& strName,
        bool singleSelection)
{
    StanceGroup* sg = new StanceGroup(strName, singleSelection);
    m_stanceGroups.push_back(sg);
    m_stanceGroups.back()->CreateWindows(this, m_nextStanceId);
    if (IsWindow(GetSafeHwnd()))
    {
        // now force an on size event to position everything
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
    // make sure the "User" group is at the start of the list and "Auto"
    // at the end
    StanceGroup* pUser = GetStanceGroup("User");
    StanceGroup* pAuto = GetStanceGroup("Auto");
    std::list<StanceGroup*> orderedStanceGroups;
    if (pUser != NULL)
    {
        orderedStanceGroups.push_back(pUser);
    }
    for (auto&& sgit: m_stanceGroups)
    {
        if (sgit->GroupName() != "User"
            && sgit->GroupName() != "Auto")
        {
            orderedStanceGroups.push_back(sgit);
        }
    }
    if (pAuto != NULL)
    {
        orderedStanceGroups.push_back(pAuto);
    }
    // now assign the ordered group list
    m_stanceGroups = orderedStanceGroups;
    return sg;
}
