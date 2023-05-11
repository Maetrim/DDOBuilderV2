// BaseDice.cpp
//
#include "StdAfx.h"
#include "BaseDice.h"

namespace
{
    const wchar_t f_saxElementName[] = L"BaseDice";
}

BaseDice::BaseDice() :
    BasicDice(f_saxElementName)
{
}

BaseDice::BaseDice(
        size_t version) :
    BasicDice(f_saxElementName, version)
{
}
