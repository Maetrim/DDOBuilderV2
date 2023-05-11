// StackTracking.cpp
//
#include "stdafx.h"
#include "StackTracking.h"

StackTracking::StackTracking(
        const std::string& name) :
    m_name(name),
    m_stacks(0)
{
}

StackTracking::~StackTracking()
{
}

const std::string& StackTracking::Name() const
{
    return m_name;
}

size_t StackTracking::Stacks() const
{
    return m_stacks;
}

void StackTracking::AddStack()
{
    ++m_stacks;
}

void StackTracking::RevokeStack()
{
    --m_stacks;
}
