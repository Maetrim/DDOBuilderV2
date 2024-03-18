// Enhancement_SelectedTrees.cpp
//
#include "StdAfx.h"
#include "LegacyEnhancementSelectedTrees.h"
namespace
{
    const wchar_t f_saxElementName[] = L"SelectedEnhancementTrees";
}

LegacyEnhancementSelectedTrees::LegacyEnhancementSelectedTrees() :
    SelectedTrees(f_saxElementName, MAX_ENHANCEMENT_TREES)
{
}
