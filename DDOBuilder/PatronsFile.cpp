// PatronsFile.cpp
//
#include "StdAfx.h"
#include "PatronsFile.h"
#include "XmlLib\SaxReader.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Patrons"; // root element name to look for
}

PatronsFile::PatronsFile(const std::string& filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}


PatronsFile::~PatronsFile(void)
{
}

void PatronsFile::Read()
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

XmlLib::SaxContentElementInterface * PatronsFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Patron q;
        if (q.SaxElementIsSelf(name, attributes))
        {
            m_loadedPatrons.push_back(q);
            subHandler = &(m_loadedPatrons.back());
        }
    }

    return subHandler;
}

void PatronsFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<Patron> & PatronsFile::Patrons() const
{
    return m_loadedPatrons;
}
