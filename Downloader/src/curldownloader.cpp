#include "curldownloader.h"
#include "singleclient.h"


namespace DownloaderLib {

    Downloader::Downloader() :
        m_threadCount(0) {}

    Downloader::~Downloader()
    {
    }

    std::thread Downloader::memberThread(const char* url, const char* filePath, void (*func)(int, const char*)) {
        return std::thread([=] {
            SingleClient sc;
            sc.download(url, filePath, func, &m_threadMtx);
            safeDecrement();
            });
    }


    int Downloader::download(const char* url, const char* filePath, void (*func)(int, const char*))
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

    const int Downloader::stringSize(const char* str)
    {
        if (str == nullptr) return 0;
        int size = 0;
        while (str[size] != '\0') size++;
        return size;
    }
}