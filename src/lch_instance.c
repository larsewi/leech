#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "lch_instance.h"
#include "lch_utils.h"

void logDebugMessenger(const LCH_Instance *instance, unsigned char severity, const char *format, ...)
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

    char message[LCH_BUFFER_SIZE];
    int size = vsnprintf(message, sizeof(message), format, ap);
    assert(size >= 0);

    va_end(ap);

    if ((unsigned long) size >= sizeof(message))
    {
        LCH_LOG_WARNING(instance, "Debug messenger output truncated (%lu > %lu)", size, sizeof(message));
    }

    instance->debugMessenger->callback(severity, message);
}
