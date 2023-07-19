// PatronsFile.h
//
// A class that handles the loading of the race file.
#pragma once
#include "Patron.h"

class PatronsFile :
    public XmlLib::SaxContentElement
{
    public:
        PatronsFile(const std::string& filename);
        ~PatronsFile(void);

        void Read();
        const std::list<Patron> & Patrons() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Patron> m_loadedPatrons;
        size_t m_loadTotal;
};
