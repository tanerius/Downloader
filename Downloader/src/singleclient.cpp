#include "singleclient.h"
#include <filesystem>
#include <ctime>
#include <iostream>
#include <fstream>
#include <regex>
#include <algorithm>
#include <cstdlib>
#ifdef DEBUG
#include <chrono>
#endif // DEBUG


namespace DownloaderLib
{

    SingleClient::SingleClient()
    {
        SetChunkSize(4194304); // 4 MB chunks default
        initCURL();
        curl_easy_setopt(m_curl, CURLoption::CURLOPT_USERAGENT, curl_version());
    }

    SingleClient::SingleClient(size_t chunkSize, const char *agent)
    {
        SetChunkSize(chunkSize);
        initCURL();
        curl_easy_setopt(m_curl, CURLoption::CURLOPT_USERAGENT, agent);
    }

    SingleClient::~SingleClient()
    {
        curl_easy_cleanup(m_curl);
    }

    void SingleClient::SetChunkSize(size_t s)
    {
        m_isProperlyInitialized = false;
        m_chunkSize = s;
        if (m_chunkSize % 1024 == 0 && m_chunkSize > sizeof(SFileMetaData))
        {
            m_isProperlyInitialized = true;
        }
    }

    std::string SingleClient::genRandomString(const int len)
    {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::string tmp_s;
        tmp_s.reserve(len);

        for (int i = 0; i < len; ++i)
        {
            tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        return tmp_s;
    }
        
    DownloadResult SingleClient::ProcessResultAndCleanup(
        const DownloadResult result, 
        void (*funcCompleted)(int, const char*), 
        const char* msg)
    {
        curl_easy_reset(m_curl);

        if (funcCompleted != nullptr)
            funcCompleted((int)result, msg);

        if (m_resourceStatus != nullptr)
            delete m_resourceStatus;
        return result;
    }

    void SingleClient::ResetMemory(MemoryStruct& m)
    {
        if (m.memory != nullptr)
        {
            free(m.memory);
            m.memory = nullptr;
        }
        m.size = 0;
    }

    DownloadResult SingleClient::download(
        const char *url,
        const char *filepath,
        void (*funcCompleted)(int, const char *),
        int (*funcProgress)(void *, double, double, double, double))
    {
        DLOG("Starting download of " << url << " to " << filepath);

#ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
#endif // DEBUG

        
        m_resourceStatus = nullptr;

        if (m_chunkSize <= sizeof(SFileMetaData))
            return ProcessResultAndCleanup(DownloadResult::CHUNK_SIZE_TOO_SMALL, funcCompleted, "Chunk size cannot be smaller than SFileMetaData");

        DownloadResult val = validateResource(url);
        if (val != DownloadResult::OK)
            return ProcessResultAndCleanup(val, funcCompleted, "Could not validate resource");

        // Get filename
        auto sFilePath = std::filesystem::path(filepath);
        SFileMetaData metaData;
        metaData.totalChunks = 0;
        metaData.chunkSize = m_chunkSize;

        // Check if requested resource is of good size
        bool doRangedDownload = static_cast<curl_off_t>(m_chunkSize) < m_resourceStatus->ContentLength;

        std::string tmpSparseFile = sFilePath.parent_path().string() + PathSeparator + sFilePath.filename().stem().string() + ".tmp";
        bool existsSparseFile = std::filesystem::exists(tmpSparseFile);

        if (doRangedDownload)
        {
            DLOG("Starting ranged download...");
            /*
            * RANGED DOWNLOAD
            * This is the case where the file is large and we should enable download resuming
            */
            
            // Check if sparse file exists
            if (existsSparseFile)
            {
                DLOG("Reading sparse file " << tmpSparseFile);
                metaData.totalSize = m_resourceStatus->ContentLength;
                auto resMeta = ReadMetaFile(metaData, tmpSparseFile.c_str());
                if (resMeta != DownloadResult::OK)
                {
                    return ProcessResultAndCleanup(resMeta, funcCompleted, "Error reading resource meta information");;
                }
            }
            else
            {
                DLOG("Creating sparse file " << tmpSparseFile);
                metaData.totalSize = m_resourceStatus->ContentLength;
                metaData.chunkSize = m_chunkSize;
                metaData.currentSavedOffset = 0;
                metaData.totalChunks = metaData.totalSize / m_chunkSize;
                if (metaData.totalSize % m_chunkSize != 0)
                    metaData.totalChunks++;

                if (metaData.totalChunks == 0)
                    return ProcessResultAndCleanup(DownloadResult::RESOURCE_HAS_ZERO_SIZE, funcCompleted, "Resource has zero size");

                // this is a fresh download
                auto resMeta = CreateSparseFile(tmpSparseFile.c_str(), metaData, true);

                if (resMeta != DownloadResult::OK)
                    return ProcessResultAndCleanup(resMeta, funcCompleted, "Error creating resource meta information");
            }

            DLOG("Setting up cURL and starting download... " );
            // set url
            curl_easy_setopt(m_curl, CURLOPT_URL, url);
            // forward all data to this func
            curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &SingleClient::WriteToMemory);
            struct MemoryStruct chunk;          /* This chunk will hold curl callback buffer info during the transfer */
            chunk.memory = nullptr;

            // at least once it should execute
            do
            {
                chunk.memory = (char*)malloc(1);    /* will be grown as needed by the realloc above */
                chunk.size = 0;                     /* no data at this point */

                bool eof = false;
                unsigned long long segmentStart = metaData.currentSavedOffset;
                unsigned long long segmentEnd = segmentStart + metaData.chunkSize - 1;

                if (segmentEnd > static_cast<unsigned long long>(metaData.totalSize))
                {
                    segmentEnd = metaData.totalSize - 1;
                    eof = true;
                }

                /* Check if there is space left to write meta info. If not get the rest of the file even bigger than chunk */
                if ((metaData.totalSize - segmentEnd) < (sizeof(SFileMetaData) + 1))
                    eof = true;

                if (segmentEnd < segmentStart)
                {
                    ResetMemory(chunk);
                    return ProcessResultAndCleanup(DownloadResult::CORRUPT_CHUNK_CALCULATION, funcCompleted, "Error in calculating chunk size");
                }

                std::string chunkRange = std::to_string(segmentStart) + ((eof) ? "-" : "-" + std::to_string(segmentEnd));

                DLOG("Requesting range : " << chunkRange);

                curl_easy_setopt(m_curl, CURLOPT_RANGE, chunkRange.c_str());

                if (funcProgress != nullptr)
                {
                    // Internal CURL progressmeter must be disabled if we provide our own callback
                    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L);
                    // Install the callback function (returning non ze`ro from this will stop curl
                    curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, funcProgress);
                }

                // write the page body to this file handle. CURLOPT_FILE is also known as CURLOPT_WRITEFILE
                curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, (void*)&chunk);
                

                // do it
                CURLcode result = curl_easy_perform(m_curl);

                if (result == CURLE_OK) /* This means curl completed ok */
                {
                    // get HTTP response code
                    int responseCode;
                    result = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);

                    if (responseCode != 206) /* For ranged tramsfers response needs to be 206 */
                    {
                        ResetMemory(chunk);
                        return ProcessResultAndCleanup(DownloadResult::INVALID_RESPONSE, funcCompleted, "Response code is bad");
                    }

                    /* If all is well chunk will have our data that we need to append */

                    // write to file
                    DownloadResult writeResult = WriteChunkData(tmpSparseFile.c_str(), chunk, metaData, eof);
                    if (writeResult != DownloadResult::OK)
                    {
                        ResetMemory(chunk);
                        return ProcessResultAndCleanup(writeResult, funcCompleted, "Was not able to write downloaded data");
                    }
                }
                else
                {
                    ResetMemory(chunk);
                    return ProcessResultAndCleanup(DownloadResult::DOWNLOADER_EXECUTE_ERROR, funcCompleted, "Error performing the curl request");
                }

                ResetMemory(chunk);

            } while (static_cast<curl_off_t>(metaData.lastDownloadedChunk) < metaData.totalChunks);

            DLOG("Finished download. Cleaning up... ");

            // check if destination file exists if so remove it
            std::remove(sFilePath.string().c_str());

            // rename it to real filename which does a move of the file so no need for explicit deletion
            int ret = std::rename(tmpSparseFile.c_str(), sFilePath.string().c_str());

            if (ret > 0)
                return ProcessResultAndCleanup(DownloadResult::CANNOT_RENAME_TEMP_FILE, funcCompleted, "Cannot rename the temp file");

            curl_easy_reset(m_curl);
        }
        else
        {
            /*
            * REGULAR DOWNLOAD
            * This is the case where the file is too small to do a ranged download so do a regular one
            */
            DLOG("Starting non-ranged download...");

            if (existsSparseFile)
                std::remove(tmpSparseFile.c_str());
            
            auto resMeta = CreateSparseFile(tmpSparseFile.c_str(), metaData, false);

            if (resMeta != DownloadResult::OK)
                return ProcessResultAndCleanup(resMeta, funcCompleted, "Error creating resource meta information");

            // set url
            curl_easy_setopt(m_curl, CURLOPT_URL, url);

            // forward all data to this func
            curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &SingleClient::WriteToFile);

            // open the file
            FILE* file = nullptr;
#ifdef __APPLE__
            file = fopen(tmpFile.c_str(), "wb");
            if (file && (file == 0))
#else
            auto res = fopen_s(&file, tmpSparseFile.c_str(), "wb");
            if (file && (res == 0))
#endif

            {
                // write the page body to this file handle. CURLOPT_FILE is also known as CURLOPT_WRITEFILE
                curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, file);

                if (funcProgress != nullptr)
                {
                    // Internal CURL progressmeter must be disabled if we provide our own callback
                    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L);
                    // Install the callback function (returning non zero from this will stop curl
                    curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, funcProgress);
                }

                // do it
                CURLcode result = curl_easy_perform(m_curl);

                std::fclose(file);

                
                
                int ret = 1;
                if (result == CURLE_OK) // only assume 200 is correct
                {
                    // get HTTP response code
                    int responseCode;
                    result = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);

                    if(responseCode < 200 || responseCode >= 300)
                        return ProcessResultAndCleanup(DownloadResult::INVALID_RESPONSE, funcCompleted, "Response code is bad");

                    // check if destination file exists if so remove it
                    std::remove(sFilePath.string().c_str());

                    // rename it to real filename which does a move of the file so no need for explicit deletion
                    ret = std::rename(tmpSparseFile.c_str(), sFilePath.string().c_str());

                    if(ret > 0)
                        return ProcessResultAndCleanup(DownloadResult::CANNOT_RENAME_TEMP_FILE, funcCompleted, "Cannot rename the temp file");
                }
                else
                    return ProcessResultAndCleanup(DownloadResult::DOWNLOADER_EXECUTE_ERROR, funcCompleted, "Error performing the curl request");

                
            }
            curl_easy_reset(m_curl);
        }

        if (funcCompleted != nullptr)
            funcCompleted((int)DownloadResult::OK, filepath);

        DLOG("Can accept ranges..." << m_resourceStatus->CanAcceptRanges);
        DLOG("ContentLength..." << m_resourceStatus->ContentLength);
        DLOG("DownloadedSize..." << m_resourceStatus->DownloadedSize);
        DLOG("IsValidated..." << m_resourceStatus->IsValidated);
        DLOG("ResponseCode..." << m_resourceStatus->ResponseCode);
        DLOG("URL..." << m_resourceStatus->URL);
        DLOG("ChunkSize..." << m_chunkSize);

#ifdef DEBUG
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        DLOG("Duration: " << duration.count() << "ms");
#endif // DEBUG

        return DownloadResult::OK;
    }

    DownloadResult SingleClient::WriteChunkData(
        const char* filePath,
        const MemoryStruct& downloadedData,
        SFileMetaData& md,
        bool isEof)
    {

        std::ofstream fs(filePath, std::ios::binary | std::ios::out | std::ios::in);
        if ((fs.rdstate() & std::ifstream::failbit) != 0)
            return DownloadResult::CANNOT_ACCESS_METAFILE;


        DLOG(md.lastDownloadedChunk << "/" << md.totalChunks << ": CurrentOffset: " << md.currentSavedOffset << " / ActualSize: " << downloadedData.size << " / ChunkSize: " << md.chunkSize);

        // first write the data
        fs.seekp(md.currentSavedOffset);
        fs.write(downloadedData.memory, downloadedData.size);
        
        if (!fs.good())
        {
            fs.close();
            return DownloadResult::CANNOT_WRITE_DOWNLOADED_DATA;
        }

        md.currentSavedOffset += downloadedData.size;
        md.lastDownloadedChunk++;

        if (isEof)
        {
            // done writing here and dl completed
            fs.close();
            return DownloadResult::OK;
        }
        
        // write metadata to file and close file
        fs.seekp(md.totalSize - sizeof(SFileMetaData));
        fs.write((char*)&md, sizeof(md));
        fs.close();

        if (!fs.good())
            return DownloadResult::CANNOT_WRITE_META_DATA;

        return DownloadResult::OK;
    }

    size_t SingleClient::WriteToFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
        return fwrite(ptr, size, nmemb, stream);
    }

    size_t SingleClient::WriteToMemory(char* receivedContent, size_t size, size_t nmemb, void* userdata)
    {
        size_t realsize = size * nmemb;
        struct MemoryStruct* mem = (struct MemoryStruct*)userdata;

        char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
        if (!ptr) {
            /* out of memory! */
            printf("not enough memory (realloc returned NULL)\n");
            return 0;
        }

        mem->memory = ptr;
        memcpy(&(mem->memory[mem->size]), receivedContent, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
    }

    size_t SingleClient::WriteToString(void *ptr, size_t size, size_t nmemb, std::string &sp)
    {
        char* data = (char*)ptr;
        size_t len = size * nmemb;
        sp.insert(sp.end(), data, data + len);
        return len;
    }

    void SingleClient::initCURL()
    {
        // init global
        static bool globalInitialized = false;
        if (!globalInitialized)
        {
            curl_global_init(CURL_GLOBAL_ALL); // there won't be curl_global_cleanup();
            globalInitialized = true;
        }

        // init current session
        m_curl = curl_easy_init();
        // try not to use signals
        curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);
        // prevent ending up in an endless redirection
        curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 5L);

#ifdef DEBUG_MODE
        // Switch on full protocol/debug output while testing
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

        // disable progress meter, set to 0L to enable and disable debug output
        // curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 1L);
#endif
    }

    DownloadResult SingleClient::ReadMetaFile(SFileMetaData &md, const char *filename)
    {
        DownloadResult res = DownloadResult::OK;
        std::ifstream ifs(filename, std::ios::binary);

        if (!ifs)
            return DownloadResult::COULD_NOT_READ_METAFILE;

        md.checkCode = 5; // in the end should be 2308075
        auto oldTotal = md.totalSize;
        ifs.seekg(md.totalSize - sizeof(SFileMetaData), std::ios::beg);
        ifs.read(reinterpret_cast<char *>(&md), sizeof(SFileMetaData));

        if (ifs.gcount() != sizeof(SFileMetaData) || md.checkCode != 2308075)
            res = DownloadResult::CORRUPT_METAFILE;

        if (oldTotal != md.totalSize)
        {
            res = DownloadResult::RESOURCE_SIZE_CHANGED;
        }

        ifs.close();
        return res;
    }

    /// <summary>
    /// Writes meta info at the end to be overwritten with the last chunk.
    /// </summary>
    /// <param name="filePath"></param>
    /// <param name="fileMeta"></param>
    /// <returns></returns>
    DownloadResult SingleClient::CreateSparseFile(const char *filePath, const SFileMetaData &fileMeta, const bool includeMeta)
    {
        std::ofstream ofs(filePath, std::ios::binary | std::ios::out);
        if ((ofs.rdstate() & std::ifstream::failbit) != 0)
            return DownloadResult::CANNOT_CREATE_METAFILE;
        if (includeMeta)
        {
            ofs.seekp(fileMeta.totalSize - sizeof(SFileMetaData));
            ofs.write((char*)&fileMeta, sizeof(fileMeta));
        }
        else
        {
            ofs.seekp(fileMeta.totalSize - 1);
            ofs.write("", 1);
        }
        ofs.close();
        return DownloadResult::OK;
    }

    size_t SingleClient::CurlHeaderCallback(
        char *buffer,
        size_t nsize,
        size_t nitems,
        SingleClient::ResourceStatus *h)
    {
        std::string temp = std::string(buffer, nitems);

        if (temp.size() > 3)
            h->Headers.push_back(temp);

        return nitems * nsize;
    }

    long SingleClient::GetLastResponseCode() const
    {
        if (m_resourceStatus == nullptr)
            return 0l;
        return m_resourceStatus->ResponseCode;
    }

    void SingleClient::CleanString(std::string &res)
    {
        res = std::regex_replace(res, std::regex("^ +| +$|( ) +"), "$1");
        res.erase(std::remove(res.begin(), res.end(), '"'), res.end());
        res.erase(std::remove(res.begin(), res.end(), '\r'), res.end());
        res.erase(std::remove(res.begin(), res.end(), '\n'), res.end());
    }

    std::string SingleClient::GetAttribute(const char *attr)
    {
        std::string tofind(attr);
        if (m_resourceStatus == nullptr)
            return "";

        for (size_t i = 0; i < (m_resourceStatus->Headers).size(); i++)
        {
            std::size_t pos = m_resourceStatus->Headers[i].find(tofind);
            if (pos != std::string::npos)
            {
                std::string res = m_resourceStatus->Headers[i].substr(pos + tofind.size());
                CleanString(res);
                return res;
            }
        }

        return "";
    }

    std::string SingleClient::GetAcceptRangesValue()
    {
        return GetAttribute("accept-ranges: ");
    }

    std::string SingleClient::GetETag()
    {
        return GetAttribute("etag: ");
    }

    std::string SingleClient::GetLastModified()
    {
        return GetAttribute("last-modified: ");
    }

    std::string SingleClient::GetContentLength()
    {
        return GetAttribute("content-length: ");
    }

    void SingleClient::PopulateResourceMetadata(const CURLcode cc)
    {
        if (cc == CURLE_OK)
        {
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &m_resourceStatus->ResponseCode);

            auto tmp = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &m_resourceStatus->ContentLength);
            if (tmp != CURLE_OK)
                m_resourceStatus->ContentLength = -1;
            tmp = curl_easy_getinfo(m_curl, CURLINFO_SIZE_DOWNLOAD_T, &m_resourceStatus->DownloadedSize);
            if (tmp != CURLE_OK)
                m_resourceStatus->DownloadedSize = -1;

            // Check if range download is supported
            std::string r = GetAcceptRangesValue();

            if (r == "bytes")
                m_resourceStatus->CanAcceptRanges = true;
            else
                m_resourceStatus->CanAcceptRanges = false;
            m_resourceStatus->IsValidated = true;
        }
        else
        {
            m_resourceStatus->IsValidated = false;
        }
    }

    DownloadResult SingleClient::validateResource(const char *url)
    {
        // Set the stuff up first!
        if (!m_isProperlyInitialized)
            return DownloadResult::DOWNLOADER_NOT_INITIALIZED;

        m_resourceStatus = new SingleClient::ResourceStatus();
        m_resourceStatus->URL = url;

        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "HEAD");
        curl_easy_setopt(m_curl, CURLOPT_URL, url);
        curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &SingleClient::CurlHeaderCallback);
        curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, m_resourceStatus);

        auto res = curl_easy_perform(m_curl);
        PopulateResourceMetadata(res);
        curl_easy_reset(m_curl);
        return DownloadResult::OK;
    }
}