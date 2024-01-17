// SentientGemsFile.cpp
//
#include "StdAfx.h"
#include "SentientGemsFile.h"
#include "XmlLib\SaxReader.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"SentientGems"; // root element name to look for
}

SentientGemsFile::SentientGemsFile(const std::string& filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}


SentientGemsFile::~SentientGemsFile(void)
{
}

void SentientGemsFile::Read()
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

XmlLib::SaxContentElementInterface * SentientGemsFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Gem gem;
        if (gem.SaxElementIsSelf(name, attributes))
        {
            m_loadedGems.push_back(gem);
            subHandler = &(m_loadedGems.back());
        }
    }

    return subHandler;
}

void SentientGemsFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<Gem> & SentientGemsFile::SentientGems() const
{
    return m_loadedGems;
}
