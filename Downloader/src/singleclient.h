#pragma once
#include "./common.h"
#include <curl/curl.h>
#include <vector>
#include <string>

namespace EZResume
{
    struct MemoryStruct {
        char* memory;
        size_t size;
        curl_off_t totalSize;
        IDownloaderHandler* callback = nullptr;
        int id = -1;
    };

    struct SFileMetaData
    {
        curl_off_t totalSize = 0;
        curl_off_t totalChunks = 0;
        size_t chunkSize = 0;
        size_t lastDownloadedChunk = 0;
        size_t checkCode = 2308075;
        size_t currentSavedOffset = 0; /* Only used in ranged downloads */
        char etag[50] = "na"; /* etag string should not be more than 50 chars long */
    };

    /**
     HTTP Client for single requests - runs in one thread
     */
    class SingleClient
    {
    public:
        struct ResourceStatus
        {
            bool IsValidated = false;
            long ResponseCode = 0;
            bool CanAcceptRanges = false;
            curl_off_t ContentLength = -1;
            std::string URL;
            std::vector<std::string> Headers;
            std::string ETag;
        };
        SingleClient() = delete;
        SingleClient(const SingleClient&) = delete;
        SingleClient(const int id);
        SingleClient(const int id, size_t chunkSize, const char *agent);
        virtual ~SingleClient();
        void SetChunkSize(size_t s);
        void SetUserAgent(const char* ua);
        unsigned long GetChunkSize() const { return static_cast<unsigned long>(m_chunkSize); }
        void GetUserAgent(char* useragent, unsigned int& size);
        void SetConfiguration(EZResume::Configutation c) { m_conf = c; }
        EZResume::Configutation GetConfiguration() { return m_conf; }

        /**
         Download url and rename it to satisfy filepath
         */
        DownloadResult Download(
            const char* url,
            const char* filepath,
            IDownloaderHandler* cbh = nullptr
            );

        char* GetVersion() const;

    private:
        enum FileMetaStatus
        {
            S_CONTINUING,
            S_CREATING,
            S_ERROR
        };

        static size_t WriteToFile(void *ptr, size_t size, size_t nmemb, FILE *stream);
        static size_t WriteToString(void *ptr, size_t size, size_t nmemb, std::string &sp);
        static size_t WriteToMemory(char* ptr, size_t size, size_t nmemb, void* userdata);
        static size_t CurlHeaderCallback(char *buffer,
                                         size_t size,
                                         size_t nitems,
                                         ResourceStatus *userdata);

        void InitCURL();
        DownloadResult ValidateResource(const char* url);
        void CleanString(std::string &);
        DownloadResult ProcessResultAndCleanup(const DownloadResult result, IDownloaderHandler* cbh, const char* msg);

        // HTTP Statuses
        long GetLastResponseCode() const;
        std::string GetAttribute(const char *attr);
        std::string GetETag();
        std::string GetAcceptRangesValue();
        std::string GetLastModified();
        std::string GetContentLength();
        void PopulateResourceMetadata(const CURLcode cc);
        void ResetMemory(MemoryStruct& m);
        void MakeStringLower(std::string& str);

        DownloadResult ReadMetaFile(SFileMetaData &md, const char *filename);
        DownloadResult CreateSparseFile(std::ofstream& ofs, const char *filePath, const SFileMetaData &fileMeta, const bool includeMeta);
        DownloadResult WriteChunkData(
            std::ofstream& fs,
            const MemoryStruct& downloadedData,
            SFileMetaData& md,
            bool isEof);

        void SetMetaDataDefaults(SFileMetaData& md);

    private:
        CURL *m_curl;
        size_t m_chunkSize = 4194304; // 4 MB
        bool m_isProperlyInitialized = false;
        SingleClient::ResourceStatus *m_resourceStatus = nullptr;
        std::string m_userAgent;
        EZResume::Configutation m_conf;
        int m_id = -1;
    };
}
