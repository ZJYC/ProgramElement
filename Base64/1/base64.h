#ifndef __BASE64_H__
#define __BASE64_H__

#include "stm32f1xx_hal.h"

typedef enum Base64ResultTypedef_
{
    BASE64_OK = 0, 
    BASE64_INVALID
}Base64ResultTypedef;

typedef struct Base64Typedef_
{
    Base64ResultTypedef (*Encode)(uint8_t *in, uint16_t inlen, uint8_t *out,uint16_t *outlen);
    Base64ResultTypedef (*Decode)(uint8_t *in, uint16_t inlen, uint8_t *out,uint16_t *outlen);
    
}Base64Typedef;

extern Base64Typedef Base64;

#endif /* __BASE64_H__ */

