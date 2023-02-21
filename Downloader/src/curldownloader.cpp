#include "EZResume.h"
#include "singleclient.h"
#include <fstream>
#include <iostream>
#include <string>

namespace EZResume
{
    Downloader::Downloader() {}

    Downloader::~Downloader() {}

    void Downloader::download(
        const char* url,
        const char* filepath,
        EZResume::Configutation config,
        void (*funcCompleted)(int, const char*),
        void (*funcProgress)(unsigned long totalToDownload, unsigned long downloadedNow),
        const unsigned long chunkSizeInBytes,
        const char* userAgent
    )
    {
        SingleClient sc;
        sc.SetChunkSize(chunkSizeInBytes);
        if (userAgent != nullptr)
            sc.SetUserAgent(userAgent);
        sc.SetConfiguration(config);
        sc.download(url, filepath, funcCompleted, funcProgress);
    }
}
