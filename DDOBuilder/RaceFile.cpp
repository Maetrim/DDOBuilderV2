// RaceFile.cpp
//
#include "StdAfx.h"
#include "RaceFile.h"
#include "XmlLib\SaxReader.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Races"; // root element name to look for
}

RaceFile::RaceFile(const std::string & filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}


RaceFile::~RaceFile(void)
{
}

void RaceFile::Read()
{
    // set up a reader with this as the expected root node
    XmlLib::SaxReader reader(this, f_saxElementName);
    // read in the xml from a file (fully qualified path)
    bool ok = reader.Open(m_filename);
    if (!ok)
    {
        std::string errorMessage = reader.ErrorMessage();
        // document has failed to load. Tell the user what we can about it
        CString text;
        text.Format("The document %s\n"
                "failed to load. The XML parser reported the following problem:\n"
                "\n", m_filename.c_str());
        text += errorMessage.c_str();
        AfxMessageBox(text, MB_ICONERROR);
    }
}

XmlLib::SaxContentElementInterface * RaceFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Race race;
        if (race.SaxElementIsSelf(name, attributes))
        {
            m_loadedRaces.push_back(race);
            subHandler = &(m_loadedRaces.back());
            // update log during load action
            CString strRaceCount;
            strRaceCount.Format("Loading Races...%d", m_loadedRaces.size());
            GetLog().UpdateLastLogEntry(strRaceCount);
        }
    }

    return subHandler;
}

void RaceFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<Race> & RaceFile::Races() const
{
    return m_loadedRaces;
}
