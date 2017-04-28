
#ifndef __HEAP_5_H__
#define __HEAP_5_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

#define portBYTE_ALIGNMENT      ( 4 )
#define portBYTE_ALIGNMENT_MASK ( 0x0003 )

#ifndef mtCOVERAGE_TEST_MARKER
    #define mtCOVERAGE_TEST_MARKER()
#endif

#define configASSERT        assert

#ifndef uint8_t
    #define uint8_t unsigned char
#endif

#ifndef uint32_t
    #define uint32_t unsigned int
#endif

typedef struct HeapRegion
{
    uint8_t *pucStartAddress;
    uint32_t xSizeInBytes;
} HeapRegion_t;


typedef struct MM_OpsTypedef_
{
    void (*Init)( const HeapRegion_t * const pxHeapRegions );
    void *(*Malloc)( uint32_t xWantedSize );
    void (*Free)( void *pv );
    uint32_t (*HeapSize)( void );
    
}MM_OpsTypedef,*pMM_OpsTypedef;

extern MM_OpsTypedef MM_Ops;

#ifdef __cplusplus
}
#endif

#endif

