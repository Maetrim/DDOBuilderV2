// RequiresNoneOf.cpp
//
#include "StdAfx.h"
#include "RequiresNoneOf.h"
#include "XmlLib\SaxWriter.h"
#include "Requirement.h"

#define DL_ELEMENT RequiresNoneOf

namespace
{
    const wchar_t f_saxElementName[] = L"RequiresNoneOf";
    DL_DEFINE_NAMES(RequiresNoneOf_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

RequiresNoneOf::RequiresNoneOf() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(RequiresNoneOf_PROPERTIES)
}

DL_DEFINE_ACCESS(RequiresNoneOf_PROPERTIES)

XmlLib::SaxContentElementInterface * RequiresNoneOf::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(RequiresNoneOf_PROPERTIES)

    return subHandler;
}

void RequiresNoneOf::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(RequiresNoneOf_PROPERTIES)
}

void RequiresNoneOf::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(RequiresNoneOf_PROPERTIES)
    writer->EndElement();
}

bool RequiresNoneOf::Met(
        const Build & build,
        size_t level,
        bool includeTomes,
        InventorySlotType slot,
        WeaponType wtMainHand,
        WeaponType wtOffHand) const
{
    // one or more of the requirements must not be met
    bool canTrain = true;
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (it != m_Requirements.end())
    {
        canTrain &= !(*it).Met(
                build,
                level,
                includeTomes,
                slot,
                wtMainHand,
                wtOffHand);
        ++it;
    }
    return canTrain;        // can't have any requirement met
}

bool RequiresNoneOf::CanTrainEnhancement(
    const Build& build,
    size_t trainedRanks) const
{
    bool canTrain = false;
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (it != m_Requirements.end())
    {
        canTrain |= (*it).CanTrainEnhancement(
            build,
            trainedRanks);
        ++it;
    }
    return !canTrain;
}

bool RequiresNoneOf::MetEnhancements(const Build& build, size_t trainedRanks) const
{
    // one or more of the requirements must not be met
    bool canTrain = true;
    bool bNone = true;
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (canTrain && it != m_Requirements.end())
    {
        if ((*it).Type() == Requirement_Enhancement)
        {
            canTrain &= !(*it).MetEnhancements(build, trainedRanks);
            bNone = false;
        }
        ++it;
    }
    return canTrain || bNone;
}

bool RequiresNoneOf::MetHardRequirements(
    const Build& build,
    size_t level,
    bool includeTomes) const
{
    // one or more of the requirements must not be met
    bool canTrain = true;
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (it != m_Requirements.end())
    {
        canTrain &= !(*it).MetHardRequirements(
                build,
                level,
                includeTomes);
        ++it;
    }
    return canTrain;       // can't have any requirement met
}

void RequiresNoneOf::CreateRequirementStrings(
        const Build & build,
        size_t level,
        bool includeTomes,
        std::vector<CString>* requirementsArray,
        std::vector<bool>* metArray) const
{
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    std::vector<CString> localRequirements;
    std::vector<bool> localMet;
    while (it != m_Requirements.end())
    {
        (*it).CreateRequirementStrings(build, level, includeTomes, &localRequirements, &localMet);
        ++it;
    }
    if (HasDisplayDescription())
    {
        // if we have a display description, this replaces all the local strings with
        // a single entry which has a single met bool.
        // This is because some items can have so many exclusions that it would
        // take up several lines of screen space
        CString description = "Requires none of: ";
        description += m_DisplayDescription.c_str();
        bool wasMet = false;
        for (int i = 0; i < (int)localRequirements.size(); ++i)
        {
            wasMet |= localMet[i];
        }
        requirementsArray->push_back(description);
        metArray->push_back(!wasMet);        // these are not required things
    }
    else
    {
        // determine any common header on all requirements. We do this because some
        // requirements may be a list of many almost identical items e.g.:
        // Improved Critical: Slashing, Improved Critical: Piercing,.... or Improved Critical: Ranged
        // this can produce a very long requirements line. It can be cut down to:
        // Improved Critical: Slashing, Piercing, ... or Ranged
        if (localRequirements.size() > 0
                && localRequirements[0].Find(':', 10) >= 0)
        {
            // looks like we may have a possible set of duplicate text
            int pos = localRequirements[0].Find(':', 10) + 2;
            CString dupText = localRequirements[0].Left(pos);
            bool isDuplicated = true;
            for (size_t i = 1; i < localRequirements.size(); ++i)
            {
                if (localRequirements[i].Left(pos) != dupText)
                {
                    isDuplicated = false;
                }
            }
            if (isDuplicated)
            {
                // we can remove the duplicate text from items 1...n but keep for item [0]
                for (size_t i = 1; i < localRequirements.size(); ++i)
                {
                    localRequirements[i] = localRequirements[i].Right(localRequirements[i].GetLength() - pos);
                }
            }
        }
        // re-package the local requirements to a single line
        if (localRequirements.size() > 0)
        {
            CString description = "Requires none of: ";
            bool wasMet = false;
            for (int i = 0; i < (int)localRequirements.size(); ++i)
            {
                localRequirements[i].Replace("Requires: ", "");    // take of leading text
                description += localRequirements[i];
                if (i <= (int)localRequirements.size() - 3)
                {
                    description += ", ";
                }
                else if (i == (int)localRequirements.size() - 2)
                {
                    description += " or ";
                }
                wasMet |= localMet[i];
            }
            requirementsArray->push_back(description);
            metArray->push_back(!wasMet);        // these are not required things
        }
    }
}

void RequiresNoneOf::AddRequirement(const Requirement& req)
{
    m_Requirements.push_back(req);
}

bool RequiresNoneOf::RequiresAnEnhancement() const
{
    bool bRequiresEnhancement = false;
    // check all the individual requirements
    for (auto&& it : m_Requirements)
    {
        bRequiresEnhancement |= it.RequiresAnEnhancement();
    }
    return bRequiresEnhancement;
}

bool RequiresNoneOf::operator==(const RequiresNoneOf& other) const
{
    return (m_hasDisplayDescription == other.m_hasDisplayDescription)
            && (m_DisplayDescription == other.m_DisplayDescription)
            && (m_Requirements == other.m_Requirements);
}

bool RequiresNoneOf::VerifyObject(
        std::stringstream * ss) const
{
    bool ok = true;
    // check all the individual requirements
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (it != m_Requirements.end())
    {
        ok &= (*it).VerifyObject(ss);
        ++it;
    }
    return ok;
}
