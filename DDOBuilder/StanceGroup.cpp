// StanceGroup.cpp
//
#include "stdafx.h"
#include "StanceGroup.h"
#include "Build.h"
#include "Character.h"

CFont StanceGroup::sm_smallFont;

StanceGroup::StanceGroup(
        const std::string& groupName,
        bool bSingleSelection) :
    m_groupName(groupName),
    m_bSingleSelection(bSingleSelection)
{
    if (NULL == (HFONT)sm_smallFont)
    {
        // shown in a small font
        LOGFONT lf;
        ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
        strcpy_s(lf.lfFaceName, "Consolas");
        lf.lfHeight = 11;
        sm_smallFont.CreateFontIndirect(&lf);
    }
}

StanceGroup::~StanceGroup()
{
    DestroyAll();
}

const std::string& StanceGroup::GroupName() const
{
    return m_groupName;
}

CWnd* StanceGroup::GroupLabel()
{
    return &m_groupLabel;
}

size_t StanceGroup::NumButtons() const
{
    return m_stanceButtons.size();
}

CWnd* StanceGroup::StanceButton(size_t index)
{
    auto sbit = m_stanceButtons.begin();
    std::advance(sbit, index);
    return (*sbit);
}

bool StanceGroup::IsSingleSelection() const
{
    return m_bSingleSelection;
}

void StanceGroup::CreateWindows(
        CWnd* pParent,
        UINT& nextControlId)
{
    // create the groups static control
    m_groupLabel.Create(
            m_groupName.c_str(),
            WS_CHILD | WS_VISIBLE,
            CRect(0, 0, 10, 10),        // gets sized later
            pParent,
            nextControlId++);
    // set the correct font
    m_groupLabel.SetFont(&sm_smallFont);
}

void StanceGroup::AddStance(
        Character* pCharacter,
        const Stance& stance,
        const std::string& /*group*/,
        CWnd* pParent,
        UINT& nextControlId)
{
    // if the stance is not already present, add it and a button
    bool bFound = false;
    for (auto&& sbi : m_stanceButtons)
    {
        if (sbi->IsYou(stance))
        {
            // already exists, add another stack of it
            sbi->AddStack();
            bFound = true;
            break;
        }
    }
    if (!bFound)
    {
        CStanceButton* stanceButton = new CStanceButton(pCharacter, stance);
        m_stanceButtons.push_back(stanceButton);
        m_stanceButtons.back()->CreateStanceWindow(pParent, nextControlId++);
        bool bActive = false;
        Build* pBuild = pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            bActive = pBuild->IsStanceActive(stance.Name());
        }
        stanceButton->SetSelected(bActive);
        bFound = true;
    }
}

bool StanceGroup::RevokeStance(
        const Stance& stance,
        const std::string& group)
{
    bool bFound = false;
    if (group == m_groupName)
    {
        auto sit = m_stanceButtons.begin();
        while (!bFound && sit != m_stanceButtons.end())
        {
            if ((*sit)->IsYou(stance))
            {
                // already exists, add another stack of it
                (*sit)->RevokeStack();
                if ((*sit)->NumStacks() == 0)
                {
                    (*sit)->DestroyWindow();
                    delete (*sit);
                    sit = m_stanceButtons.erase(sit);
                }
                else
                {
                    ++sit;
                }
                bFound = true;
                break;
            }
            else
            {
                ++sit;
            }
        }
    }
    return bFound;
}

void StanceGroup::DeactivateOtherStancesExcept(
    const std::string& stanceName,
    Build* pBuild)
{
    for (auto&& sbit : m_stanceButtons)
    {
        if (sbit->GetStance().Name() != stanceName)
        {
            if (pBuild->IsStanceActive(sbit->GetStance().Name(), Weapon_Unknown))
            {
                pBuild->DeactivateStance(sbit->GetStance());
            }
        }
    }
}

void StanceGroup::DestroyAll()
{
    m_groupLabel.DestroyWindow();
    auto sit = m_stanceButtons.begin();
    while (sit != m_stanceButtons.end())
    {
        (*sit)->DestroyWindow();
        delete (*sit);
        sit = m_stanceButtons.erase(sit);
    }
}

CStanceButton* StanceGroup::GetStance(
        const std::string& stanceName)
{
    CStanceButton* pStance = NULL;
    std::list<CStanceButton*>::iterator it = m_stanceButtons.begin();
    while (pStance == NULL && it != m_stanceButtons.end())
    {
        if ((*it)->Name() == stanceName)
        {
            pStance = (*it);
        }
        ++it;
    }
    return pStance;
}

CStanceButton* StanceGroup::GetStance(size_t index)
{
    std::list<CStanceButton*>::iterator it = m_stanceButtons.begin();
    std::advance(it, index);
    return (*it);
}


void StanceGroup::AddActiveStances(std::vector<CStanceButton*>* stances)
{
    UNREFERENCED_PARAMETER(stances);
    //for (auto&& sbi: m_stanceButtons)
    //{
        //if (sbi.GetStance().IsActive()
        //{

        //}
    //}
}

bool StanceGroup::HasStance(CStanceButton* pButton)
{
    bool bHas = false;
    for (auto&& sbi : m_stanceButtons)
    {
        if (sbi == pButton)
        {
            bHas = true;
        }
    }
    return bHas;
}
