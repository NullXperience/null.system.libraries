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
#include <system_null.hpp>

/* This function takes a string and then it looks through the package list reported by android
   to check if the app exists or not. Returns 0 if it does.
  *
  * function: system_null_isPackageInstalled
  * param: packageName -> as the name tells.
  *
*/
int system_null_isPackageInstalled(std::string packageName)
{
    FILE *syscon = popen("pm list packages | cut -d ':' -f 2", "r");
    if(!syscon) return -1;
    char sysconOut[MAX_STRING_BUFFER_SIZE];
    while(fgets(sysconOut, MAX_STRING_BUFFER_SIZE, syscon))
    {
        if(std::strstr(sysconOut, packageName.c_str()) != nullptr)
        {
            fclose(syscon);
            return 0;
        }
    }
    fclose(syscon);
    return 1;
}

/* This function returns the battery percentage of the device.
  *
  * function: system_null_getBatteryPercentage
  *
*/
int system_null_getBatteryPercentage()
{
    std::ifstream batteryBlob(batteryPercentageBlob); 
    std::string percentage; 
    return std::getline(batteryBlob, percentage) ? std::stoi(percentage) : -1;
}

/* This function does mentioned stuff to a property. Like removing it, or just changing the value if the
   prop has a different value.
  *
  * function: system_null_manageProperty
  * param: property -> name of the prop | expressivePropertyValue -> prop value | propertyState -> enum propertyTinkerState
  *
*/
bool system_null_manageProperty(const std::string& property, const std::string& expressivePropertyValue, propertyTinkerState propertyState)
{
    const std::string currentValue = system_null_getSystemProperty(property);
    switch(propertyState)
    {
        case MAYBE_SETPROP:
            if(expressivePropertyValue.empty() || currentValue == expressivePropertyValue) return false;
            return executeCommands({resetprop, property, expressivePropertyValue}, false) == 0;
        case SET_IF_DIFF:
            if(expressivePropertyValue.empty() || currentValue == expressivePropertyValue) return false;
            return executeCommands({resetprop, property, expressivePropertyValue}, false) == 0;
        case REMOVE_PROPERTY:
            return executeCommands({resetprop, "-d", property}, false) == 0;
        case SETPROP:
            if(expressivePropertyValue.empty()) return false;
            return executeCommands({resetprop, property, expressivePropertyValue}, false) == 0;
        case MATCHED_WITH_EXPECTED:
            return !expressivePropertyValue.empty() && currentValue == expressivePropertyValue;
    }
    return false;
}

bool system_null_getDeviceState(expectedDeviceState state)
{
    const std::string setupWizardMode = system_null_getSystemProperty("ro.setupwizard.mode");
    if(setupWizardMode.empty()) return false;
    switch(state)
    {
        case DEVICE_SETUP_OVER:
        {
            const std::string setupWizard = system_null_getSystemProperty("persist.sys.setupwizard");
            return setupWizard == "FINISH" || setupWizardMode == "OPTIONAL" || setupWizardMode == "DISABLED";
        }
        case BOOTANIMATION_RUNNING:
            return std::stoi(system_null_getSystemProperty("service.bootanim.progress")) == 1;
        case BOOTANIMATION_EXITED:
            return std::stoi(system_null_getSystemProperty("service.bootanim.exit")) == 1;
        case DEVICE_BOOT_COMPLETED:
            return std::stoi(system_null_getSystemProperty("sys.boot_completed")) == 1;
        case DEVICE_TURNED_ON:
        {
            FILE* fp = popen("dumpsys power | grep 'Display Power'", "r");
            if(!fp)
            {
                consoleLog(LOG_LEVEL_ERROR, "system_null_getDeviceState", "Failed to open stdout to gather information about the device display power status.");
                return false;
            }
            char buffer[4]{};
            fgets(buffer, sizeof(buffer), fp);
            pclose(fp);
            return std::strstr(buffer, "OFF") == nullptr;
        }
    }
    return false;
}

/* This function is used as a wrapper for the shell command "settings" or "cmd settings".
   Also, this is the one of the most lamest function here, thank me later!
  *
  * function: system_null_setSystemSettings
  * param: table -> enum systemTable | name -> system table name | value -> value of the table.
  *
*/
bool system_null_setSystemSettings(systemTable table, std::string& name, std::string& value)
{
    std::string state;
    switch(table)
    {
        case TABLE_GLOBAL:
            state = "global";
        break;
        case TABLE_SECURE:
            state = "secure";
        break;
        case TABLE_SYSTEM:
            state = "system";
        break;
    }
    // we are going to just use cli for now:
    return executeCommands({"settings", state, name, value}, false) == 0;
}

/* This function is used for getting the android system properties.
  *
  * function: system_null_getSystemSettings
  * param: propertyVariableName -> android system property name.
  *
*/
char *system_null_getSystemSettings(systemTable table, const std::string& name, bool skipNewlinesAtStart)
{
    std::string state;
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
    const std::string command = "settings get " + state + " " + name;
    FILE* fptr = popen(command.c_str(), "r");
    if(!fptr)
    {
        consoleLog(LOG_LEVEL_ERROR, "system_null_getSystemSettings", "Failed to get system settings value. Please try again.");
        return nullptr;
    }
    std::string value;
    char buffer[MAX_STRING_BUFFER_SIZE];
    int index = 0;
    while(fgets(buffer, sizeof(buffer), fptr))
    {
        if(skipNewlinesAtStart && index == 0 && std::strcspn(buffer, "\n") == 0)
        {
            pclose(fptr);
            consoleLog(LOG_LEVEL_ERROR, "system_null_getSystemSettings", "Provided system table variable does not seem to have a value.");
            return nullptr;
        }
        value += buffer;
        ++index;
    }
    pclose(fptr);
    if(value.empty()) return nullptr;
    return strdup(value.c_str());
}

char *system_null_getSystemProperty(const std::string& propertyVariableName)
{
    const prop_info* pi = __system_property_find(propertyVariableName.c_str());
    if(pi)
    {
        PropertyHandler propHandler;
        __system_property_read_callback(pi, androidPropertyCallback, &propHandler);
        return strdup(propHandler.propertyValue);
    }
    else {
        char sysprop[PROP_VALUE_MAX];
        const std::string command = std::string(resetprop) + " " + propertyVariableName;
        FILE *sysf = popen(command.c_str(), "r");
        if(!sysf)
        {
            consoleLog(LOG_LEVEL_ERROR, "system_null_getSystemProperty", "uh, major hiccup, failed to open resetprop in popen()");
            return nullptr;
        }
        fgets(sysprop, PROP_VALUE_MAX, sysf);
        sysprop[std::strcspn(sysprop, "\n")] = '\0';
        fclose(sysf);
        return strdup(sysprop);
    }
    return nullptr;
}

/* This function is used for notifying the user.
  *
  * function: system_null_notifyUser
  * param: notifyMethod -> enum notifyMethod -> a message -> a string that should contain what the user wants.
  *
*/
void system_null_notifyUser(notifyMethod method, std::string messageString)
{
    if(system_null_getDeviceState(DEVICE_BOOT_COMPLETED) != 1) return;
    switch(method)
    {
        case NOTIFY_TOAST:
            executeCommands({"am", "start", "-a", "android.intent.action.MAIN", "-e", "null", messageString, "-n", NULL_FRAMEWORK_TOAST_ACTIVITY}, false);
        break;
        case NOTIFY_NOTIFICATION:
            executeCommands({"cmd", "notification", "post", "-S", "bigtext", "-t", "NullSystem", "Tag", messageString}, false);
        break;
    }
}

/* This function uses manageProperty to either start or stop a daemon.
  *
  * function: daemonStateManager
  * param: daemonProp -> enum setDaemonPropertyState -> a enum that determines what we want to do.
  * daemonName -> the name string of the daemon.
  *
*/
void system_null_daemonStateManager(setDaemonPropertyState daemonProp, std::string& daemonName)
{
    switch(daemonProp)
    {
        case DAEMON_START:
            if(system_null_manageProperty("ctl.start", daemonName, SETPROP) == 0) consoleLog(LOG_LEVEL_INFO, "system_null_daemonStateManager", "Daemon {} started successfully.", daemonName);
        break;
        case DAEMON_STOP:
            if(system_null_manageProperty("ctl.stop", daemonName, SETPROP) == 0) consoleLog(LOG_LEVEL_INFO, "system_null_daemonStateManager", "Daemon {} stopped successfully.", daemonName);
        break;
    }
}

/* Callback handler for __system_property_read_callback (android api)
  *
  * function: androidPropertyCallback
  * param: cookie -> a struct | value -> the value of the prop | serial -> i don't know what it does.
  *
*/
void androidPropertyCallback(void *cookie, const char* name, const char* value, uint32_t serial)
{
    PropertyHandler* handler = static_cast<PropertyHandler*>(cookie);
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