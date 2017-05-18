
/*
****************************************************
*  文件名             : 
*  作者               : -5A4A5943-
*  版本               : 
*  编写日期           : 
*  简介               : 
*  函数列表           : 
*  历史版本           : 2016--12--24--23--18--08  发现中断和应用同时访问会造成某些数据丢失，需要加锁
*****************************************************
*/
#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

//#include "stm32f1xx_hal.h"
#include "stm32f10x.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define BUFFER_OVERFLOW (-1)
#define RB_LOCKED       (0xff)
#define RB_UNLOCK       (0x00)

typedef struct {
    uint8_t Lock;
    uint8_t *buffer;
    uint16_t size;
    uint16_t fill;
    uint8_t *read;
    uint8_t *write;
} Ringbuff_t;

typedef struct RingbuffOpsTypedef_
{
    uint16_t (*Init)    (Ringbuff_t *rb);
    /* 非零为空 */
    uint16_t (*IsEmpty)     (Ringbuff_t *rb);
    /* 非零为满 */
    uint16_t (*IsFull)      (Ringbuff_t *rb);
    /* 获取剩余值 */
    uint16_t (*Available)      (Ringbuff_t *rb);
    /* 返回写入数值 */
    uint16_t (*Used)   (Ringbuff_t *rb);
    /* 读取 */
    uint16_t (*Read)        (Ringbuff_t *rb, uint8_t* buf, uint16_t len);
    /* 试读 */
    uint16_t (*Peek)        (Ringbuff_t *rb, uint8_t* buf, uint16_t len);
    /* 写入 */
    uint16_t (*Write)       (Ringbuff_t *rb, uint8_t* buf, uint16_t len);
    
    uint16_t (*Lock)    (Ringbuff_t *rb);
    uint16_t (*Unlock)  (Ringbuff_t *rb);
    uint16_t (*IsLocked) (Ringbuff_t *rb);
    
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
