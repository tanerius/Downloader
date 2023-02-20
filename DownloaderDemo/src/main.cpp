#include "EZResume.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

/*
void testJson()
{
    std::ifstream f(".\\taner.json");
    nlohmann::json data = nlohmann::json::parse(f);
    std::cout << data.at("name");
    std::string s = data.dump();
    std::cout << s << std::endl;
}
*/

void foo(int x, const char *p)
{
    std::cout << "Thread exited..." << x << std::endl;
    std::string file = p;
    std::cout << "Downloaded: " << p << std::endl;
}

int main()
{
    auto dlInstance = new DownloaderLib::Downloader();
    dlInstance->TestDownload();
    return 0;

    /*
    auto dlInstance = new DownloaderLib::Downloader();

    // test some https downloads

    dlInstance->download("https://home.tanerius.com/samples/sample.php", ".\\taner.json", &foo);

    dlInstance->download("https://home.tanerius.com/samples/jpgs/sample1.jpg", ".\\sample1.jpg", [](int, const char* p) {
        std::cout << "Thread exited..." << std::endl;
        std::string file = p;
        std::cout << "Downloaded: " << p << std::endl;
        });

    dlInstance->download("https://home.tanerius.com/samples/jpgs/sample2.jpg", ".\\sample2.jpg", [](int, const char* p) {
        std::cout << "Thread exited..." << std::endl;
    std::string file = p;
    std::cout << "Downloaded: " << p << std::endl;
        });

    // test some http downloads
    dlInstance->download("http://home.tanerius.com/samples/jpgs/sample3.jpg", "./");
    dlInstance->download("http://home.tanerius.com/samples/jpgs/sample4.jpg", "./");

    // custom callback test
    dlInstance->download("http://home.tanerius.com/samples/jpgs/sample5.jpg", "./", [](int no)
    {
        std::cout << "Downloaded: #" << no << std::endl;
    });

    // custom name test
    DownloaderLib::Downloader::instance()->download("http://home.tanerius.com/samples/jpgs/sample6.jpg", "./", "custom.jpg");

    */
    // system("pause");
    // return 0;
}