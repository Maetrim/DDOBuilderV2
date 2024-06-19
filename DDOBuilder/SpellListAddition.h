// SpellListAddition.h
//
#pragma once

#include <string>

class SpellListAddition
{
    public:
        SpellListAddition(const std::string& ct, size_t spellLevel, const std::string& spellName);
        ~SpellListAddition();

        bool AddsToSpellList(const std::string& ct, size_t spellLevel) const;
        bool AddsToSpellList(const std::string& ct, const std::string& spellName) const;
        std::string SpellName() const;
        void AddReference();
        bool RemoveReference(); // returns true if m_count == 0
    private:
        std::string m_class;
        size_t m_spellLevel;
        std::string m_spellName;
        size_t m_count;
};
