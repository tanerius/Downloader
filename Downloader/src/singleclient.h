#pragma once
#include "Common.h"
#include <curl/curl.h>

namespace DownloaderLib
{
    /**
     HTTP Client for single requests
     */
    class SingleClient
    {
    public:
        SingleClient();
        SingleClient(std::string agent);
        virtual ~SingleClient();

        std::string get(std::string url);
        std::string post(const std::string& url, const std::string& data);
        /**
         Download url into folder
         */
        bool download(std::string url, const std::string folder);
        /**
         Download url and rename it to satisfy filepath
         */
        bool downloadAs(std::string url, std::string filepath);

    private:
        static size_t writeToFile(void* ptr, size_t size, size_t nmemb, FILE* stream);
        static size_t writeToString(char* ptr, size_t size, size_t nmemb, std::string* sp);
        std::string genRandomString(const int len);

        void initCURL();

    private:
        CURL* m_curl;
    };
}
