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
#ifndef SYSTEM_NULL_HPP
#define SYSTEM_NULL_HPP
#include <system_utils.hpp>
#include <sys/system_properties.h>
#define NULL_FRAMEWORK_PACKAGE_NAME "androidnull.system"
#define NULL_FRAMEWORK_TOAST_ACTIVITY "androidnull.system/.MainActivity"

// extern variables.
extern const std::string batteryPercentageBlob;
extern const std::string resetprop;

/*
It's worth noting a historical caveat about popen in Android NDK:
    - in very old Android versions (pre-ICS, around Android 4.0), popen() could be buggy due to its use of vfork(),
      potentially causing stack corruption. Modern Android versions and NDK releases have addressed this, so it should be safe to use now.
    
    - While it's technically supported, remember that direct execution of shell commands via popen()
      should be used with caution and only when strictly necessary, as it can introduce security vulnerabilities if not handled carefully.
*/

typedef struct {
    int found; // Hey google, shut it up!
    uint32_t propertySerial;
    char propertyName[PROP_NAME_MAX];
    char propertyValue[PROP_VALUE_MAX];
} PropertyHandler;

// used for verifying if we are in a expected state.
enum expectedDeviceState {
	DEVICE_SETUP_OVER,
	BOOTANIMATION_RUNNING,
	BOOTANIMATION_EXITED,
	DEVICE_BOOT_COMPLETED,
	DEVICE_TURNED_ON
};

// used for stopping/starting the daemon given.
enum setDaemonPropertyState {
	DAEMON_START,
	DAEMON_STOP
};

// used for manipulating the system settings configs:
enum systemTable {
	TABLE_SECURE,
	TABLE_GLOBAL,
	TABLE_SYSTEM
};

// for managing/manipulating properties.
enum propertyTinkerState {
    MAYBE_SETPROP,
    SET_IF_DIFF, 
    REMOVE_PROPERTY, 
    SETPROP,
    MATCHED_WITH_EXPECTED
};

// for notifying the user:
enum notifyMethod {
    NOTIFY_TOAST,
    NOTIFY_NOTIFICATION
};

// functions:
int system_null_isPackageInstalled(std::string packageName);
int system_null_getBatteryPercentage();
bool system_null_manageProperty(const std::string& property, const std::string& expressivePropertyValue, propertyTinkerState propertyState);
bool system_null_getDeviceState(expectedDeviceState state);
bool system_null_setSystemSettings(systemTable table, std::string& name, std::string& value);
char *system_null_getSystemSettings(systemTable table, const std::string& name, bool skipNewlinesAtStart);
char *system_null_getSystemProperty(const std::string& propertyVariableName);
void system_null_notifyUser(notifyMethod method, std::string messageString);
void system_null_daemonStateManager(setDaemonPropertyState daemonProp, std::string& daemonName);
void androidPropertyCallback(void *cookie, const char* name, const char* value, uint32_t serial);
#endif
