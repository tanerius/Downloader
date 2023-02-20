#pragma once
#include <EZResume.h>

#ifdef WIN32 
#define PathSeparator      '\\'
#else
#define PathSeparator      "/"
#endif

#ifdef DEBUG 
#define DLOG(x) (std::cout << x << std::endl)
#else 
#define DLOG(x)
#endif