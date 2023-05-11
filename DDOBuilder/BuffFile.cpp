// BuffFile.cpp
//
#include "StdAfx.h"
#include "BuffFile.h"
#include "XmlLib\SaxReader.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Buffs"; // root element name to look for
}

BuffFile::BuffFile(const std::string & filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}

BuffFile::~BuffFile(void)
{
}

void BuffFile::Read()
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

XmlLib::SaxContentElementInterface * BuffFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Buff b;
        if (b.SaxElementIsSelf(name, attributes))
        {
            m_itemBuffs.push_back(b);
            subHandler = &(m_itemBuffs.back());
            // update log during load action
            CString strFeatCount;
            strFeatCount.Format("Loading Item Buffs...%d", m_itemBuffs.size());
            GetLog().UpdateLastLogEntry(strFeatCount);
        }
    }

    return subHandler;
}

void BuffFile::EndElement()
{
    SaxContentElement::EndElement();
}

std::list<Buff> BuffFile::ItemBuffs() const
{
    return m_itemBuffs;
}