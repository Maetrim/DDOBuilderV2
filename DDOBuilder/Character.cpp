// Character.cpp
//
#include "StdAfx.h"
#include "Character.h"
#include "XmlLib\SaxWriter.h"
#include "DDOBuilderDoc.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"
#include "MainFrm.h"
#include "BreakdownsPane.h"
#include "DCPane.h"

#define DL_ELEMENT Character

namespace
{
    const wchar_t f_saxElementName[] = L"Character";
    DL_DEFINE_NAMES(Character_PROPERTIES)
    const unsigned f_verCurrent = 1;
}

Character::Character(CDDOBuilderDoc * pDoc) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_pDocument(pDoc),
    m_uiActiveLifeIndex(10000), // large number that will never occur naturally
    m_uiActiveBuildIndex(10000),// large number that will never occur naturally
    m_bShowEpicOnly(true),
    m_bShowUnavailableFeats(false)
{
    DL_INIT(Character_PROPERTIES)
}

DL_DEFINE_ACCESS(Character_PROPERTIES)

XmlLib::SaxContentElementInterface * Character::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Character_PROPERTIES)

    return subHandler;
}

void Character::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Character_PROPERTIES)
}

void Character::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName(), VersionAttributes());
    DL_WRITE(Character_PROPERTIES)
    writer->EndElement();
}

void Character::AboutToLoad()
{
    // if we are about to load, we reset all our contained data
    DL_INIT(Character_PROPERTIES)
    m_Lives.clear();
    m_uiActiveLifeIndex = 10000;    // large number that will never occur naturally
    m_uiActiveBuildIndex = 10000;   // large number that will never occur naturally
}

void Character::LoadComplete()
{
    for (auto& lit : m_Lives)
    {
        lit.LoadComplete();
    }
}

bool Character::ShowUnavailable() const
{
    return m_bShowUnavailableFeats;
}

bool Character::ShowEpicOnly() const
{
    return m_bShowEpicOnly;
}

void Character::ToggleShowEpicOnly()
{
    m_bShowEpicOnly = !m_bShowEpicOnly;
}

void Character::ToggleShowUnavailable()
{
    m_bShowUnavailableFeats = !m_bShowUnavailableFeats;
}

void Character::NotifyActiveLifeChanged()
{
    NotifyAll(&CharacterObserver::UpdateActiveLifeChanged, this);
}

void Character::NotifyActiveBuildChanged()
{
    NotifyAll(&CharacterObserver::UpdateActiveBuildChanged, this);
}

void Character::NotifyActiveBuildPositionChanged()
{
    NotifyAll(&CharacterObserver::UpdateActiveBuildPositionChanged, this);
}

const Life & Character::GetLife(size_t lifeIndex) const
{
    std::list<Life>::const_iterator lit = m_Lives.begin();
    std::advance(lit, lifeIndex);
    return (*lit);
}

void Character::SetLifeName(
        size_t lifeIndex,
        CString name)
{
    std::list<Life>::iterator lit = m_Lives.begin();
    std::advance(lit, lifeIndex);
    (*lit).SetName((LPCSTR)name);
    m_pDocument->SetModifiedFlag(TRUE);
}

size_t Character::AddLife()
{
    Life* pCurrentLife = ActiveLife();
    m_uiActiveLifeIndex = 10000;    // large number that will never occur naturally
    m_uiActiveBuildIndex = 10000;   // large number that will never occur naturally
    // all new lives start with a default build
    Life life(this);
    // add all past lives from the previously selected life to this one
    if (pCurrentLife != NULL)
    {
        const FeatsListObject& specialFeats = pCurrentLife->SpecialFeats();
        for (auto&& fit: specialFeats.Feats())
        {
            life.TrainSpecialFeat(fit.FeatName());
        }
    }
    m_Lives.push_back(life);
    m_Lives.back().AddBuild(0);
    m_pDocument->SetModifiedFlag(TRUE);
    return m_Lives.size()-1;    // 0 based
}

size_t Character::AddImportLife()
{
    // all new lives start with a default build
    Life life(this);
    m_Lives.push_back(life);
    m_Lives.back().AddBuild(0);
    m_pDocument->SetModifiedFlag(TRUE);
    return m_Lives.size()-1;    // 0 based
}

void Character::DeleteLife(size_t lifeIndex)
{
    std::list<Life>::iterator lit = m_Lives.begin();
    std::advance(lit, lifeIndex);
    m_Lives.erase(lit);
    m_pDocument->SetModifiedFlag(TRUE);
    m_uiActiveLifeIndex = 10000;    // large number that will never occur naturally
    m_uiActiveBuildIndex = 10000;   // large number that will never occur naturally
}

size_t Character::AppendLife(const Life& life)
{
    m_Lives.push_back(life);
    m_Lives.back().UpdateBuildLifePointers(this);
    m_pDocument->SetModifiedFlag(TRUE);
    return m_Lives.size() - 1;    // 0 based
}

size_t Character::AddBuild(size_t lifeIndex)
{
    size_t buildIndex = m_uiActiveBuildIndex;
    m_uiActiveLifeIndex = 10000;    // large number that will never occur naturally
    m_uiActiveBuildIndex = 10000;   // large number that will never occur naturally
    std::list<Life>::iterator lit = m_Lives.begin();
    std::advance(lit, lifeIndex);
    m_pDocument->SetModifiedFlag(TRUE);
    return (*lit).AddBuild(buildIndex);
}

void Character::DeleteBuild(
        size_t lifeIndex,
        size_t buildIndex)
{
    std::list<Life>::iterator lit = m_Lives.begin();
    std::advance(lit, lifeIndex);
    (*lit).DeleteBuild(buildIndex);
    m_pDocument->SetModifiedFlag(TRUE);
    m_uiActiveLifeIndex = 10000;    // large number that will never occur naturally
    m_uiActiveBuildIndex = 10000;   // large number that will never occur naturally
}

size_t Character::GetBuildLevel(
        size_t lifeIndex,
        size_t buildIndex) const
{
    std::list<Life>::const_iterator clit = m_Lives.begin();
    std::advance(clit, lifeIndex);
    const Build & build = (*clit).GetBuild(buildIndex);
    return build.Level();
}

void Character::SetBuildLevel(
        size_t lifeIndex,
        size_t buildIndex,
        size_t level)
{
    std::list<Life>::iterator lit = m_Lives.begin();
    std::advance(lit, lifeIndex);
    (*lit).SetBuildLevel(buildIndex, level);
}

void Character::ToggleApplyGuildBuffs()
{
    if (HasApplyGuildBuffs())
    {
        Set_ApplyGuildBuffs(!ApplyGuildBuffs());
    }
    else
    {
        Set_ApplyGuildBuffs(true);
    }
    Build *pBuild = ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->ApplyGuildBuffs(ApplyGuildBuffs());
    }
}

void Character::SetGuildLevel(size_t level)
{
    Set_GuildLevel(level);
    Build* pBuild = ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->GuildLevelChange();
    }
}

void Character::SetActiveBuild(size_t lifeIndex, size_t buildIndex, bool bOverride)
{
    if (m_uiActiveLifeIndex != lifeIndex
            || m_uiActiveBuildIndex != buildIndex
            || bOverride)
    {
        CWaitCursor longOperation;
        m_uiActiveLifeIndex = lifeIndex;
        m_uiActiveBuildIndex = buildIndex;
        m_ActiveLifeIndex = lifeIndex;
        m_ActiveBuildIndex = buildIndex;

        CWnd* pWnd = AfxGetMainWnd();
        CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
        CBreakdownsPane* pBDPane = dynamic_cast<CBreakdownsPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CBreakdownsPane)));
        if (NULL != pBDPane)
        {
            pBDPane->BuildChanging();
        }
        CDCPane* pDCPane = dynamic_cast<CDCPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CDCPane)));
        if (NULL != pDCPane)
        {
            pDCPane->BuildChanging();
        }
        if (ActiveBuild() != NULL)
        {
            std::stringstream ss;
            ss << "Selected Life \"" << ActiveLife()->UIDescription(lifeIndex) << "\" build of \"" << ActiveBuild()->ComplexUIDescription() << "\"";
            SetModifiedFlag(TRUE);
            GetLog().AddLogEntry(ss.str().c_str());
            ActiveBuild()->BuildNowActive();
        }
        else
        {
            NotifyActiveLifeChanged();
            NotifyActiveBuildChanged();
        }
    }
    else
    {
        if (ActiveBuild() == NULL)
        {
            NotifyActiveLifeChanged();
            NotifyActiveBuildChanged();
        }
    }
}

Life* Character::ActiveLife()
{
    Life* pLife = NULL;
    // find the active life
    if (m_uiActiveLifeIndex < m_Lives.size())
    {
        std::list<Life>::iterator lit = m_Lives.begin();
        std::advance(lit, m_uiActiveLifeIndex);
        pLife = &(*lit);
    }
    return pLife;
}

Build * Character::ActiveBuild()
{
    Build * pBuild = NULL;
    // find the active build
    if (m_uiActiveLifeIndex < m_Lives.size())
    {
        std::list<Life>::iterator lit = m_Lives.begin();
        std::advance(lit, m_uiActiveLifeIndex);
        pBuild = (*lit).GetBuildPointer(m_uiActiveBuildIndex);
    }
    return pBuild;
}

const Life* Character::ActiveLife() const
{
    const Life* pLife = NULL;
    // find the active life
    if (m_uiActiveLifeIndex < m_Lives.size())
    {
        std::list<Life>::const_iterator lit = m_Lives.begin();
        std::advance(lit, m_uiActiveLifeIndex);
        pLife = &(*lit);
    }
    return pLife;
}

const Build * Character::ActiveBuild() const
{
    const Build * pBuild = NULL;
    // find the active build
    if (m_uiActiveLifeIndex < m_Lives.size())
    {
        std::list<Life>::const_iterator lit = m_Lives.begin();
        std::advance(lit, m_uiActiveLifeIndex);
        pBuild = (*lit).GetBuildPointer(m_uiActiveBuildIndex);
    }
    return pBuild;
}

void Character::SetActiveBuildIndex(size_t buildIndex)
{
    if (buildIndex != m_uiActiveBuildIndex)
    {
        m_uiActiveBuildIndex = buildIndex;
        m_ActiveBuildIndex = buildIndex;
        NotifyActiveBuildPositionChanged();
    }
}

void Character::SetModifiedFlag(BOOL modified)
{
    m_pDocument->SetModifiedFlag(modified);
}

std::list<CompletedQuest> Character::CompletedQuests() const
{
    std::list<CompletedQuest> runQuests;
    // we need to collate all quests run in every build up to and including
    // the active build only. As all builds in a life are sorted by level
    // and we store which build ran which quest.
    const Life* pLife = ActiveLife();
    if (pLife != NULL)
    {
        for (size_t bi = 0; bi <= m_uiActiveBuildIndex; ++bi)
        {
            const Build& build = pLife->GetBuild(bi);
            const std::list<CompletedQuest>& runBuildQuests = build.CompletedQuests();
            runQuests.insert(runQuests.end(), runBuildQuests.begin(), runBuildQuests.end());
        }
    }
    // we now need to remove any duplicates we have which are for the same quest
    // at the same level but at lower difficulties. e.g. if you ran a Q at N, H and R3
    // it should only show up once as an R3 quest run.
    if (runQuests.size() > 1)
    {
        std::list<CompletedQuest>::iterator qit = runQuests.begin();
        while (qit != runQuests.end())
        {
            std::list<CompletedQuest>::iterator nqit = qit;
            nqit++;
            // look through all following Qs and remove qit if it is the same Q
            // at the same level but run at a lower difficulty
            bool bRunLower = false;
            while (!bRunLower && nqit != runQuests.end())
            {
                if (qit->SameQuestAndLevel(*nqit))
                {
                    bRunLower = (nqit->Difficulty() >= qit->Difficulty());
                }
                nqit++;
            }
            qit++;
        }
    }
    return runQuests;
}
