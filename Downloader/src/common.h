#pragma once

namespace DownloaderLib
{
    struct Configutation
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
}

#ifdef WIN32 
#define PathSeparator      '\\'
#else
#define PathSeparator      "/"
#endif

#define MaxDownloadThreads 5


#define DEBUG

#ifdef DEBUG 
#define DLOG(x) (std::cout << x << std::endl)
#else 
#define DLOG(x)
#endif