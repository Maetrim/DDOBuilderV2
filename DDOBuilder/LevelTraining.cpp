// LevelTraining.cpp
//
#include "StdAfx.h"
#include "LevelTraining.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT LevelTraining

namespace
{
    const wchar_t f_saxElementName[] = L"LevelTraining";
    DL_DEFINE_NAMES(LevelTraining_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

LevelTraining::LevelTraining() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(LevelTraining_PROPERTIES)
}

DL_DEFINE_ACCESS(LevelTraining_PROPERTIES)

XmlLib::SaxContentElementInterface * LevelTraining::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(LevelTraining_PROPERTIES)

    return subHandler;
}

void LevelTraining::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(LevelTraining_PROPERTIES)
}

void LevelTraining::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(LevelTraining_PROPERTIES)
    writer->EndElement();
}

void LevelTraining::TrainFeat(
        const std::string& featName,
        const std::string& type,
        size_t level,
        bool bFeatSwapWarning)
{
    TrainedFeat tf(featName, type, level, bFeatSwapWarning);
    m_TrainedFeats.push_back(tf);
}

std::string LevelTraining::RevokeFeat(const std::string& type)
{
    std::string revokedFeat;
    // find the feat in the list and remove it
    bool found = false;
    std::list<TrainedFeat>::iterator it = m_TrainedFeats.begin();
    while (it != m_TrainedFeats.end())
    {
        if ((*it).Type() == type)
        {
            // revoke this one
            revokedFeat = (*it).FeatName();
            it = m_TrainedFeats.erase(it);
            found = true;
            break;  // were done
        }
        ++it;
    }
    // we may have tried to revoke a feat that had not been trained. That's ok!
    return revokedFeat;
}

void LevelTraining::RevokeAllFeats(const std::string& type)
{
    // revoke all trained feats of a specific type
    // e.g. all Automatic feats
    std::list<TrainedFeat>::iterator it = m_TrainedFeats.begin();
    while (it != m_TrainedFeats.end())
    {
        if ((*it).Type() == type)
        {
            // revoke this one
            it = m_TrainedFeats.erase(it);
        }
        else
        {
            // keep this feat, skip to next
            ++it;
        }
    }
}

void LevelTraining::TrainSkill(SkillType skill)
{
    TrainedSkill ts;
    ts.Set_Skill(skill);
    m_TrainedSkills.push_back(ts);
    ASSERT(m_SkillPointsSpent < m_SkillPointsAvailable);
    ++m_SkillPointsSpent;
}

void LevelTraining::RevokeSkill(SkillType skill)
{
    ASSERT(m_SkillPointsSpent > 0);
    bool found = false;
    std::list<TrainedSkill>::iterator it = m_TrainedSkills.begin();
    while (it != m_TrainedSkills.end())
    {
        if ((*it).Skill() == skill)
        {
            // revoke this one
            it = m_TrainedSkills.erase(it);
            found = true;
            break;  // were done
        }
        ++it;
    }
    ASSERT(found);
    --m_SkillPointsSpent;
}

std::string LevelTraining::FeatName(const std::string& type) const
{
    std::list<TrainedFeat>::const_iterator it = m_TrainedFeats.begin();
    while (it != m_TrainedFeats.end())
    {
        if ((*it).Type() == type)
        {
            return (*it).FeatName();
        }
        ++it;
    }
    return "";
}

const std::list<TrainedFeat> & LevelTraining::AutomaticFeats() const
{
    return m_AutomaticFeats;
}

void LevelTraining::Set_AutomaticFeats(const std::list<TrainedFeat> & newValue)
{
    m_AutomaticFeats = newValue;
}
