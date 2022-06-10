#ifndef _LEECH_H
#define _LEECH_H

#include <assert.h>

#include "lch_instance.h"
#include "lch_debug_messenger.h"


typedef struct LCH_Table
{
    char **header;
    char **rows;
} LCH_Table;


void LCH_TestFunc(const LCH_Instance *instance);

#endif // _LEECH_H
