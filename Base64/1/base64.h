#ifndef __BASE64_H__
#define __BASE64_H__

#ifndef uint8_t
    #define uint8_t unsigned char
#endif
#ifndef uint16_t
    #define uint16_t unsigned int
#endif
#ifndef int8_t
    #define int8_t signed char
#endif

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

extern Base64Typedef base64;

#endif /* __BASE64_H__ */

