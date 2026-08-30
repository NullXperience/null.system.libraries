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
#ifndef SYSTEM_PLUGINS_HPP
#define SYSTEM_PLUGINS_HPP
#ifndef SYSTEM_NULL_HPP
#include <system_null.h>
#endif
#ifndef SYSTEM_CUSTOM_PROPS_HPP
#include <system_custom_props.h>
#endif
#define MODULES_PATH "/data/adb/nullex/modules"
#define MODULE_BLOCKLIST_PROP "str.null.modules.blocklists"

// extern variables
extern int currentState;

// module identifier.
typedef struct {
	int minSDK;
    int maxSDK;
	int runState;
    std::string name;
    std::string version;
    std::string author;
    std::string co_author;
    std::string pathOfTheModule;
} romPlugins;

// one hell of a junk.
enum bootTraceState {
	LATE_FS,
	INIT,
	POST_FS,
	POST_FS_DATA,
	BOOT_COMPLETED,
};

// functions lol
bool isModuleInTheBlocklist(std::string& moduleName);
void addModuleToTheBlocklist(std::string& moduleName);
void runThisModule(void *thisModule);
void verifyAndLogModule(void *runnableModule);
void *listModulesAndVerifyThem(void *args);
#endif
