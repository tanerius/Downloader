#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>

#ifdef WIN32 
#define PathSeparator      '\\'
#else
#define PathSeparator      '/'
#endif

#define MaxDownloadThreads 5

#define DEBUG_MODE
