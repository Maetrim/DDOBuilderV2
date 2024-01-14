// GuildBuffsFile.h
//
// A class that handles the loading of the spells file.
#pragma once
#include "GuildBuff.h"

class GuildBuffsFile :
    public XmlLib::SaxContentElement
{
    public:
        GuildBuffsFile(const std::string& filename);
        ~GuildBuffsFile(void);

        void Read();
        const std::list<GuildBuff>& GuildBuffs() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<GuildBuff> m_loadedGuildBuffs;
        size_t m_loadTotal;
};
