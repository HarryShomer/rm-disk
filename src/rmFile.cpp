#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <tuple>

#include "rmFile.h"


/**
    Run the filefrag cmd and return the starting & ending LBA of the file.
    
    In order to do so we need to parse out the appropriate fields. The values are
    on the 4th line and are entries 5 & 6

    Note: This doesn't work on fragmented files! 

    :return tuple - starting, ending LBA. Return -1, -1 on failure
*/
std::tuple<std::int64_t, std::int64_t> RmFile::getLbas()
{
    std::string result = UTILS_H::exec("filefrag -b512 -v " + fileName);
   
    // First split up cmd output by line - we are looking for the 4th line
    std::vector<std::string> filefragLines = UTILS_H::splitString(result, "\n");
    if(filefragLines.size() < 5) {
        std::cout << "ERROR: FileFrag cmd failed!";
        return {-1, -1};
    }

    // We now split up the 4th line by whitespace 
    // 4th entry us starting lba & the 5th is the ending lba
    std::vector<std::string> line = UTILS_H::splitString(filefragLines[3], "\\s+");
    if(line.size() < 7) {
        std::cout << "ERROR: FileFrag cmd failed!"; 
        return {-1, -1};
    }

    // Remove any crud from starting/ending Lbas 
    line[4].erase(std::remove(line[4].begin(), line[4].end(), '.'), line[4].end());
    line[5].erase(std::remove(line[5].begin(), line[5].end(), ':'), line[5].end());

    return {std::stoi(line[4]), std::stoi(line[5])};
}


/**
*   This function performs the following tasks:
*       1. Sync each
*       2. Clear the cache for the file, see here - https://unix.stackexchange.com/questions/36907/drop-a-specific-file-from-the-linux-filesystem-cache   
*       3. Actually remove the file
*       4. Close the files
*
*   1-3 are only done when the overwrite was a success.
*
*   :param success: Whether the overwrite was successfull. Determine what tasks we do
*
*   :return void
*/
void RmFile::cleanUp(bool success)
{
    if(success) {
        // 1
        fsync(diskFd);
        fsync(fileFd);

        // 2
        std::string cmdResult = UTILS_H::exec("dd of=" + fileName + " oflag=nocache conv=notrunc,fdatasync count=0 2>&1");
        if(cmdResult == "")
            std::cout  << "ERROR: Failed dropping " + fileName + " from the cache\n";

        // 3
        remove(fileName.c_str());
    }

    // 4
    close(diskFd);
    close(fileFd);
}



/**
*   Overwrite the file on the disk 
*
*   :return bool if success or not
*/
bool RmFile::overWrite()
{
    auto [startLba, endLba] = getLbas();

    // Returns -1 as both lbas on error
    if(startLba == -1 && endLba == -1)
        return false;

    // Each block is 512 bytes (specified in filefrag cmd)
    lseek(diskFd, startLba*512, SEEK_SET);

    std::cout << "Starting overwrite for " << fileName << "\n";
    
    for(auto i=0; i <= endLba-startLba; i++) {       
        std::cout << "* Writing to block " << startLba + i << "\n";
        
        std::vector<std::uint8_t> buf = UTILS_H::randomSequence(512);
        if((write(diskFd, buf.data(), buf.size())) == -1) {
            std::cout << "ERROR: Write to disk Failed!\n";
            return false;
        }
    }

    std::cout << "Overwrite Complete!\n\n";
    return true;
}


/**
*  Try deleting the file as long as we were able to read in the file and device.
*  Clean up afterwards which depends on if overwrite
*/
void RmFile::deleteFile()
{
    bool success;

    // Already printed error
    if(diskFd == -1 || fileFd == -1)
        success = false;
    else
        success = overWrite();

    cleanUp(success);
}


