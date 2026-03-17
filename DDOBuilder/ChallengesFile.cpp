// ChallengesFile.cpp
//
#include "StdAfx.h"
#include "ChallengesFile.h"
#include "XmlLib\SaxReader.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Challenges"; // root element name to look for
}

ChallengesFile::ChallengesFile(const std::string& filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}


ChallengesFile::~ChallengesFile(void)
{
}

void ChallengesFile::Read()
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

XmlLib::SaxContentElementInterface* ChallengesFile::StartElement(
        const XmlLib::SaxString& name,
        const XmlLib::SaxAttributes& attributes)
{
    XmlLib::SaxContentElementInterface* subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Challenge c;
        if (c.SaxElementIsSelf(name, attributes))
        {
            m_loadedChallenges.push_back(c);
            subHandler = &(m_loadedChallenges.back());
        }
    }

    return subHandler;
}

void ChallengesFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<Challenge>& ChallengesFile::Challenges() const
{
    return m_loadedChallenges;
}
