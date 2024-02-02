// Character.cpp
//
#include "StdAfx.h"
#include "Character.h"
#include "XmlLib\SaxWriter.h"
#include "DDOBuilderDoc.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

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

void Character::SetActiveBuild(size_t lifeIndex, size_t buildIndex)
{
    if (m_uiActiveLifeIndex != lifeIndex
            || m_uiActiveBuildIndex != buildIndex)
    {
        CWaitCursor longOperation;
        m_uiActiveLifeIndex = lifeIndex;
        m_uiActiveBuildIndex = buildIndex;
        m_ActiveLifeIndex = lifeIndex;
        m_ActiveBuildIndex = buildIndex;

        if (ActiveBuild() != NULL)
        {
            std::stringstream ss;
            ss << "Selected Life \"" << ActiveLife()->UIDescription(lifeIndex) << "\" build of \"" << ActiveBuild()->ComplexUIDescription() << "\"";
            SetModifiedFlag(TRUE);
            GetLog().AddLogEntry(ss.str().c_str());
        }

        NotifyActiveLifeChanged();
        NotifyActiveBuildChanged();
        if (ActiveBuild() != NULL)
        {
            ActiveBuild()->BuildNowActive();
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

