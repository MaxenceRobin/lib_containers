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
 * @brief Callbacks for types manipulation.
 *
 * @param size : Size in bytes to allocate for an element of this type.
 * @param copy : Callback to copy an element in another one. The signature
 * follows the one of memcpy.
 * @param comp : Callback to compare two elements in order to classify them.
 * The signature follows the one of memcmp.
 * @param destroy : Callback to destroy eventual data inside an instance of this
 * this type. This callback MUST NOT destroy the element itself, only data
 * contained inside it (for exemple pointers), if there is nothing to destroy
 * inside an instance of this type, this callback must do nothing.
 */
struct type_info {
        size_t size;
        void *(*copy)(void *, const void *, size_t);
        int (*comp)(const void *, const void *, size_t);
        void (*destroy)(void *);
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

/**
 * @brief Returns a default copy function.
 */
void *(*type_default_copy())(void *, const void *, size_t);

/**
 * @brief Returns a default comp function.
 */
int (*type_default_comp())(const void *, const void *, size_t);

/**
 * @brief Returns a default destroy function.
 */
void (*type_default_destroy())(void *);

#endif /* LIB_TYPES_H */
