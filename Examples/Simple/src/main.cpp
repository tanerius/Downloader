#include "EZResume.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdio>
#include <thread>

#ifdef WIN32 
#define PathSeparator      "\\"
#else
#define PathSeparator      "/"
#endif

class CallBackHandlerTest : public EZResume::IDownloaderHandler
{
public:
    int m_id = -1;
    EZResume::DownloadResult m_lastResult = EZResume::DownloadResult::IDLE;
    virtual ~CallBackHandlerTest() {}
public:
    void DownloadCompleted(const int id, const EZResume::DownloadResult result, const char*)
    {
        m_id = id;
        m_lastResult = result;
    }

    void DownloadProgress(const int, const EZResume::ulong, const EZResume::ulong)
    {

    }
};

int main(int, char*[])
{
    EZResume::Downloader d;
    EZResume::Configutation config;
    CallBackHandlerTest* cbh = new CallBackHandlerTest();

    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("50MB.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("50MB.tmp");
    
    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    d.Download(1, "https://home.tanerius.com/samples/files/50MB.bin", dataFile.c_str(), config,
        cbh);

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    delete cbh;

    return 0;
}