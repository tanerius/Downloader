#include "singleclient.h"



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
    }

    bool SingleClient::download(std::string url, std::string folder)
    {
        // try guess filename from the url
        auto pos = find(url.rbegin(), url.rend(), '/');
        std::string name = url.substr(distance(pos, url.rend()));

        if (folder.back() != PathSeparator) folder += PathSeparator;

        return downloadAs(url, folder + name);
    }

    bool SingleClient::downloadAs(std::string url, std::string filepath)
    {
        // set url
        curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());

        // forward all data to this func
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &SingleClient::writeToFile);

        // try to write a temp file instead of the real file
        std::string tmpFile = filepath + ".tmp";

        // open the file
        FILE* file = nullptr;
        auto res = fopen_s(&file, tmpFile.c_str(), "wb");

        if (file && (res == 0))
        {
            // write the page body to this file handle. CURLOPT_FILE is also known as CURLOPT_WRITEFILE
            curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, file);

            // do it
            CURLcode result = curl_easy_perform(m_curl);

            fclose(file);

            // get HTTP response code
            int responseCode;
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);

            if (result == CURLE_OK && responseCode == 200) // only assume 200 is correct
            {
                // rename it to real filename
                auto ret = rename(tmpFile.c_str(), filepath.c_str());
                return (ret == 0);
            }

            remove(tmpFile.c_str()); // clean tmp file
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

#ifdef DEBUG_MODE
        // Switch on full protocol/debug output while testing
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

        // disable progress meter, set to 0L to enable and disable debug output
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 1L);
#endif
    }
}