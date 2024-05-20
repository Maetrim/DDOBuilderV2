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

Quest::Quest(const std::string& name) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_difficulty(QD_notRun)
{
    DL_INIT(Quest_PROPERTIES)
    m_Name = name;
    m_hasName = true;
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

int Quest::Favor(QuestDifficulty diff) const
{
    int favor = 0;
    switch (diff)
    {
        case QD_notRun:     break;
        case QD_solo:       favor = Favor(); break;
        case QD_casual:     favor = (Favor() / 2); break;
        case QD_normal:     favor = Favor(); break;
        case QD_hard:       favor = (Favor() * 2); break;
        case QD_elite:
        case QD_reaper1:
        case QD_reaper2:
        case QD_reaper3:
        case QD_reaper4:
        case QD_reaper5:
        case QD_reaper6:
        case QD_reaper7:
        case QD_reaper8:
        case QD_reaper9:
        case QD_reaper10:   favor = (Favor() * 3); break;
    }
    return favor;
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
            if (Levels()[0] != other.Levels()[0])
            {
                bRet = Levels()[0] < other.Levels()[0];
            }
            else
            {
                // sort by patron name
                if (Patron() != other.Patron())
                {
                    bRet = Patron() < other.Patron();
                }
                else
                {
                    // same patron, sort by name
                    bRet = Name() < other.Name();
                }
            }
            break;
        case 2: // quest favor
            if (Favor() != other.Favor())
            {
                bRet = Favor() < other.Favor();
            }
            else
            {
                // sort by patron name
                bRet = Patron() < other.Patron();
            }
            break;
        case 3: // quest run at
            if (GetDifficulty() != other.GetDifficulty())
            {
                bRet = GetDifficulty() < other.GetDifficulty();
            }
            else
            {
                // sort by patron name
                bRet = Patron() < other.Patron();
            }
            break;
        case 4: // quest patron
            if (Patron() != other.Patron())
            {
                bRet = Patron() < other.Patron();
            }
            else
            {
                // sort by level
                bRet = Levels()[0] < other.Levels()[0];
            }
            break;
    }
    if (!gm_bAscending)
    {
        // switch sort direction result
        bRet = !bRet;
    }
    return bRet;
}

