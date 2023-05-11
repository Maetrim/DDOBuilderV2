// StancesFile.h
//
// A class that handles the loading of the race file.
#pragma once
#include "Stance.h"

class StancesFile :
    public XmlLib::SaxContentElement
{
    public:
        StancesFile(const std::string& filename);
        ~StancesFile(void);

        void Read();
        const std::list<Stance> & Stances() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Stance> m_loadedStances;
        size_t m_loadTotal;
};
