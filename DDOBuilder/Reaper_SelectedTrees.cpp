// Reaper_SelectedTrees.cpp
//
#include "StdAfx.h"
#include "Reaper_SelectedTrees.h"
namespace
{
    const wchar_t f_saxElementName[] = L"Reaper_SelectedTrees";
}

Reaper_SelectedTrees::Reaper_SelectedTrees() :
    SelectedTrees(f_saxElementName, MAX_REAPER_TREES)
{
}
