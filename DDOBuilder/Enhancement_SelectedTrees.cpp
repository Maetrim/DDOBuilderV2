// Enhancement_SelectedTrees.cpp
//
#include "StdAfx.h"
#include "Enhancement_SelectedTrees.h"
namespace
{
    const wchar_t f_saxElementName[] = L"Enhancement_SelectedTrees";
}

Enhancement_SelectedTrees::Enhancement_SelectedTrees() :
    SelectedTrees(f_saxElementName, MAX_ENHANCEMENT_TREES)
{
}
