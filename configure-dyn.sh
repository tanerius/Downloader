#!/bin/bash
cd "$(dirname "$0")"
rm -rf ./bin
cmake -B bin -DDOWNLOADER_LINK_STATICALLY=Off -DDOWNLOAD_EXTRACT_TIMESTAMP=On 
cd bin