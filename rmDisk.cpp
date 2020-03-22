#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <regex>
#include <vector>
#include <algorithm>
#include <tuple>
#include <random>


/**
    Generate a random sequence of seqLength bytes (therefore btwn 0-255).

    :param seqLength: Length of random sequence to generate
    :param arr: Array to fill 

    :return vector of sequence
*/
std::vector<std::uint8_t> randomSequence(int seqLength)
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
std::vector<std::string> splitString(std::string s, std::string delimiter)
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
    Execute a shell command return the output

    :param cmd: Cmd to execute

    :return cmd output
**/
std::string exec(std::string cmd)
{
    char buf[128];
    std::string cmdResult = "";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
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
    Run the filefrag cmd and return the starting & ending LBA of the file.
    
    In order to do so we need to parse out the appropriate fields. The values are
    on the 4th line and are entries 5 & 6

    Note: This doesn't work on fragmented files! 

    :param fileName: File to get data for

    :return tuple - starting, ending LBA. Return 0,0 on failure
*/
std::tuple<std::int64_t, std::int64_t> getLbas(std::string fileName)
{
    std::string result = exec("filefrag -b512 -v " + fileName);
   
    // First split up cmd output by line - we are looking for the 4th line
    std::vector<std::string> filefragLines = splitString(result, "\n");
    if(filefragLines.size() < 5) {
        std::cerr << "FileFrag cmd failed!";
        return {0, 0};
    }

    // We now split up the 4th line by whitespace 
    // 4th entry us starting lba & the 5th is the ending lba
    std::vector<std::string> line = splitString(filefragLines[3], "\\s+");
    if(line.size() < 7) {
        std::cerr << "FileFrag cmd failed!"; 
        return {0, 0};
    }

    // Remove any crud from starting/ending Lbas 
    line[4].erase(std::remove(line[4].begin(), line[4].end(), '.'), line[4].end());
    line[5].erase(std::remove(line[5].begin(), line[5].end(), ':'), line[5].end());

    return {std::stoi(line[4]), std::stoi(line[5])};
}



/**
    Open a given file. Sudo must be used to do this if a disk
  
    :param fileName: Name of file to open

    :return file descriptor
*/
int openFile(std::string fileName)
{
    int fd;
    
    std::cerr << "Reading in " << fileName << "...";
    fd = open(fileName.c_str(), O_RDWR);
    
    if (fd == -1)
        std::cout << "Failed!\n";
    else
        std::cout << "Done\n";

    return fd;
}


/**
*   Get the device that a file lives on. Done using the 'df' command
*   
*/
std::string getDevice(std::string fileName)
{
    std::cout << "Getting the device for " + fileName << "...";

    std::string cmd = exec("df " + fileName);

    // 2 lines in cmd - we want the 2nd
    std::vector<std::string> cmdLines = splitString(cmd, "\n");
    if(cmdLines.size() != 2) {
        
        std::cerr << "Failed!\n";
        return "";
    }

    // First value on the 2nd line
    std::vector<std::string> cmdVals = splitString(cmdLines[1], "\\s+"); 
    if(cmdVals.size() < 1) {
        std::cerr << "Failed!\n";
        return "";
    }

    std::cout << "Done\n";

    return cmdVals[0];
}


/**
*   Overwrite the file on the disk 
*
*   :param diskFd: File Descriptor of disk
*   :param fileName: Name of file we are overwriting
*
*   :return  bool if success or not
*/
bool overWriteDevice(int diskFd, std::string fileName)
{
    auto [startLba, endLba] = getLbas(fileName);

    // Returns 0 as both lbas on error
    if(startLba == 0)
        return false;
    
    std::vector<std::uint8_t> buf;

    // Each block is 512 bytes (specified in filefrag cmd)
    lseek(diskFd, startLba*512, SEEK_SET);

    std::cout << "Starting overwrite for " << fileName << "\n";
    
    for(auto i=0; i <= endLba-startLba; i++) {       
        std::cout << "* Writing to block " << startLba + i << "\n";
        
        buf = randomSequence(512);
        if((write(diskFd, buf.data(), buf.size())) == -1) {
            std::cerr << "Write to disk Failed!\n";
            return false;
        }
    }

    std::cout << "Overwrite Complete!\n\n";
    return true;
}


/**
*   This function performs the following tasks:
*       - Sync each
*       - Clear the cache for the file, see here - https://unix.stackexchange.com/questions/36907/drop-a-specific-file-from-the-linux-filesystem-cache   
*       - Actually remove the file
*       - Close the files
*
*   :param diskFd - File descriptor of disk
*   :param fileFd - idk why
*   :param fileName - Name of file 
*
*   :return void
*/
void cleanUp(int diskFd, int fileFd, std::string fileName)
{
    std::cout << "Syncing...";
    fsync(diskFd);
    fsync(fileFd);
    std::cout << "Done\n";

    std::cout << "Clearing Cache for " + fileName + "...";
    exec("dd of=" + fileName + " oflag=nocache conv=notrunc,fdatasync count=0 2>&1");
    std::cout << "Done\n";

    std::cout << "Removing the Inode for " + fileName + "...";
    remove(fileName.c_str());
    std::cout << "Done\n";

    std::cout << "Closing...";
    close(diskFd);
    close(fileFd);
    std::cout << "Done\n";
}


int main(int argc, char *argv[])
{
    if (argc != 2) {
	std::cerr << "Usage: ./rm-disk <file>\n";
	return 1;
    }

    // Returns nothing on failure
    std::string device = getDevice(argv[1]);
    if(device == "")
        return 1;

    int fileFd = openFile(argv[1]); 
    int diskFd = openFile(device);
    std::cout << "\n";

    // Both need to be valid to run. If not just close both
    if(diskFd != -1 && fileFd != -1) { 
        overWriteDevice(diskFd, argv[1]);    
        cleanUp(diskFd, fileFd, argv[1]);
    } else {
        close(fileFd);
        close(diskFd);
    }
}


