// BuffFile.h
//

#pragma once
#include "XmlLib\SaxContentElement.h"
#include "XmlLib\SaxWriter.h"
#include "Buff.h"

class BuffFile :
    public XmlLib::SaxContentElement
{
    public:
        BuffFile(const std::string & filename);
        ~BuffFile(void);

        void Read();
        std::list<Buff> ItemBuffs() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Buff> m_itemBuffs;
        size_t m_loadTotal;
};

