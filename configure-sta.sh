#!/bin/bash
cd "$(dirname "$0")"
rm -rf ./bin
cmake -B bin -DDOWNLOAD_EXTRACT_TIMESTAMP=On 
cd bin