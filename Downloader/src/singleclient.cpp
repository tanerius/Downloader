#include "singleclient.h"
#include <filesystem>
#include <ctime>
#include <iostream>
#include <fstream>
#include <regex>
#include <algorithm>

namespace DownloaderLib
{

    SingleClient::SingleClient()
    {
        initCURL();
        curl_easy_setopt(m_curl, CURLoption::CURLOPT_USERAGENT, curl_version());
    }

    SingleClient::SingleClient(size_t chunkSize, const char* agent)
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
        if (m_chunkSize % 1024 == 0)
        {
            m_isProperlyInitialized = true;
        }
    }

    std::string SingleClient::genRandomString(const int len) {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        
        std::string tmp_s;
        tmp_s.reserve(len);

        for (int i = 0; i < len; ++i) {
            tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        return tmp_s;
    }

    void SingleClient::download(
        const char* url, 
        const char* filepath, 
        void (*funcCompleted)(int, const char*),
        int (*funcProgress)(void*, double, double, double, double)
    )
    {
        validateResource(url);
        /*
        Pseudo steps:
        1. Check if file has been previously stopped
        1.1 If no create an info entry for the file and go to 2
        1.2 If yes get ranges from where to download and go to 2
        2. Check if server supports Range
        2.1 If no go to 3
        2.2 If yes go to 4
        3 Set entire file as next chunk in info entry and go to 5
        4 Set the range of next chunk to download in info entry and go to 5
        5 Download chunk
        6 Update info entry (either remove file or set which is next chunk)
        7 Is end of current chunk also EOF ?
        7.1 If no go to 4
        7.2 If yes go to 8
        8 Do cleanups and call callback for current file 
        */

        if (false)
            return;

        // set url
        curl_easy_setopt(m_curl, CURLOPT_URL, url);

        // forward all data to this func
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &SingleClient::writeToFile);

        // try to write a temp file instead of the real file
        std::string tmpFile = genRandomString(20) + ".tmp";
        //std::string tmpFile = filepath + ".tmp";

        while (std::filesystem::exists(tmpFile))
            tmpFile = genRandomString(10) + ".tmp";

        // open the file
        FILE* file = nullptr;
        auto res = fopen_s(&file, tmpFile.c_str(), "wb");

        if (file && (res == 0))
        {
            // write the page body to this file handle. CURLOPT_FILE is also known as CURLOPT_WRITEFILE
            curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, file);

            if (funcProgress != nullptr)
            {
                // Internal CURL progressmeter must be disabled if we provide our own callback
                curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, FALSE);
                // Install the callback function (returnont non zero from this will stop curl
                curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, funcProgress);
            }

            // do it
            CURLcode result = curl_easy_perform(m_curl);

            std::fclose(file);

            // get HTTP response code
            int responseCode;
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);
            int ret = 1;
            if (result == CURLE_OK && responseCode == 200) // only assume 200 is correct
            {
                // check if file exists if so remove it
                std::remove(filepath);

                // rename it to real filename which does a move of the file so no need for explicit deletion
                ret = std::rename(tmpFile.c_str(), filepath);

                
            }
            std::remove(tmpFile.c_str()); // clean tmp file if non zero rename

            if (funcCompleted != nullptr)
                funcCompleted(ret, filepath);
        }
    }

    size_t SingleClient::writeToFile(void* ptr, size_t size, size_t nmemb, FILE* stream)
    {
        return fwrite(ptr, size, nmemb, stream);
    }

    size_t SingleClient::writeToString(char* ptr, size_t size, size_t nmemb, std::string& sp)
    {
        size_t len = size * nmemb;
        sp.insert(sp.end(), ptr, ptr + len);
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
        //curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

        // disable progress meter, set to 0L to enable and disable debug output
        //curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 1L);
#endif
    }

    const bool SingleClient::createSparseFile(const char* filePath, const unsigned long fileSize)
    {
        std::ofstream ofs(filePath, std::ios::binary | std::ios::out);
        if ((ofs.rdstate() & std::ifstream::failbit) != 0)
            return true;
        ofs.seekp(sizeof(SFileMetaData) + fileSize);
        ofs.write("", 1);
        ofs.close();
        return false;
    }

    size_t SingleClient::CurlHeaderCallback(
        char* buffer,
        size_t nsize,
        size_t nitems,
        SingleClient::ResourceStatus* h)
    {
        std::string temp = std::string(buffer, nitems);

        if (!h->IsInitialized)
        {
            h->IsInitialized = true;
        }
        if(temp.size() > 3)
            h->Headers.push_back(temp);

        return nitems * nsize;
    }

    const long SingleClient::GetLastResponseCode() const
    {
        if (m_resourceStatus == nullptr) return 0l;
        return m_resourceStatus->ResponseCode;
    }

    void SingleClient::CleanString(std::string& res) 
    {
        res = std::regex_replace(res, std::regex("^ +| +$|( ) +"), "$1");
        res.erase(std::remove(res.begin(), res.end(), '"'), res.end());
        res.erase(std::remove(res.begin(), res.end(), '\r'), res.end());
        res.erase(std::remove(res.begin(), res.end(), '\n'), res.end());
    }

    std::string SingleClient::GetAttribute(const char* attr)
    {
        std::string tofind(attr);
        if (m_resourceStatus == nullptr) return "";

        for (size_t i = 0; i < (m_resourceStatus->Headers).size(); i++) {
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
        return GetAttribute("Accept-Ranges: ");
    }

    std::string SingleClient::GetETag()
    {
        return GetAttribute("ETag: ");
    }

    SingleClient::ResourceStatus* SingleClient::validateResource(const char* url) {
        m_resourceStatus = new SingleClient::ResourceStatus();

        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "HEAD");
        curl_easy_setopt(m_curl, CURLOPT_URL, url);
        curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist* headers = NULL;
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &SingleClient::CurlHeaderCallback);
        curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, m_resourceStatus);

        auto res = curl_easy_perform(m_curl);

        if (res == CURLE_OK) {
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &m_resourceStatus->ResponseCode);
        }

        std::cout << "Last response: " << GetLastResponseCode() << std::endl;
        std::string s = GetETag();
        std::cout << s.size() <<  " ETag: ***" << s << "***" <<std::endl;
        s = GetAcceptRangesValue();
        std::cout << s.size() << " AcceptRanges: ***" << s << "***" << std::endl;

        for (size_t i = 0; i < (m_resourceStatus->Headers).size(); i++) {
            std::cout << m_resourceStatus->Headers[i].size() << " : H: " << m_resourceStatus->Headers[i];
        }


        return m_resourceStatus;
    }
}