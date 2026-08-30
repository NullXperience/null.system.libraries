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
#include <system_plugins.hpp>

/* This function checks if the given module is in a blocklist.
  *
  * function: isModuleInTheBlocklist
  * param: moduleName -> the module/plugin name.
  *
*/
bool isModuleInTheBlocklist(std::string& moduleName)
{
    romCustProperty romcust;
    romcust.propertyName = MODULE_BLOCKLIST_PROP;
    props_null_readProperty(&romcust);
    if(romcust.propertyValue.contains(moduleName)) return true;
    return false;
}

/* This function adds a module to the blocklist.
  *
  * function: addModuleToBlocklist
  * param: moduleName -> the module/plugin name.
  *
*/
void addModuleToTheBlocklist(std::string& moduleName)
{
    std::string modVal = moduleName + ",";
    props_null_setProperty("str.null.modules.blocklists", modVal);
}

/* This function runs the module/plugin after verifying some stuff.
  *
  * function: runThisModule
  * param: thisModule -> a struct that contains info of the module/plugin
  *
*/
void runThisModule(void *thisModule)
{
    romPlugins* moduleGiven = static_cast<romPlugins*>(thisModule);
    std::string scriptSuffix;
    switch(moduleGiven->runState)
    {
        case INIT: scriptSuffix = "init.sh"; break;
        case LATE_FS: scriptSuffix = "late-fs.sh"; break;
        case POST_FS: scriptSuffix = "post-fs.sh"; break;
        case POST_FS_DATA: scriptSuffix = "post-fs-data.sh"; break;
        case BOOT_COMPLETED: scriptSuffix = "boot-completed.sh"; break;
    }
    if(moduleGiven->runState != currentState) exit(EXIT_SUCCESS);
    std::string modScriptPath = moduleGiven->pathOfTheModule + "/" + scriptSuffix;
    if(checkBlocklistedStringsNChar(scriptPath)) 
    {
        consoleLog(LOG_LEVEL_INFO, "runThisModule", "The script contains blocklisted materials, blocklisting this module.");
        return;
    }
    if(executeScripts({modScriptPath}) != 0) consoleLog(LOG_LEVEL_ERROR, "__runThisModule", "Failed to run the module script.");
}

/* This function is used for checking the module/plugin's integrity.
  *
  * function: verifyAndLogModule
  * param: runnableModule -> a struct that contains info of the module/plugin
  *
*/
void verifyAndLogModule(void *runnableModule)
{
    int systemSDK = std::stoi(system_null_getSystemSettings("ro.system.build.version.sdk"));
    romPlugins* moduleGiven = static_cast<romPlugins*>(runnableModule);
    // init state checks
    if(moduleGiven->runState != LATE_FS && moduleGiven->runState != POST_FS && moduleGiven->runState != POST_FS_DATA)
    {
        consoleLog(LOG_LEVEL_ERROR, "verifyAndLogModule", "Module doesn't have a valid init state, returning back and verifying other modules...");
        return;
    }
    // sdk checks
    if(systemSDK > moduleGiven->maxSDK) 
    {
        consoleLog(LOG_LEVEL_ERROR, "verifyAndLogModule", "Cannot run module: system SDK is higher than supported.");
        return;
    }
    else if(systemSDK < moduleGiven->minSDK) 
    {
        consoleLog(LOG_LEVEL_ERROR, "verifyAndLogModule", "Cannot run module: system SDK is lower than required.");
        return;
    }
    // log stuffs now:
    consoleLog(LOG_LEVEL_INFO, "verifyAndLogModule", "Name of the module: {}", moduleGiven->moduleName);
    consoleLog(LOG_LEVEL_INFO, "verifyAndLogModule", "Version of the module: {}", moduleGiven->moduleVersion);
    consoleLog(LOG_LEVEL_INFO, "verifyAndLogModule", "Author of the module: {}", moduleGiven->moduleAuthor);
    consoleLog(LOG_LEVEL_INFO, "verifyAndLogModule", "Co-Author of the module: {}", moduleGiven->moduleCOAuthor);
    consoleLog(LOG_LEVEL_INFO, "verifyAndLogModule", "Module will run shortly! Please wait...");
}

/* This function is used to fetch installed modules.
  *
  * function: listModulesAndVerifyThem
  * param: arg -> NULL
  *
*/
void *listModulesAndVerifyThem(void *args)
{
    (void)args;
    romPlugins modules;
    std::filesystem::path dirToSearch = MODULES_PATH;
    try {
        if(std::filesystem::exists(dirToSearch) && 
           std::filesystem::is_directory(dirToSearch))
        {
            for(const auto& dirs : std::filesystem::directory_iterator(dirToSearch))
            {
                if(dirs.path().filename() == "." || dirs.path().filename() == "..") continue;
                std::string moduleProp = MODULES_PATH + "/" + dirs.path().filename().string();
                modules.name = getpropFromFile("name", moduleProp);
                modules.author = getpropFromFile("author", moduleProp);
                modules.co_author = getpropFromFile("co-author", moduleProp);
                modules.minSDK = std::stoi(getpropFromFile("minSDK", moduleProp));
                modules.maxSDK = std::stoi(getpropFromFile("maxSDK", moduleProp));
                modules.runstate = std::stoi(getpropFromFile("runState", moduleProp));
                verifyAndLogModule(static_cast<void*>(modules));
            }
        }
    }
    catch(const fs::filesystem_error& e) 
    {
        consoleLog(LOG_LEVEL_ERROR, "listModulesAndVerifyThem", e.what());
    }
    return nullptr;
}
