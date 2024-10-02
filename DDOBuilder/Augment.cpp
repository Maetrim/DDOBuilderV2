// Augment.cpp
//
#include "StdAfx.h"
#include "Augment.h"

#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "SetBonus.h"
#include "LogPane.h"

#define DL_ELEMENT Augment

namespace
{
    const wchar_t f_saxElementName[] = L"Augment";
    DL_DEFINE_NAMES(Augment_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Augment::Augment() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Augment_PROPERTIES)
}

DL_DEFINE_ACCESS(Augment_PROPERTIES)

XmlLib::SaxContentElementInterface * Augment::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Augment_PROPERTIES)

    return subHandler;
}

void Augment::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Augment_PROPERTIES)
        // fix augments that changed name between Lamannia and Release
    if (m_Name == "Visions of the Future")
    {
        m_Name = "Visions of the Beyond";
    }
    if (m_Name == "Tough Shield")
    {
        m_Name = "Tough Shields";
    }
}

void Augment::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Augment_PROPERTIES)
    writer->EndElement();
}

bool Augment::IsCompatibleWithSlot(
    const std::string& augmentType,
    bool bMatchingColourOnly,
    const std::string& selectedAugment) const
{
    // has to be in any of the augment types list to be a match
    bool compatible = false;
    std::list<std::string>::const_iterator it = m_Type.begin();
    if (selectedAugment == Name())
    {
        compatible = true;
    }
    while (it != m_Type.end())
    {
        if ((*it) == augmentType)
        {
            compatible = true;
            break;  // no need to check the rest
        }
        if (bMatchingColourOnly)
        {
            // for matching colour only, we only check the first entry in the list
            break;
        }
        ++it;
    }
    return compatible;
}

void Augment::AddImage(CImageList * pIL) const
{
    CImage image;
    LoadImageFile(
            "DataFiles\\AugmentImages\\",
            m_Icon,
            &image,
            true);
    CBitmap bitmap;
    bitmap.Attach(image.Detach());
    pIL->Add(&bitmap, c_transparentColour);  // standard mask color (purple)
}

bool Augment::operator<(const Augment & other) const
{
    // (assumes all augment names are unique)
    // sort by name
    return (Name() < other.Name());
}

std::string Augment::CompoundDescription() const
{
    std::string description;
    description = Description();
    for (auto it: EffectDescription())
    {
        description += "\r\n";
        description += it;
    }
    return description;
}

void Augment::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";
    if (HasIcon())
    {
        if (!ImageFileExists("DataFiles\\AugmentImages\\", Icon()))
        {
            ss << "Augment is missing image file \"" << Icon() << "\"\n";
            ok = false;
        }
    }
    // must have a min level or a ChooseLevel flag
    if (HasMinLevel() && HasChooseLevel())
    {
        ss << "Augment has conflicting MinLevel and ChooseLevel flags\n";
        ok = false;
    }
    if (!HasMinLevel() && !HasChooseLevel())
    {
        ss << "Augment has missing MinLevel or ChooseLevel flags\n";
        ok = false;
    }
    if (HasChooseLevel() && !HasLevelValue())
    {
        ss << "Augment has missing LevelValue for ChooseLevel flag\n";
        ok = false;
    }
    if (HasChooseLevel() && HasDualValues() && !HasLevelValue2())
    {
        ss << "Augment has missing LevelValue2 for ChooseLevel and DualValues flags\n";
        ok = false;
    }
    // check the spell effects also
    std::list<Effect>::const_iterator it = m_Effects.begin();
    while (it != m_Effects.end())
    {
        ok &= (*it).VerifyObject(&ss);
        ++it;
    }
    //if (HasRares())
    //{
        //ok &= m_Rares.VerifyObject(&ss);
    //}

    // check any set bonuses exist
    const std::list<::SetBonus> & loadedSets = SetBonuses();
    std::list<std::string>::const_iterator sbit = m_SetBonus.begin();
    while (sbit != m_SetBonus.end())
    {
        bool bFound = false;
        std::list<::SetBonus>::const_iterator sit = loadedSets.begin();
        while (!bFound && sit != loadedSets.end())
        {
            bFound = ((*sit).Type() == (*sbit));
            ++sit;
        }
        if (!bFound)
        {
            ok = false;
            ss << "Has unknown set bonus \"" << (*sbit) << "\"\n";
        }
        ++sbit;
    }

    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}
