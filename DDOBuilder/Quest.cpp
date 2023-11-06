// Quest.cpp
//
#include "StdAfx.h"
#include "Quest.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

#define DL_ELEMENT Quest

namespace
{
    const wchar_t f_saxElementName[] = L"Quest";
    DL_DEFINE_NAMES(Quest_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Quest::Quest() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Quest_PROPERTIES)
}

DL_DEFINE_ACCESS(Quest_PROPERTIES)

XmlLib::SaxContentElementInterface * Quest::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Quest_PROPERTIES)

    return subHandler;
}

void Quest::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Quest_PROPERTIES)
}

void Quest::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Quest_PROPERTIES)
    writer->EndElement();
}

bool Quest::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "Quest \"" << m_Name << "\" ";
    if (m_Patron == Patron_Unknown)
    {
        ss << "Has bad Patron field\n";
        ok = false;
    }
    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
    return ok;
}

int Quest::MaxFavor() const
{
    int maxFavor = 0;
    if (HasReaper() || HasElite()) maxFavor = Favor() * 3;
    else if (HasHard()) maxFavor = Favor() * 2;
    else if (HasNormal() || HasSolo()) maxFavor = Favor();
    else if (HasCasual()) maxFavor = (Favor() / 2);
    return maxFavor;
}

