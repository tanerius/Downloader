#include "curldownloader.h"
#include <iostream>

int main()
{
    auto dlInstance = DownloaderLib::Downloader::instance();

    dlInstance->setCompletedCallback([](int) {
        std::cout << "All Done!" << std::endl;
        exit(0);
    });

    // test some https downloads
    dlInstance->download("https://home.tanerius.com/samples/jpgs/sample1.jpg", "./");
    dlInstance->download("https://home.tanerius.com/samples/jpgs/sample2.jpg", "./");
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

    system("pause");
    return 0;
}