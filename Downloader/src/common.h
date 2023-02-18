#pragma once

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