// RaceFile.h
//
// A class that handles the loading of the race file.
#pragma once
#include "Race.h"

class RaceFile :
    public XmlLib::SaxContentElement
{
    public:
        RaceFile(const std::string & filename);
        ~RaceFile(void);

        void Read();
        const std::list<Race> & Races() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Race> m_loadedRaces;
        size_t m_loadTotal;
};
