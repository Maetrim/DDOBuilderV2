// WeaponGroupFile.h
//

#pragma once
#include "XmlLib\SaxContentElement.h"
#include "XmlLib\SaxWriter.h"
#include "WeaponGroup.h"

class WeaponGroupFile :
    public XmlLib::SaxContentElement
{
    public:
        WeaponGroupFile(const std::string & filename);
        ~WeaponGroupFile(void);

        void Read();
        std::list<WeaponGroup> WeaponGroups() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<WeaponGroup> m_loadedGroups;
        size_t m_loadTotal;
};

