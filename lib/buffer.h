#ifndef _LEECH_BUFFER_H
#define _LEECH_BUFFER_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct LCH_Buffer LCH_Buffer;

LCH_Buffer *LCH_BufferCreate(void);

bool LCH_BufferAppend(LCH_Buffer *self, const char *format, ...);

size_t LCH_BufferLength(LCH_Buffer *self);

char *LCH_BufferGet(LCH_Buffer *self);

void LCH_BufferDestroy(LCH_Buffer *self);

#endif  // _LEECH_BUFFER_H
