// GuildBuffsFile.cpp
//
#include "StdAfx.h"
#include "GuildBuffsFile.h"
#include "XmlLib\SaxReader.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"GuildBuffs"; // root element name to look for
}

GuildBuffsFile::GuildBuffsFile(const std::string& filename) :
    SaxContentElement(f_saxElementName),
    m_filename(filename),
    m_loadTotal(0)
{
}

GuildBuffsFile::~GuildBuffsFile(void)
{
}

void GuildBuffsFile::Read()
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

XmlLib::SaxContentElementInterface * GuildBuffsFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        GuildBuff buff;
        if (buff.SaxElementIsSelf(name, attributes))
        {
            m_loadedGuildBuffs.push_back(buff);
            subHandler = &(m_loadedGuildBuffs.back());
            // update log during load action
            CString strGuildBuffsCount;
            strGuildBuffsCount.Format("Loading Guild Buffs...%d", m_loadedGuildBuffs.size());
            GetLog().UpdateLastLogEntry(strGuildBuffsCount);
        }
    }

    return subHandler;
}

void GuildBuffsFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<GuildBuff> & GuildBuffsFile::GuildBuffs() const
{
    return m_loadedGuildBuffs;
}
