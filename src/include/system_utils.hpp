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
#ifndef SYSTEM_UTILS_HPP
#define SYSTEM_UTILS_HPP
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <cstdarg>
#include <string>

// log levels:
enum elogLevel {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_ABORT
};

// extern vars. VERY IMPORTANT!!
extern std::string coreLogFile;
extern bool useStdoutForAllLogs;

// macro function(s):
#define MAX_STRING_BUFFER_SIZE 1024
#define makeDir(thisDirectory) executeCommands({"mkdir", "-p", thisDirectory}, false);

// function declarations.
int executeCommands(const std::vector<std::string>& commandAndArgs, bool __requiresOutput);
int executeScripts(const std::vector<std::string>& scriptPathAndArgs, bool __requiresOutput);
int searchBlockListedStrings(std::string filename, std::string search_str);
bool containsWithLength(std::string_view str, const char* sub_ptr, size_t sub_len);
bool parseBool(const std::string& value);
bool checkBlocklistedStringsNChar(std::string filename);
bool eraseFile(std::string filename);
std::string getpropFromFile(const std::string& variableName, const std::string& filename);
#endif
