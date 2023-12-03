// QuestDifficultyTypes.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "XmlLib\SaxContentElement.h"

// Special note - The  "Unknown" entries of each enum must be entry 0 in the
// enum map as this is used by the SAX loaders to check for badly loaded values
// these enumerations are used across multiple object types

enum QuestDifficulty
{
    QD_solo = 0,
    QD_casual,
    QD_normal,
    QD_hard,
    QD_elite,
    QD_reaper1,
    QD_reaper2,
    QD_reaper3,
    QD_reaper4,
    QD_reaper5,
    QD_reaper6,
    QD_reaper7,
    QD_reaper8,
    QD_reaper9,
    QD_reaper10,

    QD_Count,
    QD_Unknown
};

const XmlLib::enumMapEntry<QuestDifficulty> questDifficultyTypeMap[] =
{
    {QD_solo,               L"Solo"},
    {QD_casual,             L"Casual"},
    {QD_normal,             L"Normal"},
    {QD_hard,               L"Hard"},
    {QD_elite,              L"Elite"},
    {QD_reaper1,            L"R1"},
    {QD_reaper2,            L"R2"},
    {QD_reaper3,            L"R3"},
    {QD_reaper4,            L"R4"},
    {QD_reaper5,            L"R5"},
    {QD_reaper6,            L"R6"},
    {QD_reaper7,            L"R7"},
    {QD_reaper8,            L"R8"},
    {QD_reaper9,            L"R9"},
    {QD_reaper10,           L"R10"},
    {QD_Unknown,            L"Unknown"},
    {QuestDifficulty(0),    NULL}
};
