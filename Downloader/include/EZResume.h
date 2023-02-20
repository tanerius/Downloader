#pragma once
#include <Versions.h>
#if defined(EZR_WIN64) // Any windows 32 or 64 bit
#if !defined(EZRESUME_STATIC)
#if defined(EZResume_BUILD_EXPORTS)
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
    };

    class EZResume_API Downloader
    {
    public:
        Downloader();
        virtual ~Downloader();
        Downloader(const Downloader&) = delete;

        /// <summary>
        /// Download a file from a given url using http or https.
        /// </summary>
        /// <param name="url">The URL where the source file is to be downloaded from</param>
        /// <param name="filepath">Path where store the downloaded file</param>
        /// <param name="config">Configutation struct for the downloader</param>
        /// <param name="funcCompleted">Callback when the download is finished</param>
        /// <param name="funcProgress">Callback for the download progress</param>
        /// <param name="chunkSizeInBytes">How big should a download chunk be. Must be multiple of 1024</param>
        /// <param name="userAgent">Which useragent should we sent to the remote server.</param>
        void download(
            const char* url,
            const char* filepath,
            EZResume::Configutation config,
            void (*funcCompleted)(int, const char*),
            void (*funcProgress)(unsigned long totalToDownload, unsigned long downloadedNow) = nullptr,
            const unsigned long chunkSizeInBytes = 4194304, /* 4MB */
            const char* userAgent = nullptr /* Defaults to EzResumeDownloader_version*/
        );

        void TestDownload();
    };
}

