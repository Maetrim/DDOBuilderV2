// BonusDice.cpp
//
#include "StdAfx.h"
#include "BonusDice.h"

namespace
{
    const wchar_t f_saxElementName[] = L"BonusDice";
}

BonusDice::BonusDice() :
    BasicDice(f_saxElementName)
{
}

BonusDice::BonusDice(
        size_t version) :
    BasicDice(f_saxElementName, version)
{
}
