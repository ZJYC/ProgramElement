/* Ring buffer implementation */
#include "ringbuffer.h"
#include <assert.h>
#include <string.h>

static void ringbuffer_delay(uint32_t cnt)
{
    while(cnt--);
}

static uint16_t ringbuffer_empty(Ringbuff_t *rb)
{
    /* It's empty when the read and write pointers are the same. */
    if (0 == rb->fill) {
        return 1;
    }
    else {
        return 0;
    }
}

static uint16_t ringbuffer_Lock    (Ringbuff_t *rb)
{
    rb->Lock = RB_LOCKED;
    return 0;
}
static uint16_t ringbuffer_Unlock  (Ringbuff_t *rb)
{
    rb->Lock = RB_UNLOCK;
}
static uint16_t ringbuffer_IsLocked (Ringbuff_t *rb)
{
    return (rb->Lock == RB_LOCKED);
}


static uint16_t ringbuff_init(Ringbuff_t *rb)
{
    assert(rb)  ;
    rb->write = rb->buffer;
    rb->read = rb->buffer;
    return 0;
}

static uint16_t ringbuff_available(Ringbuff_t *rb)
{
    return rb->size - rb->fill;
}

static uint16_t ringbuff_used(Ringbuff_t *rb)
{
    return rb->fill;
}

static uint16_t ringbuffer_full(Ringbuff_t *rb)
{
    /* It's full when the write ponter is 1 element before the read pointer*/
    if (rb->size == rb->fill) {
        return 1;
    }else {
        return 0;
    }
}

static uint16_t ringbuffer_read(Ringbuff_t *rb, uint8_t* buf, uint16_t len)
{
    assert(len>0);
    __disable_irq();
    if (rb->fill >= len) {
        // in one direction, there is enough data for retrieving
        if (rb->write > rb->read) {
            memcpy(buf, rb->read, len);
            rb->read += len;
        }else if (rb->write < rb->read) {
            uint16_t len1 = rb->buffer + rb->size - 1 - rb->read + 1;
            if (len1 >= len) {
                memcpy(buf, rb->read, len);
                rb->read += len;
            } else {
                uint16_t len2 = len - len1;
                memcpy(buf, rb->read, len1);
                memcpy(buf + len1, rb->buffer, len2);
                rb->read = rb->buffer + len2; // Wrap around
            }
        }
        rb-> fill -= len;
        __enable_irq();
        return len;
    } else  {
        __enable_irq();
        return 0;
    }
}
/* 试读缓冲区数据 */
static uint16_t ringbuffer_peek(Ringbuff_t *rb, uint8_t* buf, uint16_t len)
{
    assert(len>0);
    __disable_irq();
    if (rb->fill >= len) {
        // in one direction, there is enough data for retrieving
        if (rb->write > rb->read) {
            memcpy(buf, rb->read, len);
            //rb->read += len;
        }else if (rb->write < rb->read) {
            uint16_t len1 = rb->buffer + rb->size - 1 - rb->read + 1;
            if (len1 >= len) {
                memcpy(buf, rb->read, len);
                //rb->read += len;
            } else {
                uint16_t len2 = len - len1;
                memcpy(buf, rb->read, len1);
                memcpy(buf + len1, rb->buffer, len2);
                //rb->read = rb->buffer + len2; // Wrap around
            }
        }
        //rb-> fill -= len;
        __enable_irq();
        return len;
    } else  {
        __enable_irq();
        return 0;
    }
}

static uint16_t ringbuffer_write(Ringbuff_t *rb, uint8_t* buf, uint16_t len)
{
    assert(len > 0);
    __disable_irq();
    if (rb->size - rb->fill < len) {
        __enable_irq();
        return 0;
    }
    else {
        if (rb->write >= rb->read) {
            uint16_t len1 = rb->buffer + rb->size - rb->write;
            if (len1 >= len) {
                memcpy(rb->write, buf, len);
                rb->write += len;
            } else {
                uint16_t len2 = len - len1;
                memcpy(rb->write, buf, len1);
                memcpy(rb->buffer, buf+len1, len2);
                rb->write = rb->buffer + len2; // Wrap around
            }
        } else {
            memcpy(rb->write, buf, len);
            rb->write += len;
        }
        rb->fill += len;
        __enable_irq();
        return len;
    }
}

RingbuffOpsTypedef RB_FUNC = 
{
    .IsEmpty    = ringbuffer_empty,
    .IsFull     = ringbuffer_full,
    .Available  = ringbuff_available,
    .Used  = ringbuff_used,
    .Read       = ringbuffer_read,
    .Write      = ringbuffer_write,
    .Init       = ringbuff_init,
    .Lock       = ringbuffer_Lock,
    .Unlock     = ringbuffer_Unlock,
    .IsLocked   = ringbuffer_IsLocked,
    .Peek       = ringbuffer_peek
};










