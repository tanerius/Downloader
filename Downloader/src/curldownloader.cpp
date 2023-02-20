#include "EZResume.h"
#include "singleclient.h"
#include <fstream>
#include <iostream>
#include <string>

namespace EZResume
{
    Downloader::Downloader() {}

    Downloader::~Downloader() {}

    void Downloader::TestDownload()
    {
        EZResume::Configutation config;
        std::string destFile = ".";
        destFile += PathSeparator;
        destFile += "1GB.bin";
        //https://home.tanerius.com/samples/files/1MB.bin
        //https://home.tanerius.com/samples/files/1GB.bin

        auto f = [](unsigned long /* total */, unsigned long /* current */) {
            // std::cout << "DL: " << current << "/" << total << std::endl;
        };

        download("https://home.tanerius.com/samples/files/1GB.bin", destFile.c_str(), config, 
            [](int code, const char* msg) {
            std::cout << "Download finished with code: " << code << " " << msg;
            }, f);
    }

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
