// LegacySentientJewel.cpp
//
#include "StdAfx.h"
#include "LegacySentientJewel.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT LegacySentientJewel

namespace
{
    const wchar_t f_saxElementName[] = L"SentientJewel";
    DL_DEFINE_NAMES(SentientJewel_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

LegacySentientJewel::LegacySentientJewel() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(SentientJewel_PROPERTIES)
    // make sure we have the default artifact filigree items
    while (m_ArtifactFiligrees.size() < MAX_ARTIFACT_FILIGREE)
    {
        m_ArtifactFiligrees.push_back(ArtifactFiligree());
    }
}

DL_DEFINE_ACCESS(SentientJewel_PROPERTIES)

XmlLib::SaxContentElementInterface * LegacySentientJewel::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SentientJewel_PROPERTIES)

    return subHandler;
}

void LegacySentientJewel::EndElement()
{
    // backwards compatibility, convert all old Filigreex/RareFiligreex to
    // Filigree objects and then remove them
    bool bConverted = false;
    m_hasSentientSpark = false;     // just clear it
    for (size_t i = 0; i < 8; ++i)  // used to be max 8 filigrees
    {
        if (Deprecated_HasFiligree(i))
        {
            WeaponFiligree f;
            f.Set_Name(Deprecated_Filigree(i));
            if (Deprecated_IsRareFiligree(i))
            {
                f.Set_Rare();
            }
            m_Filigrees.push_back(f);
            bConverted = true;
            Deprecated_ClearFiligree(i);
        }
    }
    if (bConverted)
    {
        m_NumFiligrees = m_Filigrees.size();
        m_hasNumFiligrees = true;
    }
    SaxContentElement::EndElement();
    // make sure we have the right amount of artifact filigree objects
    while (m_ArtifactFiligrees.size() > MAX_ARTIFACT_FILIGREE)
    {
        m_ArtifactFiligrees.pop_front();    // oldest are the ones not loaded
    }
    while (m_ArtifactFiligrees.size() < MAX_ARTIFACT_FILIGREE)
    {
        m_ArtifactFiligrees.push_back(ArtifactFiligree());
    }

    DL_END(SentientJewel_PROPERTIES)
}

void LegacySentientJewel::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SentientJewel_PROPERTIES)
    writer->EndElement();
}

std::string LegacySentientJewel::GetFiligree(size_t fi) const
{
    std::string name;
    if (fi < m_Filigrees.size())
    {
        std::list<WeaponFiligree>::const_iterator it = m_Filigrees.begin();
        std::advance(it, fi);
        name = (*it).Name();
    }
    return name;
}

bool LegacySentientJewel::IsRareFiligree(size_t fi) const
{
    bool bRare = false;
    if (fi < m_Filigrees.size())
    {
        std::list<WeaponFiligree>::const_iterator it = m_Filigrees.begin();
        std::advance(it, fi);
        bRare = (*it).HasRare();
    }
    return bRare;
}

void LegacySentientJewel::SetNumFiligrees(size_t count)
{
    // ensure we do not have too many filigrees setup
    m_NumFiligrees = count;
    // remove any extra filigrees we no longer have space for
    while (m_Filigrees.size() > count)
    {
        m_Filigrees.pop_back();
    }
    // add any required blank filigrees
    while (m_Filigrees.size() < count)
    {
        m_Filigrees.push_back(WeaponFiligree());  // add a blank filigree
    }
}

void LegacySentientJewel::SetPersonality(const std::string & name)
{
    Set_Personality(name);
}

void LegacySentientJewel::SetFiligree(size_t fi, const std::string & name)
{
    if (fi < m_NumFiligrees)
    {
        std::list<WeaponFiligree>::iterator it = m_Filigrees.begin();
        std::advance(it, fi);
        (*it).Set_Name(name);
    }
}

void LegacySentientJewel::SetFiligreeRare(size_t fi, bool isRare)
{
    if (fi < m_NumFiligrees)
    {
        std::list<WeaponFiligree>::iterator it = m_Filigrees.begin();
        std::advance(it, fi);
        if (isRare)
        {
            (*it).Set_Rare();
        }
        else
        {
            (*it).Clear_Rare();
        }
    }
}

std::string LegacySentientJewel::GetArtifactFiligree(size_t fi) const
{
    std::string name;
    if (fi < m_ArtifactFiligrees.size())
    {
        std::list<ArtifactFiligree>::const_iterator it = m_ArtifactFiligrees.begin();
        std::advance(it, fi);
        name = (*it).Name();
    }
    return name;
}

bool LegacySentientJewel::IsRareArtifactFiligree(size_t fi) const
{
    bool bRare = false;
    if (fi < m_ArtifactFiligrees.size())
    {
        std::list<ArtifactFiligree>::const_iterator it = m_ArtifactFiligrees.begin();
        std::advance(it, fi);
        bRare = (*it).HasRare();
    }
    return bRare;
}

void LegacySentientJewel::SetArtifactFiligree(size_t fi, const std::string & name)
{
    if (fi < MAX_ARTIFACT_FILIGREE)
    {
        std::list<ArtifactFiligree>::iterator it = m_ArtifactFiligrees.begin();
        std::advance(it, fi);
        (*it).Set_Name(name);
    }
}

void LegacySentientJewel::SetArtifactFiligreeRare(size_t fi, bool isRare)
{
    if (fi < MAX_ARTIFACT_FILIGREE)
    {
        std::list<ArtifactFiligree>::iterator it = m_ArtifactFiligrees.begin();
        std::advance(it, fi);
        if (isRare)
        {
            (*it).Set_Rare();
        }
        else
        {
            (*it).Clear_Rare();
        }
    }
}

bool LegacySentientJewel::Deprecated_HasFiligree(size_t fi) const
{
    bool bHas = false;
    switch (fi)
    {
    case 0: bHas = HasFiligree1(); break;
    case 1: bHas = HasFiligree2(); break;
    case 2: bHas = HasFiligree3(); break;
    case 3: bHas = HasFiligree4(); break;
    case 4: bHas = HasFiligree5(); break;
    case 5: bHas = HasFiligree6(); break;
    case 6: bHas = HasFiligree7(); break;
    case 7: bHas = HasFiligree8(); break;
    }
    return bHas;
}

std::string LegacySentientJewel::Deprecated_Filigree(size_t fi) const
{
    std::string filigree;       // return "" if no filigree
    switch (fi)
    {
    case 0: if (HasFiligree1()) filigree = Filigree1(); break;
    case 1: if (HasFiligree2()) filigree = Filigree2(); break;
    case 2: if (HasFiligree3()) filigree = Filigree3(); break;
    case 3: if (HasFiligree4()) filigree = Filigree4(); break;
    case 4: if (HasFiligree5()) filigree = Filigree5(); break;
    case 5: if (HasFiligree6()) filigree = Filigree6(); break;
    case 6: if (HasFiligree7()) filigree = Filigree7(); break;
    case 7: if (HasFiligree8()) filigree = Filigree8(); break;
    }
    return filigree;
}

bool LegacySentientJewel::Deprecated_IsRareFiligree(size_t fi) const
{
    bool isRare = false;
    switch (fi)
    {
    case 0: isRare = HasRareFiligree1(); break;
    case 1: isRare = HasRareFiligree2(); break;
    case 2: isRare = HasRareFiligree3(); break;
    case 3: isRare = HasRareFiligree4(); break;
    case 4: isRare = HasRareFiligree5(); break;
    case 5: isRare = HasRareFiligree6(); break;
    case 6: isRare = HasRareFiligree7(); break;
    case 7: isRare = HasRareFiligree8(); break;
    }
    return isRare;
}

void LegacySentientJewel::Deprecated_ClearFiligree(size_t fi)
{
    switch (fi)
    {
    case 0: Clear_Filigree1(); break;
    case 1: Clear_Filigree2(); break;
    case 2: Clear_Filigree3(); break;
    case 3: Clear_Filigree4(); break;
    case 4: Clear_Filigree5(); break;
    case 5: Clear_Filigree6(); break;
    case 6: Clear_Filigree7(); break;
    case 7: Clear_Filigree8(); break;
    }
}
