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
    m_SpecialFeats(L"SpecialFeats"),
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
    m_hasStrTome = true;
    m_hasDexTome = true;
    m_hasConTome = true;
    m_hasIntTome = true;
    m_hasWisTome = true;
    m_hasChaTome = true;
    m_hasSpecialFeats = true;
    m_hasTomes = true;
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
    m_uiActiveLifeIndex = 10000;    // large number that will never occur naturally
    m_uiActiveBuildIndex = 10000;   // large number that will never occur naturally
    // all new lives start with a default build. they auto inherit Tomes and Special feats
    Life life(this);
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

void Character::SetAbilityTome(AbilityType ability, size_t value)
{
    if (value != AbilityTomeValue(ability))
    {
        switch (ability)
        {
        case Ability_Strength:
            Set_StrTome(value);
            break;
        case Ability_Dexterity:
            Set_DexTome(value);
            break;
        case Ability_Constitution:
            Set_ConTome(value);
            break;
        case Ability_Intelligence:
            Set_IntTome(value);
            break;
        case Ability_Wisdom:
            Set_WisTome(value);
            break;
        case Ability_Charisma:
            Set_ChaTome(value);
            break;
        default:
            ASSERT(FALSE);
            break;
        }
    }
}

size_t Character::AbilityTomeValue(AbilityType ability) const
{
    size_t value = 0;
    switch (ability)
    {
    case Ability_Strength:
        value = StrTome();
        break;
    case Ability_Dexterity:
        value = DexTome();
        break;
    case Ability_Constitution:
        value = ConTome();
        break;
    case Ability_Intelligence:
        value = IntTome();
        break;
    case Ability_Wisdom:
        value = WisTome();
        break;
    case Ability_Charisma:
        value = ChaTome();
        break;
    }
    return value;
}

size_t Character::GetSpecialFeatTrainedCount(
        const std::string& featName) const
{
    // return the count of how many times this particular feat has
    // been trained.
    size_t count = 0;
    const std::list<TrainedFeat>& specialFeats = SpecialFeats().Feats();
    for (auto&& sfit: specialFeats)
    {
        if (sfit.FeatName() == featName)
        {
            ++count;    // it is present, count it
        }
    }
    // also need to check the builds favor feats
    const Build* pBuild = ActiveBuild();
    if (pBuild != NULL)
    {
        const std::list<TrainedFeat>& favorFeats = pBuild->FavorFeats().Feats();
        for (auto&& ffit : favorFeats)
        {
            if (ffit.FeatName() == featName)
            {
                ++count;    // it is present, count it
            }
        }
    }
    return count;
}

void Character::TrainSpecialFeat(
        const std::string& featName)
{
    const Feat& feat = FindFeat(featName);
    // just add a copy of the feat name to the current list
    TrainedFeat tf(featName, (LPCTSTR)EnumEntryText(feat.Acquire(), featAcquisitionMap), 0);
    m_SpecialFeats.Add(tf);
    m_hasSpecialFeats = true;
}

bool Character::RevokeSpecialFeat(
        const std::string& featName)
{
    // just remove the first copy of the feat name from the current list
    std::list<TrainedFeat> trainedFeats = SpecialFeats().Feats();
    std::list<TrainedFeat>::iterator it = trainedFeats.begin();
    bool found = false;
    while (!found && it != trainedFeats.end())
    {
        if ((*it).FeatName() == featName)
        {
            // this is the first occurrence, remove it
            it = trainedFeats.erase(it);
            found = true;
        }
        else
        {
            ++it;
        }
    }
    if (found)
    {
        FeatsListObject flo(L"SpecialFeats", trainedFeats);
        Set_SpecialFeats(flo);
    }
    return found;
}

void Character::SetSkillTome(SkillType skill, size_t value)
{
    m_Tomes.SetSkillTome(skill, value);
}

size_t Character::SkillTomeValue(SkillType skill, size_t level) const
{
    // max tome value for level applies
    // +1 and +2 Tomes will be applied at level 1 and higher
    // +3 Tomes will be applied at level 3 and higher
    // +4 Tomes will be applied at level 7 and higher
    // +5 Tomes will be applied at level 11 and higher
    size_t maxTome = 2;
    if (level >= 3) ++maxTome;
    if (level >= 7) ++maxTome;
    if (level >= 11) ++maxTome;
    if (level >= 15) ++maxTome;     // assumed progression
    if (level >= 19) ++maxTome;
    if (level >= 23) ++maxTome;
    if (level >= 27) ++maxTome;
    if (level >= 31) ++maxTome;
    size_t value = m_Tomes.SkillTomeValue(skill);
    value = min(maxTome, value);
    return value;
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

void Character::AddSpecialFeats(const FeatsListObject& featsToAdd)
{
    const std::list<TrainedFeat>& feats = featsToAdd.Feats();
    // we need to compact the feats to a single entry for each and the count
    // we then see if the Character level already has the same or less
    // number of these feats. If its less, we update the Character level count
    // to match the new maximum only
    std::map<std::string, size_t> counts;
    for (auto&& fit: feats)
    {
        if (counts.find(fit.FeatName()) != counts.end())
        {
            counts[fit.FeatName()]++;
        }
        else
        {
            counts[fit.FeatName()] = 1;
        }
    }

    for (auto&& cit: counts)
    {
        size_t trainedCount = GetSpecialFeatTrainedCount(cit.first);
        size_t newCount = cit.second;
        if (newCount > trainedCount)
        {
            while (trainedCount < newCount)
            {
                TrainSpecialFeat(cit.first);
                trainedCount++;
            }
        }
    }
}

void Character::SetSkillTomes(const SkillTomes& tomes)
{
    Set_Tomes(tomes);
}

void Character::SetSpecialFeats(const FeatsListObject& feats)
{
    Set_SpecialFeats(feats);
}

