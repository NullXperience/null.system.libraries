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
#ifndef SYSTEM_CUSTOM_PROPS_HPP
#define SYSTEM_CUSTOM_PROPS_HPP
#include <system_utils.hpp>
#include <filesystem>
#include <variant>

// variables to use to cache the properties.
extern std::size_t propertiesCount;
extern bool didAnyPropertyGetChanged;
extern bool isEverythingCached;
extern std::vector<std::string> properties_cached;
extern std::vector<std::string> propertiesValue_cached;

// macro:
#define PROPERTY_FILE "/data/adb/nullex/thisProperty"

// enum for casting property value.
enum propertyCastType {
    CAST_TYPE_INT,
    CAST_TYPE_FLOAT,
    CAST_TYPE_STRING,
    CAST_TYPE_BOOL,
    CAST_NONE
};

// struct that is used for property stuffs like
// returning it's value or finding the index and type
typedef struct {
    std::size_t propertyIndex;
    bool found;
    std::string propertyName;
    std::variant<int, float, bool, std::string> propertyValue;
    enum propertyCastType typeProp;
} romCustProperty;

// functions:
bool props_null_setProperty(std::string propertyName, std::string propertyValue);
void props_null_readProperty(void *propInfo);
void props_null_cacheProperties();
void props_null_saveState();
void props_null_deleteProperty(std::string propertyName);
void props_init_properties();
void props_deinit_properties(bool saveTheProps);
romCustProperty initializeThisStruct();
#endif