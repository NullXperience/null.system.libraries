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
#include <system_custom_props.hpp>
#include <system_null.hpp>
std::size_t propertiesCount = 0;
bool useStdoutForAllLogs = false;
bool didAnyPropertyGetChanged = false;
bool isEverythingCached = false;
std::string coreLogFile = "/data/local/tmp/core";
std::vector<std::string> properties_cached;
std::vector<std::string> propertiesValue_cached;
const std::string batteryPercentageBlob = "/sys/devices/platform/charger/power_supply/battery/capacity";
const std::string resetprop = "/system/bin/setprop";
int main(void){
    eraseFile(coreLogFile);
    std::cout << system_null_isPackageInstalled(NULL_FRAMEWORK_PACKAGE_NAME) << std::endl;
    std::cout << system_null_getBatteryPercentage() << std::endl;
    std::cout << "start of void" << std::endl;
    std::string messageString = "Hello neighbour!";
    system_null_notifyUser(NOTIFY_NOTIFICATION, messageString);
    system_null_notifyUser(NOTIFY_TOAST, messageString);
    consoleLog(LOG_LEVEL_INFO, "main", "hello gng");
}