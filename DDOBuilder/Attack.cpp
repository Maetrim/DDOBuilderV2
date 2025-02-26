// Attack.cpp
//
#include "StdAfx.h"
#include "Attack.h"

#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT Attack

namespace
{
    const wchar_t f_saxElementName[] = L"Attack";
    const wchar_t f_saxElementNameThisAttack[] = L"ThisAttack";
    const wchar_t f_saxElementNameFollowOn[] = L"FollowOn";
    DL_DEFINE_NAMES(Attack_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Attack::Attack() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_ThisAttack(f_saxElementNameThisAttack),
    m_FollowOn(f_saxElementNameFollowOn),
    m_stacks(0)
{
    DL_INIT(Attack_PROPERTIES)
}

Attack::Attack(
    const std::string& name,
    const std::string& description,
    const std::string& icon) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_ThisAttack(f_saxElementNameThisAttack),
    m_FollowOn(f_saxElementNameFollowOn),
    m_stacks(0)
{
    // this constructor should only ever be called for a dud attack item
    DL_INIT(Attack_PROPERTIES)
    m_hasName = true;
    m_Name = name;
    m_hasDescription = true;
    m_Description = description;
    m_hasIcon = true;
    m_Icon = icon;
}

DL_DEFINE_ACCESS(Attack_PROPERTIES)

XmlLib::SaxContentElementInterface * Attack::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Attack_PROPERTIES)

    return subHandler;
}

void Attack::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Attack_PROPERTIES)
}

void Attack::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Attack_PROPERTIES)
    writer->EndElement();
}

size_t Attack::Stacks() const
{
    return m_stacks;
}

void Attack::AddStack()
{
    ++m_stacks;
}

void Attack::RevokeStack()
{
    --m_stacks;
}

void Attack::SetCooldown(double cooldown)
{
    // this function should only ever be called for the "Basic Attack"
    if (Name() != "Basic Attack")
    {
        std::stringstream ss;
        ss << "ERROR: Cooldown for attack was erroneously set, can only be done for \"Basic Attack\"";
        GetLog().AddLogEntry(ss.str().c_str());
    }
    else
    {
        std::vector<double> values;
        values.push_back(cooldown);
        Set_Cooldown(values);
    }
}
