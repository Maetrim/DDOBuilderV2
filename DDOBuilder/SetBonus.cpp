// SetBonus.cpp
//
#include "StdAfx.h"
#include "SetBonus.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

// A set bonus is a series of buffs that apply when a given number of "items"
// with the same bonus have been equipped. Set bonus's are unique by "Type"
// Items, enhancements, Filigrees etc can have more than 1 set bonus

#define DL_ELEMENT SetBonus

namespace
{
    const wchar_t f_saxElementName[] = L"SetBonus";
    DL_DEFINE_NAMES(SetBonus_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

SetBonus::SetBonus() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(SetBonus_PROPERTIES)
}

DL_DEFINE_ACCESS(SetBonus_PROPERTIES)

XmlLib::SaxContentElementInterface * SetBonus::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SetBonus_PROPERTIES)

    return subHandler;
}

void SetBonus::EndElement()
{
    // now check out our object
    SaxContentElement::EndElement();
    DL_END(SetBonus_PROPERTIES)
}

void SetBonus::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SetBonus_PROPERTIES)
    writer->EndElement();
}

void SetBonus::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "--- Set Bonus " << m_Type << "\n";
    if (!ImageFileExists("DataFiles\\SetBonusImages\\", Icon())
            && !ImageFileExists("DataFiles\\FiligreeImages\\", Icon())
            && !ImageFileExists("DataFiles\\AugmentImages\\", Icon()))
    {
        ss << "...is missing image file \"" << Icon() << "\"\n";
        ok = false;
    }
    // check the effects also
    for (auto&& it : m_Buffs)
    {
        for (auto&& bit : it.Effects())
        {
            ok &= bit.VerifyObject(&ss);
        }
    }
    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

bool SetBonus::operator==(const SetBonus& other) const
{
    return (m_Type == other.Type());
}

std::list<Effect> SetBonus::ActiveEffects(size_t stacks) const
{
    // we only add the effects which are at this stack size
    std::list<Effect> effects;
    for (auto&& bit : m_Buffs)
    {
        if (bit.EquippedCount() == stacks)
        {
            effects.insert(effects.end(), bit.Effects().begin(), bit.Effects().end());
        }
    }
    // make sure the effects have the set name if they don't have a specific
    // override name set in their data
    for (auto&& eit : effects)
    {
        if (!eit.HasDisplayName())
        {
            eit.SetDisplayName(Type());
        }
    }
    return effects;
}

