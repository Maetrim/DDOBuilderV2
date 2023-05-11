// AutomaticAcquisition.cpp
//
#include "StdAfx.h"
#include "AutomaticAcquisition.h"

namespace
{
    const wchar_t f_saxElementName[] = L"AutomaticAcquisition";
    const unsigned f_verCurrent = 1;
}

AutomaticAcquisition::AutomaticAcquisition() :
    RequirementsBase(f_saxElementName, f_verCurrent)
{
}
