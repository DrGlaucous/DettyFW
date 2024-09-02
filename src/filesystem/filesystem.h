#pragma once

#include <Arduino.h>

namespace filesystem{

    bool listDir(const char * dirname, uint8_t levels);

    bool createDir(const char * path);

    bool removeDir(const char * path);

    bool readFile(const char * path, uint8_t** buffer, size_t* size);

    void readFile2(const char * path);

    bool writeFile(const char * path, const uint8_t * buffer, size_t size);

    bool appendFile(const char * path, const uint8_t * buffer, size_t size);

    bool renameFile(const char * path1, const char * path2);

    bool deleteFile(const char * path);

    bool initialize();

};

