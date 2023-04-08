#pragma once
#include <Versions.h>

#if defined(EZR_WIN64) // Any windows 32 or 64 bit
    #ifndef EZRESUME_STATIC
        #ifdef EZResume_BUILD_EXPORTS
            #define EZResume_API __declspec(dllexport)
        #else
            #define EZResume_API __declspec(dllimport)
        #endif
    #else
        #define EZResume_API
    #endif
#elif defined(EZR_APPLE)
    #define EZResume_API
#else
    #error "Unknown Apple platform!"
#endif

namespace EZResume
{
    typedef unsigned long ulong;

    /// @brief This scoped enum described the Download info state
    enum class DownloadResult : int
    {
        OK = 0,                             /* Operation finished without errors */
        IDLE,                               /* Download not started */
        DOWNLOADING,                        /* Download is currently in progress */
        COULD_NOT_READ_METAFILE,            /* Unable to open the tmp metafile */
        CORRUPT_METAFILE,                   /* Metafile has been corrupted */
        RESOURCE_MODIFIED,                  /* Source file has changed */
        CANNOT_CREATE_METAFILE,             /* Unable to create metafile. */
        CANNOT_OPEN_METAFILE,               /* Unable to open the tmp metafile */
        CHUNK_SIZE_TOO_SMALL,               /* Currently set chunk size is too small */
        RESOURCE_HAS_ZERO_SIZE,             /* The requested download has a size of 0 */
        DOWNLOADER_NOT_INITIALIZED,         /* Downloader was not correctly initialized */
        DOWNLOADER_EXECUTE_ERROR,           /* Unable to perform the CURL request */
        INVALID_RESPONSE,                   /* Unexpected result HTTP response from the server */
        CORRUPT_CHUNK_CALCULATION,          /* Starting offset is larger than ending offset */
        CANNOT_WRITE_DOWNLOADED_DATA,       /* Was unable to write the chunk data to the download file */
        CANNOT_WRITE_META_DATA,             /* Was unable to write the meta data to the download file */
        CANNOT_RENAME_TEMP_FILE,            /* Failed to rename the tmp metafile after download */
        DESTINATION_FILE_EXISTS,            /* The destination file exists and overwrite flag is not set */
    };

    /// <summary>
    /// Struct used to configure the downloader
    /// </summary>
    struct EZResume_API Configutation
    {
        /*
        * If destination file exists should we overwrite it.
        */
        bool OverwriteIfDestinationExists = false;
        /*
        * If download was stopped and in the meantime the source file changed
        * should we retry from the beginning or return with an error.
        */
        bool ReturnErrorIfSourceChanged = true;
        /*
        * If meta file is corrupt should we silently restart download
        * or return with an error
        */
        bool RestartDownloadIfMetaInfoCorrupt = false;
        /*
        * This is the chunk size for a resumable download. 4 MB chunk is the default. If the requested file is smaller
        * it will be downloaded as a single file otherwise it will be a segmented download with ability to resume.
        */
        unsigned long ChunkSizeInBytes = 4194304; /* 4MB */
    };

    /// <summary>
    /// An Interface to enable users to implement callbacks for the downloader
    /// </summary>
    class EZResume_API IDownloaderHandler {
    public:
        /// <summary>
        /// Will be called when a download has completed.
        /// </summary>
        /// <param name="id">Instance id of the downloader object.</param>
        /// <param name="result">A DownloadResult value for the finished task</param>
        /// <param name="msg">A friendly message</param>
        virtual void DownloadCompleted(const int id, const DownloadResult result, const char* msg) = 0;

        /// <summary>
        /// Called each time a downloaded buffer is processed. Should be used to update progress
        /// </summary>
        /// <param name="id">Instance id of the downloader object.</param>
        /// <param name="totalToDownload">Total size in bytes remaining to download.</param>
        /// <param name="downloadedNow">Number of bytes downloaded in the current stream buffer.</param>
        virtual void DownloadProgress(const int id, const ulong totalToDownload, const ulong downloadedNow) = 0;
    };


    /// <summary>
    /// Main Downloader class used to start a download
    /// </summary>
    class EZResume_API Downloader
    {
    public:
        Downloader();
        virtual ~Downloader();
        Downloader(const Downloader&) = delete;

        /// <summary>
        /// Download a file from a given url using http or https.
        /// </summary>
        /// <param name="id">An instance ID for this downloader given by the user. The user should make sure these are unique when using downloader in multiple threads.</param>
        /// <param name="url">The URL where the source file is to be downloaded from</param>
        /// <param name="filepath">Path where store the downloaded file</param>
        /// <param name="config">Configutation struct for the downloader</param>
        /// <param name="cbh">Pointer to a ccallback handler interface</param>
        /// <param name="funcProgress">Callback for the download progress</param>
        /// <param name="userAgent">Which useragent should we sent to the remote server.</param>
        void Download(
            const int id,
            const char* url,
            const char* filepath,
            EZResume::Configutation config,
            IDownloaderHandler* cbh = nullptr,
            const char* userAgent = nullptr /* Defaults to EzResumeDownloader_version*/
        );

        char* GetVersion(int& length);
    };
}

