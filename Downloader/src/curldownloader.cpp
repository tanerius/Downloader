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
        const int id,
        const char* url,
        const char* filepath,
        EZResume::Configutation config,
        DownloadCompletedCallback completedCallback,
        DownloadProgressCallback progressCallback,
        const unsigned long chunkSizeInBytes,
        const char* userAgent
    )
    {
        SingleClient sc(id);
        sc.SetChunkSize(chunkSizeInBytes);
        if (userAgent != nullptr)
            sc.SetUserAgent(userAgent);
        sc.SetConfiguration(config);
        sc.download(url, filepath, completedCallback, progressCallback);
    }
}
