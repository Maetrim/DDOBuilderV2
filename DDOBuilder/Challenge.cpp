// Challenge.cpp
//
#include "StdAfx.h"
#include "Challenge.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

#define DL_ELEMENT Challenge

namespace
{
    const wchar_t f_saxElementName[] = L"Challenge";
    DL_DEFINE_NAMES(Challenge_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

int Challenge::gm_column = 0;
bool Challenge::gm_bAscending = false;

Challenge::Challenge() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_difficulty(QD_notRun)
{
    DL_INIT(Challenge_PROPERTIES)
}

Challenge::Challenge(const std::string& name) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_difficulty(QD_notRun)
{
    DL_INIT(Challenge_PROPERTIES)
    m_Name = name;
    m_hasName = true;
}

DL_DEFINE_ACCESS(Challenge_PROPERTIES)

XmlLib::SaxContentElementInterface * Challenge::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Challenge_PROPERTIES)

    return subHandler;
}

void Challenge::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Challenge_PROPERTIES)
}

void Challenge::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Challenge_PROPERTIES)
    writer->EndElement();
}

bool Challenge::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "Challenge \"" << m_Name << "\" ";
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

void Challenge::SetSortInfo(int column, bool bAscending)
{
    gm_column = column;
    gm_bAscending = bAscending;
}

void Challenge::SetDifficulty(QuestDifficulty diff)
{
    m_difficulty = diff;
}

QuestDifficulty Challenge::GetDifficulty() const
{
    return m_difficulty;
}
int Challenge::MaxFavor() const
{
    int maxFavor = 6;       // always 6 for a challenge
    return maxFavor;
}

int Challenge::Favor(QuestDifficulty diff) const
{
    int favor = 0;
    switch (diff)
    {
        case QD_notRun:     break;
        case QD_1Star:      favor = 1; break;
        case QD_2Star:      favor = 2; break;
        case QD_3Star:      favor = 3; break;
        case QD_4Star:      favor = 4; break;
        case QD_5Star:      favor = 5; break;
        case QD_6Star:      favor = 6; break;
    }
    return favor;
}

bool Challenge::operator<(const Challenge& other) const
{
    bool bRet = false;

    switch (gm_column)
    {
        case 0: // Challenge name
            if (Name() != other.Name())
            {
                bRet = Name() < other.Name();
            }
            else
            {
                // sort by level
                if (LevelRange()[0] != other.LevelRange()[0])
                {
                    // start level range is the different
                    bRet = LevelRange()[0] < other.LevelRange()[0];
                }
                else
                {
                    // sort by end level range
                    bRet = LevelRange()[1] < other.LevelRange()[1];
                }
            }
            break;
        case 1: // Challenge level
            if (LevelRange()[0] != other.LevelRange()[0])
            {
                // sort by start level range
                bRet = LevelRange()[0] < other.LevelRange()[0];
            }
            else
            {
                if (LevelRange()[1] != other.LevelRange()[1])
                {
                    // sort by end level range
                    bRet = LevelRange()[1] < other.LevelRange()[1];
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
            }
            break;
        case 2: // Challenge favor
            if (Favor(QD_1Star) != other.Favor(QD_1Star))
            {
                bRet = Favor(QD_1Star) < other.Favor(QD_1Star);
            }
            else
            {
                // sort by patron name
                bRet = Patron() < other.Patron();
            }
            break;
        case 3: // Challenge run at
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
        case 4: // Challenge patron
            if (Patron() != other.Patron())
            {
                bRet = Patron() < other.Patron();
            }
            else
            {
                if (LevelRange()[0] != other.LevelRange()[0])
                {
                    // start level range is the different
                    bRet = LevelRange()[0] < other.LevelRange()[0];
                }
                else
                {
                    // sort by end level range
                    bRet = LevelRange()[1] < other.LevelRange()[1];
                }
            }
            break;
        case 5: // adventure pack
            if (AdventurePack() != other.AdventurePack())
            {
                bRet = AdventurePack() < other.AdventurePack();
            }
            else
            {
                if (LevelRange()[0] != other.LevelRange()[0])
                {
                    // start level range is the different
                    bRet = LevelRange()[0] < other.LevelRange()[0];
                }
                else
                {
                    // sort by end level range
                    bRet = LevelRange()[1] < other.LevelRange()[1];
                }
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

