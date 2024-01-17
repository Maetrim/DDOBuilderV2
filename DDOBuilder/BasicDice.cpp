// BasicDice.cpp
//
#include "StdAfx.h"
#include "BasicDice.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT BasicDice

namespace
{
    DL_DEFINE_NAMES(BasicDice_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

BasicDice::BasicDice(const XmlLib::SaxString& elementName) :
    XmlLib::SaxContentElement(elementName, f_verCurrent)
{
    DL_INIT(BasicDice_PROPERTIES)
}

BasicDice::BasicDice(const XmlLib::SaxString& elementName, size_t version) :
    XmlLib::SaxContentElement(elementName, version)
{
    DL_INIT(BasicDice_PROPERTIES)
}

DL_DEFINE_ACCESS(BasicDice_PROPERTIES)

XmlLib::SaxContentElementInterface * BasicDice::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(BasicDice_PROPERTIES)

    return subHandler;
}

void BasicDice::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(BasicDice_PROPERTIES)
}

void BasicDice::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(BasicDice_PROPERTIES)
    writer->EndElement();
}

CString BasicDice::DiceAsText() const
{
    CString dt;
    if (HasNumber() && HasSides())
    {
        if (HasBonus())
        {
            dt.Format("%dd%d%+d", Number(), Sides(), Bonus());
        }
        else
        {
            dt.Format("%dd%d", Number(), Sides());
        }
    }
    else if (HasBonus())
    {
        dt.Format("%d", Bonus());
    }

    return dt;
}

double BasicDice::MinDamage() const
{
    double minDmg = HasBonus() ? Bonus() : 0;
    if (HasNumber() && HasSides())
    {
        minDmg += Number();
    }
    return minDmg;
}

double BasicDice::MaxDamage() const
{
    double maxDmg = HasBonus() ? Bonus() : 0;
    if (HasNumber() && HasSides())
    {
        maxDmg += static_cast<double>(Sides()) * static_cast<double>(Number());
    }
    return maxDmg;
}

double BasicDice::AverageDamage() const
{
    double avDmg = HasBonus() ? Bonus() : 0;
    if (HasNumber() && HasSides())
    {
        avDmg += (1 + Sides() / 2.0) * Number();
    }
    return avDmg;
}

void BasicDice::Set(int number, int sides, int bonus)
{
    if (number != 0)
    {
        m_hasNumber = true;
        m_Number = number;
    }
    else
    {
        m_hasNumber = false;
    }
    if (sides != 0)
    {
        m_hasSides = true;
        m_Sides = sides;
    }
    else
    {
        m_hasSides = false;
    }
    if (bonus != 0)
    {
        m_hasBonus = true;
        m_Bonus = bonus;
    }
    else
    {
        m_hasBonus = false;
    }
}
