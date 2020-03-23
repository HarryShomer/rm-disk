#ifndef RM_FILE_H
#define RM_FILE_H

#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <tuple>
#include "utils.h"


class RmFile
{
public:
    std::string fileName;
    int fileFd;
    int diskFd;

    RmFile(std::string fn)
    {
        this->fileName = fn;
        this->fileFd = UTILS_H::openFile(fn);
        this->diskFd = UTILS_H::openFile(UTILS_H::getDevice(fn));
    }

    std::tuple<std::int64_t, std::int64_t> getLbas();
    void cleanUp(bool success);
    bool overWrite();
    void deleteFile();
};
 

#endif