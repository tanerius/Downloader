#include "curldownloader.h"
#include "singleclient.h"
#include <fstream>
#include <iostream>
#include <string>

namespace DownloaderLib
{

    Downloader::Downloader() : m_threadCount(0) {}

    Downloader::~Downloader()
    {
    }

    std::thread Downloader::memberThread(const char *url, const char *filePath, void (*func)(int, const char *))
    {
        return std::thread([=]
                           {
            SingleClient sc;
            sc.download(url, filePath, func, nullptr);
            safeDecrement(); });
    }

    void Downloader::TestDownload()
    {
        std::string destFile = ".";
        destFile += PathSeparator;
        destFile += "1MB.bin";
        //https://home.tanerius.com/samples/files/1MB.bin
        //https://home.tanerius.com/samples/files/1GB.bin
        download("https://home.tanerius.com/samples/files/1MB.bin", destFile.c_str(), [](int code, const char* msg) {
            std::cout << "Download finished with code: " << code << " " << msg;
            }, nullptr);
    }

    void Downloader::download(
        const char* url,
        const char* filepath,
        void (*funcCompleted)(int, const char*),
        int (*funcProgress)(void*, double, double, double, double) = nullptr
    )
    {
        SingleClient sc;
        auto ret = sc.download(url, filepath, funcCompleted, funcProgress);
    }

    void Downloader::safeDecrement()
    {
        m_threadMtx.lock();
        m_threadCount--;
        m_threadMtx.unlock();
    }

    int Downloader::stringSize(const char *str)
    {
        if (str == nullptr)
            return 0;
        int size = 0;
        while (str[size] != '\0')
            size++;
        return size;
    }

}