#ifndef LIB_GENYD_HPP_
#define LIB_GENYD_HPP_

#include <map>
#include <string>
#include <cutils/log.h>

// Define Log tag
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "Genymotion"
#endif

class LibGenyd {

public:
    // Constructor
    LibGenyd(void);

private:
    // Destructor
    ~LibGenyd(void);

    // Copy constructor
    LibGenyd(const LibGenyd &);

    // Copy operator
    LibGenyd operator=(const LibGenyd &);

    // Singleton instance
    static LibGenyd instance;

    // Get singleton object
    static LibGenyd &getInstance(void);

    // Callbacks lists
    typedef int (LibGenyd::*t_dispatcher_member)(const char *, char *, size_t);
    std::map<std::string, t_dispatcher_member> sensor_callbacks;

    typedef int (LibGenyd::*t_callback_member)(char *, size_t);
    std::map<std::string, t_callback_member> battery_callbacks;

public:
    // Overload /proc values with genymotion configuration
    static int getValueFromProc(const char* path, char *buf, size_t size);

    // Return true if the real value must be used
    static bool useRealValue(const char *key);

private:
    // Global dispatcher
    t_dispatcher_member getSensorCallback(const char *path);

    // Store current value to Genymotion cache
    void cacheCurrentValue(const char *path, const char *buf, const size_t size);

    // =================
    // Battery callbacks
    // =================

    // Plug battery callbacks
    void initBatteryCallbacks();

    // Battery dispatcher
    int batteryCallback(const char *path, char *buff, size_t size);

    // Get battery value when full
    int batteryFull(char *buff, size_t size);

    // Get current battery value
    int batteryValue(char *buff, size_t size);
};

#endif // #define LIB_GENYD_HPP_
