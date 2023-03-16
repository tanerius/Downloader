# EZResume File Downloader Library in C++  

Work in progress...
  
This is my implementation of a file downloader which supports multiple protocols (such as http, https, ftp), is thread safe, supports resumable download, is cross-platform (tested on win and mac), which uses open source libraries. I wrote this becaus I could not find one to satisfy a simple interface and ease of use written in C++. EZResume is based on libcurl, which can support the following features:

✅ Support Multi-protocol. Since teemo based on libcurl, so it supports all protocols that same as libcurl.

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
CMakeLists
## TODO  
1. make it swag - refactor
2. Improve this documentation

# Downloader
