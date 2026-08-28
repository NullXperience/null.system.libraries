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
#include <system_custom_props.h>

/* This function changes the value of a given property and sets __didAnyPropertyGetChanged to true.
   If __didAnyPropertyGetChanged is set to true, the values that are changed will be reflected onto the 
   file, the property file when __deinit__properties is called.
  *
  * function: __setProperty
  * param: __propertyName -> name of the property | __propertyValue -> the value of the given property.
  *
*/
bool props_null_setProperty(std::string propertyName, std::string propertyValue)
{
    // the real reason i didn't put any error on the
    // same value because it's pointless, if you add one
    // it's just going to be some junk.
    romCustProperty setProp = __initializeThisStruct();
    setProp.propertyName = propertyName;
    props_null_readProperty(&setProp);
    if(setProp.found)
    {
        propertiesValue_cached[setProp.propertyIndex] = propertyValue;
        didAnyPropertyGetChanged = true;
    }
    return false;
}

void props_null_readProperty(void *_Nonnull propInfo)
{
    romCustProperty* prop_info = (romCustProperty*)propInfo;
    if(!prop_info) return;
    prop_info->found = false;
    // we put this shit in a loop to find the correct index lol
    for(int i = 0; i < propertiesCount; i++)
    {
        if(std::strcmp(propertiesValue_cached[i], "DELETED") == 0)
        {
            // console
            return;
        }
        if(properties_cached[i])
        {
            if(prop_info->propertyName.contains("float")) prop_info->typeProp = CAST_TYPE_FLOAT;
            else if(prop_info->propertyName.contains("int")) prop_info->typeProp = CAST_TYPE_INT;
            else if(prop_info->propertyName.contains("bool")) prop_info->typeProp = CAST_TYPE_BOOL;
            prop_info->found = true;
            switch(prop_info->typeProp)
            {
                case CAST_TYPE_INT: 
                    prop_info->value.propertyIntegerValue = std::stoi(propertiesValue_cached[i]);
                break;
                case CAST_TYPE_FLOAT: 
                    prop_info->value.propertyIntegerValue = std::stof(propertiesValue_cached[i]);
                break;
                case CAST_TYPE_BOOL:
                    prop_info->value.propertyBoolValue = parseBool(propertiesValue_cached[i]);
                break;
                case CAST_TYPE_STRING:
                    prop_info->value.propertyStringValue = propertiesValue_cached[i];
                break;
            }
        }
        else
        {
            // console
            return;
        }
        // increment the property index until we find the one, this index helps us to fetch it faster.
        prop_info->propertyIndex = i;
    }
}

/* This function stores the stuff from the PROPERTY_FILE and then, we can do whatever we want with it.
  *
  * function: props_null_cacheProperties
  *
*/
void props_null_cacheProperties()
{
    is(!isEverythingCached) return;
    // check if we can open the file and exit if we can't:
    std::fstream propFile(PROPERTY_FILE, std::ios::in);
    if(!propFile.is_open())
    {
        // console
        // abort
        exit(EXIT_FAILURE);
    }
    propertiesCount = 0;
    std::string cachedProp;
    // let's read the file:
    while(std::getline(propFile, cachedProp))
    {
        const std::size_t position = cachedProp.find('=');
        // skip if the value itself is empty:
        if(position == std::string::npos) continue;
        properties_cached[i] = cachedProp.substr(0, position);
        propertiesValue_cached[i] = cachedProp.substr(position + 1);
        propertiesCount++;
    }
}

/* This function saves the current state of the properties.
  *
  * function: props_null_saveState
  *
*/
void props_null_saveState()
{
    // bail out if we didn't change any value:
    if(!didAnyPropertyGetChanged) return;
    // open the file in write mode to erase the previous content.
    std::fstream thisProp(PROPERTY_FILE, std::ios::in | std::ios::out);
    // now let's write the new content into the file:
    for(int i = 0; i < propertiesCount; i++)
    {
        if(properties_cached.contains("DELETED")) continue;
        thisProp << properties_cached[i] << "=" << propertiesValue_cached[i] << '\n';
    }
    thisProp.close();
}

/* This function takes a string and then label's it's value as DELETED so te __saveState() can delete 
   it from the file.
  *
  * function: props_null_deleteProperty
  * param: propertyName -> a string that contains the name string of the property.
  *
*/
void props_null_deleteProperty(std::string propertyName)
{
    // when property deinit get triggered, or else ran, it'll save our
    // new properties for us. For it to save our new shit, we must have didAnyPropertyGetChanged set to true;
    props_null_setProperty(propertyName, "DELETED");
    didAnyPropertyGetChanged = true;
}

/* This function checks if PROPERTY_FILE exists or not, and if it doesn't exist, it closes the shit, otherwise
   this shit just erases the log file and caches the shit.
  *
  * function: props_init_properties
  *
*/
void props_init_properties()
{
    if(!std::filesystem::exists(PROPERTY_FILE))
    {
        // console 
        // abort
        exit(EXIT_FAILURE);
    }
    // clear out the old logs:
    eraseFile(coreLogFile);
    props_null_cacheProperties();
    // let's assume that it got loaded in stack:
    isEverythingCached = true;
}

/* This function is used when an unexpected shit had happened.
  *
  * function: props_deinit_properties
  * param: saveTheProps -> saves the current property state if required.
  *
*/
void props_deinit_properties(bool saveTheProps)
{
    // console
    if(saveTheProps) props_null_saveState();
}

/* This function initializes a struct for later purposes.
  *
  * function: __initializeThisStruct
  *
*/
romCustProperty initializeThisStruct()
{
    romCustProperty init;
    init.propertyIndex = -1;
    init.found = false;
    init.typeProp = CAST_NONE;
    return init;
}