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
#ifndef PLUGINS_H
#define PLUGINS_H
#ifndef NULL_H
#include <null.h>
#endif
#ifndef NULL_CUSTOM_PROPS_H
#include <null_custom_props.h>
#endif
#include <dirent.h>

// max prop length:
#define MAX_MODULE_PROPERTY_NAME 50
#define MAX_MODULE_PROPERTY_VERSION 16 
#define MAX_MODULE_PROPERTY_AUTHOR 20
#define MAX_MODULE_PROPERTY_CO_AUTHOR 64 
#define MAX_MODULE_PATH 512
#define MODULES_PATH "/data/adb/nullex/modules"

// extern variables
extern int currentState;

// module identifier.
typedef struct {
	int moduleRunState;
	int maxSDK;
	int minSDK;
	char *_Nonnull moduleName;
	char *_Nonnull moduleVersion;
    char *_Nonnull moduleAuthor;
    char *_Nullable moduleCOAuthor;
	char *_Nonnull pathOfTheModule;
} romPlugins;

// one hell of a junk.
enum bootTraceState {
	LATE_FS,
	INIT,
	POST_FS,
	POST_FS_DATA,
	BOOT_COMPLETED,
};

// function declarations:
bool __isModuleInTheBlocklist(char *_Nonnull moduleNameAuthor);
void __addModuleToBlocklist(char*_Nonnull  moduleName);
void __runThisModule(void *_Nonnull thisModule);
void __cleanModuleMetadata(void *_Nonnull __moduleMetadata__);
void __cleanPointers();
void __verifyAndLogModule(void *_Nonnull runnableModule);
void *_Nullable __listModulesAndVerifyThem(void *_Nullable arg);
#endif
