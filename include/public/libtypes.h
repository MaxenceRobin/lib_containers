/**
 * @author Maxence ROBIN
 * @brief Types descriptors
 */

#ifndef LIB_TYPES_H
#define LIB_TYPES_H

/* Includes ------------------------------------------------------------------*/

#include <stddef.h>

/* Definitions ---------------------------------------------------------------*/

/**
 * @brief Mandatory information about a given type. ALL info MUST be specified
 * when defining a new type, otherwise undefined behavior or crashes may happen.
 */
struct type_info {
        size_t size;
        void *(*copy)(void *, const void *, size_t);
        int (*comp)(const void *, const void *, size_t);
};

/* API -----------------------------------------------------------------------*/

const struct type_info *type_char();
const struct type_info *type_uchar();

const struct type_info *type_short();
const struct type_info *type_ushort();

const struct type_info *type_int();
const struct type_info *type_uint();

const struct type_info *type_long();
const struct type_info *type_ulong();

const struct type_info *type_long_long();
const struct type_info *type_ulong_long();

const struct type_info *type_float();
const struct type_info *type_double();
const struct type_info *type_long_double();

#endif /* LIB_TYPES_H */
