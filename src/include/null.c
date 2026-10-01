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
#include <null.h>

/* This function takes a string and then it looks through the package list reported by android
   to check if the app exists or not. Returns 0 if it does.
  *
  * function: isPackageInstalled
  * param: packageName -> as the name tells.
  *
*/
int isPackageInstalled(const char *_Nonnull packageName)
{
    FILE *fptr = popen("pm list packages | cut -d ':' -f 2", "r");
    if(!fptr) return -1;
    char string[MAX_STRING_BUFFER_SIZE];
    while(fgets(string, MAX_STRING_BUFFER_SIZE, fptr))
    {
        // let's just compare it to the whole string but not with the newline char.
        if(strncmp(packageName, string, strlen(packageName) - 1) == 0)
        {
            fclose(fptr);
            return 0;
        }
    }
    fclose(fptr);
    return 1;
}

/* This function returns the battery percentage of the device.
  *
  * function: getBatteryPercentage
  *
*/
int getBatteryPercentage()
{
    for(int i = 0; !batteryPercentageBlobFilePaths[i]; i++)
    {
        if(access(batteryPercentageBlobFilePaths[i], F_OK) == 0)
        {
            batteryPercentageBlobIndex = i;
            break;
        }
    }
    FILE *batteryBlob = fopen(batteryPercentageBlobFilePaths[batteryPercentageBlobIndex], "r");
    // return -1 if we cannot open the file.
    if(!batteryBlob) return -1;
    // uh, 5-6 should be more than enough i guess...
    char percent[6];
    fgets(percent, sizeof(percent), batteryBlob);
    percent[strcspn(percent, "\n")] = '\0';
    fclose(batteryBlob);
    return atoi(percent);
}

/* This function does mentioned stuff to a property. Like removing it, or just changing the value if the
   prop has a different value.
  *
  * function: manageProperty
  * param: property -> name of the prop | expressivePropertyValue -> prop value | propertyState -> enum propertyTinkerState
  *
*/
bool manageProperty(char *_Nonnull property, void*_Nullable expressivePropertyValue, enum propertyTinkerState propertyState)
{
    char* currentValue = strdup(getSystemProperty(property));
    if(!currentValue) return -1;
    switch(propertyState)
    {
        case MAYBE_SETPROP:
            if(!expressivePropertyValue) return false;
            if(strcmp(currentValue, (char *)expressivePropertyValue) != 0) return (executeCommands(resetprop, (char* const[]){ resetprop, property, (char *)expressivePropertyValue, NULL }, false) == 0);
        case SET_IF_DIFF:
            if(!expressivePropertyValue) return false;
            if(strcmp(currentValue, (char *)expressivePropertyValue) == 0) return false;
            else return (executeCommands(resetprop, (char* const[]){resetprop, property, (char *)expressivePropertyValue, NULL}, false) == 0);
        case REMOVE_PROPERTY:
            return (executeCommands(resetprop, (char *const[]){resetprop, "-d", property}, false) == 0);
        case SETPROP:
            if(!expressivePropertyValue) return false;
            return (executeCommands(resetprop, (char *const[]) {resetprop, property, (char *)expressivePropertyValue, NULL}, false) == 0);
        case MATCHED_WITH_EXPECTED:
            if(!expressivePropertyValue) return false;
            return (strcmp(getSystemProperty(property), (char *)expressivePropertyValue) == 0);
    }
}

/* This function takes an enum and then does WHAT THE FREAK IT IS SUPPOSED TO DO!
  *
  * function: getDeviceState
  * param: exptx -> enum expectedDeviceState
  *
*/
bool getDeviceState(enum expectedDeviceState exptx)
{
    char *currentSetupWizardMode = getSystemProperty("ro.setupwizard.mode");
    if(!currentSetupWizardMode) return false;
    switch(exptx)
    {
        case DEVICE_SETUP_OVER:
            if(strcmp(getSystemProperty("persist.sys.setupwizard"), "FINISH") == 0 || strcmp(currentSetupWizardMode, "OPTIONAL")  == 0 || strcmp(currentSetupWizardMode, "DISABLED") == 0) return true;
        case BOOTANIMATION_RUNNING:
            return (atoi(getSystemProperty("service.bootanim.progress")) == 1);
        case BOOTANIMATION_EXITED:
            return (atoi(getSystemProperty("service.bootanim.exit")) == 1);
        case DEVICE_BOOT_COMPLETED:
            return (atoi(getSystemProperty("sys.boot_completed")) == 1);
        case DEVICE_TURNED_ON: {
            FILE *fp = popen("dumpsys power | grep 'Display Power'", "r");
            if(!fp) {
                consoleLog(LOG_LEVEL_ERROR, "getDeviceState", "Failed to open stdout to gather information about the device display power status.");
                return false;
            }
            char buffer[4];
            fgets(buffer, sizeof(buffer), fp);
            pclose(fp);
            return (strstr(buffer, "OFF") == NULL);
        }
    }
    return false;
}

/* This function is used as a wrapper for the shell command "settings" or "cmd settings".
   Also, this is the one of the most lamest function here, thank me later!
  *
  * function: setSystemSettings
  * param: table -> enum systemTable | name -> system table name | value -> value of the table.
  *
*/
bool setSystemSettings(enum systemTable table, char *_Nonnull name, char *_Nullable value)
{
    char *state;
    switch(table)
    {
        case TABLE_GLOBAL:
            state = "global";
        break;    
        case TABLE_SECURE:
            state = "global";
        break;
        case TABLE_SYSTEM:
        default:
            state = "system";
        break;
    }
    if(!state) return false;
    // since we can only use cli for now, let's just abuse it.
    return (executeCommands("settings", (char *const[]) {"settings", "put", state, name, value}, false) == 0);
}

/* This function is used as a wrapper for the shell command "settings" or "cmd settings".
   Also, cast the value as an int, float or even a bool if you know wat you are getting.
  *
  * function: getSystemSettings
  * param: table -> enum systemTable | name -> system table name | skipNewlinesAtStart -> skips looking for values if the first line has a new line.
  *
*/
char *_Nullable getSystemSettings(enum systemTable table, char *_Nonnull name, bool skipNewlinesAtStart)
{
    int index = 0;
    char *state;
    char *command;
    char value[MAX_STRING_BUFFER_SIZE];
    switch(table)
    {
        case TABLE_GLOBAL:
            state = "global";
        break;    
        case TABLE_SECURE:
            state = "secure";
        break;
        case TABLE_SYSTEM:
        default:
            state = "system";
        break;
    }
    if(!state) return NULL;
    size_t sizeOfCommand = strlen(name) + strlen(state) + strlen("settings get ") + 2;
    command = malloc(sizeOfCommand);
    if(!command)
    {
        consoleLog(LOG_LEVEL_ERROR, "getSystemSettings", "Failed to get system settings value. Please try again.");
        consoleLog(LOG_LEVEL_DEBUG, "getSystemSettings", "!command");
        return NULL;
    }
    snprintf(command, sizeOfCommand, "settings get %s %s", state, name);
    FILE *fptr = popen(command, "r");
    if(!fptr)
    {
        __freeThisPointer((void**)&command);
        consoleLog(LOG_LEVEL_ERROR, "getSystemSettings", "Failed to get system settings value. Please try again.");
        consoleLog(LOG_LEVEL_DEBUG, "getSystemSettings", "!fptr");
        return NULL;
    }
    while(fgets(value, MAX_STRING_BUFFER_SIZE, fptr))
    {
        // if we suspect a NULL newline value at the beginning and especially in certain circumstances, let's just put a useless warning and end the loop 
        // if and only the dev thinks that this is a weird unga bunga moment.
        if(skipNewlinesAtStart && index == 0 && strcspn(value, "\n") == 0)
        {
            __freeThisPointer((void**)&command);
            pclose(fptr);
            consoleLog(LOG_LEVEL_ERROR, "getSystemSettings", "Provided system table variable does not seem to have a value.");
            return NULL;
        }
        // we'll increment this just to not hit that if statement:
        index++;
    }
    __freeThisPointer((void**)&command);
    pclose(fptr);
    return strdup(value);
}

/* This function is used for getting the android system properties.
  *
  * function: getSystemProperty
  * param: propertyVariableName -> android system property name.
  *
*/
char *_Nullable getSystemProperty(const char *_Nonnull propertyVariableName)
{
    // Update: use native functions from android-ndk itself!
    const prop_info* pi = __system_property_find(propertyVariableName);
    if(pi)
    {
        PropertyHandler ctx = {0};
        __system_property_read_callback(pi, androidPropertyCallback, &ctx);
        return strdup(ctx.propertyValue);
    }
    else {
        static char globalPropertyValueBuffer[PROP_VALUE_MAX];
        consoleLog(LOG_LEVEL_ERROR, "getSystemProperty", "%s not found in system, trying to gather property value from resetprop...", propertyVariableName);
        FILE *fptr = popen(combineStringsFormatted("%s %s", resetprop, propertyVariableName), "r");
        if(!fptr) {
            consoleLog(LOG_LEVEL_ERROR, "getSystemProperty", "uh, major hiccup, failed to open resetprop in popen()");
            return NULL;
        }
        fgets(globalPropertyValueBuffer, PROP_VALUE_MAX, fptr);
        // remove the dawn newline char to get a clear value.
        globalPropertyValueBuffer[strcspn(globalPropertyValueBuffer, "\n")] = '\0';
        fclose(fptr);
        return globalPropertyValueBuffer;
    }
    return NULL;
}

/* This function is used for notifying the user.
  *
  * function: alertUser
  * param: message -> a string that should contain what the user wants.
  *
*/
void alertUser(char *_Nonnull message)
{
    if(getDeviceState(DEVICE_BOOT_COMPLETED) != 0) return;
    if(isPackageInstalled("androidnull.system") == 0) executeCommands("am", (char *const[]) {"am", "start", "-a", "android.intent.action.MAIN", "-e", "null", message, "-n", "androidnull.system/.MainActivity", NULL}, false);
    else executeCommands("cmd", (char *const[]) {"cmd", "notification", "post", "-S", "bigtext", "-t", "Null", "Tag", message, NULL}, false);
}

/* This function is used to make the class open recovery script.
  *
  * function: prepareStockRecoveryCommandFile
  * param: ors -> enum openRecoveryScriptNextCommand -> a enum that determines wat we want to do.
  * actionArgOne -> *optional argument* | actionArgTwo -> *optional argument*
  *
*/
void prepareStockRecoveryCommandFile(enum openRecoveryScriptNextCommand ors, char * _Nullable actionArgOne, char * _Nullable actionArgTwo)
{
    makeDir("/cache/recovery");
    FILE *recoveryCommandFile = fopen("/cache/recovery/command", "w");
    if(!recoveryCommandFile) abort_instance("prepareStockRecoveryCommandFile", "Failed to open recovery command file to prepare given action on next boot.");
    if(ors == WIPE_DATA) fputs("--wipe_data\n", recoveryCommandFile);
    else if(ors == WIPE_CACHE) fputs("--wipe_cache\n", recoveryCommandFile);
    else if(ors == INSTALL_PACKAGE) fprintf(recoveryCommandFile, "--update_package=%s\n", actionArgOne);
    else if(ors == SWITCH_LOCALE) fprintf(recoveryCommandFile, "--locale=%s_%s\n", stringCase(actionArgOne, LOWER), stringCase(actionArgTwo, UPPER));
    fclose(recoveryCommandFile);
}

/* This function uses manageProperty to either start or stop a daemon.
  *
  * function: daemonStateManager
  * param: daemonProp -> enum setDaemonPropertyState -> a enum that determines what we want to do.
  * daemonName -> the name string of the daemon.
  *
*/
void daemonStateManager(enum setDaemonPropertyState daemonProp, char *_Nonnull daemonName)
{
    switch(daemonProp)
    {
        case DAEMON_START:
            if(manageProperty("ctl.start", (void *)daemonName, SETPROP) == 0) consoleLog(LOG_LEVEL_INFO, "startDaemon", "Daemon %s started successfully.", daemonName);
            else consoleLog(LOG_LEVEL_WARN, "daemonStateManager", "Failed to start %s daemon service.", daemonName);
        break;
        case DAEMON_STOP:
            if(manageProperty("ctl.stop", (void *)daemonName, SETPROP) == 0) consoleLog(LOG_LEVEL_INFO, "stopDaemon", "Daemon %s stopped successfully.", daemonName);
            else consoleLog(LOG_LEVEL_WARN, "daemonStateManager", "Failed to stop %s daemon service.", daemonName);
        break;
    }
}

/* Callback handler for __system_property_read_callback (android api)
  *
  * function: androidPropertyCallback
  * param: cookie -> a struct | value -> the value of the prop | serial -> i don't know what it does.
  *
*/
void androidPropertyCallback(void*_Nonnull cookie, const char*_Nonnull name, const char*_Nonnull value, uint32_t serial)
{
    PropertyHandler* handler = (PropertyHandler*)cookie;
    if(handler == NULL) 
    {
        consoleLog(LOG_LEVEL_ERROR, "androidPropertyCallback", "Error: Callback 'cookie' (PropertyHandler pointer) is NULL!");
        return;
    }
    snprintf(handler->propertyName, sizeof(handler->propertyName), "%s", name);
    snprintf(handler->propertyValue, sizeof(handler->propertyValue), "%s", value);
    handler->propertySerial = serial;
    handler->found = 1;
}
