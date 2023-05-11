// ClassFile.h
//
// A class that handles the loading of the race file.
#pragma once
#include "Class.h"

class ClassFile :
    public XmlLib::SaxContentElement
{
    public:
        ClassFile(const std::string & filename);
        ~ClassFile(void);

        void Read();
        const std::list<Class> & Classes() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Class> m_loadedClasses;
        size_t m_loadTotal;
};
