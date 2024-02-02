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

int Quest::gm_column = 0;
bool Quest::gm_bAscending = false;

Quest::Quest() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_difficulty(QD_notRun)
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

void Quest::SetSortInfo(int column, bool bAscending)
{
    gm_column = column;
    gm_bAscending = bAscending;
}

void Quest::SetDifficulty(QuestDifficulty diff)
{
    m_difficulty = diff;
}

QuestDifficulty Quest::GetDifficulty() const
{
    return m_difficulty;
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

bool Quest::operator<(const Quest& other) const
{
    bool bRet = false;

    switch (gm_column)
    {
        case 0: // quest name
            if (Name() != other.Name())
            {
                bRet = Name() < other.Name();
            }
            else
            {
                // sort by level
                bRet = Levels()[0] < other.Levels()[0];
            }
            break;
        case 1: // quest level
            bRet = Levels()[0] < other.Levels()[0];
            break;
        case 2: // quest favor
            bRet = Favor() < other.Favor();
            break;
        case 3: // quest run at
            bRet = GetDifficulty() < other.GetDifficulty();
            break;
        case 4: // quest patron
            bRet = Patron() < other.Patron();
            break;
    }
    if (!gm_bAscending)
    {
        // switch sort direction result
        bRet = !bRet;
    }
    return bRet;
}

