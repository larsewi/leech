#include <stdio.h>
#include <assert.h>

#include "lch_utils.h"
#include "lch_instance.h"
#include "lch_debug_messenger.h"

#define LCH_COLOR_RED     "\x1b[31m"
#define LCH_COLOR_GREEN   "\x1b[32m"
#define LCH_COLOR_YELLOW  "\x1b[33m"
#define LCH_COLOR_BLUE    "\x1b[34m"
#define LCH_COLOR_CYAN    "\x1b[36m"
#define LCH_COLOR_RESET   "\x1b[0m"


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
