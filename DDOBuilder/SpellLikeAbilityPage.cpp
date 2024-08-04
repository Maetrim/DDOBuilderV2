// SpellLikeAbilityPage.cpp
//
#include "stdafx.h"
#include "SpellLikeAbilityPage.h"
#include "GlobalSupportFunctions.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellLikeAbilityPage property page

CSpellLikeAbilityPage::CSpellLikeAbilityPage() :
    CPropertyPage(CSpellLikeAbilityPage::IDD),
    m_pCharacter(NULL)
{
    //{{AFX_DATA_INIT(CSpellLikeAbilityPage)
    //}}AFX_DATA_INIT
}

CSpellLikeAbilityPage::~CSpellLikeAbilityPage()
{
}

void CSpellLikeAbilityPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSpellLikeAbilityPage)
    DDX_Control(pDX, IDC_SPELLS, m_slas);
    //}}AFX_DATA_MAP
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CSpellLikeAbilityPage, CPropertyPage)
    //{{AFX_MSG_MAP(CSpellLikeAbilityPage)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// CSpellLikeAbilityPage message handlers

BOOL CSpellLikeAbilityPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpellLikeAbilityPage::OnSize(UINT nType, int cx, int cy)
{
    CPropertyPage::OnSize(nType, cx, cy);
    if (IsWindow(m_slas.GetSafeHwnd()))
    {
        // spells control takes entire client area
        m_slas.MoveWindow(0, 0, cx, cy);
    }
}

BOOL CSpellLikeAbilityPage::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_SPELLS,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CSpellLikeAbilityPage::SetCharacter(Character * pCharacter)
{
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->DetachObserver(this);
        }
        m_pCharacter->DetachObserver(this);
    }
    m_pCharacter = pCharacter;
    m_slas.SetCharacter(pCharacter);
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->AttachObserver(this);
        }
        m_pCharacter->AttachObserver(this);
    }
}

void CSpellLikeAbilityPage::UpdateActiveBuildChanged(Character*)
{
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->AttachObserver(this);
    }
}

void CSpellLikeAbilityPage::UpdateFeatEffectApplied(
        Build*,
        const Effect& effect)
{
    // only interested in Effect_SpellLikeAbility
    if (effect.IsType(Effect_SpellLikeAbility))
    {
        m_slas.AddSLA(effect.Item().front(), 1);
    }
}

void CSpellLikeAbilityPage::UpdateFeatEffectRevoked(
        Build*,
        const Effect& effect)
{
    if (effect.IsType(Effect_SpellLikeAbility))
    {
        m_slas.RevokeSLA(effect.Item().front());
    }
}

void CSpellLikeAbilityPage::UpdateItemEffectApplied(
        Build*,
        const Effect& effect)
{
    if (effect.IsType(Effect_SpellLikeAbility))
    {
        m_slas.AddSLA(effect.Item().front(), 1);
    }
}

void CSpellLikeAbilityPage::UpdateItemEffectRevoked(
        Build*,
        const Effect& effect)
{
    if (effect.IsType(Effect_SpellLikeAbility))
    {
        m_slas.RevokeSLA(effect.Item().front());
    }
}

void CSpellLikeAbilityPage::UpdateEnhancementEffectApplied(
        Build*,
        const Effect& effect)
{
    if (effect.IsType(Effect_SpellLikeAbility))
    {
        m_slas.AddSLA(effect.Item().front(), 1);
    }
}

void CSpellLikeAbilityPage::UpdateEnhancementEffectRevoked(
        Build*,
        const Effect& effect)
{
    if (effect.IsType(Effect_SpellLikeAbility))
    {
        m_slas.RevokeSLA(effect.Item().front());
    }
}

CSLAControl* CSpellLikeAbilityPage::Control()
{
    return &m_slas;
}

