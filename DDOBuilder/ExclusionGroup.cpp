// ExclusionGroup.cpp
//
#include "stdafx.h"
#include "ExclusionGroup.h"

ExclusionGroup::ExclusionGroup(
        const std::string& name,
        const std::string& enhancementId) :
    m_name(name),
    m_enhancementId(enhancementId),
    m_count(0)
{
}

ExclusionGroup::~ExclusionGroup()
{
}

const std::string& ExclusionGroup::Name() const
{
    return m_name;
}

const std::string& ExclusionGroup::EnhancementId() const
{
    return m_enhancementId;
}

void ExclusionGroup::AddStack()
{
    ++m_count;
}

bool ExclusionGroup::RevokeStack()
{
    --m_count;
    return (m_count == 0);
}
