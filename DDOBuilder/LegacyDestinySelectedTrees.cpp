// LegacyDestinySelectedTrees.cpp
//
#include "StdAfx.h"
#include "LegacyDestinySelectedTrees.h"
namespace
{
    const wchar_t f_saxElementName[] = L"SelectedDestinyTrees";
}

LegacyDestinySelectedTrees::LegacyDestinySelectedTrees() :
    SelectedTrees(f_saxElementName, MAX_DESTINY_TREES + 1)
{
}
