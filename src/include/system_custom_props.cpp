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
#include <system_custom_props.hpp>

/* This function changes the value of a given property and sets didAnyPropertyGetChanged to true.
   If didAnyPropertyGetChanged is set to true, the values that are changed will be reflected onto the 
   file, the property file when __deinit__properties is called.
  *
  * function: props_null_setProperty
  * param: propertyName -> name of the property | propertyValue -> the value of the given property.
  *
*/
bool props_null_setProperty(std::string propertyName, std::string propertyValue)
{
    // the real reason i didn't put any error on the
    // same value because it's pointless, if you add one
    // it's just going to be some junk.
    romCustProperty setProp;
    setProp.propertyName = propertyName;
    props_null_readProperty(&setProp);
    if(setProp.found)
    {
        propertiesValue_cached[setProp.propertyIndex] = propertyValue;
        didAnyPropertyGetChanged = true;
    }
    return false;
}

/* This function takes a struct as a void *ptr which contains the property name and then we get values and 
   stuff from the pre-cached array for a proper typecast. We save the property value to the appropriate datatype.
  *
  * function: props_null_readProperty
  * param: propInfo -> a romCustProperty that is casted as a void pointer.
  *
*/
void props_null_readProperty(void *propInfo)
{
    romCustProperty* prop_info = static_cast<romCustProperty*>(propInfo);
    if(!prop_info) return;
    prop_info->found = false;
    // we put this shit in a loop to find the correct index lol
    for(std::size_t i = 0; i < propertiesCount; i++)
    {
        if(propertiesValue_cached[i].contains("DELETED"))
        {
            // console
            return;
        }
        if(!properties_cached[i].empty())
        {
            if(prop_info->propertyName.contains("float")) prop_info->typeProp = CAST_TYPE_FLOAT;
            else if(prop_info->propertyName.contains("int")) prop_info->typeProp = CAST_TYPE_INT;
            else if(prop_info->propertyName.contains("bool")) prop_info->typeProp = CAST_TYPE_BOOL;
            prop_info->found = true;
            switch(prop_info->typeProp)
            {
                case CAST_TYPE_INT: 
                    prop_info->propertyValue = std::stoi(propertiesValue_cached[i]);
                break;
                case CAST_TYPE_FLOAT: 
                    prop_info->propertyValue = std::stof(propertiesValue_cached[i]);
                break;
                case CAST_TYPE_BOOL:
                    prop_info->propertyValue = parseBool(propertiesValue_cached[i]);
                break;
                case CAST_TYPE_STRING:
                    prop_info->propertyValue = propertiesValue_cached[i];
                break;
                case CAST_NONE:
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
    if(!isEverythingCached) return;
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
        properties_cached[propertiesCount] = cachedProp.substr(0, position);
        propertiesValue_cached[propertiesCount] = cachedProp.substr(position + 1);
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
    for(std::size_t i = 0; i < propertiesCount; i++)
    {
        if(propertiesValue_cached[i].contains("DELETED")) continue;
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
