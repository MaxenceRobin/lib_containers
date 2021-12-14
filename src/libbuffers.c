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
        return (struct meta *)buffer - 1;
}

static struct buffer *meta_to_buffer(const struct meta *meta)
{
        return (struct buffer *)(meta + 1);
}

static char *data_offset(const struct buffer *buffer, unsigned int pos)
{
        const struct meta *meta = buffer_to_meta(buffer);
        return (char *)buffer + meta->type->size * pos;
}

/* API -----------------------------------------------------------------------*/

struct buffer *buffer_create(const struct type_info *type, size_t count)
{
        if (!type)
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
        if (!buffer)
                return;

        struct meta *meta = buffer_to_meta(buffer);

        for (unsigned int i = 0; i < meta->count; ++i)
                meta->type->destroy(data_offset(buffer, i));

        free(meta);
}

int buffer_push(struct buffer *buffer, const void *data)
{
        if (!buffer || !data)
                return -EINVAL;

        struct meta *meta = buffer_to_meta(buffer);
        if (meta->status == BUFFER_FULL)
                return -ENOBUFS;

        char *offset = data_offset(buffer, meta->write);
        meta->type->copy(offset, data, meta->type->size);

        meta->write = (meta->write + 1) % meta->count;
        meta->status = (meta->write == meta->read) ? BUFFER_FULL : BUFFER_NONE;

        return 0;
}

int buffer_pop(struct buffer *buffer)
{
        if (!buffer)
                return -EINVAL;

        struct meta *meta = buffer_to_meta(buffer);
        if (meta->status == BUFFER_EMPTY)
                return -ENOMEM;

        meta->type->destroy(data_offset(buffer, meta->read));

        meta->read = (meta->read + 1) % meta->count;
        meta->status = (meta->read == meta->write) ? BUFFER_EMPTY : BUFFER_NONE;

        return 0;
}

const void *buffer_data(const struct buffer *buffer)
{
        if (!buffer)
                return NULL;

        const struct meta *meta = buffer_to_meta(buffer);
        if (meta->status == BUFFER_EMPTY)
                return NULL;

        return data_offset(buffer, meta->read);
}

bool buffer_is_empty(const struct buffer *buffer)
{
        if (!buffer)
                return false;

        return (buffer_to_meta(buffer)->status == BUFFER_EMPTY);
}

bool buffer_is_full(const struct buffer *buffer)
{
        if (!buffer)
                return false;

        return (buffer_to_meta(buffer)->status == BUFFER_FULL);
}

ssize_t buffer_count(const struct buffer *buffer)
{
        if (!buffer)
                return -EINVAL;

        return (ssize_t)buffer_to_meta(buffer)->count;
}
