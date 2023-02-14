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
        SingleClient(const char* agent);
        virtual ~SingleClient();

        std::string get(const char* url);
        std::string post(const char* url, const char* data);

        /**
         Download url and rename it to satisfy filepath
         */
        void download(const char* url, const char* filepath, void (*func)(int, const char*), 
            std::mutex* callbackMutex);

    private:
        static size_t writeToFile(void* ptr, size_t size, size_t nmemb, FILE* stream);
        static size_t writeToString(char* ptr, size_t size, size_t nmemb, std::string* sp);
        std::string genRandomString(const int len);

        void initCURL();

    private:
        CURL* m_curl;
        curl_off_t m_lastruntime;
    };
}
