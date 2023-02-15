#pragma once
#include "Common.h"
#include <curl/curl.h>
#include <vector>
#include <string>

namespace DownloaderLib
{
    struct SFileMetaData
    {
        size_t chunkSize;
        size_t totalChunks;
        size_t currentChunk;
    };

    /**
     HTTP Client for single requests - runs in one thread
     */
    class SingleClient
    {
    public:

        struct ResourceStatus {
            bool IsInitialized = false;
            long ResponseCode = 0;
            std::vector<std::string> Headers;
        };

        SingleClient();
        SingleClient(size_t chunkSize, const char* agent);
        virtual ~SingleClient();
        void SetChunkSize(size_t s);

        /**
         Download url and rename it to satisfy filepath
         */
        void download(
            const char* url, 
            const char* filepath, 
            void (*funcCompleted)(int, const char*),
            int (*funcProgress)(void*, double, double, double, double) = nullptr
            );

    private:
        enum FileMetaStatus
        {
            S_CONTINUING,
            S_CREATING,
            S_ERROR
        };

        static size_t writeToFile(void* ptr, size_t size, size_t nmemb, FILE* stream);
        static size_t writeToString(char* ptr, size_t size, size_t nmemb, std::string& sp);
        std::string genRandomString(const int len);
        const bool createSparseFile(const char* filePath, const unsigned long fileSize);
        ResourceStatus* validateResource(const char* url);
        static size_t CurlHeaderCallback(char* buffer,
            size_t size,
            size_t nitems,
            ResourceStatus* userdata);

        void initCURL();

        void CleanString(std::string&);

        // HTTP Statuses
        const long GetLastResponseCode() const;
        std::string GetAttribute(const char* attr);
        std::string GetETag();
        std::string GetAcceptRangesValue();
         
    private:
        CURL* m_curl;
        //curl_off_t m_lastruntime;
        size_t m_chunkSize = 4194304; // 4 MB
        bool m_isProperlyInitialized = false;
        SingleClient::ResourceStatus* m_resourceStatus = nullptr;
    };
}
