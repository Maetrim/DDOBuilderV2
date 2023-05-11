// StancesFile.cpp
//
#include "StdAfx.h"
#include "StancesFile.h"
#include "XmlLib\SaxReader.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Stances"; // root element name to look for
}

StancesFile::StancesFile(const std::string& filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}


StancesFile::~StancesFile(void)
{
}

void StancesFile::Read()
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

XmlLib::SaxContentElementInterface * StancesFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Stance stance;
        if (stance.SaxElementIsSelf(name, attributes))
        {
            m_loadedStances.push_back(stance);
            subHandler = &(m_loadedStances.back());
            // update log during load action
            CString strStanceCount;
            strStanceCount.Format("Loading Stances...%d", m_loadedStances.size());
            GetLog().UpdateLastLogEntry(strStanceCount);
        }
    }

    return subHandler;
}

void StancesFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<Stance> & StancesFile::Stances() const
{
    return m_loadedStances;
}
