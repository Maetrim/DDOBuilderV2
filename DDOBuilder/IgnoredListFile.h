// IgnoredListFile.h
//

#pragma once
#include "XmlLib\SaxContentElement.h"
#include "XmlLib\SaxWriter.h"

class IgnoredListFile :
    public XmlLib::SaxContentElement
{
    public:
        IgnoredListFile(const std::string & filename);
        ~IgnoredListFile(void);

        void Read();
        const std::list<std::string> & IgnoredItems();
        void Save(std::list<std::string> & ignoredList);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<std::string> m_ignoredItems;
        size_t m_loadTotal;
};

