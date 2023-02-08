#include "singleclient.h"
#include <filesystem>
#include <ctime>
#include <iostream>

namespace DownloaderLib
{

    SingleClient::SingleClient()
    {
        initCURL();
    }

    SingleClient::SingleClient(std::string agent) : SingleClient()
    {
        curl_easy_setopt(m_curl, CURLoption::CURLOPT_USERAGENT, agent.c_str());
    }


    SingleClient::~SingleClient()
    {
        curl_easy_cleanup(m_curl);
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

    std::string SingleClient::post(const std::string& url, const std::string& data)
    {
        struct curl_slist* slist1;
        slist1 = NULL;
        slist1 = curl_slist_append(slist1, "Content-Type: application/json");


        // set url
        curl_easy_setopt(m_curl, CURLoption::CURLOPT_URL, url.c_str());

        // forward all data to this func
        curl_easy_setopt(m_curl, CURLoption::CURLOPT_WRITEFUNCTION, &SingleClient::writeToString);
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, slist1);

        std::string response;

        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response);

        // do it
        curl_easy_perform(m_curl);

        return response;
        // curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    }

    std::string SingleClient::get(std::string url)
    {
        // set url
        curl_easy_setopt(m_curl, CURLoption::CURLOPT_URL, url.c_str());

        // forward all data to this func
        curl_easy_setopt(m_curl, CURLoption::CURLOPT_WRITEFUNCTION, &SingleClient::writeToString);

        std::string response;

        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response);

        // do it
        curl_easy_perform(m_curl);

        return response;
        // curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    }

    bool SingleClient::download(const char* url, const char* folder)
    {
        // try guess filename from the url
        std::string inturl = url;
        std::string intfolder = folder;
        auto pos = std::find(inturl.rbegin(), inturl.rend(), '/');
        std::string name = inturl.substr(std::distance(pos, inturl.rend()));

        if (intfolder.back() != PathSeparator) intfolder += PathSeparator;

        return downloadAs(inturl.c_str(), (intfolder + name).c_str());
    }

    bool SingleClient::downloadAs(const char* url, const char* filepath)
    {
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

            // do it
            CURLcode result = curl_easy_perform(m_curl);

            std::fclose(file);

            // get HTTP response code
            int responseCode;
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);

            if (result == CURLE_OK && responseCode == 200) // only assume 200 is correct
            {
                // check if file exists if so remove it
                std::remove(filepath);

                // rename it to real filename which does a move of the file so no need for explicit deletion
                auto ret = std::rename(tmpFile.c_str(), filepath);
                return (ret == 0);
            }

            std::remove(tmpFile.c_str()); // clean tmp file if non zero rename
        }

        return false;
    }

    size_t SingleClient::writeToFile(void* ptr, size_t size, size_t nmemb, FILE* stream)
    {
        return fwrite(ptr, size, nmemb, stream);
    }

    size_t SingleClient::writeToString(char* ptr, size_t size, size_t nmemb, std::string* sp)
    {
        size_t len = size * nmemb;
        sp->insert(sp->end(), ptr, ptr + len);
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
        // set a default user agent
        curl_easy_setopt(m_curl, CURLOPT_USERAGENT, curl_version());
        // prevent ending up in an endless redirection 
        curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 50L);

#ifdef DEBUG_MODE
        // Switch on full protocol/debug output while testing
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

        // disable progress meter, set to 0L to enable and disable debug output
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 1L);
#endif
    }
}