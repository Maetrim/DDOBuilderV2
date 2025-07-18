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
    // setup the default tree selections
    SetTree(0, "Dire Thaumaturge");
    SetTree(1, "Dread Adversary");
    SetTree(2, "Grim Barricade");
}
