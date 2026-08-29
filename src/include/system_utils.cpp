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
#include <system_utils.hpp>

/* This function is used to execute shell commands.
  *
  * function: executeCommands
  * param: commandAndArgs -> a vector that contains the command and it's args | requiresOutput -> set it to true if oyu want to log.
  *
*/
int executeCommands(const std::vector<std::string>& commandAndArgs, bool requiresOutput)
{
    pid_t proc = ::fork();
    switch(proc)
    {
        case FORK_FAILED: {
            consoleLog(LOG_LEVEL_ERROR, "executeCommands", "Failed to fork a child process.");
            return 1;
        }
        case FORK_SUCCESS: {
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
            consoleLog(LOG_LEVEL_ERROR, "executeCommands", "Failed to execute command: {}", argv[0]);
            return 1;
        }
        default: {
            consoleLog(LOG_LEVEL_DEBUG, "executeCommands", "Waiting for {} to finish it's process.", commandAndArgs[0]);
            int exitStatus;
            wait(&exitStatus);
            consoleLog(LOG_LEVEL_DEBUG, "executeCommands", "This instance returns {} normally", exitStatus);
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
    switch(proc)
    {
        case FORK_FAILED: {
            consoleLog(LOG_LEVEL_ERROR, "executeScripts", "Failed to fork process.");
            return 1;
        }
        case FORK_SUCCESS: {
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
            consoleLog(LOG_LEVEL_ERROR, "executeScripts", "Failed to execute {}", argv[0]);
            return 1;
        }
        default: {
            consoleLog(LOG_LEVEL_DEBUG, "executeScripts", "Waiting for script to finish it's process.");
            int exitStatus;
            wait(&exitStatus);
            consoleLog(LOG_LEVEL_DEBUG, "executeCommands", "This instance returns {} normally", exitStatus);
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
            consoleLog(LOG_LEVEL_INFO, "searchBlockListedStrings", "Expected string found in given file: {}", filename);
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
    for(std::size_t i = 0; i < blocklistedStrings.size() - 1; i++)
    {
        if(searchBlockListedStrings(filename, blocklistedStrings[i]) == 0) 
        {
            consoleLog(LOG_LEVEL_INFO, "checkBlocklistedStringsNChar", "Found Blocklisted string: {}", blocklistedStrings[i]);
            consoleLog(LOG_LEVEL_ERROR, "checkBlocklistedStringsNChar", "The script is not safe to execute! Stopping execution process...");
            return true;
        }
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

/* This function returns the elogLevel type as a whole string;
  *
  * function: logType
  * param: enum elogLevel -> type -> enum that contains the log types:
  *
*/
std::string logType(elogLevel type)
{
    switch (type)
    {
        case LOG_LEVEL_INFO: return "INFO: ";
        case LOG_LEVEL_WARN: return "WARN: ";
        case LOG_LEVEL_ERROR: return "ERROR: ";
        case LOG_LEVEL_DEBUG: return "DEBUG: ";
        case LOG_LEVEL_ABORT: return "ABORT: ";
    }
}

/* This function logs/outputs a string to the log file/console
  *
  * function: consoleLog
  * param: loglevel -> enum elogLevel > log level | service -> a function name 
  *
*/
template <typename... Args>
void consoleLog(elogLevel type, const std::string& service, const std::string& message, Args&&... extras)
{
    if(useStdoutForAllLogs)
    {
        if(type == LOG_LEVEL_DEBUG) return;
        std::cout << logType(type) << service << ": " << std::vformat(message, std::make_format_args(extras...)) << '\n';
    } 
    else 
    {
        std::fstream cloggg(coreLogFile, std::ios::app);
        if(!cloggg) return;
        cloggg << logType(type) << service << ": " << std::vformat(message, std::make_format_args(extras...)) << '\n';
        cloggg.close();
    }
}

void consoleLog(elogLevel type, const std::string& service, const std::string& message)
{
    if(useStdoutForAllLogs)
    {
        if(type == LOG_LEVEL_DEBUG) return;
        std::cout << logType(type) << service << ": " << message << '\n';
    }
    else 
    {
        std::fstream cloggg(coreLogFile, std::ios::app);
        if(!cloggg) return;
        cloggg << logType(type) << service << ": " << message << '\n';
        cloggg.close();
    }
}
