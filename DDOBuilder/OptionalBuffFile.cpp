// OptionalBuffFile.cpp
//
#include "StdAfx.h"
#include "OptionalBuffFile.h"
#include "XmlLib\SaxReader.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"SelfAndPartyBuffs"; // root element name to look for
}

OptionalBuffFile::OptionalBuffFile(const std::string & filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}

OptionalBuffFile::~OptionalBuffFile(void)
{
}

void OptionalBuffFile::Read()
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

XmlLib::SaxContentElementInterface * OptionalBuffFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        OptionalBuff b;
        if (b.SaxElementIsSelf(name, attributes))
        {
            m_optionalBuffs.push_back(b);
            subHandler = &(m_optionalBuffs.back());
        }
    }

    return subHandler;
}

void OptionalBuffFile::EndElement()
{
    SaxContentElement::EndElement();
}

std::list<OptionalBuff> OptionalBuffFile::OptionalBuffs() const
{
    return m_optionalBuffs;
}