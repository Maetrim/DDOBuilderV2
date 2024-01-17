// QuestsFile.cpp
//
#include "StdAfx.h"
#include "QuestsFile.h"
#include "XmlLib\SaxReader.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Quests"; // root element name to look for
}

QuestsFile::QuestsFile(const std::string& filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}


QuestsFile::~QuestsFile(void)
{
}

void QuestsFile::Read()
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

XmlLib::SaxContentElementInterface * QuestsFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Quest q;
        if (q.SaxElementIsSelf(name, attributes))
        {
            m_loadedQuests.push_back(q);
            subHandler = &(m_loadedQuests.back());
        }
    }

    return subHandler;
}

void QuestsFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<Quest> & QuestsFile::Quests() const
{
    return m_loadedQuests;
}
