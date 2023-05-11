// Destiny_SelectedTrees.cpp
//
#include "StdAfx.h"
#include "Destiny_SelectedTrees.h"
namespace
{
    const wchar_t f_saxElementName[] = L"Destiny_SelectedTrees";
}

Destiny_SelectedTrees::Destiny_SelectedTrees() :
    SelectedTrees(f_saxElementName, MAX_DESTINY_TREES + 1)
{
}
