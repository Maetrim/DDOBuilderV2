// AttackBuff.h
//
// Tracks a specific buff applied to an attack chain
// not saved as generated on the fly
#pragma once
#include "Dice.h"

#include "BreakdownTypes.h"

enum class BuffType
{
    BT_vulnerability,
};

class AttackBuff
{
    public:
        AttackBuff(enum class BuffType bt, double value, double startTime, double duration);
        ~AttackBuff();

        bool Expired(double timePoint) const;

    private:
        enum class BuffType m_eType;
        double m_value;
        double m_startTime;
        double m_duration;
};
