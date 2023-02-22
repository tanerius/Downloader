#include "EZResume.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>

int result = -1;
int dlID = -1;

TEST(GTestInit, Tests_gtest_is_working)
{
    EXPECT_EQ(1, 1);
}

// Tests that the Foo::Bar() method does Abc.
TEST(DownloaderInit, Tests_downloader_init) {
    EZResume::Downloader *d = new EZResume::Downloader();
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
    
    std::remove(".\\50MB.bin");
    std::remove(".\\50MB.tmp");

    d.download(1, "https://home.tanerius.com/samples/files/50MB.bin", ".\\50MB.bin", config,
        [](int id, int code, const char*) {
            result = code;
            dlID = id;
        }, nullptr);

    EXPECT_EQ(dlID, 1);
    std::remove(".\\50MB.bin");
    std::remove(".\\50MB.tmp");
    EXPECT_EQ(result, (int)EZResume::DownloadResult::OK);
}

TEST(Download, Test100KB_default) {
    EZResume::Downloader d;
    EZResume::Configutation config;

    std::remove(".\\100KB.bin");
    std::remove(".\\100KB.tmp");

    d.download(1, "https://home.tanerius.com/samples/files/100KB.bin", ".\\100KB.bin", config,
        [](int id, int code, const char*) {
            result = code;
            dlID = id;
        }, nullptr);

    EXPECT_EQ(dlID, 1);
    std::remove(".\\100KB.bin");
    std::remove(".\\100KB.tmp");
    EXPECT_EQ(result, (int)EZResume::DownloadResult::OK);
}

TEST(Download, Test_chunk_too_small) {
    EZResume::Downloader d;
    EZResume::Configutation config;

    std::remove(".\\100KB.bin");
    std::remove(".\\100KB.tmp");

    d.download(1, "https://home.tanerius.com/samples/files/100KB.bin", ".\\100KB.bin", config,
        [](int id, int code, const char*) {
            result = code;
            dlID = id;
        }, nullptr, 512);

    std::remove(".\\100KB.bin");
    std::remove(".\\100KB.tmp");
    EXPECT_EQ(dlID, 1);
    EXPECT_EQ(result, (int)EZResume::DownloadResult::CHUNK_SIZE_TOO_SMALL);
}


TEST(Download, Test_meta_file_corrupt) {
    EZResume::Downloader d;
    EZResume::Configutation config;

    std::remove(".\\10MB.bin");
    std::remove(".\\10MB.tmp");

    // create a corrupted meta file
    std::ofstream ofs(".\\10MB.tmp", std::ios::binary | std::ios::out);
    if ((ofs.rdstate() & std::ifstream::failbit) != 0)
        FAIL();

    char c[6] = {"Test!"};

    ofs.write(c, 6);
    ofs.close();


    d.download(1, "https://home.tanerius.com/samples/files/10MB.bin", ".\\10MB.bin", config,
        [](int id, int code, const char*) {
            result = code;
            dlID = id;
        }, nullptr);

    EXPECT_EQ(dlID, 1);
    std::remove(".\\10MB.bin");
    std::remove(".\\10MB.tmp");
    EXPECT_EQ(result, (int)EZResume::DownloadResult::CORRUPT_METAFILE);
}

TEST(Download, Test_override_corrupt_metafile) {
    EZResume::Downloader d;
    EZResume::Configutation config;

    config.RestartDownloadIfMetaInfoCorrupt = true;

    std::remove(".\\10MB.bin");
    std::remove(".\\10MB.tmp");

    // create a corrupted meta file
    std::ofstream ofs(".\\10MB.tmp", std::ios::binary | std::ios::out);
    if ((ofs.rdstate() & std::ifstream::failbit) != 0)
        FAIL();

    char c[6] = { "Test!" };

    ofs.write(c, 6);
    ofs.close();


    d.download(1, "https://home.tanerius.com/samples/files/10MB.bin", ".\\10MB.bin", config,
        [](int id, int code, const char*) {
            result = code;
            dlID = id;
        }, nullptr);

    std::remove(".\\10MB.bin");
    std::remove(".\\10MB.tmp");
    EXPECT_EQ(dlID, 1);
    EXPECT_EQ(result, (int)EZResume::DownloadResult::OK);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}