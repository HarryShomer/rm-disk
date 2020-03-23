#include <iostream>
#include <string>
#include <vector>

#include "rmFile.h"


void printUsage()
{
    std::cout << "Usage: ./rm-disk <list of files>\n";
    std::cout << "       ./rm-disk -r <list of directories>\n";
}


/**
*   List all the files for the various directories given. Does so recursively
*
*   :param argv: dirs are entries 2-end
*   :param numDir: number of directories given
*
*   :return vector of files
*/
std::vector<std::string> listDirFiles(char *argv[], int numDirs)
{
    std::vector<std::string> allFiles;

    for(int i=0; i < numDirs; i++) {
        // Each file is on its own line
        std::string dirResults = UTILS_H::exec("find " + std::string(argv[2+i]) + " -type f");
        std::vector<std::string> dirFiles = UTILS_H::splitString(dirResults, "\n");

        allFiles.insert(allFiles.end(), dirFiles.begin(), dirFiles.end());
    }


    return allFiles;
}


/**
*   Go through the files on by one and delete them
*
*   :param files: List of files to delete. Include relative path
*
*   :return void
*/
void deleteFiles(std::vector<std::string> files)
{
    for(auto file : files) {
        RM_FILE_H::RmFile fileToRemove(file);
        fileToRemove.deleteFile();
    }
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        printUsage();
        return 1;
    }

    // Know at least 2 args...valid for file but not for directories
    if (strcmp(argv[1], "-r") == 0 && argc < 3) {
        std::cout << "No directories specified!\n";
        printUsage();
        return 1;
    }

    std::vector<std::string> files;

    // Files are either derived from the dirs given or the args themselves
    if (strcmp(argv[1], "-r") == 0)
        files = listDirFiles(argv, argc-2);
    else
        files = std::vector<std::string>(argv + 1, argv + argc);

    deleteFiles(files);
}


