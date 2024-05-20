// CompletedQuest.cpp
//
#include "StdAfx.h"
#include "CompletedQuest.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "Quest.h"

#define DL_ELEMENT CompletedQuest

namespace
{
    const wchar_t f_saxElementName[] = L"CompletedQuest";
    DL_DEFINE_NAMES(CompletedQuest_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

CompletedQuest::CompletedQuest() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(CompletedQuest_PROPERTIES)
}

DL_DEFINE_ACCESS(CompletedQuest_PROPERTIES)

XmlLib::SaxContentElementInterface * CompletedQuest::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(CompletedQuest_PROPERTIES)

    return subHandler;
}

void CompletedQuest::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(CompletedQuest_PROPERTIES)
}

void CompletedQuest::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(CompletedQuest_PROPERTIES)
    writer->EndElement();
}

bool CompletedQuest::SameQuestAndLevel(const CompletedQuest& other) const
{
    bool bSame = false;
    if (Name() == other.Name()
            && Level() == other.Level())
    {
        bSame = true;
    }
    return bSame;
}

bool CompletedQuest::Supports(QuestDifficulty qd) const
{
    bool bSupported = false;
    const Quest& q = FindQuest(Name());
    switch (qd)
    {
        case QD_notRun:     bSupported = true; // always
        case QD_solo:       bSupported = q.HasSolo(); break;
        case QD_casual:     bSupported = q.HasCasual(); break;
        case QD_normal:     bSupported = q.HasNormal(); break;
        case QD_hard:       bSupported = q.HasHard(); break;
        case QD_elite:      bSupported = q.HasElite(); break;
        case QD_reaper1:
        case QD_reaper2:
        case QD_reaper3:
        case QD_reaper4:
        case QD_reaper5:
        case QD_reaper6:
        case QD_reaper7:
        case QD_reaper8:
        case QD_reaper9:
        case QD_reaper10:   bSupported = q.HasReaper(); break;
    }
    return bSupported;
}

