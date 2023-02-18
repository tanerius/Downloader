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
        SingleClient sc;
        std::string destFile = ".";
        destFile += PathSeparator;
        destFile += "1GB.bin";
        sc.download("https://home.tanerius.com/samples/files/1GB.bin", destFile.c_str(), [](int code, const char* msg) {
            std::cout << "Download finished with code: " << code << " " << msg;
            }, nullptr);
    }

    int Downloader::download(const char *url, const char *filePath, void (*func)(int, const char *))
    {
        // check thread count
        bool canCreateThread = false;

        m_threadMtx.lock();
        if (m_threadCount < MaxDownloadThreads)
        {
            canCreateThread = true;
            ++m_threadCount;
        }
        m_threadMtx.unlock();

        if (canCreateThread)
        {
            // create new thread to execute the task
            std::thread t = memberThread(url, filePath, func);
            t.join();
        }

        return 0;
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