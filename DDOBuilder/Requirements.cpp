// Requirements.cpp
//
#include "StdAfx.h"
#include "Requirements.h"

#define DL_ELEMENT Requirements

namespace
{
    const wchar_t f_saxElementName[] = L"Requirements";
    const unsigned f_verCurrent = 1;
}

Requirements::Requirements() :
    RequirementsBase(f_saxElementName, f_verCurrent)
{
}
