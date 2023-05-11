// WeaponGroup.cpp
//
#include "stdafx.h"
#include "WeaponGroup.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "Build.h"

#define DL_ELEMENT WeaponGroup

namespace
{
    const wchar_t f_saxElementName[] = L"WeaponGroup";
    DL_DEFINE_NAMES(WeaponGroup_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

WeaponGroup::WeaponGroup() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(WeaponGroup_PROPERTIES)
}

WeaponGroup::WeaponGroup(
        const std::string & name) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_Name(name),
    m_hasName(true)
{
    DL_INIT(WeaponGroup_PROPERTIES)
    m_Name = name;
    m_hasName = true;
}

DL_DEFINE_ACCESS(WeaponGroup_PROPERTIES)

XmlLib::SaxContentElementInterface * WeaponGroup::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(WeaponGroup_PROPERTIES)

    return subHandler;
}

void WeaponGroup::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(WeaponGroup_PROPERTIES)
}

void WeaponGroup::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(WeaponGroup_PROPERTIES)
    writer->EndElement();
}

bool WeaponGroup::HasWeapon(WeaponType wt) const
{
    // return true if this weapon type is in the list
    bool bPresent = false;
    std::list<WeaponType>::const_iterator it = m_Weapon.begin();
    while (!bPresent && it != m_Weapon.end())
    {
        bPresent = ((*it) == wt);
        ++it;
    }
    return bPresent;
}

bool WeaponGroup::HasWeapon(WeaponType wt, const Build& build) const
{
    // return true if this weapon type is in the list
    bool bPresent = HasWeapon(wt);
    if (!bPresent)
    {
        // couls also be in the list of weapons with requirements
        for (auto&& wwrit : m_weaponsWithRequirements)
        {
            if (wwrit.Type() == wt
                && wwrit.RequirementsMet(build))
            {
                bPresent = true;
                break;
            }
        }
    }
    if (!bPresent)
    {
        // also need to check any merge groups we may have
        for (auto&& mgit : m_mergedGroups)
        {
            bPresent = build.IsWeaponInGroup(mgit, wt);
            if (bPresent) break;
        }
    }
    return bPresent;
}

void WeaponGroup::AddWeapon(WeaponType wt)
{
    // multiple entries of same weapn type is ok
    m_Weapon.push_back(wt);
}

void WeaponGroup::RemoveWeapon(WeaponType wt)
{
    // remove the first occurrence from the list if found
    std::list<WeaponType>::iterator it = m_Weapon.begin();
    while (it != m_Weapon.end())
    {
        if ((*it) == wt)
        {
            m_Weapon.erase(it);
            it = m_Weapon.end();    // were done
        }
        else
        {
            ++it;
        }
    }
}

void WeaponGroup::AddWeapon(WeaponType wt, const Requirements& requirements)
{
    // multiple entries of same weapn type is ok
    WeaponGroupRequirements wwr(wt, requirements);
    m_weaponsWithRequirements.push_back(wwr);
}

void WeaponGroup::RemoveWeapon(WeaponType wt, const Requirements& requirements)
{
    WeaponGroupRequirements wwr(wt, requirements);
    // remove the first occurrence from the list if found
    std::list<WeaponGroupRequirements>::iterator it = m_weaponsWithRequirements.begin();
    while (it != m_weaponsWithRequirements.end())
    {
        if ((*it) == wwr)
        {
            m_weaponsWithRequirements.erase(it);
            it = m_weaponsWithRequirements.end();    // were done
        }
        else
        {
            ++it;
        }
    }
}

void WeaponGroup::AddMergeGroup(const std::string& groupName)
{
    m_mergedGroups.push_back(groupName);
}

void WeaponGroup::RemoveMergeGroup(const std::string& groupName)
{
    std::list<std::string>::iterator it = m_mergedGroups.begin();
    while (it != m_mergedGroups.end())
    {
        if ((*it) == groupName)
        {
            m_mergedGroups.erase(it);
            it = m_mergedGroups.end();    // were done
        }
        else
        {
            ++it;
        }
    }
}

bool WeaponGroup::operator<(const WeaponGroup & other) const
{
    return Name() < other.Name();
}

void WeaponGroup::VerifyObject() const
{
    // make sure we don't have any Unknown weapons
    if (HasWeapon(Weapon_Unknown))
    {
        std::stringstream ss;
        ss << "Weapon group \"" << m_Name << "\" has bad enum entry\r\n";
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

void WeaponGroup::DumpToLog(const Build& build) const
{
    std::stringstream ss;
    ss << "Weapon group \"" << m_Name << "\" has " << m_Weapon.size() << " simple entries and " << m_weaponsWithRequirements.size() << " complex entries\r\n";
    GetLog().AddLogEntry(ss.str().c_str());
    if (m_Weapon.size() > 0)
    {
        GetLog().AddLogEntry("  Simple:\r\n");
        for (auto&& wit : m_Weapon)
        {
            std::stringstream wss;
            wss << "    " << EnumEntryText(wit, weaponTypeMap) << "\r\n";
            GetLog().AddLogEntry(wss.str().c_str());
        }
    }
    if (m_weaponsWithRequirements.size() > 0)
    {
        GetLog().AddLogEntry("  Complex:\r\n");
        for (auto&& wit : m_weaponsWithRequirements)
        {
            std::stringstream wss;
            wss << "    " << EnumEntryText(wit.Type(), weaponTypeMap);
            if (wit.RequirementsMet(build)) wss << " (Active)\r\n"; else wss << " (Inactive)\r\n";
            GetLog().AddLogEntry(wss.str().c_str());
        }
    }
    if (m_mergedGroups.size() > 0)
    {
        GetLog().AddLogEntry("  Merged Groups:\r\n");
        for (auto&& mgit : m_mergedGroups)
        {
            std::stringstream mgss;
            mgss << "    " << mgit << "\r\n";
            GetLog().AddLogEntry(mgss.str().c_str());
        }
    }
}
