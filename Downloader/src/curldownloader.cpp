#include "curldownloader.h"
#include "singleclient.h"


namespace DownloaderLib {

    std::mutex Downloader::s_waiting;
    std::mutex Downloader::s_running;
    std::mutex Downloader::s_callback;
    std::mutex Downloader::s_threadCount;

    void staticRunNextTask()
    {
        // instance method instead of static
        Downloader::instance()->runNextTask();
        return;
    }

    Downloader* Downloader::instance()
    {
        static Downloader* s_instance = nullptr;

        if (s_instance == nullptr)
        {
            s_instance = new Downloader;
        }

        return s_instance;
    }

    Downloader::Downloader() 
        : m_lastId(0), 
        m_threadCount(0), 
        m_onCompleted([](int) {})
    {

    }

    Downloader::~Downloader()
    {
    }

    void Downloader::setCompletedCallback(DownloaderCallback callback)
    {
        m_onCompleted = callback;
    }

    int Downloader::download(const char* url, const char* folder)
    {
        return download(url, folder, "");
    }

    int Downloader::download(const char* url, const char* folder, DownloaderCallback callback)
    {
        return download(url, folder, "", callback);
    }

    int Downloader::download(const char* url, const char* folder, const char* filename)
    {
        return download(url, folder, filename, [](int) {});
    }

    int Downloader::download(const char* url, const char* folder, const char* filename, DownloaderCallback callback)
    {
        Task task;
        task.id = ++m_lastId;
        task.url = strdup(url);
        task.folder = strdup(folder);
        task.filename = (filename == nullptr) ? nullptr : strdup(filename);
        task.callback = callback;
        
        // check thread count
        bool canCreateThread = false;

        s_threadCount.lock();
        if (m_threadCount < MaxDownloadThreads)
        {
            canCreateThread = true;
            ++m_threadCount;
        }
        s_threadCount.unlock();

        if (canCreateThread)
        {
            // create new thread to execute the task
            std::thread t(staticRunNextTask);
            t.detach();
        }

        return task.id;
    }

    const int Downloader::stringSize(const char* str)
    {
        if (str == nullptr) return 0;
        int size = 0;
        while (str[size] != '\0') size++;
        return size;
    }

    // caution: this method will be called in threads!
    void Downloader::runNextTask()
    {
        SingleClient client;

        // alwasy looking for new task to execute
        while (true)
        {
            Task task;

            // try to fetch next task
            s_waiting.lock();
            if (!m_waiting.empty())
            {
                task = m_waiting.front();
                m_waiting.pop();
            }
            s_waiting.unlock();

            if (task.id == 0) break; // no new task

            // remember it
            s_running.lock();
            m_running[task.id] = std::this_thread::get_id();
            s_running.unlock();

            // use httpclient to download it
            if (stringSize(task.filename) == 0)
            {
                client.download(task.url.c_str(), task.folder.c_str());
            }
            else
            {
                client.downloadAs(task.url.c_str(), (task.folder + PathSeparator + task.filename).c_str());
            }

            // remove me from the running list
            s_running.lock();
            m_running.erase(task.id);
            s_running.unlock();

            callCallbackSafely(task.callback, task.id);
        }

        // at last, minus thread count
        s_threadCount.lock();
        bool allDone = --m_threadCount == 0;
        s_threadCount.unlock();

        if (allDone) callCallbackSafely(m_onCompleted, 0);
    }

    void Downloader::callCallbackSafely(DownloaderCallback callback, int p)
    {
        if (m_onCompleted != nullptr)
        {
            const std::lock_guard<std::mutex> lock(s_callback);
            callback(p);
        }
    }
}