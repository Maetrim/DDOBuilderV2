// IgnoredListFile.cpp
//
#include "StdAfx.h"
#include "IgnoredListFile.h"
#include "XmlLib\SaxReader.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"IgnoredList"; // root element name to look for
}

IgnoredListFile::IgnoredListFile(const std::string & filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}

IgnoredListFile::~IgnoredListFile(void)
{
}

void IgnoredListFile::Read()
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

XmlLib::SaxContentElementInterface * IgnoredListFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        std::string featName;
        if (name == L"Ignored")
        {
            m_ignoredItems.push_back(featName);
            subHandler = HandleSimpleElement(&(m_ignoredItems.back()));
        }
    }

    return subHandler;
}

void IgnoredListFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<std::string> & IgnoredListFile::IgnoredItems()
{
    return m_ignoredItems;
}

void IgnoredListFile::Save(std::list<std::string> & featList)
{
    // load all the global data required by the program
    // all data files are in the same directory as the executable
    std::string folderPath = DataFolder();
    std::string filename = folderPath;
    filename += "IgnoredList.xml";

    try
    {
        XmlLib::SaxWriter writer;
        writer.Open(filename);
        writer.StartDocument(f_saxElementName);
        std::list<std::string>::const_iterator it = featList.begin();
        while (it != featList.end())
        {
            writer.WriteSimpleElement(L"Ignored", (*it));
            ++it;
        }
        writer.EndDocument();
    }
    catch (const std::exception & e)
    {
        std::string errorMessage = e.what();
        // document has failed to save. Tell the user what we can about it
        CString text;
        text.Format("The document %s\n"
                "failed to save. The XML parser reported the following problem:\n"
                "\n", filename.c_str());
        text += errorMessage.c_str();
        AfxMessageBox(text, MB_ICONERROR);
    }
}

