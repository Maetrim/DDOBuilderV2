#pragma once

#include <string>
#include <map>
#include <deque>

class WikiDownloader
{
    public:
        WikiDownloader();
        ~WikiDownloader();

        void Start();

    private:
        bool DownloadUrl(const std::string& url);
        void ParseDownloadedFile(const std::string& url, const std::string& goodUrlContent);
        void ParseFile(const std::string& url, const std::string& filename, const std::string& goodUrlContent);
        std::string MakeUrlFilename(const std::string& url);
        bool DirectoryExists(const std::string& dir);

        int m_pageIndex;
        std::map<std::string, int> m_knownUrls;
        std::deque<std::string> m_pagesToProcess;
        char m_path[_MAX_PATH];
};

