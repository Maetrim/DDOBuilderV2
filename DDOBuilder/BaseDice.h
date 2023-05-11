// BaseDice.h
//
// An XML object wrapper that holds information on enhancements trained in a specific tree
#pragma once
#include "BasicDice.h"

class BaseDice :
    public BasicDice
{
    public:
        BaseDice();
        BaseDice(size_t version);
};
