// RequirementsBase.cpp
//
#include "StdAfx.h"
#include "RequirementsBase.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

// A requirements object if it exists list the things that a given item needs
// to be either selectable or active in the context in which it is being used
// A given object can be made up of:
// 1: A list of individual requirements
//    e.g. Cleric(3)
// 2: A list of things that must not be met
//    e.g. cannot have THF trailed
// 3: Multiple lists of optional requirements where at least 1 requirement is met
//    e.g. one of Improved Critical: <Type>
//
// The same Requirement object can be used for effects/feats/enhancements etc
// and they know how to lookup and determine whether they are met or not
// by querying their sub-object as required.

namespace
{
    DL_DEFINE_NAMES(RequirementsBase_PROPERTIES)
}

#define DL_ELEMENT RequirementsBase

RequirementsBase::RequirementsBase(const XmlLib::SaxString& elementName, unsigned verCurrent) :
    XmlLib::SaxContentElement(elementName, verCurrent)
{
    DL_INIT(RequirementsBase_PROPERTIES)
}

DL_DEFINE_ACCESS(RequirementsBase_PROPERTIES)

XmlLib::SaxContentElementInterface* RequirementsBase::StartElement(
    const XmlLib::SaxString& name,
    const XmlLib::SaxAttributes& attributes)
{
    XmlLib::SaxContentElementInterface* subHandler =
        SaxContentElement::StartElement(name, attributes);

    DL_START(RequirementsBase_PROPERTIES)

    return subHandler;
}

void RequirementsBase::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(RequirementsBase_PROPERTIES)
}

void RequirementsBase::Write(XmlLib::SaxWriter* writer) const
{
    writer->StartElement(ElementName(), VersionAttributes());
    DL_WRITE(RequirementsBase_PROPERTIES)
        writer->EndElement();
}

bool RequirementsBase::Met(
        const Build & build,
        size_t level,
        bool includeTomes,
        WeaponType wtMainHand,
        WeaponType wtOffhand) const
{
    bool met = true;
    // check all individual requirements
    auto itR = m_Requires.begin();
    while (met && itR != m_Requires.end())
    {
        met = (*itR).Met(build, level, includeTomes, wtMainHand, wtOffhand);
        ++itR;
    }
    // evaluate all Requires OneOf items
    auto itOO = m_OneOf.begin();
    while (met && itOO != m_OneOf.end())
    {
        met = (*itOO).Met(build, level, includeTomes, wtMainHand, wtOffhand);
        ++itOO;
    }
    // evaluate all Requires NoneOf items
    auto itNO = m_NoneOf.begin();
    while (met && itNO != m_NoneOf.end())
    {
        met = (*itNO).Met(build, level, includeTomes, wtMainHand, wtOffhand);
        ++itNO;
    }
    return met;
}

bool RequirementsBase::CanTrainEnhancement(
    const Build& build,
    size_t trainedRanks) const
{
    // only need to check any enhancement requirements
    bool met = true;
    std::list<Requirement>::const_iterator it = m_Requires.begin();
    while (met && it != m_Requires.end())
    {
        met = (*it).CanTrainEnhancement(build, trainedRanks);
        ++it;
    }
    std::list<RequiresOneOf>::const_iterator rooIt = OneOf().begin();
    while (met && rooIt != OneOf().end())
    {
        met = rooIt->CanTrainEnhancement(build, trainedRanks);
        rooIt++;
    }
    std::list<RequiresNoneOf>::const_iterator rnoIt = NoneOf().begin();
    while (met && rnoIt != NoneOf().end())
    {
        met = rnoIt->CanTrainEnhancement(build, trainedRanks);
        rnoIt++;
    }
    return met;
}

bool RequirementsBase::MetHardRequirements(
    const Build& build,
    size_t level,
    bool includeTomes) const
{
    bool met = true;
    // check all individual requirements
    auto itR = m_Requires.begin();
    while (met && itR != m_Requires.end())
    {
        met = (*itR).MetHardRequirements(build, level, includeTomes);
        ++itR;
    }
    // evaluate all Requires OneOf items
    auto itOO = m_OneOf.begin();
    while (met && itOO != m_OneOf.end())
    {
        met = (*itOO).MetHardRequirements(build, level, includeTomes);
        ++itOO;
    }

    // evaluate all Requires NoneOf items
    auto itNO = m_NoneOf.begin();
    while (met && itNO != m_NoneOf.end())
    {
        met = !(*itNO).MetHardRequirements(build, level, includeTomes);
        ++itNO;
    }
    return met;
}

bool RequirementsBase::MetEnhancements(
    const Build& build,
    size_t trainedRanks) const
{
    bool met = true;
    // note that for enhancements we only check direct requirements on specific enhancements
    for (auto&& rit : m_Requires)
    {
        met &= rit.MetEnhancements(build, trainedRanks);
    }
    return met;
}

void RequirementsBase::CreateRequirementStrings(
        const Build& build,
        size_t level,
        bool includeTomes,
        std::vector<CString>* requirements,
        std::vector<bool>* met) const
{
    std::list<Requirement>::const_iterator it = m_Requires.begin();
    while (it != m_Requires.end())
    {
        (*it).CreateRequirementStrings(build, level, includeTomes, requirements, met);
        ++it;
    }
    auto itOO = m_OneOf.begin();
    while (met && itOO != m_OneOf.end())
    {
        (*itOO).CreateRequirementStrings(build, level, includeTomes, requirements, met);
        ++itOO;
    }
    auto itNO = m_NoneOf.begin();
    while (met && itNO != m_NoneOf.end())
    {
        (*itNO).CreateRequirementStrings(build, level, includeTomes, requirements, met);
        ++itNO;
    }
}

bool RequirementsBase::operator==(const RequirementsBase& other) const
{
    return (m_Requires == other.m_Requires)
            && (m_OneOf == other.m_OneOf)
            && (m_NoneOf == other.m_NoneOf);
}

void RequirementsBase::SetRequirement(const Requirement& r)
{
    m_Requires.push_back(r);
}


void RequirementsBase::GetExclusionGroup(
        std::string* enhancementId,
        std::string* group) const
{
    // look through our list of requirements only
    for (auto&& it : Requires())
    {
        if (it.Type() == Requirement_Exclusive)
        {
            *enhancementId = it.Item().front();
            *group = it.Item().back();
        }
    }
}

bool RequirementsBase::RequiresEnhancement(const std::string& name) const
{
    bool bRequiresIt = false;
    // check all the individual requirements
    for (auto&& it: m_Requires)
    {
        bRequiresIt |= it.RequiresEnhancement(name);
    }
    return bRequiresIt;
}

void RequirementsBase::AddRequirement(const Requirement& req)
{
    m_Requires.push_back(req);
}

void RequirementsBase::AddRequiresOneOf(const RequiresOneOf& roo)
{
    m_OneOf.push_back(roo);
}

void RequirementsBase::AddRequiresNoneOf(const RequiresNoneOf& rno)
{
    m_NoneOf.push_back(rno);
}

bool RequirementsBase::VerifyObject(
        std::stringstream * ss) const
{
    bool ok = true;
    // check all the individual requirements
    std::list<Requirement>::const_iterator it = m_Requires.begin();
    while (it != m_Requires.end())
    {
        ok &= (*it).VerifyObject(ss);
        ++it;
    }
    std::list<RequiresOneOf>::const_iterator ooit = m_OneOf.begin();
    while (ooit != m_OneOf.end())
    {
        ok &= (*ooit).VerifyObject(ss);
        ++ooit;
    }
    std::list<RequiresNoneOf>::const_iterator noit = m_NoneOf.begin();
    while (noit != m_NoneOf.end())
    {
        ok &= (*noit).VerifyObject(ss);
        ++noit;
    }
    if (m_Requires.empty() && m_OneOf.empty() && m_NoneOf.empty())
    {
        (*ss) << "Requirements empty\n";
        ok = false;
    }
    return ok;
}

