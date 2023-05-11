// ClassFile.cpp
//
#include "StdAfx.h"
#include "ClassFile.h"
#include "XmlLib\SaxReader.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Classes"; // root element name to look for
}

ClassFile::ClassFile(const std::string & filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}


ClassFile::~ClassFile(void)
{
}

void ClassFile::Read()
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

XmlLib::SaxContentElementInterface * ClassFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Class race(m_loadedClasses.size());
        if (race.SaxElementIsSelf(name, attributes))
        {
            m_loadedClasses.push_back(race);
            subHandler = &(m_loadedClasses.back());
            // update log during load action
            CString strClassCount;
            strClassCount.Format("Loading Classes...%d", m_loadedClasses.size());
            GetLog().UpdateLastLogEntry(strClassCount);
        }
    }

    return subHandler;
}

void ClassFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<Class> & ClassFile::Classes() const
{
    return m_loadedClasses;
}
