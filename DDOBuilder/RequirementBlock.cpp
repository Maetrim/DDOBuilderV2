// RequirementBlock.cpp
//
#include "StdAfx.h"
#include "RequirementBlock.h"
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
    const wchar_t f_saxElementName[] = L"Block";
    DL_DEFINE_NAMES(RequirementBlock_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

#define DL_ELEMENT RequirementBlock

RequirementBlock::RequirementBlock() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(RequirementBlock_PROPERTIES)
}

DL_DEFINE_ACCESS(RequirementBlock_PROPERTIES)

XmlLib::SaxContentElementInterface* RequirementBlock::StartElement(
    const XmlLib::SaxString& name,
    const XmlLib::SaxAttributes& attributes)
{
    XmlLib::SaxContentElementInterface* subHandler =
        SaxContentElement::StartElement(name, attributes);

    DL_START(RequirementBlock_PROPERTIES)

    return subHandler;
}

void RequirementBlock::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(RequirementBlock_PROPERTIES)
}

void RequirementBlock::Write(XmlLib::SaxWriter* writer) const
{
    writer->StartElement(ElementName(), VersionAttributes());
    DL_WRITE(RequirementBlock_PROPERTIES)
        writer->EndElement();
}

bool RequirementBlock::Met(
        const Build & build,
        size_t level,
        bool includeTomes) const
{
    bool met = true;
    // check all individual requirements
    auto itR = m_Requires.begin();
    while (met && itR != m_Requires.end())
    {
        met = (*itR).Met(build, level, includeTomes);
        ++itR;
    }
    // evaluate all Requires OneOf items
    auto itOO = m_OneOf.begin();
    while (met && itOO != m_OneOf.end())
    {
        met = (*itOO).Met(build, level, includeTomes);
        ++itOO;
    }
    // evaluate any requires none-of items
    if (met
            && HasNoneOf())
    {
        met = NoneOf().Met(build, level, includeTomes);
    }
    return met;
}

void RequirementBlock::CreateRequirementStrings(
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
    if (HasNoneOf())
    {
        NoneOf().CreateRequirementStrings(build, level, includeTomes, requirements, met);
    }
}

bool RequirementBlock::operator==(const RequirementBlock& other) const
{
    return (m_Requires == other.m_Requires)
            && (m_OneOf == other.m_OneOf)
            && (m_hasNoneOf == other.m_hasNoneOf)
            && (m_NoneOf == other.m_NoneOf);
}

bool RequirementBlock::VerifyObject(
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
    if (HasNoneOf())
    {
        ok &= m_NoneOf.VerifyObject(ss);
    }
    if (m_Requires.empty() && m_OneOf.empty() && !HasNoneOf())
    {
        (*ss) << "Requirements empty\n";
        ok = false;
    }
    return ok;
}

