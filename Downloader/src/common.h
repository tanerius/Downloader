#pragma once
#include <thread>
#include <mutex>

#ifdef WIN32 
#define PathSeparator      '\\'
#else
#define PathSeparator      '/'
#endif

#define MaxDownloadThreads 5

#define DEBUG_MODE
