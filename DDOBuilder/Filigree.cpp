// Filigree.cpp
//
#include "StdAfx.h"
#include "Filigree.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT Filigree

namespace
{
    const wchar_t f_saxElementName[] = L"Filigree";
    DL_DEFINE_NAMES(Filigree_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Filigree::Filigree() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Filigree_PROPERTIES)
}

DL_DEFINE_ACCESS(Filigree_PROPERTIES)

XmlLib::SaxContentElementInterface * Filigree::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Filigree_PROPERTIES)

    return subHandler;
}

void Filigree::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Filigree_PROPERTIES)
}

void Filigree::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Filigree_PROPERTIES)
    writer->EndElement();
}

bool Filigree::operator==(const Filigree & other) const
{
    bool bEqual = (m_Name == other.m_Name);
    return bEqual;
}

std::list<Effect> Filigree::NormalEffects() const
{
    std::list<Effect> effects;
    for (auto&& eit : m_Effects)
    {
        if (!eit.HasRare())
        {
            effects.push_back(eit);
        }
    }
    return effects;
}

std::list<Effect> Filigree::RareEffects() const
{
    std::list<Effect> effects;
    for (auto&& eit : m_Effects)
    {
        if (eit.HasRare())
        {
            effects.push_back(eit);
        }
    }
    return effects;
}

bool Filigree::operator<(const Filigree & other) const
{
    bool bBefore = false;
    // we first sort by Menu and then name
    if (Menu() == other.Menu())
    {
        // sort by name if in the same menu
        // (assumes all filigree names are unique)
        bBefore =  (Name() < other.Name());
    }
    else
    {
        // sort by what menu they are in
        bBefore =  (Menu() < other.Menu());
    }
    return bBefore;
}

void Filigree::VerifyObject() const
{
    // TBD
    //bool ok = true;
    //std::stringstream ss;
    //ss << "=====" << m_Name << "=====\n";

    //if (BAB().size() != MAX_CLASS_LEVEL)
    //{
    //    ss << "Has incorrect BAB vector size\n";
    //    ok = false;
    //}

    //if (!ok)
    //{
    //    GetLog().AddLogEntry(ss.str().c_str());
    //}
}
