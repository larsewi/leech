#ifndef _LEECH_DEFINITIONS_H
#define _LEECH_DEFINITIONS_H

#define LCH_KIBIBYTE(n) (n * 1024UL)
#define LCH_MEBIBYTE(n) (n * 1024UL * 1024UL)
#define LCH_GIBIBYTE(n) (n * 1024ULL * 1024ULL * 1024ULL)

#define LCH_BUFFER_SIZE LCH_KIBIBYTE(4)

#define LCH_LENGTH(x) (sizeof(x) / sizeof(*x))

#define UNUSED(x) (void)x

#endif // _LEECH_DEFINITIONS_H
