// BonusDice.h
//
// An XML object wrapper that holds information on enhancements trained in a specific tree
#pragma once
#include "BasicDice.h"

class BonusDice :
    public BasicDice
{
    public:
        BonusDice();
        BonusDice(size_t version);
};
