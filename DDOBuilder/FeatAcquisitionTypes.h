// FeatAcquisitionTypes.h
//
#pragma once

#include "XmlLib\SaxContentElement.h"

// Special note - The  "Unknown" entries of each enum must be entry 0 in the
// enum map as this is used by the SAX loaders to check for badly loaded values
// these enumerations are used across multiple object types
enum FeatAcquisitionType
{
    FeatAcquisition_Unknown = 0,
    FeatAcquisition_Automatic,
    FeatAcquisition_HeroicPastLife,
    FeatAcquisition_RacialPastLife,
    FeatAcquisition_IconicPastLife,
    FeatAcquisition_EpicPastLife,
    FeatAcquisition_Special,
    FeatAcquisition_UniversalTree,
    FeatAcquisition_EpicDestinyTree,
    FeatAcquisition_Favor,
    FeatAcquisition_Train,
};
const XmlLib::enumMapEntry<FeatAcquisitionType> featAcquisitionMap[] =
{
    {FeatAcquisition_Unknown,           L"Unknown"},
    {FeatAcquisition_Automatic,         L"Automatic"},
    {FeatAcquisition_HeroicPastLife,    L"HeroicPastLife"},
    {FeatAcquisition_RacialPastLife,    L"RacialPastLife"},
    {FeatAcquisition_IconicPastLife,    L"IconicPastLife"},
    {FeatAcquisition_EpicPastLife,      L"EpicPastLife"},
    {FeatAcquisition_Special,           L"Special"},
    {FeatAcquisition_UniversalTree,     L"UniversalTree"},
    {FeatAcquisition_EpicDestinyTree,   L"EpicDestinyTree"},
    {FeatAcquisition_Favor,             L"Favor"},
    {FeatAcquisition_Train,             L"Train"},
    {FeatAcquisitionType(0), NULL}
};

