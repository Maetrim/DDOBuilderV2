// SentientGemsFile.h
//
// A class that handles the loading of the sentient gems file.
#pragma once
#include "Gem.h"

class SentientGemsFile :
    public XmlLib::SaxContentElement
{
    public:
        SentientGemsFile(const std::string& filename);
        ~SentientGemsFile(void);

        void Read();
        const std::list<Gem> & SentientGems() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Gem> m_loadedGems;
        size_t m_loadTotal;
};
