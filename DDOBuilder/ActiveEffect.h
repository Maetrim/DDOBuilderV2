// ActiveEffect.h
//
// these are the effects in the list for any given breakdown
#pragma once

// common parts for all active effects
class BaseActiveEffect
{
public:
    BaseActiveEffect(const std::string & name, size_t stacks = 1);
    virtual ~BaseActiveEffect();

    const std::string & Name() const;
    size_t NumStacks() const;
    void AddStack();
    void RemoveStack();
    void SetNumStacks(size_t numStacks);

private:
    std::string m_name;
    size_t m_numStacksOfEffect;
};