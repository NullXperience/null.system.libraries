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
#include <null_custom_props.h>

/* This function changes the value of a given property and sets __didAnyPropertyGetChanged to true.
   If __didAnyPropertyGetChanged is set to true, the values that are changed will be reflected onto the 
   file, the property file when __deinit__properties is called.
  *
  * function: __setProperty
  * param: __propertyName -> name of the property | __propertyValue -> the value of the given property.
  *
*/
bool __setProperty(const char *_Nonnull __propertyName, const void *_Nonnull __propertyValue)
{
    // the real reason i didn't put any error on the
    // same value because it's pointless, if you add one
    // it's just going to be some junk.
    romCustProperty setProp = __initializeThisStruct();
    setProp.__propertyName = malloc(MAX_PROPERTY_NAME_LENGTH);
    snprintf(setProp.__propertyName, MAX_PROPERTY_NAME_LENGTH, "%s", __propertyName);
    __readProperty(&setProp);
    if(setProp.__found)
    {
        // we wipe out the previous value and then we malloc' it to save the new value.
        __freeThisPointer((void**)&__propertiesValue_cached[setProp.__propertyIndex]);
        __propertiesValue_cached[setProp.__propertyIndex] = malloc(MAX_PROPERTY_VALUE_LENGTH);
        if(!__propertiesValue_cached[setProp.__propertyIndex]) 
        {
            consoleLog(LOG_LEVEL_DEBUG, "__setProperty", "__propertiesValue_cached[setProp.__propertyIndex]: malloc");
            consoleLog(LOG_LEVEL_ERROR, "__setProperty", "Failed to set given property.");
            __wipeMetadata((void**)&setProp);
            return false;
        }
        snprintf(__propertiesValue_cached[setProp.__propertyIndex], MAX_PROPERTY_VALUE_LENGTH, "%s", (const char *)__propertyValue);
        __didAnyPropertyGetChanged = true;
    }
    // we couldn't find it's value.
    __wipeMetadata((void**)&setProp);
    return false;
}

/* This function takes a struct as a void *ptr which contains the property name and then we get values and 
   stuff from the pre-cached array for a proper typecast. We save the property value to the appropriate datatype.
  *
  * function: __readProperty
  * param: __cookie -> a romCustProperty that is casted as a void pointer.
  *
*/
void __readProperty(void *_Nonnull __cookie)
{
    romCustProperty* thisInstanceCustProp = (romCustProperty*)__cookie;
    if(!thisInstanceCustProp) return;
    thisInstanceCustProp->__found = false;
    // we put this shit in a loop to find the correct index lol
    for(int i = 0; i < __properties_count; i++)
    {
        if(strcmp(__propertiesValue_cached[i], "DELETED") == 0)
        {
            consoleLog(LOG_LEVEL_ERROR, "__readPropertyHelper", "Property is yet to be deleted after a sync, skipping fetching metadata for %s", __properties_cached[i]);
            return;
        }
        if(__properties_cached[i])
        {
            // set the property type, set string as the default case.
            thisInstanceCustProp->typeProp = CAST_TYPE_STRING;
            if(strstr(thisInstanceCustProp->__propertyName, "float")) thisInstanceCustProp->typeProp = CAST_TYPE_FLOAT;
            else if(strstr(thisInstanceCustProp->__propertyName, "int")) thisInstanceCustProp->typeProp = CAST_TYPE_INT;
            else if(strstr(thisInstanceCustProp->__propertyName, "bool")) thisInstanceCustProp->typeProp = CAST_TYPE_BOOL;
            thisInstanceCustProp->__found = true;
            switch(thisInstanceCustProp->typeProp)
            {
            case CAST_TYPE_INT:
                thisInstanceCustProp->value.__propertyIntegerValue = atoi(__propertiesValue_cached[i]);
            break;
            case CAST_TYPE_FLOAT:
                thisInstanceCustProp->value.__propertyFloatValue = (float)strtof(__propertiesValue_cached[i], NULL);
            break;
            case CAST_TYPE_BOOL:
                thisInstanceCustProp->value.__propertyBoolValue = (strcmp(__propertiesValue_cached[i], "true") == 0 || strcmp(__propertiesValue_cached[i], "1") == 0);
            break;
            case CAST_TYPE_STRING:
            default:
                thisInstanceCustProp->value.__propertyStringValue = malloc(MAX_PROPERTY_VALUE_LENGTH);
                if(thisInstanceCustProp->value.__propertyStringValue) 
                {
                    snprintf(thisInstanceCustProp->value.__propertyStringValue, MAX_PROPERTY_VALUE_LENGTH, "%s", __propertiesValue_cached[i]);
                }
                break;
            }
            return;
        }
        else 
        {
            consoleLog(LOG_LEVEL_ERROR, "__readPropertyHelper", "Failed to fetch property value.");
            return;
        }
        // increment the property index until we find the one, this index helps us to fetch it faster.
        thisInstanceCustProp->__propertyIndex = i;
    }
}

/* This function stores the stuff from the PROPERTY_FILE and then, we can do whatever we want with it.
  *
  * function: __cacheProperties
  *
*/
void __cacheProperties()
{
    if(__isEverythingCached) return;
    // check if we can open the file and exit if we can't.
    FILE *propertyFilePointer = fopen(PROPERTY_FILE, "r");
    if(!propertyFilePointer)
    {
        consoleLog(LOG_LEVEL_ERROR, "__cacheProperties", "Failed to open up property file.");
        exit(EXIT_FAILURE);
    }
    // reset value back, just in case:
    __properties_count = 0;
    char cachedProp[MAX_STRING_BUFFER_SIZE];
    // we can have this thing go up to a thousand.
    __properties_cached = malloc(sizeof(char*) * MAX_PROPERTIES);
    if(!__properties_cached)
    {
        consoleLog(LOG_LEVEL_DEBUG, "__cacheProperties", "__properties_cached: malloc error.");
        consoleLog(LOG_LEVEL_ERROR, "__cacheProperties", "Failed to cache properties.");
        return;
    }
    __propertiesValue_cached = malloc(sizeof(char*) * MAX_PROPERTIES);
    if(!__propertiesValue_cached)
    {
        consoleLog(LOG_LEVEL_DEBUG, "__cacheProperties", "__propertiesValue_cached: malloc error.");
        consoleLog(LOG_LEVEL_ERROR, "__cacheProperties", "Failed to cache properties.");
        return;
    }
    // loop-read the file.
    while(fgets(cachedProp, MAX_STRING_BUFFER_SIZE, propertyFilePointer)) 
    {
        // check if we exceed the MAX_PROPERTIES cap and quit the application cuz we dont have- 
        // we can't do anything with this.
        if(__properties_count >= MAX_PROPERTIES) 
        {
            consoleLog(LOG_LEVEL_ERROR, "__cacheProperties", "Exceeded MAX_PROPERTIES limit, quitting the application...");
            exit(EXIT_FAILURE);
        }
        // anyways, malloc-cate the pointers so we can give it some value.
        __properties_cached[__properties_count] = malloc(MAX_PROPERTY_NAME_LENGTH);
        if(!__properties_cached[__properties_count]) 
        {
            consoleLog(LOG_LEVEL_DEBUG, "__cacheProperties", "__properties_cached[%d]: malloc error.", __properties_count);
            consoleLog(LOG_LEVEL_ERROR, "__cacheProperties", "Failed to cache properties.");
            __deinit__properties(false);
        }
        __propertiesValue_cached[__properties_count] = malloc(MAX_PROPERTY_VALUE_LENGTH);
        if(!__propertiesValue_cached[__properties_count])
        { 
            consoleLog(LOG_LEVEL_DEBUG, "__cacheProperties", "__propertiesValue_cached[%d]: malloc error.", __properties_count);
            consoleLog(LOG_LEVEL_ERROR, "__cacheProperties", "Failed to cache properties.");
            __deinit__properties(false);
        }
        // copy the values to the thing.
        char *key = strtok(cachedProp, "=");
        char *value = strtok(NULL, "\n");
        // skip if the value itself is empty.
        if(!value) continue;
        snprintf(__properties_cached[__properties_count], MAX_PROPERTY_NAME_LENGTH, "%s", key);
        snprintf(__propertiesValue_cached[__properties_count], MAX_PROPERTY_VALUE_LENGTH, "%s", value);
        __properties_count++;
    }
}

/* This function saves the current state of the properties.
  *
  * function: __saveState
  *
*/
void __saveState()
{
    // bail out if we didn't change any value.
    if(!__didAnyPropertyGetChanged) return;
    // open the file in write mode to erase the previous content.
    FILE *thisPropertyFile = fopen(PROPERTY_FILE, "w");
    if(!thisPropertyFile)
    {
        consoleLog(LOG_LEVEL_DEBUG, "__saveState", "thisPropertyFile: fopen");
        consoleLog(LOG_LEVEL_ERROR, "__saveState", "Failed to save the current state.");
        return;
    }
    // write the new content back to the file.
    for(int i = 0; i < __properties_count; i++)
    {
        if(!__properties_cached[i] || !__propertiesValue_cached[i]) continue;
        if(strcmp(__propertiesValue_cached[i], "DELETED") == 0) continue;
        fprintf(thisPropertyFile, "%s=%s\n", __properties_cached[i], __propertiesValue_cached[i]);
    }
    fclose(thisPropertyFile);
}

/* This function takes a string and then label's it's value as DELETED so te __saveState() can delete 
   it from the file.
  *
  * function: __deleteProperty
  * param: __propertyName -> a string that contains the name string of the property.
  *
*/
void __deleteProperty(const char *_Nonnull __propertyName)
{
    // when __deinit__properties get triggered, or else ran, it'll save our
    // new properties for us. For it to save our new shit, we must have __didAnyPropertyGetChanged set to true;
    __setProperty(__propertyName, "DELETED");
    __didAnyPropertyGetChanged = true;
}

/* This function checks if PROPERTY_FILE exists or not, and if it doesn't exist, it closes the shit, otherwise
   this shit just erases the log file and caches the shit.
  *
  * function: __init__properties
  *
*/
void __init__properties()
{
    // before caching and all of that stuff, let's check some stuff.
    if(access(PROPERTY_FILE, F_OK) != 0)
    {
        consoleLog(LOG_LEVEL_DEBUG, "__init__properties", "access(PROPERTY_FILE, F_OK): %d", access(PROPERTY_FILE, F_OK));
        consoleLog(LOG_LEVEL_ERROR, "__init__properties", "Failed to initialize property stuff.");
        exit(EXIT_FAILURE);
    }
    // wipe the old logs because we don't want it to be like Epic- ifykyk.
    eraseFile(coreLogFile);
    __cacheProperties();
    // let's just assume that it got loaded in memory.
    __isEverythingCached = true;
}

/* This function is used when an unexpected shit had happened.
  *
  * function: __deinit__properties
  * param: saveTheProps -> saves the current property state if required.
  *
*/
void __deinit__properties(bool saveTheProps)
{
    consoleLog(LOG_LEVEL_DEBUG, "__deinit__properties", "deinit started, savin' state and clearin' some stuff up.");
    if(saveTheProps) __saveState();
    for(int i = 0; i < __properties_count; i++)
    {
        __freeThisPointer((void **)&__properties_cached[i]);
        __freeThisPointer((void **)&__propertiesValue_cached[i]);
    }
    __freeThisPointer((void **)&__properties_cached);
    __freeThisPointer((void **)&__propertiesValue_cached);
}

/* This function wipes the romCustProperty struct.
  *
  * function: __wipeMetadata
  * param: __cookie -> the given struct. 
  *
*/
void __wipeMetadata(void *_Nonnull __cookie)
{
    romCustProperty* __thisProperty = (romCustProperty*)__cookie;
    switch(__thisProperty->typeProp)
    {
        case CAST_NONE:
            printf("no. 1 party anthem.");
        break;
        case CAST_TYPE_INT:
            __thisProperty->value.__propertyIntegerValue = 0;
        break;
        case CAST_TYPE_FLOAT:
            __thisProperty->value.__propertyFloatValue = 0.0;
        break;
        case CAST_TYPE_BOOL:
            __thisProperty->value.__propertyBoolValue = false;
        break;
        case CAST_TYPE_STRING:
            __freeThisPointer((void**)&__thisProperty->value.__propertyStringValue);
        break;
    }
    __thisProperty->__found = false;
    __thisProperty->__propertyIndex = -1;
    __freeThisPointer((void**)&__thisProperty->__propertyName);
    __thisProperty->typeProp = CAST_NONE;
}

/* This function initializes a struct for later purposes.
  *
  * function: __initializeThisStruct
  *
*/
romCustProperty __initializeThisStruct(void)
{
    romCustProperty __init = {
        .__propertyIndex = -1,
        .__found = false,
        // malloc for name:
        .__propertyName = malloc(MAX_PROPERTY_NAME_LENGTH),
        .typeProp = CAST_NONE
    };
    return __init;
}

romCustProperty __initializeThisStructWithSize(size_t mallocSize)
{
    romCustProperty __init = {
        .__propertyIndex = -1,
        .__found = false,
        // malloc for name:
        .__propertyName = malloc(mallocSize),
        .typeProp = CAST_NONE
    };
    return __init;
}
