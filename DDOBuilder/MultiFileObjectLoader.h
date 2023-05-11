// MultiFileObjectLoader.h
//

#pragma once
#include "XmlLib\SaxContentElement.h"
#include "XmlLib\SaxWriter.h"
#include "XmlLib\SaxReader.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

template <class T>
class MultiFileObjectLoader :
    public XmlLib::SaxContentElement
{
    public:
        MultiFileObjectLoader(
                const XmlLib::SaxString& rootElement,
                const std::string& path,
                const std::string& filter) :
            SaxContentElement(rootElement),
            m_path(path),
            m_filter(filter),
            m_loadTotal(0)
        {
        };
        ~MultiFileObjectLoader(void) {};

        void ReadFiles(const std::string& prompt)
        {
            m_prompt = prompt;
            GetLog().AddLogEntry(prompt.c_str());

            std::string fileFilter = m_path;
            fileFilter += m_filter;
            // read all the item files found in the Items sub-directory
            // first enumerate each file and load it
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(fileFilter.c_str(), &findFileData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                std::string fullFilename = m_path;
                fullFilename += findFileData.cFileName;
                ReadFile(fullFilename);
                while (FindNextFile(hFind, &findFileData))
                {
                    fullFilename = m_path;
                    fullFilename += findFileData.cFileName;
                    ReadFile(fullFilename);
                }
                FindClose(hFind);
            }
        };

        const std::list<T>& LoadedObjects() const
        {
            return m_loadedObjects;
        };

    protected:
        XmlLib::SaxContentElementInterface* StartElement(
                const XmlLib::SaxString& name,
                const XmlLib::SaxAttributes& attributes)
        {
            XmlLib::SaxContentElementInterface* subHandler =
                SaxContentElement::StartElement(name, attributes);
            if (subHandler == NULL)
            {
                T object;
                if (object.SaxElementIsSelf(name, attributes))
                {
                    m_loadedObjects.push_back(object);
                    subHandler = &(m_loadedObjects.back());
                    // update log during load action
                    CString strTreeCount;
                    strTreeCount.Format(
                            "%s%d",
                            m_prompt.c_str(),
                            m_loadedObjects.size());
                    GetLog().UpdateLastLogEntry(strTreeCount);
                }
            }

            return subHandler;
        };

        void EndElement()
        {
            SaxContentElement::EndElement();
        };

        bool ReadFile(const std::string& filename)
        {
            // set up a reader with this as the expected root node
            XmlLib::SaxReader reader(this, ElementName());
            // read in the xml from a file (fully qualified path)
            bool ok = reader.Open(filename);
            if (!ok)
            {
                std::string errorMessage = reader.ErrorMessage();
                // document has failed to load. Tell the user what we can about it
                CString text;
                text.Format("The document %s\n"
                    "failed to load. The XML parser reported the following problem:\n"
                    "\n", filename.c_str());
                text += errorMessage.c_str();
                AfxMessageBox(text, MB_ICONERROR);
            }
            else
            {
                m_loadedObjects.back().SetFilename(filename);
            }
            return ok;
        };

        std::string m_path;
        std::string m_filter;
        std::list<T> m_loadedObjects;
        size_t m_loadTotal;
        std::string m_prompt;
};

