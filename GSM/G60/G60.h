

#include <assert.h>
#include "includes.h"


//#define (uint16_t)G60_True                 (0xff)
//#define (uint16_t)G60_False                (0x00)
//#define (uint16_t)G60_DISCONNECTED         (0x03)
//#define (uint16_t)G60_RetryCost            (0x05)
//#define (uint16_t)G60_NeedNotMatch         (0x06)     

#define G60_FLAG_RxLock          (0x01)
#define G60_FLAG_CONNECTED       (0x02)
#define G60_FLAG_INITED          (0x04)


#define G60_CHK_BIT(val,bit)     ((val) & (1 << (bit)))
#define G60_SET_BIT(val,bit)     {(val) |= (1 << (bit));}
#define G60_CLR_BIT(val,bit)     {(val) &= ~(1 << (bit));}
#define G60_ABS(x)               ((x) > 0 ? (x):(-(x)))
#define G60_STR_CMP(x,y)         (strcmp((const char *)(x),(const char *)(y)) == (0))

typedef enum G60_ResTypedef_
{

    /*  */
    G60_True = 0,           /* OK */
    G60_DISCONNECTED,       /* �����ѶϿ� */
    G60_RetryCost,          /* ���Դ����þ� */
    G60_NeedNotMatch,       /* �����߲��ü��Match�� */
    G60_EchoFail,           /* �رջ���ʧ�� */
    G60_SetBDFail,          /* ���ò�����ʧ�� */
    G60_SignalFail,         /* �ź�����ѯʧ�� */
    G60_SIMFail,            /* SIM��ѯʧ�� */
    G60_SignalLow,          /* �źŵ� */
    ///////////////////////////////
    /* ���±�����AT_STATE���� */
    G60_IP_INITIAL,
    G60_IP_START,
    G60_IP_CONFIG,
    G60_IP_IND,
    G60_IP_GPRSACT,
    G60_IP_STATUS,
    G60_TCP_CONNECTING,
    G60_UDP_CONNECTING,
    G60_IP_CLOSE,
    G60_CONNECT_OK,
    G60_PDP_DEACT,
    ///////////////////////////////
    
    G60_False = 0xFF        /* ʧ�� */
    
}G60_ResTypedef;

typedef struct G60_PriDataTypedef_
{
    /* ��ǰ��ɫ */
    uint8_t CurRole;
    /* �ַ��ֽ���� */
    uint8_t *ParamSplit[10];
    /* ��־�� */
    uint16_t FlagGroup;
    /* ָ�������ڴ˻������� */
    uint8_t InstructionBuff[60];
    /* �ź��� */
    uint8_t CSQ_Signal[4];
    uint8_t CSQ_Ber[4];
    /* �������ݴ洢 */
    uint8_t LastIP[20];
    uint8_t LastPort[10];
    uint8_t LastAPN[20];
    /* ������ */
    uint32_t Counter;
}G60_PriDataTypedef;

typedef struct G60_UartTypedef_
{
    
    uint16_t(*Init)(uint16_t Baud);
    uint16_t(*Send)(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
    uint16_t(*Recv)(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
    
}G60_UartTypedef;

typedef struct G60_DriverTypedef_
{
    G60_UartTypedef         Uart;
    G60_PriDataTypedef      PriData;
}G60_DriverTypedef;

typedef struct G60_CommandItemTypedef_
{
    uint8_t * Command;
    uint8_t MatchIndex;
    uint8_t * Match;
    uint8_t Retry;
    uint16_t Timeout;
}G60_CommandItemTypedef;


extern G60_DriverTypedef G60_Driver;


uint8_t AT_Init(void);
uint8_t AT_TCP(uint8_t * IP,uint8_t * Port);
uint8_t AT_TCP_Close(void);
uint16_t TCP_SendFinished(void);
uint16_t AT_STATE(void);
uint16_t TCP_Send(uint8_t * Data,uint16_t Len);
uint16_t TCP_Recv(uint8_t * Data,uint16_t MaxLen,uint16_t Timeout,uint16_t * RxLen);
uint16_t AT_SetApn(uint8_t * APN);
uint16_t AT_CSQ(uint8_t * CSQ,uint8_t * Count);






