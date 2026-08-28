//
// Copyright (C) 2026 peachpixels <309287626+makimaa-thr1z@users.noreply.github.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <system_utils.h>

/* This function is used to execute shell commands.
  *
  * function: executeCommands
  * param: commandAndArgs -> a vector that contains the command and it's args | requiresOutput -> set it to true if oyu want to log.
  *
*/
int executeCommands(const std::vector<std::string>& commandAndArgs, bool requiresOutput)
{
    pid_t proc = ::fork();
    // console logs:
    switch(proc)
    {
        case -1: {
            // console
            return 1;
        }
        case 0: {
            int zeroFile;
            // throw stderr and stdout to /dev/null if that's the fate,
            if(!requiresOutput) zeroFile = open("/dev/null", O_WRONLY);
            // or throw stderr and stdout to coreLogFile if possible.
            else zeroFile = open(coreLogFile.c_str(), O_WRONLY);
            if(zeroFile == -1) exit(EXIT_FAILURE);
            dup2(zeroFile, STDOUT_FILENO);
            dup2(zeroFile, STDERR_FILENO);
            close(zeroFile);
            std::vector<char*> argv;
            for(auto& s : commandAndArgs) argv.push_back(const_cast<char*>(s.c_str()));
            argv.push_back(nullptr);
            execvp(argv[0], argv.data());
            // console
            return 1;
        }
        default: {
            // console
            int exitStatus;
            wait(&exitStatus);
            return (WIFEXITED(exitStatus)) ? WEXITSTATUS(exitStatus) : 1;
        }
    }
    return -1;
}

/* This function is used to execute shell scripts.
  *
  * function: executeScripts
  * param: scriptPathAndArgs -> a vector that contains the script path and it's args | requiresOutput -> set it to true if oyu want to log.
  *
*/
int executeScripts(const std::vector<std::string>& scriptPathAndArgs, bool requiresOutput)
{
    pid_t proc = fork();
    // console logs:
    switch(proc)
    {
        case -1: {
            // console
            return 1;
        }
        case 0: {
            int zeroFile;
            // throw stderr and stdout to /dev/null if that's the fate,
            if(!requiresOutput) zeroFile = open("/dev/null", O_WRONLY);
            // or throw stderr and stdout to coreLogFile if possible.
            else zeroFile = open(coreLogFile.c_str(), O_WRONLY);
            if(zeroFile == -1) exit(EXIT_FAILURE);
            dup2(zeroFile, STDOUT_FILENO);
            dup2(zeroFile, STDERR_FILENO);
            close(zeroFile);
            std::vector<char*> argv;
            for(auto& s : scriptPathAndArgs) argv.push_back(const_cast<char*>(s.c_str()));
            argv.push_back(nullptr);
            execvp(argv[0], argv.data());
            // console
            return 1;
        }
        default: {
            // console
            int exitStatus;
            wait(&exitStatus);
            return (WIFEXITED(exitStatus)) ? WEXITSTATUS(exitStatus) : 1;
        }
    }
    return -1;
}

/* This function is used to check substrings inside a file
  *
  * function: searchBlockListedStrings
  * param: filename -> the file | search_str -> string to search
  *
*/
int searchBlockListedStrings(std::string filename, std::string search_str)
{
    std::string line;
    std::fstream haystack(filename, std::ios::in);
    while(std::getline(haystack, line))
    {
        if(containsWithLength(line, search_str.c_str(), search_str.length() - 1))
        {
            haystack.close();
            // got em1
            return 0;
        }
    }
    haystack.close();
    return 1;
}

/* This function is a wrapper for .contains.
  *
  * function: checkBlocklistedStringsNChar
  * param: filename -> the file
  *
*/
bool containsWithLength(std::string_view str, const char* sub_ptr, size_t sub_len) 
{
    return str.contains(std::string_view(sub_ptr, sub_len));
}

/* This function parses a string into a boolean value.
 *
 * function: parseBool
 * param: value -> the string value to parse
 *
 * returns: true if the value is "true" or "1", otherwise false
 *
*/
bool parseBool(const std::string& value)
{
    return value == "1" || value == "true";
}

/* This function is used to check if a script contains strings that ressemble a crititcal partition.
  *
  * function: checkBlocklistedStringsNChar
  * param: filename -> the file
  *
*/
bool checkBlocklistedStringsNChar(std::string filename)
{
    std::vector<std::string> blocklistedStrings = {
        "/xbl_config",
        "/fsc",
        "/fsg",
        "/modem",
        "/modemst1",
        "/modemst2",
        "/abl",
        "/keymaster",
        "/sda",
        "/sdb",
        "/sdc",
        "/sdd",
        "/sde",
        "/sdf",
        "/splash",
        "/dtbo",
        "/bluetooth",
        "/cust",
        "/xbl",
        "nvram",
        "/persist",
        "/dev/block/bootdevice/by-name/",
        "/dev/block/by-name/",
        "/dev/block/",
        "/system/bin/dd",
        "/vendor/bin/dd",
        "dd",
        "/dev/block/mmcblk",
        "/dev/mmcblk"
    };
    for(int i = 0; i < blocklistedStrings.size() - 1; i++)
    {
        // found the str:
        if(searchBlockListedStrings(filename, blocklistedStrings[i]) == 0) return true;
    }
    return false;
}

/* This function opens the given file in write mode to erase it's previous content.
  *
  * function: eraseFile
  * param: filename -> the file that is supposed to be erased.
  *
*/
bool eraseFile(std::string filename)
{
    std::fstream file(filename, std::ios::out);
    if(!file.is_open()) return false;
    file.close();
    return true;
}

/* This function gets a specified value from a file.
  *
  * function: getpropFromFile
  * param: variableName -> a string | propFile -> file 
  *
*/
std::string getpropFromFile(const std::string& variableName, const std::string& filename)
{
    std::fstream propFile(filename, std::ios::in);
    if(!propFile.is_open()) return "";
    std::string line;
    while(std::getline(propFile, line))
    {
        if(line.compare(0, variableName.length(), variableName) == 0)
        {
            const std::size_t pos = line.find('=');
            if(pos == std::string::npos) return "";
            return line.substr(pos + 1);
        }
    }
    return "";
}