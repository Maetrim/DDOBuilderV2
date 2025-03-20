// AttackChain.cpp
//
#include "StdAfx.h"
#include "AttackChain.h"
#include "GlobalSupportFunctions.h"

#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT AttackChain

namespace
{
    const wchar_t f_saxElementName[] = L"AttackChain";
    DL_DEFINE_NAMES(AttackChain_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

AttackChain::AttackChain() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(AttackChain_PROPERTIES)
}

DL_DEFINE_ACCESS(AttackChain_PROPERTIES)

XmlLib::SaxContentElementInterface * AttackChain::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(AttackChain_PROPERTIES)

    return subHandler;
}

void AttackChain::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(AttackChain_PROPERTIES)
}

void AttackChain::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(AttackChain_PROPERTIES)
    writer->EndElement();
}

void AttackChain::SetName(const std::string& newName)
{
    Set_Name(newName);
}

void AttackChain::SetAttacks(const std::list<std::string>& attacks)
{
    Set_Attacks(attacks);
}

void AttackChain::AddAttack(const std::string& attackName, size_t insertLoc)
{
    std::list<std::string>::iterator it = m_Attacks.begin();
    std::advance(it, insertLoc);
    m_Attacks.insert(it, attackName);
    std::stringstream ss;
    ss << "Attack chain \"" << Name() << "\" had Attack \"" << attackName << "\" added at location " << insertLoc;
    GetLog().AddLogEntry(ss.str().c_str());
}

void AttackChain::RemoveAttackAt(size_t loc)
{
    std::list<std::string>::iterator it = m_Attacks.begin();
    std::advance(it, loc);
    std::string removedAttack = (*it);
    m_Attacks.erase(it);
    std::stringstream ss;
    ss << "Attack chain \"" << Name() << "\" had Attack \"" << removedAttack << "\" removed at location " << loc;
    GetLog().AddLogEntry(ss.str().c_str());
}
