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
#include <plugins.h>

/* This function checks if the given module is in a blocklist.
  *
  * function: __isModuleInTheBlocklist
  * param: moduleName -> the module/plugin name.
  *
*/
bool __isModuleInTheBlocklist(char *_Nonnull __moduleName)
{
    romCustProperty __this = __initializeThisStruct();
    __readProperty(&__this);
    if(!__this.value.__propertyStringValue)
    {
        consoleLog(LOG_LEVEL_DEBUG, "__isModuleInTheBlocklist", "!__this.value.__propertyStringValue");
        __wipeMetadata((void*)&__this);
        return false;
    }
    char *eval = strdup(__this.value.__propertyStringValue);
    char *regexExp = strtok(eval, ",");
    consoleLog(LOG_LEVEL_INFO, "__isModuleInTheBlocklist", "Checking if %s is inside the blocklists...", __moduleName);
    while(!regexExp)
    {
        if(strcmp(regexExp, __moduleName) == 0)
        {
            __wipeMetadata((void*)&__this);
            return true;
        }
        // look for the others:
        regexExp = strtok(NULL, ",");
    }
    __wipeMetadata((void*)&__this);
    return false;
}

/* This function adds a module to the blocklist.
  *
  * function: __addModuleToBlocklist
  * param: moduleName -> the module/plugin name.
  *
*/
void __addModuleToBlocklist(char *_Nonnull __moduleName)
{
    romCustProperty __this = __initializeThisStructWithSize(MAX_STRING_BUFFER_SIZE / 36);
    snprintf(__this.__propertyName, MAX_STRING_BUFFER_SIZE / 36, "str.null.modules.blocklists");
    __readProperty(&__this);
    consoleLog(LOG_LEVEL_INFO, "__addModuleToBlocklist", "Added %s to the blocklists", __moduleName);
    __setProperty(__this.__propertyName, combineStringsFormatted("%s,", __moduleName));
    __wipeMetadata((void*)&__this);
}

/* This function runs the module/plugin after verifying some stuff.
  *
  * function: __runThisModule
  * param: thisModule -> a struct that contains info of the module/plugin
  *
*/
void __runThisModule(void *_Nonnull thisModule)
{
    romPlugins* __thisModule = (romPlugins*)thisModule;
    const char *suffix;
    switch(__thisModule->moduleRunState)
    {
        case INIT: suffix = "init.sh"; break;
        case LATE_FS: suffix = "late-fs.sh"; break;
        case POST_FS: suffix = "post-fs.sh"; break;
        case POST_FS_DATA: suffix = "post-fs-data.sh"; break;
        case BOOT_COMPLETED: suffix = "boot-completed.sh"; break;
    }
    // if the module state is not the current state, skip it now.
    if(__thisModule->moduleRunState != currentState)
    {
        __cleanModuleMetadata((void *)&__thisModule);
        exit(EXIT_SUCCESS);
    } 
    size_t lenOfStr = strlen(__thisModule->pathOfTheModule) + strlen(suffix) + 2;
    char *scriptPath = malloc(lenOfStr);
    if(!scriptPath)
    {
        consoleLog(LOG_LEVEL_ERROR, "__runThisModule", "Failed to run this module due to unknown circumstances, please try again.");
        __cleanModuleMetadata((void *)&__thisModule);
        return;
    }
    snprintf(scriptPath, lenOfStr, "%s/%s", __thisModule->pathOfTheModule, suffix);
    if(checkBlocklistedStringsNChar(scriptPath) != 0)
    {
        consoleLog(LOG_LEVEL_INFO, "__runThisModule", "The script contains blocklisted materials, blocklisting this module.");
        __addModuleToBlocklist(__thisModule->moduleName);
        __freeThisPointer((void**)&scriptPath);
        __cleanModuleMetadata((void *)&__thisModule);
        return;
    }
    /// execute the script and log the output lol.
    if(executeScripts(scriptPath, (char * const[]){scriptPath, NULL}, false) != 0) consoleLog(LOG_LEVEL_ERROR, "__runThisModule", "Failed to run the module script.");
    __freeThisPointer((void**)&scriptPath);
    __cleanModuleMetadata((void *)&__thisModule);
}

/* This function is used to clean a module's metadata.
  *
  * function: __cleanModuleMetadata
  * param: __moduleMetadata__ -> a struct that contains info of the module/plugin
  *
*/
void __cleanModuleMetadata(void *_Nonnull __moduleMetadata__)
{
    romPlugins* __moduleMetadata = (romPlugins*)__moduleMetadata__;
    __moduleMetadata->moduleRunState = 0;
    __moduleMetadata->maxSDK = 0;
    __moduleMetadata->minSDK = 0;
    __freeThisPointer((void **)&__moduleMetadata->moduleName);
    __freeThisPointer((void **)&__moduleMetadata->moduleVersion);
    __freeThisPointer((void **)&__moduleMetadata->moduleAuthor);
    __freeThisPointer((void **)&__moduleMetadata->moduleCOAuthor);
    __freeThisPointer((void **)&__moduleMetadata->pathOfTheModule);
    __freeThisPointer((void **)&__moduleMetadata__);
}

/* This function is used for checking the module/plugin's integrity.
  *
  * function: __verifyAndLogModule
  * param: runnableModule -> a struct that contains info of the module/plugin
  *
*/
void __verifyAndLogModule(void *_Nonnull runnableModule)
{
    int systemSDK = atoi(getSystemProperty("ro.system.build.version.sdk")); // i tried to cast the freaking pointer to an int. What kinda freaking piece of shit coder i am?
    romPlugins* thisInstanceModule = (romPlugins*)runnableModule;
    // init state checks
    if(thisInstanceModule->moduleRunState != LATE_FS && thisInstanceModule->moduleRunState != POST_FS && thisInstanceModule->moduleRunState != POST_FS_DATA)
    {
        consoleLog(LOG_LEVEL_ERROR, "__verifyAndLogModule", "Module doesn't have a valid init state, returning back and verifying other modules...");
        return;
    }
    // sdk checks
    if(systemSDK > thisInstanceModule->maxSDK) 
    {
        consoleLog(LOG_LEVEL_ERROR, "__verifyAndLogModule", "Cannot run module: system SDK is higher than supported.");
        return;
    }
    else if(systemSDK < thisInstanceModule->minSDK) 
    {
        consoleLog(LOG_LEVEL_ERROR, "__verifyAndLogModule", "Cannot run module: system SDK is lower than required.");
        return;
    }
    // log stuffs now:
    consoleLog(LOG_LEVEL_INFO, "__verifyAndLogModule", "Name of the module: %s", thisInstanceModule->moduleName);
    consoleLog(LOG_LEVEL_INFO, "__verifyAndLogModule", "Version of the module: %s", thisInstanceModule->moduleVersion);
    consoleLog(LOG_LEVEL_INFO, "__verifyAndLogModule", "Author of the module: %s", thisInstanceModule->moduleAuthor);
    consoleLog(LOG_LEVEL_INFO, "__verifyAndLogModule", "Co-Author of the module: %s", thisInstanceModule->moduleCOAuthor);
    consoleLog(LOG_LEVEL_INFO, "__verifyAndLogModule", "Module will run shortly! Please wait...");
    __runThisModule((void *)runnableModule);
}

/* This function is used to fetch installed modules.
  *
  * function: __listModulesAndVerifyThem
  * param: arg -> NULL
  *
*/
void *__listModulesAndVerifyThem(void *_Nullable arg)
{
    (void)arg;
    romPlugins *module = calloc(1, sizeof(*module));
    if(!module)
    {
        __cleanModuleMetadata((void *)&module);
        abort_instance("__listModulesAndVerifyThem", "Failed to allocate bytes for module metadata.");
    }
    DIR *baseDirectory = opendir(MODULES_PATH);
    if(!baseDirectory) abort_instance("__listModulesAndVerifyThem", "Failed to open module directory.");
    struct dirent *directories;
    while((directories = readdir(baseDirectory)))
    {
        // skip . and ..
        if(strcmp(directories->d_name, ".") == 0 || strcmp(directories->d_name, "..") == 0) continue;
        // allocate and do misc stuff with these shits:
        module->moduleName = malloc(MAX_MODULE_PROPERTY_NAME);
        module->moduleAuthor = malloc(MAX_MODULE_PROPERTY_AUTHOR);
        module->moduleCOAuthor = malloc(MAX_MODULE_PROPERTY_CO_AUTHOR);
        module->moduleVersion = malloc(MAX_MODULE_PROPERTY_VERSION);
        module->pathOfTheModule = malloc(MAX_MODULE_PATH);
        // check for malloc errors:
        if(!module->moduleName || !module->moduleVersion || !module->moduleAuthor || !module->moduleCOAuthor || !module->pathOfTheModule)
        {
            __cleanModuleMetadata((void *)&module);
            consoleLog(LOG_LEVEL_ERROR, "__listModulesAndVerifyThem", "Failed to allocate blocks for module metadata, skipping this module...");
            continue;
        }
        // necessary variables:
        char moduleProp[MAX_STRING_BUFFER_SIZE / 2];
        snprintf(module->pathOfTheModule, MAX_MODULE_PATH, "%s/%s", MODULES_PATH, directories->d_name);
        snprintf(moduleProp, sizeof(moduleProp), "%s/module.prop", module->pathOfTheModule);
        // let's just grep the module name to see if it's in the blocklist
        module->moduleName = getpropFromFile("name", moduleProp);
        if(__isModuleInTheBlocklist(module->moduleName)) __cleanModuleMetadata((void *)&module);
        module->moduleVersion = getpropFromFile("version", moduleProp);
        module->moduleAuthor = getpropFromFile("author", moduleProp);
        module->moduleCOAuthor = getpropFromFile("co-author", moduleProp);
        module->minSDK = atoi(getpropFromFile("minSDK", moduleProp));
        module->maxSDK = atoi(getpropFromFile("maxSDK", moduleProp));
        module->moduleRunState = atoi(getpropFromFile("runState", moduleProp));
        // run and free these variables after.
        __verifyAndLogModule((void *)module);
        __cleanModuleMetadata((void *)&module);
    }
    closedir(baseDirectory);
    return NULL;
}
