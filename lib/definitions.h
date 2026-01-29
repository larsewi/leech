#ifndef _LEECH_DEFINITIONS_H
#define _LEECH_DEFINITIONS_H

/**
 * @brief Bump this when ever changing the specification of a patch.
 */
#define LCH_PATCH_VERSION 1

/**
 * @brief Bump this when ever changing the specification of a block.
 */
#define LCH_BLOCK_VERSION 1

/**
 * @brief Utility macro to specify kilo bytes as an exponential with base 2.
 */
#define LCH_KIBIBYTE(n) (n * 1024UL)

/**
 * @brief Utility macro to specify mega bytes as an exponential with base 2.
 */
#define LCH_MEBIBYTE(n) (n * 1024UL * 1024UL)

/**
 * @brief Utility macro to specify giga bytes as an exponential with base 2.
 */
#define LCH_GIBIBYTE(n) (n * 1024ULL * 1024ULL * 1024ULL)

/**
 * @breif The first block in the chain is called the genisis block. It always
 * has the parent block ID zero.
 */
#define LCH_GENISIS_BLOCK_ID "0000000000000000000000000000000000000000"

/**
 * @breif Utility macro to compute the length of an array.
 */
#define LCH_LENGTH(x) (sizeof(x) / sizeof(*x))

/**
 * @breif Utility function to get the smaller of two numbers.
 */
#define LCH_MIN(a, b) ((a < b) ? a : b)

/**
 * @breif Utility function to get the bigger of two numbers.
 */
#define LCH_MAX(a, b) ((a > b) ? a : b)

/**
 * @brief Use this macro when defining a variable that is only used in debug
 * mode. E.g., if the return value of a function is assinged to a variable that
 * is only used in asserts.
 */
#define LCH_UNUSED __attribute__((unused))
#ifdef NDEBUG
#define LCH_NDEBUG_UNUSED __attribute__((unused))
#else
#define LCH_NDEBUG_UNUSED
#endif

/**
 * @breif This macro resolves to the platform specific path separator. I.e.,
 * backslash on Windows, forward slash on all other platforms.
 */
#ifdef _WIN32
#define LCH_PATH_SEP '\\'
#else  // _WIN32
#define LCH_PATH_SEP '/'
#endif  // _WIN32

#endif  // _LEECH_DEFINITIONS_H
