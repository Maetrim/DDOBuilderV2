// SpellsPage.cpp
//
#include "stdafx.h"
#include "SpellsPage.h"
#include "GlobalSupportFunctions.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellsPage property page

CSpellsPage::CSpellsPage(const std::string& ct, AbilityType ability) :
    CPropertyPage(CSpellsPage::IDD, 0),
    m_pCharacter(NULL),
    m_classType(ct),
    m_abilityType(ability)
{
    //{{AFX_DATA_INIT(CSpellsPage)
    //}}AFX_DATA_INIT
}

CSpellsPage::~CSpellsPage()
{
}

void CSpellsPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSpellsPage)
    DDX_Control(pDX, IDC_SPELLS, m_spells);
    //}}AFX_DATA_MAP
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CSpellsPage, CPropertyPage)
    //{{AFX_MSG_MAP(CSpellsPage)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// CSpellsPage message handlers

BOOL CSpellsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpellsPage::OnSize(UINT nType, int cx, int cy)
{
    CPropertyPage::OnSize(nType, cx, cy);
    if (IsWindow(m_spells.GetSafeHwnd()))
    {
        // spells control takes entire client area
        m_spells.MoveWindow(0, 0, cx, cy);
    }
}

BOOL CSpellsPage::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_SPELLS,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CSpellsPage::SetCharacter(Character * pCharacter)
{
    m_pCharacter = pCharacter;
    m_spells.SetCharacter(pCharacter, m_classType);
    if (m_pCharacter != NULL)
    {
        if (m_pCharacter->ActiveBuild() != NULL)
        {
            m_pCharacter->ActiveBuild()->AttachObserver(this);
        }
    }
}

void CSpellsPage::SetTrainableSpells(const std::vector<size_t> & spellsPerLevel)
{
    m_spells.SetTrainableSpells(spellsPerLevel);
}

//void CSpellsPage::UpdateSpellTrained(
//        Character * charData,
//        const TrainedSpell & spell)
//{
//    if (spell.Class() == m_classType)
//    {
//        // this spell is for us, update our control
//        m_spells.UpdateSpells(CasterLevel(charData, m_classType));
//    }
//}

CSpellsControl * CSpellsPage::SpellsControl()
{
    return &m_spells;
}

bool CSpellsPage::IsClassType(const std::string& ct) const
{
    return (ct == m_classType);
}

//void CSpellsPage::UpdateSpellRevoked(
//        Character * charData,
//        const TrainedSpell & spell)
//{
//    if (spell.Class() == m_classType)
//    {
//        // this spell is for us, update our control
//        m_spells.UpdateSpells(CasterLevel(charData, m_classType));
//    }
//}
//
void CSpellsPage::UpdateFeatEffectApplied(
        Build*,
        const Effect& effect)
{
    // only interested in Effect_GrantSpell
    if (effect.IsType(Effect_GrantSpell)
            && effect.Item().back() == m_classType)
    {
        m_spells.AddFixedSpell(
                effect.Item().front(),
                (int)effect.Amount()[0],         // level
                (int)effect.Amount()[1],         // sp cost
                (int)effect.Amount()[2]);        // max catsre level
    }
}

void CSpellsPage::UpdateFeatEffectRevoked(
        Build*,
        const Effect& effect)
{
    if (effect.IsType(Effect_GrantSpell)
            && effect.Item().back() == m_classType)
    {
        m_spells.RevokeFixedSpell(
                effect.Item().front(),
                (int)effect.Amount()[0],         // level
                (int)effect.Amount()[1],         // sp cost
                (int)effect.Amount()[2]);        // max catsre level
    }
}

void CSpellsPage::UpdateEnhancementEffectApplied(
        Build*,
        const Effect& effect)
{
    // only interested in Effect_GrantSpell
    if (effect.IsType(Effect_GrantSpell)
            && effect.Item().back() == m_classType)
    {
        m_spells.AddFixedSpell(
                effect.Item().front(),
                (int)effect.Amount()[0],         // level
                (int)effect.Amount()[1],         // sp cost
                (int)effect.Amount()[2]);        // max catsre level
    }
}

void CSpellsPage::UpdateEnhancementEffectRevoked(
        Build*,
        const Effect& effect)
{
    if (effect.IsType(Effect_GrantSpell)
            && effect.Item().back() == m_classType)
    {
        m_spells.RevokeFixedSpell(
                effect.Item().front(),
                (int)effect.Amount()[0],         // level
                (int)effect.Amount()[1],         // sp cost
                (int)effect.Amount()[2]);        // max catsre level
    }
}

void CSpellsPage::UpdateItemEffectApplied(
        Build*,
        const Effect& effect)
{
    // only interested in Effect_GrantSpell
    if (effect.IsType(Effect_GrantSpell)
            && effect.Item().back() == m_classType)
    {
        m_spells.AddFixedSpell(
                effect.Item().front(),
                (int)effect.Amount()[0],         // level
                (int)effect.Amount()[1],         // sp cost
                (int)effect.Amount()[2]);        // max catsre level
    }
}

void CSpellsPage::UpdateItemEffectRevoked(
        Build*,
        const Effect& effect)
{
    if (effect.IsType(Effect_GrantSpell)
            && effect.Item().back() == m_classType)
    {
        m_spells.RevokeFixedSpell(
                effect.Item().front(),
                (int)effect.Amount()[0],         // level
                (int)effect.Amount()[1],         // sp cost
                (int)effect.Amount()[2]);        // max catsre level
    }
}
