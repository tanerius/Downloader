#include "curldownloader.h"
#include "singleclient.h"
#include <fstream>
#include <iostream>
#include <string>

namespace DownloaderLib
{

    Downloader::Downloader() {}

    Downloader::~Downloader() {}

    void Downloader::TestDownload()
    {
        DownloaderLib::Configutation config;
        std::string destFile = ".";
        destFile += PathSeparator;
        destFile += "1GB.bin";
        //https://home.tanerius.com/samples/files/1MB.bin
        //https://home.tanerius.com/samples/files/1GB.bin
        download("https://home.tanerius.com/samples/files/1GB.bin", destFile.c_str(), config, 
            [](int code, const char* msg) {
            std::cout << "Download finished with code: " << code << " " << msg;
            }, nullptr);
    }

    void Downloader::download(
        const char* url,
        const char* filepath,
        DownloaderLib::Configutation config,
        void (*funcCompleted)(int, const char*),
        int (*funcProgress)(void*, double, double, double, double),
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
