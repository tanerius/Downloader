#include "EZResume.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdio>
#include <thread>
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>


int result = -1;
int dlID = -1;

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

TEST(GTestInit, Tests_gtest_is_working)
{
    EXPECT_EQ(1, 1);
}

TEST(DownloaderInit, Tests_downloader_init) {
    EZResume::Downloader* d = new EZResume::Downloader();
    ASSERT_NE(d, nullptr);

    EZResume::Configutation config;

    EXPECT_FALSE(config.OverwriteIfDestinationExists);
    EXPECT_FALSE(config.RestartDownloadIfMetaInfoCorrupt);
    EXPECT_TRUE(config.ReturnErrorIfSourceChanged);
    delete d;
}

TEST(Download, Test50MB_default) {
    EZResume::Downloader d;
    EZResume::Configutation config;
    CallBackHandlerTest* cbh = new CallBackHandlerTest();

    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("50MB.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("50MB.tmp");
    
    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    d.Download(1, "https://home.tanerius.com/samples/files/50MB.bin", dataFile.c_str(), config,
        cbh);

    EXPECT_EQ(cbh->m_id, 1);
    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
    EXPECT_EQ(cbh->m_lastResult, EZResume::DownloadResult::OK);
    delete cbh;
}

TEST(Download, Test100KB_default) {
    EZResume::Downloader d;
    EZResume::Configutation config;
    CallBackHandlerTest* cbh = new CallBackHandlerTest();

    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("100KB.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("100KB.tmp");

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    d.Download(1, "https://home.tanerius.com/samples/files/100KB.bin", dataFile.c_str(), config,
        cbh);

    EXPECT_EQ(cbh->m_id, 1);
    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
    EXPECT_EQ(cbh->m_lastResult, EZResume::DownloadResult::OK);
    delete cbh;
}

TEST(Download, Test_chunk_too_small) {
    EZResume::Downloader d;
    EZResume::Configutation config;
    CallBackHandlerTest* cbh = new CallBackHandlerTest();

    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("100KB.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("100KB.tmp");

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    config.ChunkSizeInBytes = 512L;

    d.Download(1, "https://home.tanerius.com/samples/files/100KB.bin", dataFile.c_str(), config,
        cbh);

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
    EXPECT_EQ(cbh->m_id, 1);
    EXPECT_EQ(cbh->m_lastResult, EZResume::DownloadResult::CHUNK_SIZE_TOO_SMALL);
    delete cbh;
}


TEST(Download, Test_meta_file_corrupt) {
    EZResume::Downloader d;
    EZResume::Configutation config;

    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("10MB.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("10MB.tmp");

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    // create a corrupted meta file
    std::ofstream ofs(metaFile.c_str(), std::ios::binary | std::ios::out);
    if ((ofs.rdstate() & std::ifstream::failbit) != 0)
        FAIL();

    char c[6] = {"Test!"};

    ofs.write(c, 6);
    ofs.close();

    CallBackHandlerTest* cbh = new CallBackHandlerTest();
    d.Download(1, "https://home.tanerius.com/samples/files/10MB.bin", dataFile.c_str(), config,
        cbh);

    EXPECT_EQ(cbh->m_id, 1);
    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
    EXPECT_EQ(cbh->m_lastResult, EZResume::DownloadResult::CORRUPT_METAFILE);
    delete cbh;
}

TEST(Download, Test_override_corrupt_metafile) {
    EZResume::Downloader d;
    EZResume::Configutation config;

    config.RestartDownloadIfMetaInfoCorrupt = true;
    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("10MB.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("10MB.tmp");

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    // create a corrupted meta file
    std::ofstream ofs(metaFile.c_str(), std::ios::binary | std::ios::out);
    if ((ofs.rdstate() & std::ifstream::failbit) != 0)
        FAIL();

    char c[6] = { "Test!" };

    ofs.write(c, 6);
    ofs.close();

    CallBackHandlerTest* cbh = new CallBackHandlerTest();
    d.Download(1, "https://home.tanerius.com/samples/files/10MB.bin", dataFile.c_str(), config,
        cbh);

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
    EXPECT_EQ(cbh->m_id, 1);
    EXPECT_EQ(cbh->m_lastResult, EZResume::DownloadResult::OK);
    delete cbh;
}

int idlocal1 = 0;
int retlocal1 = -1;

void DownloadTask_1(int taskid)
{
    EZResume::Downloader d;
    EZResume::Configutation config;

    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("10MB-1.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("10MB-1.tmp");

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
        
    d.Download(taskid, "https://home.tanerius.com/samples/files/10MB.bin", dataFile.c_str(), config,
        nullptr);

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
}

int idlocal2 = 0;
int retlocal2 = -1;

void DownloadTask_2(int taskid)
{
    EZResume::Downloader d;
    EZResume::Configutation config;

    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("10MB-2.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("10MB-2.tmp");

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    d.Download(taskid, "https://home.tanerius.com/samples/files/10MB.bin", dataFile.c_str(), config,
        nullptr);

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
}

TEST(ThreaddedDownload, Test_multithreaded_download) {
    // Constructs the new thread and runs it. Does not block execution.
    std::thread t1(DownloadTask_1, 2);
    std::thread t2(DownloadTask_2, 3);

    // Do other things...

    // Makes the main thread wait for the new thread to finish execution, therefore blocks its own execution.
    t1.join();
    t2.join();

    // TODO: Make a good test for multithreading
}


TEST(FTPDownload, Test200MB_default) {
    EZResume::Downloader d;
    EZResume::Configutation config;

    std::string dataFile = std::string(".") + std::string(PathSeparator) + std::string("200MB_ftp.bin");
    std::string metaFile = std::string(".") + std::string(PathSeparator) + std::string("200MB_ftp.tmp");

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());

    CallBackHandlerTest* cbh = new CallBackHandlerTest();
    d.Download(1, "ftp://212.183.159.230/pub/200MB.zip", dataFile.c_str(), config,
        cbh);

    EXPECT_EQ(cbh->m_id, 1);
    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
    EXPECT_EQ(cbh->m_lastResult, EZResume::DownloadResult::OK);
    delete cbh;
}



int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}