// DDOBuilderView.cpp
//
#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DDOBuilder.h"
#endif

#include "DDOBuilderDoc.h"
#include "DDOBuilderView.h"
#include "GlobalSupportFunctions.h"
#include "MainFrm.h"
//#include "ActiveStanceDialog.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDDOBuilderView
namespace
{
    const int c_controlSpacing = 3;
    COLORREF f_abilityOverspendColour = RGB(0xE9, 0x96, 0x7A); // dark salmon
}

IMPLEMENT_DYNCREATE(CDDOBuilderView, CFormView)

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CDDOBuilderView, CFormView)
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, &CDDOBuilderView::OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, &CDDOBuilderView::OnLoadComplete)
    ON_REGISTERED_MESSAGE(UWM_THEME_CHANGED, &CDDOBuilderView::OnThemeChanged)
    ON_BN_CLICKED(IDC_BUTTON_STR_PLUS, &CDDOBuilderView::OnButtonStrPlus)
    ON_BN_CLICKED(IDC_BUTTON_STR_MINUS, &CDDOBuilderView::OnButtonStrMinus)
    ON_BN_CLICKED(IDC_BUTTON_DEX_PLUS, &CDDOBuilderView::OnButtonDexPlus)
    ON_BN_CLICKED(IDC_BUTTON_DEX_MINUS, &CDDOBuilderView::OnButtonDexMinus)
    ON_BN_CLICKED(IDC_BUTTON_CON_PLUS, &CDDOBuilderView::OnButtonConPlus)
    ON_BN_CLICKED(IDC_BUTTON_CON_MINUS, &CDDOBuilderView::OnButtonConMinus)
    ON_BN_CLICKED(IDC_BUTTON_INT_PLUS, &CDDOBuilderView::OnButtonIntPlus)
    ON_BN_CLICKED(IDC_BUTTON_INT_MINUS, &CDDOBuilderView::OnButtonIntMinus)
    ON_BN_CLICKED(IDC_BUTTON_WIS_PLUS, &CDDOBuilderView::OnButtonWisPlus)
    ON_BN_CLICKED(IDC_BUTTON_WIS_MINUS, &CDDOBuilderView::OnButtonWisMinus)
    ON_BN_CLICKED(IDC_BUTTON_CHA_PLUS, &CDDOBuilderView::OnButtonChaPlus)
    ON_BN_CLICKED(IDC_BUTTON_CHA_MINUS, &CDDOBuilderView::OnButtonChaMinus)
    ON_CBN_SELENDOK(IDC_COMBO_RACE, &CDDOBuilderView::OnSelendokComboRace)
    ON_CBN_SELENDOK(IDC_COMBO_ALIGNMENT, &CDDOBuilderView::OnSelendokComboAlignment)
    ON_CBN_SELENDOK(IDC_COMBO_TOME_STR, &CDDOBuilderView::OnSelendokComboTomeStr)
    ON_CBN_SELENDOK(IDC_COMBO_TOME_DEX, &CDDOBuilderView::OnSelendokComboTomeDex)
    ON_CBN_SELENDOK(IDC_COMBO_TOME_CON, &CDDOBuilderView::OnSelendokComboTomeCon)
    ON_CBN_SELENDOK(IDC_COMBO_TOME_INT, &CDDOBuilderView::OnSelendokComboTomeInt)
    ON_CBN_SELENDOK(IDC_COMBO_TOME_WIS, &CDDOBuilderView::OnSelendokComboTomeWis)
    ON_CBN_SELENDOK(IDC_COMBO_TOME_CHA, &CDDOBuilderView::OnSelendokComboTomeCha)
    ON_EN_CHANGE(IDC_NAME, &CDDOBuilderView::OnChangeName)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL4_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel4)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL8_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel8)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL12_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel12)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL16_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel16)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL20_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel20)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL24_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel24)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL28_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel28)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL32_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel32)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL36_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel36)
    ON_CBN_SELENDOK(IDC_COMBO_LEVEL40_ABILITY, &CDDOBuilderView::OnSelendokComboAbilityLevel40)
    ON_WM_SIZE()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_CHECK_GUILD_BUFFS, &CDDOBuilderView::OnButtonGuildBuffs)
    ON_EN_KILLFOCUS(IDC_EDIT_GUILD_LEVEL, &CDDOBuilderView::OnKillFocusGuildLevel)
    ON_BN_CLICKED(IDC_RADIO_28PT, &CDDOBuilderView::OnBnClickedRadio28pt)
    ON_BN_CLICKED(IDC_RADIO_32PT, &CDDOBuilderView::OnBnClickedRadio32pt)
    //ON_COMMAND(ID_EDIT_RESETBUILD, &CDDOBuilderView::OnEditResetbuild)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FEATS_EPICONLY, &CDDOBuilderView::OnEditFeatsUpdateEpicOnly)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FEATS_SHOWUNAVAILABLE, &CDDOBuilderView::OnEditFeatsUpdateShowUnavailable)
    ON_COMMAND(ID_EDIT_FEATS_EPICONLY, &CDDOBuilderView::OnEditFeatsEpicOnly)
    ON_COMMAND(ID_EDIT_FEATS_SHOWUNAVAILABLE, &CDDOBuilderView::OnEditFeatsShowUnavailable)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CDDOBuilderView::OnTtnNeedText)
    ON_COMMAND(ID_DEVELOPMENT_DUMPWEAPONGROUPSTOLOG, &CDDOBuilderView::OnDumpWeaponGroups)
    ON_UPDATE_COMMAND_UI(ID_DEVELOPMENT_DUMPWEAPONGROUPSTOLOG, &CDDOBuilderView::OnUpdateDumpWeaponGroups)
END_MESSAGE_MAP()
#pragma warning(pop)

// CDDOBuilderView construction/destruction

CDDOBuilderView::CDDOBuilderView() :
    CFormView(CDDOBuilderView::IDD),
    m_pCharacter(NULL),
    m_bIgnoreFocus(false),
    m_bHadIntialUpdate(false)
{
}

CDDOBuilderView::~CDDOBuilderView()
{
}

void CDDOBuilderView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_BUILD, m_staticBuildDescription);
    DDX_Control(pDX, IDC_RADIO_28PT, m_button28Pt);
    DDX_Control(pDX, IDC_RADIO_32PT, m_button32Pt);
    DDX_Control(pDX, IDC_RADIO_34PT, m_button34Pt);
    DDX_Control(pDX, IDC_RADIO_36PT, m_button36Pt);
    DDX_Control(pDX, IDC_NAME, m_editName);
    DDX_Control(pDX, IDC_COMBO_RACE, m_comboRace);
    DDX_Control(pDX, IDC_COMBO_ALIGNMENT, m_comboAlignment);
    DDX_Control(pDX, IDC_EDIT_STR, m_editStr);
    DDX_Control(pDX, IDC_EDIT_DEX, m_editDex);
    DDX_Control(pDX, IDC_EDIT_CON, m_editCon);
    DDX_Control(pDX, IDC_EDIT_INT, m_editInt);
    DDX_Control(pDX, IDC_EDIT_WIS, m_editWis);
    DDX_Control(pDX, IDC_EDIT_CHA, m_editCha);
    DDX_Control(pDX, IDC_EDIT_STR_COST, m_editCostStr);
    DDX_Control(pDX, IDC_EDIT_DEX_COST, m_editCostDex);
    DDX_Control(pDX, IDC_EDIT_CON_COST, m_editCostCon);
    DDX_Control(pDX, IDC_EDIT_INT_COST, m_editCostInt);
    DDX_Control(pDX, IDC_EDIT_WIS_COST, m_editCostWis);
    DDX_Control(pDX, IDC_EDIT_CHA_COST, m_editCostCha);
    DDX_Control(pDX, IDC_COMBO_TOME_STR, m_comboTomeStr);
    DDX_Control(pDX, IDC_COMBO_TOME_DEX, m_comboTomeDex);
    DDX_Control(pDX, IDC_COMBO_TOME_CON, m_comboTomeCon);
    DDX_Control(pDX, IDC_COMBO_TOME_INT, m_comboTomeInt);
    DDX_Control(pDX, IDC_COMBO_TOME_WIS, m_comboTomeWis);
    DDX_Control(pDX, IDC_COMBO_TOME_CHA, m_comboTomeCha);
    DDX_Control(pDX, IDC_BUTTON_STR_PLUS, m_buttonStrPlus);
    DDX_Control(pDX, IDC_BUTTON_STR_MINUS, m_buttonStrMinus);
    DDX_Control(pDX, IDC_BUTTON_DEX_PLUS, m_buttonDexPlus);
    DDX_Control(pDX, IDC_BUTTON_DEX_MINUS, m_buttonDexMinus);
    DDX_Control(pDX, IDC_BUTTON_CON_PLUS, m_buttonConPlus);
    DDX_Control(pDX, IDC_BUTTON_CON_MINUS, m_buttonConMinus);
    DDX_Control(pDX, IDC_BUTTON_INT_PLUS, m_buttonIntPlus);
    DDX_Control(pDX, IDC_BUTTON_INT_MINUS, m_buttonIntMinus);
    DDX_Control(pDX, IDC_BUTTON_WIS_PLUS, m_buttonWisPlus);
    DDX_Control(pDX, IDC_BUTTON_WIS_MINUS, m_buttonWisMinus);
    DDX_Control(pDX, IDC_BUTTON_CHA_PLUS, m_buttonChaPlus);
    DDX_Control(pDX, IDC_BUTTON_CHA_MINUS, m_buttonChaMinus);
    DDX_Control(pDX, IDC_CHECK_GUILD_BUFFS, m_buttonGuildBuffs);
    DDX_Control(pDX, IDC_EDIT_GUILD_LEVEL, m_editGuildLevel);
    DDX_Control(pDX, IDC_STATIC_AVAILABLE_POINTS, m_staticAvailableSpend);
    DDX_Control(pDX, IDC_COMBO_LEVEL4_ABILITY, m_comboAILevel4);
    DDX_Control(pDX, IDC_COMBO_LEVEL8_ABILITY, m_comboAILevel8);
    DDX_Control(pDX, IDC_COMBO_LEVEL12_ABILITY, m_comboAILevel12);
    DDX_Control(pDX, IDC_COMBO_LEVEL16_ABILITY, m_comboAILevel16);
    DDX_Control(pDX, IDC_COMBO_LEVEL20_ABILITY, m_comboAILevel20);
    DDX_Control(pDX, IDC_COMBO_LEVEL24_ABILITY, m_comboAILevel24);
    DDX_Control(pDX, IDC_COMBO_LEVEL28_ABILITY, m_comboAILevel28);
    DDX_Control(pDX, IDC_COMBO_LEVEL32_ABILITY, m_comboAILevel32);
    DDX_Control(pDX, IDC_COMBO_LEVEL36_ABILITY, m_comboAILevel36);
    DDX_Control(pDX, IDC_COMBO_LEVEL40_ABILITY, m_comboAILevel40);

    // theme controls
    DDX_Control(pDX, IDC_STATIC_NAME, m_staticTheme[0]);
    DDX_Control(pDX, IDC_STATIC_RACE, m_staticTheme[1]);
    DDX_Control(pDX, IDC_STATIC_ALIGNMENT, m_staticTheme[2]);
    DDX_Control(pDX, IDC_STATIC_ABILITY_VALUE, m_staticTheme[3]);
    DDX_Control(pDX, IDC_STATIC_COST, m_staticTheme[4]);
    DDX_Control(pDX, IDC_STATIC_TOME, m_staticTheme[5]);
    DDX_Control(pDX, IDC_STATIC_STR, m_staticTheme[6]);
    DDX_Control(pDX, IDC_STATIC_DEX, m_staticTheme[7]);
    DDX_Control(pDX, IDC_STATIC_CON, m_staticTheme[8]);
    DDX_Control(pDX, IDC_STATIC_INT, m_staticTheme[9]);
    DDX_Control(pDX, IDC_STATIC_WIS, m_staticTheme[10]);
    DDX_Control(pDX, IDC_STATIC_CHA, m_staticTheme[11]);
    DDX_Control(pDX, IDC_STATIC_LEVEL, m_staticTheme[12]);
    DDX_Control(pDX, IDC_STATIC_LEVEL4, m_staticTheme[13]);
    DDX_Control(pDX, IDC_STATIC_LEVEL8, m_staticTheme[14]);
    DDX_Control(pDX, IDC_STATIC_LEVEL12, m_staticTheme[15]);
    DDX_Control(pDX, IDC_STATIC_LEVEL16, m_staticTheme[16]);
    DDX_Control(pDX, IDC_STATIC_LEVEL20, m_staticTheme[17]);
    DDX_Control(pDX, IDC_STATIC_LEVEL24, m_staticTheme[18]);
    DDX_Control(pDX, IDC_STATIC_LEVEL28, m_staticTheme[19]);
    DDX_Control(pDX, IDC_STATIC_LEVEL32, m_staticTheme[20]);
    DDX_Control(pDX, IDC_STATIC_LEVEL36, m_staticTheme[21]);
    DDX_Control(pDX, IDC_STATIC_LEVEL40, m_staticTheme[22]);
}

void CDDOBuilderView::OnInitialUpdate()
{
    if (!m_bHadIntialUpdate)
    {
        CFormView::OnInitialUpdate();
        m_pCharacter = GetDocument()->GetCharacter();
        m_pCharacter->AttachObserver(this);
        // controls disabled until data load complete on program startup
        DisableControls();
        m_bHadIntialUpdate = true;
        OnThemeChanged(DarkModeEnabled(), 0L);
    }
}

LRESULT CDDOBuilderView::OnNewDocument(WPARAM, LPARAM lParam)
{
    if (m_bHadIntialUpdate)
    {
        Character* pCharacter = (Character*)(lParam);
        if (pCharacter != NULL)
        {
            pCharacter->AttachObserver(this);
        }
        PopulateComboboxes();
        UpdateRadioPoints();

        RestoreControls();      // puts controls to values of loaded data
        EnableButtons();
        UpdateBuildDescription();
    }
    return 0;
}

LRESULT CDDOBuilderView::OnLoadComplete(WPARAM, LPARAM)
{
    PopulateComboboxes();
    UpdateRadioPoints();

    RestoreControls();      // puts controls to values of loaded data
    EnableButtons();
    UpdateBuildDescription();
    return 0;
}

LRESULT CDDOBuilderView::OnThemeChanged(WPARAM wParam, LPARAM)
{
    // wParam = 1 for dark mode, else = 0 for all others
    bool bDark = (wParam != 0);
    COLORREF clrText = bDark ? RGB(255, 255, 255) : RGB(0, 0, 0);   // white text in dark mode, else black
    m_staticBuildDescription.SetTextColour(clrText);
    m_staticAvailableSpend.SetTextColour(clrText);
    for (size_t i = 0; i < MSS_Number; ++i)
    {
        m_staticTheme[i].SetTextColour(clrText);
    }
    return 0;
}

void CDDOBuilderView::OnRButtonUp(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CDDOBuilderView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    UNREFERENCED_PARAMETER(pWnd);
    UNREFERENCED_PARAMETER(point);
#ifndef SHARED_HANDLERS
    theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

BOOL CDDOBuilderView::OnEraseBkgnd(CDC* pDC)
{
    // note that static controls are not in this list so they get drawn with the current
    // theme background
    static int controlsNotToBeErased[] =
    {
        IDC_RADIO_28PT,
        IDC_RADIO_32PT,
        IDC_RADIO_34PT,
        IDC_RADIO_36PT,
        IDC_NAME,
        IDC_COMBO_RACE,
        IDC_COMBO_ALIGNMENT,
        IDC_BUTTON_STR_PLUS,
        IDC_EDIT_STR,
        IDC_BUTTON_STR_MINUS,
        IDC_EDIT_STR_COST,
        IDC_COMBO_TOME_STR,
        IDC_BUTTON_DEX_PLUS,
        IDC_EDIT_DEX,
        IDC_BUTTON_DEX_MINUS,
        IDC_EDIT_DEX_COST,
        IDC_COMBO_TOME_DEX,
        IDC_BUTTON_CON_PLUS,
        IDC_EDIT_CON,
        IDC_BUTTON_CON_MINUS,
        IDC_EDIT_CON_COST,
        IDC_COMBO_TOME_CON,
        IDC_BUTTON_INT_PLUS,
        IDC_EDIT_INT,
        IDC_BUTTON_INT_MINUS,
        IDC_EDIT_INT_COST,
        IDC_COMBO_TOME_INT,
        IDC_BUTTON_WIS_PLUS,
        IDC_EDIT_WIS,
        IDC_BUTTON_WIS_MINUS,
        IDC_EDIT_WIS_COST,
        IDC_COMBO_TOME_WIS,
        IDC_BUTTON_CHA_PLUS,
        IDC_EDIT_CHA,
        IDC_BUTTON_CHA_MINUS,
        IDC_EDIT_CHA_COST,
        IDC_COMBO_TOME_CHA,
        IDC_CHECK_GUILD_BUFFS,
        IDC_EDIT_GUILD_LEVEL,
        IDC_COMBO_LEVEL4_ABILITY,
        IDC_COMBO_LEVEL8_ABILITY,
        IDC_COMBO_LEVEL12_ABILITY,
        IDC_COMBO_LEVEL16_ABILITY,
        IDC_COMBO_LEVEL20_ABILITY,
        IDC_COMBO_LEVEL24_ABILITY,
        IDC_COMBO_LEVEL28_ABILITY,
        0 // end marker
    };

    pDC->SaveDC();

    const int * pId = controlsNotToBeErased;
    while (*pId != 0)
    {
        // Get rectangle of the control.
        CWnd * pControl = GetDlgItem(*pId);
        if (pControl && pControl->IsWindowVisible())
        {
            CRect controlClip;
            pControl->GetWindowRect(&controlClip);
            ScreenToClient(&controlClip);
            if (pControl->IsKindOf(RUNTIME_CLASS(CComboBox)))
            {
                // combo boxes return the height of the whole control, including the drop rectangle
                // reduce the height to the control size without it shown
                controlClip.bottom = controlClip.top
                        + GetSystemMetrics(SM_CYHSCROLL)
                        + GetSystemMetrics(SM_CYEDGE) * 2;
                // special case for combo boxes with image lists
                CComboBoxEx * pCombo = dynamic_cast<CComboBoxEx*>(pControl);
                if (pCombo != NULL)
                {
                    CImageList * pImage = pCombo->GetImageList();
                    if (pImage != NULL)
                    {
                        IMAGEINFO info;
                        pImage->GetImageInfo(0, &info);
                        // limit to the the height of the selection combo
                        controlClip.bottom = controlClip.top
                                + info.rcImage.bottom
                                - info.rcImage.top
                                + GetSystemMetrics(SM_CYEDGE) * 2;
                    }
                }
            }
            pDC->ExcludeClipRect(&controlClip);
        }

        // next in list
        ++pId;
    }
    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}
// CDDOBuilderView diagnostics

#ifdef _DEBUG
void CDDOBuilderView::AssertValid() const
{
    CFormView::AssertValid();
}

void CDDOBuilderView::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}

CDDOBuilderDoc* CDDOBuilderView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDDOBuilderDoc)));
    return (CDDOBuilderDoc*)m_pDocument;
}
#endif //_DEBUG

// CDDOBuilderView message handlers
void CDDOBuilderView::PopulateComboboxes()
{
    // setup the combo-boxes to have selectable content
    if (m_comboRace.GetCount() == 0)        // only do once
    {
        // Available races
        const std::list<Race>& races = Races();
        std::list<Race>::const_iterator cri = races.begin();
        while (cri != races.end())
        {
            CString text = (*cri).Name().c_str();
            m_comboRace.AddString(text);
            ++cri;
        }

        // Available alignments
        for (size_t ai = Alignment_Unknown + 1; ai < Alignment_Count; ++ai)
        {
            CString text = EnumEntryText(
                (AlignmentType)ai,
                alignmentTypeMap);
            size_t i = m_comboAlignment.AddString(text);
            m_comboAlignment.SetItemData(i, ai);
        }

        // ability tomes
        int maxAbilityTome = 8;
        CSettingsStoreSP regSP;
        CSettingsStore& reg = regSP.Create(FALSE, TRUE);
        if (reg.Open("Settings"))
        {
            reg.Read("MaxAbilityTome", maxAbilityTome);
        }
        for (int ti = 0; ti <= maxAbilityTome; ++ti)
        {
            CString text;
            if (ti > 0)
            {
                text.Format("%+d Tome", ti);
            }
            else
            {
                text = "No Tome";
            }
            // add to all 6 ability tome combo-boxes
            size_t i = m_comboTomeStr.InsertString(ti, text);
            m_comboTomeStr.SetItemData(i, ti);
            if (ti > 0)
            {
                // Supreme option on 1st stat
                CString supreme;
                supreme.Format("%+d Supreme Tome", ti);
                i = m_comboTomeStr.AddString(supreme);
                m_comboTomeStr.SetItemData(i, -ti); // negative tome values for supreme
            }
            i = m_comboTomeDex.AddString(text);
            m_comboTomeDex.SetItemData(i, ti);
            i = m_comboTomeCon.AddString(text);
            m_comboTomeCon.SetItemData(i, ti);
            i = m_comboTomeInt.AddString(text);
            m_comboTomeInt.SetItemData(i, ti);
            i = m_comboTomeWis.AddString(text);
            m_comboTomeWis.SetItemData(i, ti);
            i = m_comboTomeCha.AddString(text);
            m_comboTomeCha.SetItemData(i, ti);
        }

        // level up stats
        for (int ai = Ability_Unknown + 1; ai < Ability_Count; ++ai)
        {
            CString text = EnumEntryText(
                (AbilityType)ai,
                abilityTypeMap);
            CString textAll = "All ";
            textAll += text;
            size_t i = m_comboAILevel4.AddString(text);
            m_comboAILevel4.SetItemData(i, ai);
            i = m_comboAILevel4.AddString(textAll);
            m_comboAILevel4.SetItemData(i, -ai); // negative stat value for all
            i = m_comboAILevel8.AddString(text);
            m_comboAILevel8.SetItemData(i, ai);
            i = m_comboAILevel12.AddString(text);
            m_comboAILevel12.SetItemData(i, ai);
            i = m_comboAILevel16.AddString(text);
            m_comboAILevel16.SetItemData(i, ai);
            i = m_comboAILevel20.AddString(text);
            m_comboAILevel20.SetItemData(i, ai);
            i = m_comboAILevel24.AddString(text);
            m_comboAILevel24.SetItemData(i, ai);
            i = m_comboAILevel28.AddString(text);
            m_comboAILevel28.SetItemData(i, ai);
            i = m_comboAILevel32.AddString(text);
            m_comboAILevel32.SetItemData(i, ai);
            i = m_comboAILevel36.AddString(text);
            m_comboAILevel36.SetItemData(i, ai);
            i = m_comboAILevel40.AddString(text);
            m_comboAILevel40.SetItemData(i, ai);
        }
    }
}

void CDDOBuilderView::RestoreControls()
{
    // restore loaded values
    // name
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        EnableToolTips(TRUE);
        m_editName.EnableWindow(TRUE);
        m_editName.SetWindowText(pBuild->Name().c_str());
        // Race
        m_comboRace.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->Race(), &m_comboRace);

        // alignment
        m_comboAlignment.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->Alignment(), &m_comboAlignment);

        // tomes
        m_comboTomeStr.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->StrTome(), &m_comboTomeStr);
        m_comboTomeDex.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->DexTome(), &m_comboTomeDex);
        m_comboTomeCon.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->ConTome(), &m_comboTomeCon);
        m_comboTomeInt.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->IntTome(), &m_comboTomeInt);
        m_comboTomeWis.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->WisTome(), &m_comboTomeWis);
        m_comboTomeCha.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->ChaTome(), &m_comboTomeCha);

        DisplayAbilityValue(Ability_Strength, &m_editStr);
        DisplayAbilityValue(Ability_Dexterity, &m_editDex);
        DisplayAbilityValue(Ability_Constitution, &m_editCon);
        DisplayAbilityValue(Ability_Intelligence, &m_editInt);
        DisplayAbilityValue(Ability_Wisdom, &m_editWis);
        DisplayAbilityValue(Ability_Charisma, &m_editCha);

        DisplaySpendCost(Ability_Strength, &m_editCostStr);
        DisplaySpendCost(Ability_Dexterity, &m_editCostDex);
        DisplaySpendCost(Ability_Constitution, &m_editCostCon);
        DisplaySpendCost(Ability_Intelligence, &m_editCostInt);
        DisplaySpendCost(Ability_Wisdom, &m_editCostWis);
        DisplaySpendCost(Ability_Charisma, &m_editCostCha);
        UpdateAvailableSpend();

        // ability increase selections
        // only visible if build level allows
        m_comboAILevel4.EnableWindow(TRUE);
        m_comboAILevel8.EnableWindow(TRUE);
        m_comboAILevel12.EnableWindow(TRUE);
        m_comboAILevel16.EnableWindow(TRUE);
        m_comboAILevel20.EnableWindow(TRUE);
        m_comboAILevel24.EnableWindow(TRUE);
        m_comboAILevel28.EnableWindow(TRUE);
        m_comboAILevel32.EnableWindow(TRUE);
        m_comboAILevel36.EnableWindow(TRUE);
        m_comboAILevel40.EnableWindow(TRUE);
        SelectComboboxEntry(pBuild->Level4(), &m_comboAILevel4);
        SelectComboboxEntry(pBuild->Level8(), &m_comboAILevel8);
        SelectComboboxEntry(pBuild->Level12(), &m_comboAILevel12);
        SelectComboboxEntry(pBuild->Level16(), &m_comboAILevel16);
        SelectComboboxEntry(pBuild->Level20(), &m_comboAILevel20);
        SelectComboboxEntry(pBuild->Level24(), &m_comboAILevel24);
        SelectComboboxEntry(pBuild->Level28(), &m_comboAILevel28);
        SelectComboboxEntry(pBuild->Level32(), &m_comboAILevel32);
        SelectComboboxEntry(pBuild->Level36(), &m_comboAILevel36);
        SelectComboboxEntry(pBuild->Level40(), &m_comboAILevel40);
        ShowHidControls( 4, &m_comboAILevel4,  IDC_STATIC_LEVEL4);
        ShowHidControls( 8, &m_comboAILevel8,  IDC_STATIC_LEVEL8);
        ShowHidControls(12, &m_comboAILevel12, IDC_STATIC_LEVEL12);
        ShowHidControls(16, &m_comboAILevel16, IDC_STATIC_LEVEL16);
        ShowHidControls(20, &m_comboAILevel20, IDC_STATIC_LEVEL20);
        ShowHidControls(24, &m_comboAILevel24, IDC_STATIC_LEVEL24);
        ShowHidControls(28, &m_comboAILevel28, IDC_STATIC_LEVEL28);
        ShowHidControls(32, &m_comboAILevel32, IDC_STATIC_LEVEL32);
        ShowHidControls(36, &m_comboAILevel36, IDC_STATIC_LEVEL36);
        ShowHidControls(40, &m_comboAILevel40, IDC_STATIC_LEVEL40);

        // guild level
        CString level;
        level.Format("%d", m_pCharacter->HasGuildLevel() ? m_pCharacter->GuildLevel() : 0);
        m_editGuildLevel.SetWindowText(level);
        bool bApplyGuildBuffs = m_pCharacter->HasApplyGuildBuffs() ? m_pCharacter->ApplyGuildBuffs() : false;
        m_buttonGuildBuffs.SetCheck(bApplyGuildBuffs ? BST_CHECKED : BST_UNCHECKED);
        m_editGuildLevel.EnableWindow(TRUE);
        m_buttonGuildBuffs.EnableWindow(TRUE);
    }
    else
    {
        EnableToolTips(FALSE);
        DisableControls();
    }
}

void CDDOBuilderView::ShowHidControls(
        size_t level,
        CComboBox * pCombo,
        UINT idStatic)
{
    // tome level up controls shown based on build level
    Build * pBuild = m_pCharacter->ActiveBuild();
    pCombo->ShowWindow(pBuild->Level() >= level ? SW_SHOW : SW_HIDE);
    GetDlgItem(idStatic)->ShowWindow(pBuild->Level() >= level ? SW_SHOW : SW_HIDE);
}

void CDDOBuilderView::DisableControls()
{
    m_editName.EnableWindow(FALSE);
    m_editName.SetWindowText("");
    m_comboRace.EnableWindow(FALSE);
    m_comboRace.SetCurSel(CB_ERR);
    m_comboAlignment.EnableWindow(FALSE);
    m_comboAlignment.SetCurSel(CB_ERR);
    GetDlgItem(IDC_BUTTON_STR_PLUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_STR_MINUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_DEX_PLUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_DEX_MINUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_CON_PLUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_CON_MINUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_INT_PLUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_INT_MINUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_WIS_PLUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_WIS_MINUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_CHA_PLUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_CHA_MINUS)->EnableWindow(FALSE);
    m_comboTomeStr.EnableWindow(FALSE);
    m_comboTomeStr.SetCurSel(CB_ERR);
    m_comboTomeDex.EnableWindow(FALSE);
    m_comboTomeDex.SetCurSel(CB_ERR);
    m_comboTomeCon.EnableWindow(FALSE);
    m_comboTomeCon.SetCurSel(CB_ERR);
    m_comboTomeInt.EnableWindow(FALSE);
    m_comboTomeInt.SetCurSel(CB_ERR);
    m_comboTomeWis.EnableWindow(FALSE);
    m_comboTomeWis.SetCurSel(CB_ERR);
    m_comboTomeCha.EnableWindow(FALSE);
    m_comboTomeCha.SetCurSel(CB_ERR);
    m_comboAILevel4.ShowWindow(SW_SHOW);
    m_comboAILevel4.EnableWindow(FALSE);
    m_comboAILevel4.SetCurSel(CB_ERR);
    m_comboAILevel8.ShowWindow(SW_SHOW);
    m_comboAILevel8.EnableWindow(FALSE);
    m_comboAILevel8.SetCurSel(CB_ERR);
    m_comboAILevel12.ShowWindow(SW_SHOW);
    m_comboAILevel12.EnableWindow(FALSE);
    m_comboAILevel12.SetCurSel(CB_ERR);
    m_comboAILevel16.ShowWindow(SW_SHOW);
    m_comboAILevel16.EnableWindow(FALSE);
    m_comboAILevel16.SetCurSel(CB_ERR);
    m_comboAILevel20.ShowWindow(SW_SHOW);
    m_comboAILevel20.EnableWindow(FALSE);
    m_comboAILevel20.SetCurSel(CB_ERR);
    m_comboAILevel24.ShowWindow(SW_SHOW);
    m_comboAILevel24.EnableWindow(FALSE);
    m_comboAILevel24.SetCurSel(CB_ERR);
    m_comboAILevel28.ShowWindow(SW_SHOW);
    m_comboAILevel28.EnableWindow(FALSE);
    m_comboAILevel28.SetCurSel(CB_ERR);
    m_comboAILevel32.ShowWindow(SW_SHOW);
    m_comboAILevel32.EnableWindow(FALSE);
    m_comboAILevel32.SetCurSel(CB_ERR);
    m_comboAILevel36.ShowWindow(SW_SHOW);
    m_comboAILevel36.EnableWindow(FALSE);
    m_comboAILevel36.SetCurSel(CB_ERR);
    m_comboAILevel40.ShowWindow(SW_SHOW);
    m_comboAILevel40.EnableWindow(FALSE);
    m_comboAILevel40.SetCurSel(CB_ERR);
    m_editGuildLevel.EnableWindow(FALSE);
    m_editGuildLevel.SetWindowText("");
    m_buttonGuildBuffs.EnableWindow(FALSE);
    m_buttonGuildBuffs.SetCheck(BST_UNCHECKED);
}

void CDDOBuilderView::EnableButtons()
{
    // depending on the selected build points, enable/disable
    // all the ability increase/decrease buttons based on current spends
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        const AbilitySpend & as = pBuild->BuildPoints();
        m_buttonStrPlus.EnableWindow(as.CanSpendOnAbility(Ability_Strength));
        m_buttonStrMinus.EnableWindow(as.CanRevokeSpend(Ability_Strength));
        m_buttonDexPlus.EnableWindow(as.CanSpendOnAbility(Ability_Dexterity));
        m_buttonDexMinus.EnableWindow(as.CanRevokeSpend(Ability_Dexterity));
        m_buttonConPlus.EnableWindow(as.CanSpendOnAbility(Ability_Constitution));
        m_buttonConMinus.EnableWindow(as.CanRevokeSpend(Ability_Constitution));
        m_buttonIntPlus.EnableWindow(as.CanSpendOnAbility(Ability_Intelligence));
        m_buttonIntMinus.EnableWindow(as.CanRevokeSpend(Ability_Intelligence));
        m_buttonWisPlus.EnableWindow(as.CanSpendOnAbility(Ability_Wisdom));
        m_buttonWisMinus.EnableWindow(as.CanRevokeSpend(Ability_Wisdom));
        m_buttonChaPlus.EnableWindow(as.CanSpendOnAbility(Ability_Charisma));
        m_buttonChaMinus.EnableWindow(as.CanRevokeSpend(Ability_Charisma));
    }
}

void CDDOBuilderView::DisplayAbilityValue(
        AbilityType ability,
        CEdit * control)
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        size_t value = pBuild->BaseAbilityValue(ability);
        CString text;
        text.Format("%d", value);
        control->SetWindowText(text);
    }
    else
    {
        control->SetWindowText("");
    }
}

void CDDOBuilderView::DisplaySpendCost(
        AbilityType ability,
        CEdit * control)
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        const AbilitySpend & as = pBuild->BuildPoints();
        size_t nextCost = as.NextPointsSpentCost(ability);
        CString text;
        if (nextCost > 0)
        {
            // show how many build points the next increase will cost
            text.Format("%d", nextCost);
        }
        else
        {
            // max spend on this ability has been reached
            text = "MAX";
        }
        control->SetWindowText(text);
    }
    else
    {
        control->SetWindowText("");
    }
}

void CDDOBuilderView::UpdateAvailableSpend()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        // show how many build points are yet to be spent
        const AbilitySpend & as = pBuild->BuildPoints();
        int availablePoints = (int)as.AvailableSpend() - (int)as.PointsSpent();
        CString text;
        text.Format("Available Build points: %d", availablePoints);
        m_staticAvailableSpend.SetWindowText(text);
    }
    else
    {
        m_staticAvailableSpend.SetWindowText("");
    }
}

HBRUSH CDDOBuilderView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
    HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
    // colour the control based on whether the user has over spent
    // the number of build points available. This can happen
    // if they adjust down the number of past lives
    bool setWarning = false;
    if (pWnd == &m_staticAvailableSpend)
    {
        if (m_pDocument != NULL)
        {
            Build * pBuild = m_pCharacter->ActiveBuild();
            if (pBuild != NULL)
            {
                const AbilitySpend & as = pBuild->BuildPoints();
                int availablePoints = (int)as.AvailableSpend() - (int)as.PointsSpent();
                setWarning = (availablePoints < 0);
            }
        }
    }
    if (setWarning)
    {
        pDC->SetTextColor(f_abilityOverspendColour);
    }

    return hbr;
}

void CDDOBuilderView::OnButtonStrPlus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->SpendOnAbility(Ability_Strength);
        EnableButtons();
        DisplayAbilityValue(Ability_Strength, &m_editStr);
        DisplaySpendCost(Ability_Strength, &m_editCostStr);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonStrMinus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->RevokeSpendOnAbility(Ability_Strength);
        EnableButtons();
        DisplayAbilityValue(Ability_Strength, &m_editStr);
        DisplaySpendCost(Ability_Strength, &m_editCostStr);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonDexPlus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->SpendOnAbility(Ability_Dexterity);
        EnableButtons();
        DisplayAbilityValue(Ability_Dexterity, &m_editDex);
        DisplaySpendCost(Ability_Dexterity, &m_editCostDex);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonDexMinus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->RevokeSpendOnAbility(Ability_Dexterity);
        EnableButtons();
        DisplayAbilityValue(Ability_Dexterity, &m_editDex);
        DisplaySpendCost(Ability_Dexterity, &m_editCostDex);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonConPlus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->SpendOnAbility(Ability_Constitution);
        EnableButtons();
        DisplayAbilityValue(Ability_Constitution, &m_editCon);
        DisplaySpendCost(Ability_Constitution, &m_editCostCon);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonConMinus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->RevokeSpendOnAbility(Ability_Constitution);
        EnableButtons();
        DisplayAbilityValue(Ability_Constitution, &m_editCon);
        DisplaySpendCost(Ability_Constitution, &m_editCostCon);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonIntPlus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->SpendOnAbility(Ability_Intelligence);
        EnableButtons();
        DisplayAbilityValue(Ability_Intelligence, &m_editInt);
        DisplaySpendCost(Ability_Intelligence, &m_editCostInt);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonIntMinus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->RevokeSpendOnAbility(Ability_Intelligence);
        EnableButtons();
        DisplayAbilityValue(Ability_Intelligence, &m_editInt);
        DisplaySpendCost(Ability_Intelligence, &m_editCostInt);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonWisPlus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->SpendOnAbility(Ability_Wisdom);
        EnableButtons();
        DisplayAbilityValue(Ability_Wisdom, &m_editWis);
        DisplaySpendCost(Ability_Wisdom, &m_editCostWis);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonWisMinus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->RevokeSpendOnAbility(Ability_Wisdom);
        EnableButtons();
        DisplayAbilityValue(Ability_Wisdom, &m_editWis);
        DisplaySpendCost(Ability_Wisdom, &m_editCostWis);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonChaPlus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->SpendOnAbility(Ability_Charisma);
        EnableButtons();
        DisplayAbilityValue(Ability_Charisma, &m_editCha);
        DisplaySpendCost(Ability_Charisma, &m_editCostCha);
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnButtonChaMinus()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->RevokeSpendOnAbility(Ability_Charisma);
        EnableButtons();
        DisplayAbilityValue(Ability_Charisma, &m_editCha);
        DisplaySpendCost(Ability_Charisma, &m_editCostCha);
        UpdateAvailableSpend();
    }
}


void CDDOBuilderView::OnSelendokComboRace()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        // race has been chosen
        CString race;
        m_comboRace.GetLBText(m_comboRace.GetCurSel(), race);
        pBuild->SetRace((LPCTSTR)race);

        // race may affect any ability stat value
        DisplayAbilityValue(Ability_Strength, &m_editStr);
        DisplayAbilityValue(Ability_Dexterity, &m_editDex);
        DisplayAbilityValue(Ability_Constitution, &m_editCon);
        DisplayAbilityValue(Ability_Intelligence, &m_editInt);
        DisplayAbilityValue(Ability_Wisdom, &m_editWis);
        DisplayAbilityValue(Ability_Charisma, &m_editCha);
        m_comboRace.SetFocus(); // ensure it keeps the focus
        // race changes can cause build points to change
        UpdateRadioPoints();
        UpdateAvailableSpend();
    }
}

void CDDOBuilderView::OnSelendokComboAlignment()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AlignmentType alignment = static_cast<AlignmentType>(GetComboboxSelection(&m_comboAlignment));
        pBuild->SetAlignment(alignment);
        // views affected by alignment changes update automatically
        m_comboAlignment.SetFocus(); // ensure it keeps the focus
    }
}

void CDDOBuilderView::OnSelendokComboTomeStr()
{
    int value = GetComboboxSelection(&m_comboTomeStr);
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        // negative values are supreme tomes that set for all abilities
        if (value < 0)
        {
            value = -value;     // make it positive
            pBuild->SetAbilityTome(Ability_Strength, value);
            pBuild->SetAbilityTome(Ability_Dexterity, value);
            pBuild->SetAbilityTome(Ability_Constitution, value);
            pBuild->SetAbilityTome(Ability_Intelligence, value);
            pBuild->SetAbilityTome(Ability_Wisdom, value);
            pBuild->SetAbilityTome(Ability_Charisma, value);
            // keep controls up to date also
            SelectComboboxEntry(pBuild->StrTome(), &m_comboTomeStr);
            SelectComboboxEntry(pBuild->DexTome(), &m_comboTomeDex);
            SelectComboboxEntry(pBuild->ConTome(), &m_comboTomeCon);
            SelectComboboxEntry(pBuild->IntTome(), &m_comboTomeInt);
            SelectComboboxEntry(pBuild->WisTome(), &m_comboTomeWis);
            SelectComboboxEntry(pBuild->ChaTome(), &m_comboTomeCha);
        }
        else
        {
            pBuild->SetAbilityTome(Ability_Strength, value);
        }
    }
}

void CDDOBuilderView::OnSelendokComboTomeDex()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        size_t value = GetComboboxSelection(&m_comboTomeDex);
        pBuild->SetAbilityTome(Ability_Dexterity, value);
    }
}

void CDDOBuilderView::OnSelendokComboTomeCon()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        size_t value = GetComboboxSelection(&m_comboTomeCon);
        pBuild->SetAbilityTome(Ability_Constitution, value);
    }
}

void CDDOBuilderView::OnSelendokComboTomeInt()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        size_t value = GetComboboxSelection(&m_comboTomeInt);
        pBuild->SetAbilityTome(Ability_Intelligence, value);
    }
}

void CDDOBuilderView::OnSelendokComboTomeWis()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        size_t value = GetComboboxSelection(&m_comboTomeWis);
        pBuild->SetAbilityTome(Ability_Wisdom, value);
    }
}

void CDDOBuilderView::OnSelendokComboTomeCha()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        size_t value = GetComboboxSelection(&m_comboTomeCha);
        pBuild->SetAbilityTome(Ability_Charisma, value);
    }
}

void CDDOBuilderView::OnChangeName()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        CString name;
        m_editName.GetWindowText(name);
        if (name != pBuild->Name().c_str())
        {
            // name has changed
            pBuild->SetName((LPCTSTR)name);
        }
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel4()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        int sel = GetComboboxSelection(&m_comboAILevel4);
        if (sel < 0)
        {
            // its an "All Ability" selection
            AbilityType value = (AbilityType)(-sel);
            pBuild->SetAbilityLevelUp(4, value);
            pBuild->SetAbilityLevelUp(8, value);
            pBuild->SetAbilityLevelUp(12, value);
            pBuild->SetAbilityLevelUp(16, value);
            pBuild->SetAbilityLevelUp(20, value);
            pBuild->SetAbilityLevelUp(24, value);
            pBuild->SetAbilityLevelUp(28, value);
            pBuild->SetAbilityLevelUp(32, value);
            pBuild->SetAbilityLevelUp(36, value);
            pBuild->SetAbilityLevelUp(40, value);
            // keep UI up to date also
            SelectComboboxEntry(pBuild->Level4(), &m_comboAILevel4);
            SelectComboboxEntry(pBuild->Level8(), &m_comboAILevel8);
            SelectComboboxEntry(pBuild->Level12(), &m_comboAILevel12);
            SelectComboboxEntry(pBuild->Level16(), &m_comboAILevel16);
            SelectComboboxEntry(pBuild->Level20(), &m_comboAILevel20);
            SelectComboboxEntry(pBuild->Level24(), &m_comboAILevel24);
            SelectComboboxEntry(pBuild->Level28(), &m_comboAILevel28);
            SelectComboboxEntry(pBuild->Level32(), &m_comboAILevel32);
            SelectComboboxEntry(pBuild->Level36(), &m_comboAILevel36);
            SelectComboboxEntry(pBuild->Level40(), &m_comboAILevel40);
        }
        else
        {
            AbilityType value = (AbilityType)sel;
            pBuild->SetAbilityLevelUp(4, value);
        }
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel8()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel8);
        pBuild->SetAbilityLevelUp(8, value);
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel12()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel12);
        pBuild->SetAbilityLevelUp(12, value);
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel16()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel16);
        pBuild->SetAbilityLevelUp(16, value);
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel20()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel20);
        pBuild->SetAbilityLevelUp(20, value);
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel24()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel24);
        pBuild->SetAbilityLevelUp(24, value);
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel28()
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel28);
        pBuild->SetAbilityLevelUp(28, value);
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel32()
{
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel32);
        pBuild->SetAbilityLevelUp(32, value);
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel36()
{
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel36);
        pBuild->SetAbilityLevelUp(36, value);
    }
}

void CDDOBuilderView::OnSelendokComboAbilityLevel40()
{
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        AbilityType value = (AbilityType)GetComboboxSelection(&m_comboAILevel40);
        pBuild->SetAbilityLevelUp(40, value);
    }
}

void CDDOBuilderView::UpdateBuildDescription()
{
    Build *pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        CString text = pBuild->ComplexUIDescription().c_str();
        m_staticBuildDescription.SetWindowText(text);
   }
   else
   {
        m_staticBuildDescription.SetWindowText("No active build");
   }
}

// Character overrides
//void CDDOBuilderView::UpdateAvailableBuildPointsChanged(Character * pCharacter)
//{
//    // show the now available build points
//    UpdateRadioPoints();
//    UpdateAvailableSpend();
//    EnableButtons();
//}

//void CDDOBuilderView::UpdateClassChanged(
//        Character * pCharacter,
//        ClassType classFrom,
//        ClassType classTo,
//        size_t level)
//{
//    // level up view changed a class selection, keep our overall build description
//    // up to date.
//    UpdateBuildDescription();
//}

//void CDDOBuilderView::UpdateRaceChanged(Character * charData, RaceType race)
//{
//    // build points can change when Drow selected
//    UpdateRadioPoints();
//}

void CDDOBuilderView::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);
}

void CDDOBuilderView::OnButtonGuildBuffs()
{
    // toggle the enabled guild buffs status
    m_pCharacter->ToggleApplyGuildBuffs();
}

void CDDOBuilderView::OnKillFocusGuildLevel()
{
    // change the guild level
    CString text;
    m_editGuildLevel.GetWindowText(text);
    size_t level = atoi(text);      // its ES_NUMBER so guaranteed to work
    m_pCharacter->SetGuildLevel(level);
}

void CDDOBuilderView::UpdateRadioPoints()
{
    // although these are a group of 4 radio controls for selecting the number
    // of builds points the user has to spend there are a couple of rules that
    // apply to them as follows:

    // Each race has a defined series of build points defined for it.
    // if the character has no past lives buttons [0] and [1] are available, [2]/[3] disabled
    // if the character has 1+ past life all buttons are disabled but the relevant number
    // of build points available is checked.
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        m_bIgnoreFocus = true;
        size_t activeBuildPoints = pBuild->DetermineBuildPoints();
        const Race & race = FindRace(pBuild->Race());
        std::vector<size_t> buildPoints = race.BuildPoints();
        CString strText;
        strText.Format("%d", buildPoints[0]);
        m_button28Pt.SetWindowText(strText);    // adventurer
        strText.Format("%d", buildPoints[1]);
        m_button32Pt.SetWindowText(strText);    // champion
        strText.Format("%d", buildPoints[2]);
        m_button34Pt.SetWindowText(strText);    // hero
        strText.Format("%d", buildPoints[3]);
        m_button36Pt.SetWindowText(strText);    // legend

        m_button28Pt.SetCheck(buildPoints[0] == activeBuildPoints ? BST_CHECKED : BST_UNCHECKED);
        m_button32Pt.SetCheck(buildPoints[1] == activeBuildPoints ? BST_CHECKED : BST_UNCHECKED);
        m_button34Pt.SetCheck(buildPoints[2] == activeBuildPoints ? BST_CHECKED : BST_UNCHECKED);
        m_button36Pt.SetCheck(buildPoints[3] == activeBuildPoints ? BST_CHECKED : BST_UNCHECKED);

        size_t numPastLifeFeats = pBuild->BuildPointsPastLifeCount();
        if (numPastLifeFeats == 0)
        {
            // user can select between [0] or [1] points
            // if they want [2]/[3] they need to train past lives
            m_button28Pt.EnableWindow(TRUE);
            m_button32Pt.EnableWindow(TRUE);
        }
        else
        {
            m_button28Pt.EnableWindow(FALSE);
            m_button32Pt.EnableWindow(FALSE);
        }
        m_button34Pt.EnableWindow(FALSE);   // never selectable, controlled by past lives
        m_button36Pt.EnableWindow(FALSE);   // never selectable, controlled by past lives
        m_bIgnoreFocus = false;
    }
    else
    {
        m_button28Pt.SetWindowText("-");
        m_button32Pt.SetWindowText("-");
        m_button34Pt.SetWindowText("-");
        m_button36Pt.SetWindowText("-");
        m_button28Pt.EnableWindow(FALSE);
        m_button32Pt.EnableWindow(FALSE);
        m_button34Pt.EnableWindow(FALSE);
        m_button36Pt.EnableWindow(FALSE);
    }
}

void CDDOBuilderView::OnBnClickedRadio28pt()
{
    if (!m_bIgnoreFocus)
    {
        // set the number of build points to [0]
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            const Race & race = FindRace(pBuild->Race());
            std::vector<size_t> buildPoints = race.BuildPoints();
            // set the number of build points to [0]
            pBuild->SetBuildPoints(buildPoints[0]);
            RestoreControls();
            EnableButtons();
            UpdateRadioPoints();
        }
    }
}

void CDDOBuilderView::OnBnClickedRadio32pt()
{
    if (!m_bIgnoreFocus)
    {
        // set the number of build points to [1]
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            const Race & race = FindRace(pBuild->Race());
            std::vector<size_t> buildPoints = race.BuildPoints();
            // set the number of build points to [1]
            pBuild->SetBuildPoints(buildPoints[1]);
            RestoreControls();
            EnableButtons();
            UpdateRadioPoints();
        }
    }
}

void CDDOBuilderView::OnEditResetbuild()
{
    //// make sure the user really wants to do this, as its a big change
    //UINT ret = AfxMessageBox("Warning: This command will reset to default all of the following:\r\n"
    //        "Ability Point Spends\r\n"
    //        "Class Selections\r\n"
    //        "Feat Selections\r\n"
    //        "Skill Points\r\n"
    //        "Enhancement Tree Selections\r\n"
    //        "Notes\r\n"
    //        "\r\n"
    //        "It will not touch any selected Past Lives, Gear setups or Tomes,\r\n"
    //        "but may affect Epic Destinies where the selections now become\r\n"
    //        "invalid due to missing feats.\r\n"
    //        "\r\n"
    //        "Are you sure you wish to do this?",
    //        MB_ICONQUESTION | MB_YESNO);
    //if (ret == IDYES)
    //{
    //    CWaitCursor longOperation;
    //    CWnd * pWnd = AfxGetMainWnd();
    //    CMainFrame * pMainFrame = dynamic_cast<CMainFrame*>(pWnd);
    //    if (pMainFrame != NULL)
    //    {
    //        // reset the windows first
    //        pMainFrame->SetActiveDocumentAndCharacter(NULL, NULL);
    //        // reset the build with nothing displayed
    //        m_pCharacter->ResetBuild();
    //        // notify the main frame that we are active so that everything gets
    //        // reset correctly in the views
    //        pMainFrame->SetActiveDocumentAndCharacter(GetDocument(), m_pCharacter);
    //    }
    //    // ensure this view is up to date
    //    UpdateRadioPoints();

    //    RestoreControls();
    //    EnableButtons();
    //    UpdateBuildDescription();
    //}
}

void CDDOBuilderView::OnEditFeatsUpdateEpicOnly(CCmdUI * pCmdUi)
{
    UNREFERENCED_PARAMETER(pCmdUi);
    pCmdUi->Enable(!m_pCharacter->ShowUnavailable());
    pCmdUi->SetCheck(m_pCharacter->ShowEpicOnly());
}

void CDDOBuilderView::OnEditFeatsUpdateShowUnavailable(CCmdUI * pCmdUi)
{
    UNREFERENCED_PARAMETER(pCmdUi);
    pCmdUi->SetCheck(m_pCharacter->ShowUnavailable());
}

void CDDOBuilderView::OnEditFeatsEpicOnly()
{
    m_pCharacter->ToggleShowEpicOnly();
}

void CDDOBuilderView::OnEditFeatsShowUnavailable()
{
    m_pCharacter->ToggleShowUnavailable();
}

// Character overrides
void CDDOBuilderView::UpdateActiveLifeChanged(Character* pCharacter)
{
    UNREFERENCED_PARAMETER(pCharacter);
    // we always get an UpdateActiveBuildChanged, so work already done
}

void CDDOBuilderView::UpdateActiveBuildChanged(Character * pCharacter)
{
    UpdateRadioPoints();

    RestoreControls();      // puts controls to values of loaded data
    EnableButtons();
    UpdateBuildDescription();

    Build* pBuild = pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->AttachObserver(this);
    }
    Life* pLife = pCharacter->ActiveLife();
    if (pLife != NULL)
    {
        pLife->AttachObserver(this);
    }
}

// Build overrides
void CDDOBuilderView::UpdateBuildLevelChanged(Build * pBuild)
{
    UNREFERENCED_PARAMETER(pBuild);
    UpdateBuildDescription();
    RestoreControls();
}

void CDDOBuilderView::UpdateClassChanged(
        Build * pBuild,
        const std::string& classFrom,
        const std::string& classTo,
        size_t level)
{
    UNREFERENCED_PARAMETER(pBuild);
    UNREFERENCED_PARAMETER(classFrom);
    UNREFERENCED_PARAMETER(classTo);
    UNREFERENCED_PARAMETER(level);
    UpdateBuildDescription();
}

void CDDOBuilderView::UpdateLifeFeatTrained(Life*, const std::string&)
{
    EnableButtons();
    UpdateRadioPoints();
    UpdateAvailableSpend();
}

void CDDOBuilderView::UpdateLifeFeatRevoked(Life*, const std::string&)
{
    EnableButtons();
    UpdateRadioPoints();
    UpdateAvailableSpend();
}

BOOL CDDOBuilderView::OnTtnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(pResult);

    UINT_PTR nID = pNMHDR->idFrom;
    nID = ::GetDlgCtrlID((HWND)nID);

    switch (nID)
    {
    case IDC_EDIT_STR:
    case IDC_EDIT_DEX:
    case IDC_EDIT_CON:
    case IDC_EDIT_INT:
    case IDC_EDIT_WIS:
    case IDC_EDIT_CHA:
        m_tipText = "Current ability value of Base + Racial + Spent points";
        break;
    case IDC_BUTTON_STR_PLUS:
    case IDC_BUTTON_DEX_PLUS:
    case IDC_BUTTON_CON_PLUS:
    case IDC_BUTTON_INT_PLUS:
    case IDC_BUTTON_WIS_PLUS:
    case IDC_BUTTON_CHA_PLUS:
        m_tipText = "Enabled if you have enough points to buy the next rank of this ability";
        break;
    case IDC_BUTTON_STR_MINUS:
    case IDC_BUTTON_DEX_MINUS:
    case IDC_BUTTON_CON_MINUS:
    case IDC_BUTTON_INT_MINUS:
    case IDC_BUTTON_WIS_MINUS:
    case IDC_BUTTON_CHA_MINUS:
        m_tipText = "Enabled if you can revoke a rank of this ability";
        break;

    case IDC_COMBO_RACE:
        {
            m_tipText = "Select your race\r\n\r\n";

            const Race & race = FindRace(m_pCharacter->ActiveBuild()->Race());
            m_tipText += race.Description().c_str();
        }
        break;
    case IDC_COMBO_ALIGNMENT:
        m_tipText = "Choose your alignment, note that some classes are Alignment restricted.";
        break;
    case IDC_COMBO_TOME_STR:
    case IDC_COMBO_TOME_DEX:
    case IDC_COMBO_TOME_CON:
    case IDC_COMBO_TOME_INT:
    case IDC_COMBO_TOME_WIS:
    case IDC_COMBO_TOME_CHA:
        m_tipText = "Set the Tome you have for this ability.\r\n"
                  "You can set all Tomes at the same time for all Abilities in the Strength option.";
        break;
    case IDC_NAME:
        m_tipText = "Set the name of your character.";
        break;
    case IDC_COMBO_LEVEL4_ABILITY:
    case IDC_COMBO_LEVEL8_ABILITY:
    case IDC_COMBO_LEVEL12_ABILITY:
    case IDC_COMBO_LEVEL16_ABILITY:
    case IDC_COMBO_LEVEL20_ABILITY:
    case IDC_COMBO_LEVEL24_ABILITY:
    case IDC_COMBO_LEVEL28_ABILITY:
        m_tipText = "Select the ability that gains 1 additional point at this level.\r\n"
                "To set all level ups at the same time, select the \"all\" option in the Level 4\r\n"
                "ability option.";
        break;
    case IDC_CHECK_GUILD_BUFFS:
        m_tipText = "Set to enable guild buffs to be applied for your stated guild level.";
        break;
    case IDC_EDIT_GUILD_LEVEL:
        m_tipText = "Set the level of your guild for which buffs are available.";
        break;
    case IDC_RADIO_28PT:
    case IDC_RADIO_32PT:
        m_tipText = "Choose between the Adventurer and Hero level of build points for your character.";
        break;
    case IDC_RADIO_34PT:
    case IDC_RADIO_36PT:
        m_tipText = "To be able to spend Champion and Legend level of build points you need to assign yourself the relevant past lives.";
        break;
    }
    // ensure multiline tooltips
    ::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 800);
    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    pTTTA->lpszText = m_tipText.GetBuffer();
    return TRUE;
}

void CDDOBuilderView::OnDumpWeaponGroups()
{
    m_pCharacter->ActiveBuild()->DumpWeaponGroups();
}

void CDDOBuilderView::OnUpdateDumpWeaponGroups(CCmdUI* pCmdUi)
{
    pCmdUi->Enable(m_pCharacter->ActiveBuild() != NULL);
}
