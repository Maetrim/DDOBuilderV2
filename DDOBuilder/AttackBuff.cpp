// AttackBuff.cpp
//
#include "StdAfx.h"
#include "AttackBuff.h"

AttackBuff::AttackBuff(enum class BuffType bt, double value, double startTime, double duration) :
    m_eType(bt),
    m_value(value),
    m_startTime(startTime),
    m_duration(duration)
{
}

AttackBuff::~AttackBuff()
{
}

bool AttackBuff::Expired(double timePoint) const
{
    bool expired = ((m_startTime + m_duration) < timePoint);
    return expired;
}
