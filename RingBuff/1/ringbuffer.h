#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "stm32f1xx_hal.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define BUFFER_OVERFLOW (-1)

typedef struct {
    uint8_t *buffer;
    uint16_t size;
    uint16_t fill;
    uint8_t *read;
    uint8_t *write;
} Ringbuff_t;

typedef struct RingbuffOpsTypedef_
{
    /* 非零为空 */
    uint16_t (*IsEmpty)     (Ringbuff_t *rb);
    /* 非零为满 */
    uint16_t (*IsFull)      (Ringbuff_t *rb);
    /* 获取剩余值 */
    uint16_t (*Remain)      (Ringbuff_t *rb);
    /* 返回写入数值 */
    uint16_t (*HasFilled)   (Ringbuff_t *rb);
    /* 读取 */
    uint16_t (*Read)        (Ringbuff_t *rb, uint8_t* buf, uint16_t len);
    /* 写入 */
    uint16_t (*Write)       (Ringbuff_t *rb, uint8_t* buf, uint16_t len);
    
}RingbuffOpsTypedef;

#define RINGBUFFER_NEW(name, size) \
    static uint8_t ringmem##name[size]; \
    Ringbuff_t name = {ringmem##name, (size), 0, ringmem##name, ringmem##name};

#define RINGBUFFER_EXTERN(name) extern Ringbuff_t name;

extern RingbuffOpsTypedef RB_FUNC;

#if defined(__cplusplus)
}
#endif

#endif
