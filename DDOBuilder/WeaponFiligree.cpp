// WeaponFiligree.cpp
//
#include "StdAfx.h"
#include "WeaponFiligree.h"

#define DL_ELEMENT WeaponFiligree

namespace
{
    const wchar_t f_saxElementName[] = L"Filigree";
    const unsigned f_verCurrent = 1;
}

WeaponFiligree::WeaponFiligree() :
    TrainedFiligree(f_saxElementName, f_verCurrent)
{
}
