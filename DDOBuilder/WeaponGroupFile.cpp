// WeaponGroupFile.cpp
//
#include "StdAfx.h"
#include "WeaponGroupFile.h"
#include "XmlLib\SaxReader.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"WeaponGroupings"; // root element name to look for
}

WeaponGroupFile::WeaponGroupFile(const std::string & filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}

WeaponGroupFile::~WeaponGroupFile(void)
{
}

void WeaponGroupFile::Read()
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

XmlLib::SaxContentElementInterface * WeaponGroupFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        WeaponGroup wg;
        if (wg.SaxElementIsSelf(name, attributes))
        {
            m_loadedGroups.push_back(wg);
            subHandler = &(m_loadedGroups.back());
            // update log during load action
            CString strFeatCount;
            strFeatCount.Format("Loading Weapon Groups...%d", m_loadedGroups.size());
            GetLog().UpdateLastLogEntry(strFeatCount);
        }
    }

    return subHandler;
}

void WeaponGroupFile::EndElement()
{
    SaxContentElement::EndElement();
}

std::list<WeaponGroup> WeaponGroupFile::WeaponGroups() const
{
    return m_loadedGroups;
}