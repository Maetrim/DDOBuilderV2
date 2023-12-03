#include "stdafx.h"
#include "WikiDownloader.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "MainFrm.h"

WikiDownloader::WikiDownloader() :
    m_pageIndex(0)
{
    // get the path location for wget
    char fullPath[MAX_PATH];
    ::GetModuleFileName(NULL, fullPath, MAX_PATH);

    char drive[_MAX_DRIVE];
    char folder[_MAX_PATH];
    _splitpath_s(fullPath, drive, _MAX_DRIVE, folder, _MAX_PATH, NULL, 0, NULL, 0);

    _makepath_s(m_path, _MAX_PATH, drive, folder, NULL, NULL);
}

WikiDownloader::~WikiDownloader()
{
}

void WikiDownloader::Start()
{
    GetLog().AddLogEntry("Starting download of ddowiki file...");
    // from the top level url, we want to get all the hrefs with "Category:" present
    // we ignore all the others. After that All the category pages are then parsed
    // looking only for links that have "page/Item" present
    m_knownUrls["https://www.ddowiki.com/page/Items"] = m_pageIndex++;
    m_pagesToProcess.push_back("https://www.ddowiki.com/page/Items");

    m_knownUrls["https://www.ddowiki.com/page/Update_61_named_items"] = m_pageIndex++;
    m_pagesToProcess.push_back("https://www.ddowiki.com/page/Update_61_named_items");

    std::string url = m_pagesToProcess.front();
    m_pagesToProcess.pop_front();
    if (!DownloadUrl(url))
    {
        GetLog().AddLogEntry("Page download failed....aborted.");
    }
    else
    {
        ParseDownloadedFile(url, "page/Category:");
    }

    while (!m_pagesToProcess.empty())
    {
        url = m_pagesToProcess.front();
        m_pagesToProcess.pop_front();
        if (!DownloadUrl(url))
        {
            GetLog().AddLogEntry("Page download failed....aborted.");
            break;
        }
        else
        {
            ParseDownloadedFile(url, "page/Item"); // ignore talk pages also , "page/Item_talk");
        }
        // allow UI to function
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            AfxGetThread()->PumpMessage();
        }
        CWnd* pWnd = AfxGetApp()->m_pMainWnd;
        CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(pWnd);
        if (pMainFrame != NULL)
        {
            CString strProgress;
            strProgress.Format("%d Urls of %d to go", m_pagesToProcess.size(), m_knownUrls.size());
            pMainFrame->m_wndStatusBar.SetPaneText(0, strProgress);
        }
    }

    GetLog().AddLogEntry("Wiki download complete");
}

bool WikiDownloader::DownloadUrl(const std::string& url)
{
    bool bSuccess = true;
    CString le;

    // only download the file if we don't already have it
    std::string fullFilename = MakeUrlFilename(url);
    CFile file;
    if (FALSE == file.Open(fullFilename.c_str(), CFile::modeRead))
    {
        le.Format("...Getting page %s", url.c_str());
        GetLog().AddLogEntry(le);

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // create a command prompt to download the file.
        // trying to run wget directly from CreateProcess can randomly fail.
        CString params;
        params += "cmd /C wget.exe -random-wait \"";
        params += url.c_str();
        params += "\"";

        char* pCommandLine = new char[params.GetLength() + 1];
        memset(pCommandLine, 0, params.GetLength() + 1);
        memcpy(pCommandLine, (LPCTSTR)params, params.GetLength());

        // Start the child process.
        if (!CreateProcess(
            NULL,                   // module name
            pCommandLine,           // Command line
            NULL,                   // Process handle not inheritable
            NULL,                   // Thread handle not inheritable
            FALSE,                  // Set handle inheritance to FALSE
            CREATE_NO_WINDOW,       // don't show console window
            NULL,                   // Use parent's environment block
            m_path,                 // Use parent's starting directory 
            &si,                    // Pointer to STARTUPINFO structure
            &pi)                    // Pointer to PROCESS_INFORMATION structure
            )
        {
            CString err;
            err.Format("CreateProcess failed (%d).\n", GetLastError());
            GetLog().AddLogEntry(err);
            delete[]pCommandLine;
            bSuccess = false;
        }
        else
        {
            // Wait until child process exits.
            WaitForSingleObject(pi.hProcess, INFINITE);
            // Close process and thread handles. 
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            delete[]pCommandLine;
        }
    }
    else
    {
        le.Format("...Processing existing page \"%s\"", url.c_str());
        GetLog().AddLogEntry(le);
    }

    return bSuccess;
}

void WikiDownloader::ParseDownloadedFile(
        const std::string& url,
        const std::string& goodUrlContent)
{
    // first we need to find the file that has just been downloaded
    // which is based off the url we used
    std::string fullFilename = MakeUrlFilename(url);
    ParseFile(fullFilename, goodUrlContent);
}

void WikiDownloader::ParseFile(
        const std::string& filename,
        const std::string& goodUrlContent)
{
    // to find urls, we load the entire file into a buffer
    CFile file;
    if (file.Open(filename.c_str(), CFile::modeRead))
    {
        ULONGLONG filesize = file.GetLength();
        char* pBuffer = new char[static_cast<unsigned int>(filesize+1)];
        memset(pBuffer, 0, static_cast<size_t>(filesize + 1));
        file.Read(pBuffer, static_cast<unsigned int>(filesize));
        file.Close();
        std::string fileContent = pBuffer;
        delete[]pBuffer;

        // now look for all href="......."
        size_t pos = fileContent.find("href=\"/", 0);
        while (pos != std::string::npos)
        {
            size_t hrefEnd = fileContent.find('"', pos + 6);    // skip "href="
            std::string url = fileContent.substr(pos + 6, hrefEnd - pos - 6);
            pos = fileContent.find("href=\"/", hrefEnd);
            // we throw out any urls which don't have good content
            if (url.find(goodUrlContent) != std::string::npos
                    && url.find("page/Item_talk") == std::string::npos)
            {
                // we want this url
                std::string fullUrl("https://www.ddowiki.com");
                fullUrl += url;
                // only add it if its not a duplicate page we already know about
                if (m_knownUrls.find(fullUrl) == m_knownUrls.end())
                {
                    // its a new url
                    m_knownUrls[fullUrl] = m_pageIndex++;
                    m_pagesToProcess.push_back(fullUrl);
                }
            }
        }
    }
    else
    {
        CString text;
        text.Format("Failed to process file \"%s\"", filename.c_str());
        GetLog().AddLogEntry(text);
    }
}

std::string WikiDownloader::MakeUrlFilename(const std::string& url)
{
    std::string fullFilename = m_path;
    std::string filepart = url.substr(url.find(':', 0) + 3);
    if (filepart == "www.ddowiki.com")
    {
        // top level node special case
        filepart += "\\index.html";
    }
    size_t pos = filepart.find(':', 0);
    while (pos != std::string::npos)
    {
        filepart.replace(pos, 1, "%3A");
        pos = filepart.find(':', 0);
    }
    pos = filepart.find("%2B", 0);
    while (pos != std::string::npos)
    {
        filepart.replace(pos, 3, "+");
        pos = filepart.find("%2B", 0);
    }
    pos = filepart.find("%26", 0);
    while (pos != std::string::npos)
    {
        filepart.replace(pos, 3, "&");
        pos = filepart.find("%26", 0);
    }
    pos = filepart.find("%27", 0);
    while (pos != std::string::npos)
    {
        filepart.replace(pos, 3, "'");
        pos = filepart.find("%27", 0);
    }
    pos = filepart.find("/", 0);
    while (pos != std::string::npos)
    {
        filepart.replace(pos, 1, "\\");
        pos = filepart.find("/", 0);
    }
    fullFilename += filepart;
    if (DirectoryExists(fullFilename))
    {
        // file will have a .1 extension
        fullFilename += ".1";
    }
    return fullFilename;
}

bool WikiDownloader::DirectoryExists(const std::string& dir)
{
    DWORD dwAttrib = GetFileAttributes(dir.c_str());

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}