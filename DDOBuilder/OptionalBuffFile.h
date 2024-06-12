// OptionalBuffFile.h
//

#pragma once
#include "XmlLib\SaxContentElement.h"
#include "XmlLib\SaxWriter.h"
#include "OptionalBuff.h"

class OptionalBuffFile :
    public XmlLib::SaxContentElement
{
    public:
        OptionalBuffFile(const std::string & filename);
        ~OptionalBuffFile(void);

        void Read();
        std::list<OptionalBuff> OptionalBuffs() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<OptionalBuff> m_optionalBuffs;
        size_t m_loadTotal;
};

