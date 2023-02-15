#pragma once
#include "common.h"

namespace DownloaderLib {

    class Downloader
    {
    public:
        Downloader();
        virtual ~Downloader();

        /**
         Download a file from url.
         @return: task ID. If failed, return -1
         */
        int download(const char* url, const char* filePath, void (*func)(int, const char*));
        void safeDecrement();
        std::mutex m_threadMtx;
        std::mutex m_callbackMtx;
        void TestDownload();

    private:
        std::thread memberThread(const char* url, const char* filePath, void (*func)(int, const char*));
        const int stringSize(const char* str);

    private:
        int m_threadCount;
        

    };
}
