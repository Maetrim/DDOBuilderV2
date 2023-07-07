// QuestsFile.h
//
// A class that handles the loading of the race file.
#pragma once
#include "Quest.h"

class QuestsFile :
    public XmlLib::SaxContentElement
{
    public:
        QuestsFile(const std::string& filename);
        ~QuestsFile(void);

        void Read();
        const std::list<Quest> & Quests() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Quest> m_loadedQuests;
        size_t m_loadTotal;
};
