#pragma once
#include "common.h"
namespace DownloaderLib
{

    class Downloader
    {
    public:
        Downloader();
        virtual ~Downloader();

        /**
         Download a file from a url.
         @return: task ID. If failed, return -1
         */
        void download(
            const char* url,
            const char* filepath,
            DownloaderLib::Configutation config,
            void (*funcCompleted)(int, const char*),
            void (*funcProgress)(unsigned long totalToDownload, unsigned long downloadedNow) = nullptr,
            const unsigned long chunkSizeInBytes = 4194304, /* 4MB */
            const char* userAgent = nullptr /* Defaults to EzResumeDownloader_version*/
        );

        void TestDownload();
    };
}
