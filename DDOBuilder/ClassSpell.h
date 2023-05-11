// ClassSpell.h
//
// An XML object wrapper that holds information on a spell
#pragma once
#include "XmlLib\DLMacros.h"

class Build;

class ClassSpell :
    public XmlLib::SaxContentElement
{
    public:
        ClassSpell(void);
        ClassSpell(int level, size_t cost, size_t mcl);
        void Write(XmlLib::SaxWriter * writer) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define ClassSpell_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_SIMPLE(_, int, Level, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, Cost, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, MaxCasterLevel, 0)

        DL_DECLARE_ACCESS(ClassSpell_PROPERTIES)
        DL_DECLARE_VARIABLES(ClassSpell_PROPERTIES)
};

class FixedSpell
{
    public:
        FixedSpell(const std::string & name, size_t level, size_t cost, int mcl) :
                m_spellName(name), m_spellLevel(level), m_cost(cost), m_mcl(mcl)
        {
        };
        ~FixedSpell() {};

        const std::string & Name() const
        {
            return m_spellName;
        }

        size_t Level() const
        {
            return m_spellLevel;
        }
        size_t Cost() const
        {
            return m_cost;
        }
        int MaxCasterLevel() const
        {
            return m_mcl;
        }
        bool operator==(const FixedSpell & other) const
        {
            return (m_spellName == other.m_spellName)
                    && (m_spellLevel == other.m_spellLevel)
                    && (m_cost == other.m_cost)
                    && (m_mcl == other.m_mcl);
        }
    private:
        std::string m_spellName;
        size_t m_spellLevel;
        size_t m_cost;
        int m_mcl;
};

