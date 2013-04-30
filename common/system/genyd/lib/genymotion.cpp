#include <cutils/properties.h>

#include "genymotion.hpp"
#define __NO_PROTO
#include "global.hpp"

// Singleton object
Genymotion Genymotion::instance = Genymotion();

// Constructor
Genymotion::Genymotion(void)
{
    // Populate callbacks links
    initBatteryCallbacks();
}

// Destructor
Genymotion::~Genymotion(void)
{
}

// Get singleton object
Genymotion &Genymotion::getInstance(void)
{
    return instance;
}

// Store current value to Genymotion cache
void Genymotion::storeCurrentValue(const char *key, const char *buf, const size_t size)
{
    char final_key[PROPERTY_KEY_MAX];
    char value[PROPERTY_VALUE_MAX];
    int maxSize = size > sizeof(value) ? sizeof(value) : size;

    // Prepare cache key name
    snprintf(final_key, sizeof(final_key), "%s%s", CACHE_PREFIX, key);

    // Prepare value
    snprintf(value, maxSize, "%s", buf);

    // Store value
    property_set(final_key, value);
    SLOGD("Caching value %s = '%s'", final_key, value);
}


// Overload /proc values with genymotion configuration
int Genymotion::getValueFromProc(const char *path, char *buf, size_t size)
{
    SLOGI("Searching system value from '%s': '%s'", path, buf);

    Genymotion &instance = Genymotion::getInstance();

    Genymotion::t_dispatcher_member sensorCallback = instance.getSensorCallback(path);

    if (sensorCallback) {
        return (instance.*sensorCallback)(path, buf, size);
    }
    SLOGI("%s No callback found. Returning", __FUNCTION__);
    return -1;
}

bool Genymotion::useRealValue(const char *key)
{
    char property[PROPERTY_VALUE_MAX];
    property_get(key, property, VALUE_USE_REAL);
    return !strcmp(property, VALUE_USE_REAL);
}

Genymotion::t_dispatcher_member Genymotion::getSensorCallback(const char *path)
{
    std::map<std::string, Genymotion::t_dispatcher_member>::iterator begin = sensor_callbacks.begin();
    std::map<std::string, Genymotion::t_dispatcher_member>::iterator end = sensor_callbacks.end();

    std::string haystack(path);

    while (begin != end) {
        // if haystack starts with
        if (haystack.find(begin->first) == 0)
            return begin->second;
        ++begin;
    }

    return NULL;
}
