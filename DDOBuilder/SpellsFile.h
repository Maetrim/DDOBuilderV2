// SpellsFile.h
//
// A class that handles the loading of the spells file.
#pragma once
#include "Spell.h"

class SpellsFile :
    public XmlLib::SaxContentElement
{
    public:
        SpellsFile(const std::string& filename, bool bItemClickies);
        ~SpellsFile(void);

        void Read();
        const std::list<Spell>& Spells() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Spell> m_loadedSpells;
        size_t m_loadTotal;
        bool m_bItemClickies;
};
