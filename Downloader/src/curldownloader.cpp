#include "EZResume.h"
#include "singleclient.h"
#include <fstream>
#include <iostream>
#include <string>

namespace EZResume
{
    Downloader::Downloader() {}

    Downloader::~Downloader() {}

    char* Downloader::GetVersion(int& length)
    {
        SingleClient sc(0);
        length = 10;
        return sc.GetVersion();
    }

    void Downloader::Download(
        const int id,
        const char* url,
        const char* filepath,
        EZResume::Configutation config,
        IDownloaderHandler* cbh,
        const char* userAgent
    )
    {
        SingleClient sc(id);
        sc.SetChunkSize(config.ChunkSizeInBytes);
        if (userAgent != nullptr)
            sc.SetUserAgent(userAgent);
        sc.SetConfiguration(config);
        sc.Download(url, filepath, cbh);
    }
}
