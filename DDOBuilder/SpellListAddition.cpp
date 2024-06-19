// SpellListAddition.cpp
//
#include "StdAfx.h"
#include "SpellListAddition.h"

SpellListAddition::SpellListAddition(
        const std::string& ct,
        size_t spellLevel,
        const std::string& spellName) :
    m_class(ct),
    m_spellLevel(spellLevel),
    m_spellName(spellName),
    m_count(1)
{
}

SpellListAddition::~SpellListAddition()
{
}

bool SpellListAddition::AddsToSpellList(const std::string& ct, size_t spellLevel) const
{
    return (m_class == ct
            && m_spellLevel == spellLevel);
}

bool SpellListAddition::AddsToSpellList(const std::string& ct, const std::string& spellName) const
{
    return (m_class == ct
            && spellName == m_spellName);
}

std::string SpellListAddition::SpellName() const
{
    return m_spellName;
}

void SpellListAddition::AddReference()
{
    ++m_count;
}

bool SpellListAddition::RemoveReference()
{
    --m_count;
    // returns true if m_count == 0
    return (m_count == 0);
}
