#ifndef _LEECH_MODULE_H
#define _LEECH_MODULE_H

#include <stdbool.h>

void *LCH_ModuleLoad(const char *path);

void *LCH_ModuleGetSymbol(void *handle, const char *const symbol);

void LCH_ModuleDestroy(void *handle);

#endif  // _LEECH_MODULE_H
