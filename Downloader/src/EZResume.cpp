#include "EZResume.h"
#include "singleclient.h"
#include <fstream>
#include <iostream>
#include <string>

namespace EZResume
{
    Downloader::Downloader() {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    Downloader::~Downloader() {
        curl_global_cleanup();
    }

    const char* Downloader::GetVersion() const
    {
        return SingleClient::GetVersion();
    }

    void Downloader::Download(
        const char* url,
        const char* filepath,
        EZResume::Configutation config,
        IDownloaderHandler* cbh,
        const char* userAgent
    )
    {
        SingleClient sc(config);
        if (userAgent != nullptr)
            sc.SetUserAgent(userAgent);
        sc.Download(url, filepath, cbh);
    }
}
