#include "EZResume.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>


TEST(sample_test_case, sample_test)
{
    EXPECT_EQ(1, 1);
}

/*
void testJson()
{
    std::ifstream f(".\\taner.json");
    nlohmann::json data = nlohmann::json::parse(f);
    std::cout << data.at("name");
    std::string s = data.dump();
    std::cout << s << std::endl;
}


unsigned long totalSize = 0;
unsigned long currentSize = 0;

void progress(unsigned long total , unsigned long  current )
{
    currentSize += current;
    std::cout << "Downloading..." << currentSize << "/" << total << std::endl;
}

int main(int argc, char* argv[])
{

    if (argc != 3)
    {
        std::cout << "Arguments are: DownloaderTest.exe <URL> <output filename>" << std::endl;
        system("pause");
        return 0;
    }

    currentSize = 0;
    EZResume::Downloader d;
    EZResume::Configutation config;
    config.OverwriteIfDestinationExists = true; 

    //https://home.tanerius.com/samples/files/1MB.bin
    //https://home.tanerius.com/samples/files/1GB.bin

    d.download(argv[1], argv[2], config,
        [](int code, const char* msg) {
            std::cout << "Download finished with code: " << code << " " << msg << std::endl;
        }, nullptr);

    system("pause");
    return 0;
}
*/