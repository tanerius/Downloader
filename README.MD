# EZResume File Downloader Library in C++  

Current version is: *1.3.0*  
  
Work in progress...  
  
This is my implementation of a file downloader which supports multiple protocols (such as http, https, ftp), is thread safe, supports resumable download, is cross-platform (tested on win and mac), which uses open source libraries. I wrote this becaus I could not find one to satisfy a simple interface and ease of use written in C++. EZResume is based on libcurl, which can support the following features:

✅ Support Multi-protocol. Since this library is based on libcurl, so it supports all protocols that same as libcurl.

✅ Support multi-threaded download.

✅ Support detection of files modified at source.

✅ Support auto resuming interrupted downloads.

✅ Support customizable chunk sizes.

✅ Support per file download configuration.

## How it works
In a nutshell EZResume is called by invoking a download function from its API. It initially checks for a previously created sparse file which also contains meta information for the file to be downloaded. If it cant find it, it creates a new one. This sparse file will have the size of the final downloaded file. Initially it will be created as a blank file only to guarantee that there is enough disk space. Then, the actual download begins. The data is downloaded in customizable chunks (default is 4MB chunks). Should a download be interrupted, the most data that will be lost is less or equal to a chunk. This means at most we would have to resume from the last interrupted chunk. A chunk must be divisable by 1024 and cannot be smaller than the size of the MetaData structure (which is small and probably can never be the case). If the source file is smaller that the chunk than download resuming is disabled and the file is downloaded in one go. Otherwise, resumable download is automatically activated.  
  
An interrupted download will fail to resume if:  

- The source file has been modified
- The Metadata in the sparse (tmp) file has been corrupted
- The sparse file (tmp) has been deleted or is in some other way inaccessable
- Unrelated reasons like not enough memory to hold a chunk etc.
  
In the above cases EZResume will simply restart the download in resumable or non resumable mode based on if file size > chunk.

## Usage

In oder to include this downloader to your project you can either: 

- Include and build the library from source using CMake.
- Download a prebuilt static or dynamic library version from the releases and then include it in your project.

You can use CMake to add the library project to your own project and then link ```EZResume::EZResume``` to your target by using CMake's ```target_link_libraries```.  
  
### Using the library in C++ example

Start by including ```EZResume.h``` header file in your source. This exposes the ```EZresume::Downloader``` class which is used to initiate a download. In this example we will 
request to download lets say a file from *https://example.com/somefile.zip* and save it locally as ```myfile.zip```. Below is some sample code of how to do this in c++:

```cpp

// A very simple Example of how to use the library
#include <EZResume.h>

// We will define a class to handle the callbacks
class CallBackHandlerTest : public EZResume::IDownloaderHandler
{
public:
    int m_id = -1;
    EZResume::DownloadResult m_lastResult = EZResume::DownloadResult::IDLE;
public:
    // Called when download has completed
    void DownloadCompleted(const int id, const EZResume::DownloadResult result, const char*)
    {
        m_id = id;
        m_lastResult = result;
    }

    // Called every after every stream buffer written locally
    void DownloadProgress(const int, const EZResume::ulong, const EZResume::ulong)
    {

    }
};

int main() {
    // First we create a doenloader object 
    EZResume::Downloader d;
    // We also create a configuration object with the default values
    EZResume::Configutation config;
    // and finally we create a result variable to store our download result
    EZResume::DownloadResult result;

    // we declare the class to handle callbacks ... this isnt necessary but useful to check if everything went well
    CallBackHandlerTest* cbh = new CallBackHandlerTest();
    
    // then we simply start the download
    d.Download(
        "https://example.com/somefile.zip",  // the url where to download from
        "./myfile.zip",                     // output filename (can be a relative too)
        config,                             // the config
        cbh);

    // finally we check if the download was ok
    if(cbh->m_lastResult == EZResume::DownloadResult::OK)
    {
        // Download completed successfully
        return 0;
    }

    int ret  = (int) cbh->m_lastResult;
    delete cbh;

    return ret;
}
```

### Using the library in a C++ thread example

Similar to the previous example we will see how to start downloading two different files in threads. Please note that the `EZResume::Downloader d;` is NOT thread safe, nor is it meant to be called outside the main thread. Only the `d.Download(...)` method with its own config can be called in a new thread. Below is an example

```cpp
int main() {
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

    return 0;
}
```

## Update Log

### Version 1.3.0

- Now uses systems libcurl instead of own.
- Threads are handled better
- Id of download is in configuration now
- Simpler Downloader constructor
- Does not validate host certificates as this lib is purely a simple downloader and shouldn't care about more than getting the file across.
- Some readability fixes
- Removed a failing test because of third party URL failure (download file via FTP)
  