/**
    Hold all helper functions used
**/
#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <regex>
#include <vector>
#include <algorithm>
#include <random>



/**
    Generate a random sequence of seqLength bytes (therefore btwn 0-255).

    :param seqLength: Length of random sequence to generate
    :param arr: Array to fill 

    :return vector of sequence
*/
inline std::vector<std::uint8_t> randomSequence(int seqLength)
{
    std::random_device rd;
    std::mt19937 eng(rd());  // seed
    std::uniform_int_distribution<> dist(0, 255); 

    std::vector<std::uint8_t> randSeq;
    for(int i=0; i<seqLength; i++) {
        randSeq.push_back(dist(eng)); 
    }

    return randSeq;
}


/**
    Given a string and a delimiter split the string by that delimiter and return
    in the form of a vector

    :param s - String to split
    :param delimiter - delimiter to split by

    :return vector of strings
*/
inline std::vector<std::string> splitString(std::string s, std::string delimiter)
{ 
    std::regex rgx(delimiter);
    std::regex_token_iterator<std::string::iterator> rit(s.begin(), s.end(), rgx, -1);
    std::regex_token_iterator<std::string::iterator> rend;

    std::vector<std::string> rMatches;

    while (rit != rend) {
        rMatches.push_back(*rit);
        rit++;
    }

    return rMatches;
}


/**
    Execute a shell command return the output. Returns "" when fails

    :param cmd: Cmd to execute

    :return cmd output
**/
inline std::string exec(std::string cmd)
{
    char buf[128];
    std::string cmdResult = "";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        pclose(pipe);
        return cmdResult;
    }

    // Read until end of result in chunks of 128
    while (!feof(pipe)) {
        if (fgets(buf, 128, pipe) != NULL)
            cmdResult += buf;
    }

    pclose(pipe);

    return cmdResult;
}


/**
*   Get the device that a file lives on. Done using the 'df' command
*   
*/
inline std::string getDevice(std::string fileName)
{
    std::string cmd = exec("df " + fileName);

    // 2 lines in cmd - we want the 2nd
    std::vector<std::string> cmdLines = splitString(cmd, "\n");
    if(cmdLines.size() != 2) {
        std::cout << "ERROR: Failed getting the device for " + fileName + "\n";
        return NULL;
    }

    // First value on the 2nd line
    std::vector<std::string> cmdVals = splitString(cmdLines[1], "\\s+"); 
    if(cmdVals.size() < 1) {
        std::cout << "ERROR: Failed getting the device for " + fileName + "\n";
        return NULL;
    }

    return cmdVals[0];
}


/**
    Open a given file. Sudo must be used to do this if a disk
  
    :param fileName: Name of file to open

    :return file descriptor
*/
inline int openFile(std::string fileName)
{
    int fd = open(fileName.c_str(), O_RDWR);
    
    if (fd == -1)
        std::cout << "ERROR: Failed reading in " + fileName + "\n";

    return fd;
}

#endif