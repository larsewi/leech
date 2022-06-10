#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "leech.h"

#define BUFFER_SIZE 4096UL

#define LOG_DEBUG(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT, __VA_ARGS__)
#define LOG_VERBOSE(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT, __VA_ARGS__)
#define LOG_INFO(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_INFO_BIT, __VA_ARGS__)
#define LOG_WARNING(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT, __VA_ARGS__)
#define LOG_ERROR(instance, ...) logDebugMessenger(instance, LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT, __VA_ARGS__)


void LCH_DebugMessengerCallback(unsigned char severity, const char *message)
{
    assert(message != NULL);
    int rc = 0;
    switch (severity)
    {
    case LCH_DEBUG_MESSAGE_TYPE_DEBUG_BIT:
        rc = fprintf(stdout, "[" LCH_COLOR_BLUE "DBUG" LCH_COLOR_RESET "]: %s\n", message);
        break;
    case LCH_DEBUG_MESSAGE_TYPE_VERBOSE_BIT:
        rc = fprintf(stdout, "[" LCH_COLOR_CYAN "VERB" LCH_COLOR_RESET "]: %s\n", message);
        break;
    case LCH_DEBUG_MESSAGE_TYPE_INFO_BIT:
        rc = fprintf(stdout, "[" LCH_COLOR_GREEN "INFO" LCH_COLOR_RESET "]: %s\n", message);
        break;
    case LCH_DEBUG_MESSAGE_TYPE_WARNING_BIT:
        rc = fprintf(stdout, "[" LCH_COLOR_YELLOW "WARN" LCH_COLOR_RESET "]: %s\n", message);
        break;
    case LCH_DEBUG_MESSAGE_TYPE_ERROR_BIT:
        rc = fprintf(stderr, "[" LCH_COLOR_RED "ERRR" LCH_COLOR_RESET "]: %s\n", message);
        break;
    default:
        break;
    }
    assert(rc >= 0);
}


static void logDebugMessenger(const LCH_Instance *instance, unsigned char severity, const char *format, ...)
{
    assert(instance != NULL);
    if (instance->debugMessenger == NULL)
    {
        return;
    }
    if ((instance->debugMessenger->severity & severity) == 0)
    {
        return;
    }
    assert(instance->debugMessenger->callback != NULL);

    va_list ap;
    va_start(ap, format);

    char message[BUFFER_SIZE];
    int size = vsnprintf(message, sizeof(message), format, ap);
    assert(size >= 0);

    va_end(ap);

    if ((unsigned long) size >= BUFFER_SIZE)
    {
        LOG_WARNING(instance, "Debug messenger output truncated (%lu > %lu)", size, BUFFER_SIZE);
    }

    instance->debugMessenger->callback(severity, message);
}


void LCH_TestFunc(const LCH_Instance *instance)
{
    LOG_DEBUG(instance, "This is a debug message");
    LOG_VERBOSE(instance, "This is a verbose message");
    LOG_INFO(instance, "This is a info message");
    LOG_WARNING(instance, "This is a warning message");
    LOG_ERROR(instance, "This is an error message");
}
