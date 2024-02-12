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

    d.Download("https://home.tanerius.com/samples/files/50MB.bin", dataFile.c_str(), config,
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

    d.Download("https://home.tanerius.com/samples/files/100KB.bin", dataFile.c_str(), config,
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

    d.Download("https://home.tanerius.com/samples/files/100KB.bin", dataFile.c_str(), config,
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
    d.Download("https://home.tanerius.com/samples/files/10MB.bin", dataFile.c_str(), config,
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
    d.Download("https://home.tanerius.com/samples/files/10MB.bin", dataFile.c_str(), config,
        cbh);

    std::remove(dataFile.c_str());
    std::remove(metaFile.c_str());
    EXPECT_EQ(cbh->m_id, 1);
    EXPECT_EQ(cbh->m_lastResult, EZResume::DownloadResult::OK);
    delete cbh;
}

TEST(ThreaddedDownload, Test_multithreaded_download) {
    EZResume::Downloader d;
    EZResume::Configutation config_1;
    config_1.Id = 1;
    EZResume::Configutation config_2;
    config_2.Id = 2;

    // set up stuff for thread 1
    std::string dataFile_1 = std::string(".") + std::string(PathSeparator) + std::string("10MB-1.bin");
    std::string metaFile_1 = std::string(".") + std::string(PathSeparator) + std::string("10MB-1.tmp");

    std::remove(dataFile_1.c_str());
    std::remove(metaFile_1.c_str());

    // set up stuff for thread 2
    std::string dataFile_2 = std::string(".") + std::string(PathSeparator) + std::string("10MB-2.bin");
    std::string metaFile_2 = std::string(".") + std::string(PathSeparator) + std::string("10MB-2.tmp");

    std::remove(dataFile_2.c_str());
    std::remove(metaFile_2.c_str());

    // Constructs the new thread and runs it. Does not block execution.

    std::thread t1([&d, &dataFile_1, &config_1]() {
        d.Download("https://home.tanerius.com/samples/files/10MB.bin", dataFile_1.c_str(), config_1, nullptr);
        });
    
    std::thread t2([&d, &dataFile_2, &config_2]() {
        d.Download("https://home.tanerius.com/samples/files/10MB.bin", dataFile_2.c_str(), config_2, nullptr);
        });

    // Makes the main thread wait for the new thread to finish execution, therefore blocks its own execution.
    t1.join();
    t2.join();

    std::remove(dataFile_1.c_str());
    std::remove(metaFile_1.c_str());
    std::remove(dataFile_2.c_str());
    std::remove(metaFile_2.c_str());
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}