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
#include <system_utils.h>
#include <system_custom_props.h>
std::string coreLogFile = "core";
bool useStdoutForAllLogs = true;
int properties_count = 0;
bool didAnyPropertyGetChanged = false;
bool isEverythingCached = false;
std::vector<std::string> properties_cached;
std::vector<std::string> propertiesValue_cached;
int main(void){}