/* Ring Buffer implementation */
#include "ringbuffer.h"
#include <assert.h>
#include <string.h>

static uint16_t ringbuffer_empty(Ringbuff_t *rb)
{
    /* It's empty when the ReadPointer and WritePointer pointers are the same. */
    if (0 == rb->FilledSize) 
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}

static uint16_t ringbuffer_Lock    (Ringbuff_t *rb)
{
    rb->Locked = RB_LOCKED;
    return 0;
}
static uint16_t ringbuffer_Unlock  (Ringbuff_t *rb)
{
    rb->Locked = RB_UNLOCK;
}
static uint16_t ringbuffer_IsLocked (Ringbuff_t *rb)
{
    return (rb->Locked == RB_LOCKED);
}


static uint16_t ringbuff_init(Ringbuff_t *rb)
{
    assert(rb)  ;
    rb->WritePointer = rb->Buffer;
    rb->ReadPointer = rb->Buffer;
    return 0;
}

static uint16_t ringbuff_available(Ringbuff_t *rb)
{
    return rb->TotalSize - rb->FilledSize;
}

static uint16_t ringbuff_used(Ringbuff_t *rb)
{
    return rb->FilledSize;
}

static uint16_t ringbuffer_full(Ringbuff_t *rb)
{
    /* It's full when the WritePointer ponter is 1 element before the ReadPointer pointer*/
    if (rb->TotalSize == rb->FilledSize) {
        return 1;
    }else {
        return 0;
    }
}

static uint16_t ringbuffer_read(Ringbuff_t *rb, uint8_t* buf, uint16_t len)
{
    assert(len>0);
    __disable_irq();
    if (rb->FilledSize >= len) {
        // in one direction, there is enough data for retrieving
        if (rb->WritePointer > rb->ReadPointer) {
            memcpy(buf, rb->ReadPointer, len);
            rb->ReadPointer += len;
        }else if (rb->WritePointer < rb->ReadPointer) {
            uint16_t len1 = rb->Buffer + rb->TotalSize - 1 - rb->ReadPointer + 1;
            if (len1 >= len) {
                memcpy(buf, rb->ReadPointer, len);
                rb->ReadPointer += len;
            } else {
                uint16_t len2 = len - len1;
                memcpy(buf, rb->ReadPointer, len1);
                memcpy(buf + len1, rb->Buffer, len2);
                rb->ReadPointer = rb->Buffer + len2; // Wrap around
            }
        }
        rb-> FilledSize -= len;
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
    if (rb->FilledSize >= len) {
        // in one direction, there is enough data for retrieving
        if (rb->WritePointer > rb->ReadPointer) {
            memcpy(buf, rb->ReadPointer, len);
            //rb->ReadPointer += len;
        }else if (rb->WritePointer < rb->ReadPointer) {
            uint16_t len1 = rb->Buffer + rb->TotalSize - 1 - rb->ReadPointer + 1;
            if (len1 >= len) {
                memcpy(buf, rb->ReadPointer, len);
                //rb->ReadPointer += len;
            } else {
                uint16_t len2 = len - len1;
                memcpy(buf, rb->ReadPointer, len1);
                memcpy(buf + len1, rb->Buffer, len2);
                //rb->ReadPointer = rb->Buffer + len2; // Wrap around
            }
        }
        //rb-> FilledSize -= len;
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
    if (rb->TotalSize - rb->FilledSize < len) {
        __enable_irq();
        return 0;
    }
    else {
        if (rb->WritePointer >= rb->ReadPointer) {
            uint16_t len1 = rb->Buffer + rb->TotalSize - rb->WritePointer;
            if (len1 >= len) {
                memcpy(rb->WritePointer, buf, len);
                rb->WritePointer += len;
            } else {
                uint16_t len2 = len - len1;
                memcpy(rb->WritePointer, buf, len1);
                memcpy(rb->Buffer, buf+len1, len2);
                rb->WritePointer = rb->Buffer + len2; // Wrap around
            }
        } else {
            memcpy(rb->WritePointer, buf, len);
            rb->WritePointer += len;
        }
        rb->FilledSize += len;
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
    .Locked       = ringbuffer_Lock,
    .Unlock     = ringbuffer_Unlock,
    .IsLocked   = ringbuffer_IsLocked,
    .Peek       = ringbuffer_peek
};










