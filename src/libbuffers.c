/**
 * @author Maxence ROBIN
 * @brief Provides simple circular buffers manipulation
 */

/* Includes ------------------------------------------------------------------*/

#include "libbuffers.h"

#include <errno.h>
#include <stdlib.h>

/* Definitions ---------------------------------------------------------------*/

struct meta {
        const struct type_info *type;
        size_t count;
        unsigned int read;
        unsigned int write;
        enum { BUFFER_NONE, BUFFER_EMPTY, BUFFER_FULL } status;
};

/* Static functions ----------------------------------------------------------*/

static struct meta *buffer_to_meta(const struct buffer *buffer)
{
        return buffer ?
                (struct meta *)buffer - 1
                : NULL;
}

static struct buffer *meta_to_buffer(const struct meta *meta)
{
        return meta ?
                (struct buffer *)(meta + 1)
                : NULL;
}

static char *data_offset(const struct meta *meta, unsigned int pos)
{
        return (char *)meta_to_buffer(meta) + meta->type->size * pos;
}

/**
 * @brief Adds 'data' to 'meta'.
 *
 * @return 0 or ENOBUFS, respectively if 'meta' is not full or if 'meta' is
 * full after this call.
 */
static int push_value(struct meta *meta, const void *data)
{
        char *offset = data_offset(meta, meta->write);

        meta->type->copy(offset, data);
        meta->write = (meta->write + 1) % meta->count;

        if (meta->write == meta->read) {
                meta->status = BUFFER_FULL;
                return ENOBUFS;
        }

        meta->status = BUFFER_NONE;
        return 0;
}

static void destroy_values(const struct meta *meta)
{
        if (meta->status == BUFFER_EMPTY)
                return;

        for (unsigned int i = meta->read;
                        i != meta->write;
                        i = (i + 1) % meta->count)
                meta->type->destroy(data_offset(meta, i));
}

/* API -----------------------------------------------------------------------*/

struct buffer *buffer_create(const struct type_info *type, size_t count)
{
        if (!type || count == 0)
                return NULL;

        struct meta *meta = malloc(sizeof(*meta) + type->size * count);
        if (!meta)
                return NULL;

        meta->type = type;
        meta->count = count;
        meta->read = 0;
        meta->write = 0;
        meta->status = BUFFER_EMPTY;

        return meta_to_buffer(meta);
}

void buffer_destroy(const struct buffer *buffer)
{
        struct meta *meta = buffer_to_meta(buffer);
        if (!meta)
                return;

        destroy_values(meta);
        free(meta);
}

int buffer_push(struct buffer *buffer, const void *data)
{
        struct meta *meta = buffer_to_meta(buffer);
        if (!meta || !data)
                return -EINVAL;

        if (meta->status == BUFFER_FULL)
                return -ENOBUFS;

        return push_value(meta, data);
}

int buffer_f_push(struct buffer *buffer, const void *data)
{
        struct meta *meta = buffer_to_meta(buffer);
        if (!meta || !data)
                return -EINVAL;

        if (meta->status == BUFFER_FULL)
                meta->read = (meta->read + 1) % meta->count;

        return push_value(meta, data);
}

int buffer_pop(struct buffer *buffer)
{
        struct meta *meta = buffer_to_meta(buffer);
        if (!meta)
                return -EINVAL;

        if (meta->status == BUFFER_EMPTY)
                return -ENOMEM;

        meta->type->destroy(data_offset(meta, meta->read));
        meta->read = (meta->read + 1) % meta->count;

        if (meta->read == meta->write) {
                meta->status = BUFFER_EMPTY;
                return ENOMEM;
        }

        meta->status = BUFFER_NONE;
        return 0;
}

int buffer_clear(struct buffer *buffer)
{
        struct meta *meta = buffer_to_meta(buffer);
        if (!meta)
                return -EINVAL;

        destroy_values(meta);
        meta->read = meta->write;
        meta->status = BUFFER_EMPTY;

        return 0;
}

void *buffer_data(const struct buffer *buffer)
{
        const struct meta *meta = buffer_to_meta(buffer);
        if (!meta)
                return NULL;

        if (meta->status == BUFFER_EMPTY)
                return NULL;

        return data_offset(meta, meta->read);
}

bool buffer_is_empty(const struct buffer *buffer)
{
        const struct meta *meta = buffer_to_meta(buffer);
        if (!meta)
                return false;

        return (meta->status == BUFFER_EMPTY);
}

bool buffer_is_full(const struct buffer *buffer)
{
        const struct meta *meta = buffer_to_meta(buffer);
        if (!meta)
                return false;

        return (meta->status == BUFFER_FULL);
}

ssize_t buffer_count(const struct buffer *buffer)
{
        const struct meta *meta = buffer_to_meta(buffer);
        if (!meta)
                return -EINVAL;

        return (ssize_t)meta->count;
}
