// TrainedFiligree.cpp
//
#include "StdAfx.h"
#include "TrainedFiligree.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT TrainedFiligree

namespace
{
    DL_DEFINE_NAMES(TrainedFiligree_PROPERTIES)
}

TrainedFiligree::TrainedFiligree(const XmlLib::SaxString& elementName, unsigned verCurrent) :
    XmlLib::SaxContentElement(elementName, verCurrent)
{
    DL_INIT(TrainedFiligree_PROPERTIES)
}

DL_DEFINE_ACCESS(TrainedFiligree_PROPERTIES)

XmlLib::SaxContentElementInterface * TrainedFiligree::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(TrainedFiligree_PROPERTIES)

    return subHandler;
}

void TrainedFiligree::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(TrainedFiligree_PROPERTIES)
}

void TrainedFiligree::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(TrainedFiligree_PROPERTIES)
    writer->EndElement();
}

bool TrainedFiligree::operator==(const TrainedFiligree & other) const
{
    bool bEqual = (m_Name == other.m_Name);
    return bEqual;
}

void TrainedFiligree::TranslateOldNamesFromV1()
{
    static std::string nameTranslations[] =
    {
        // old filigree name                            new filigree name
        "The Serpent: Negative and Poison Spellpower",  "The Serpent: +9 Negative/Poison Spell Power",
        "Eye of the Beholder: Spellpower",              "Eye of the Beholder: +6 Universal Spell Power",
        "Lunar Magic: Universal Spellpower",            "Lunar Magic: +6 Universal Spell Power",
        "Nystul's Mystical Defence: Constitution",      "Nystul's Mystical Defense: +1 Constitution",
        "Nystul's Mystical Defence: Electric Absorption", "Nystul's Mystical Defense: +5% Electric Absorption",
        "Nystul's Mystical Defence/Electrocution: +6 MRR", "Nystul's Mystical Defense/Electrocution +6 MRR"
    };
    size_t count = sizeof(nameTranslations) / sizeof(std::string);
    if (count % 2 != 0)
    {
        throw "Must be an multiple of 2";
    }
    for (size_t i = 0; i < count; i += 2)
    {
        if (m_Name == nameTranslations[i])
        {
            m_Name = nameTranslations[i + 1];
            break;
        }
    }
}
