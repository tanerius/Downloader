#pragma once
#include "common.h"
#include <string>
#include <curl/curl.h>

namespace DownloaderLib {

    typedef std::function<void(int)> DownloaderCallback;
    static void staticRunNextTask(); // for thread start

    class Downloader
    {
    public:
        static Downloader* instance();
        virtual ~Downloader();

        /**
         Setup a callback function. When all tasks are done, call it.
         */
        void setCompletedCallback(DownloaderCallback callback);

        /**
         Download a file from url.
         @return: task ID. If failed, return -1
         */
        int download(const char* url, const char* folder);
        int download(const char* url, const char* folder, DownloaderCallback callback);
        int download(const char* url, const char* folder, const char* filename);
        int download(const char* url, const char* folder, const char* filename, DownloaderCallback callback);

        void runNextTask(); // for download the next task in the waiting list

    protected:
        /**
         Description of a download task
         */
        struct Task
        {
            int id;
            std::string url;
            std::string folder;
            std::string filename;
            DownloaderCallback callback;

            Task() : id(0) {}
        };

    private:
        Downloader(); // for singleton
        void callCallbackSafely(DownloaderCallback callback, int); // make sure the callbacks are executed one by one

    private:
        int m_lastId;

        std::queue<Task> m_waiting;
        static std::mutex s_waiting;

        std::unordered_map<int, std::thread::id> m_running;
        static std::mutex s_running;

        int m_threadCount;
        static std::mutex s_threadCount;

        DownloaderCallback m_onCompleted;
        static std::mutex s_callback;

    };
}
