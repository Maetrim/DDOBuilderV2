// ActiveEffect.cpp
//
#include "stdafx.h"
#include "ActiveEffect.h"

BaseActiveEffect::BaseActiveEffect(
        const std::string & name,
        size_t stacks /* = 1 */):
    m_name(name),
    m_numStacksOEffect(stacks)
{
}

BaseActiveEffect::~BaseActiveEffect()
{
}

const std::string & BaseActiveEffect::Name() const
{
    return m_name;
}

size_t BaseActiveEffect::NumStacks() const
{
    return m_numStacksOfEffect;
}

void BaseActiveEffect::AddStack()
{
    m_numStacksOfEffect++;
}

void BaseActiveEffect::RemoveStack()
{
    m_numStacksOfEffect--;
}

void BaseActiveEffect::SetNumStacks(size_t numStacks)
{
    m_numStacksOfEffect = numStacks;
}

