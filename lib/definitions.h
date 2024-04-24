#ifndef _LEECH_DEFINITIONS_H
#define _LEECH_DEFINITIONS_H

#define LCH_PATCH_VERSION 1
#define LCH_BLOCK_VERSION 1

#define LCH_KIBIBYTE(n) (n * 1024UL)
#define LCH_MEBIBYTE(n) (n * 1024UL * 1024UL)
#define LCH_GIBIBYTE(n) (n * 1024ULL * 1024ULL * 1024ULL)

#define LCH_GENISIS_BLOCK_ID "0000000000000000000000000000000000000000"

#define LCH_LENGTH(x) (sizeof(x) / sizeof(*x))
#define LCH_MIN(a, b) ((a < b) ? a : b)
#define LCH_MAX(a, b) ((a > b) ? a : b)

#define LCH_UNUSED __attribute__((unused))
#ifdef NDEBUG
#define LCH_NDEBUG_UNUSED __attribute__((unused))
#else
#define LCH_NDEBUG_UNUSED
#endif

#ifdef _WIN32
#define LCH_PATH_SEP '\\'
#else  // _WIN32
#define LCH_PATH_SEP '/'
#endif  // _WIN32

#endif  // _LEECH_DEFINITIONS_H
