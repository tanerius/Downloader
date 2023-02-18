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
            void (*funcCompleted)(int, const char*),
            int (*funcProgress)(void*, double, double, double, double) = nullptr
        );

        void TestDownload();

    private:
        int stringSize(const char *str);
    };
}
