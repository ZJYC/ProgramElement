

//#include <G63_Asert.h>
#include "includes.h"

/* ȥ��������� */
#define G63_Simulate

#ifndef G63_Simulate
    #define G63_Asert(x)        assert(x)
#else
    #define G63_Asert(x)        do{}while(0)
#endif

#define G63_FLAG_RxLock          (0x01)
#define G63_FLAG_CONNECTED       (0x02)
#define G63_FLAG_INITED          (0x04)


#define G63_CHK_BIT(val,bit)     ((val) & (1 << (bit)))
#define G63_SET_BIT(val,bit)     {(val) |= (1 << (bit));}
#define G63_CLR_BIT(val,bit)     {(val) &= ~(1 << (bit));}
#define G63_ABS(x)               ((x) > 0 ? (x):(-(x)))
#define G63_STR_EQL(x,y)         (strcmp((const char *)(x),(const char *)(y)) == (0))
#define G63_STR_CPY(x,y)         strcpy((char *)(x),(char *)(y))

typedef enum G63_ResTypedef_
{
    G63_True = 0,           /* OK */
    G63_DISCONNECTED,       /* �����ѶϿ� */
    G63_RetryCost,          /* ���Դ����þ� */
    G63_NeedNotMatch,       /* �����߲��ü��Match�� */
    G63_EchoFail,           /* �رջ���ʧ�� */
    G63_SetBDFail,          /* ���ò�����ʧ�� */
    G63_SignalFail,         /* �ź�����ѯʧ�� */
    G63_SIMFail,            /* SIM��ѯʧ�� */
    G63_SignalLow,          /* �źŵ� */
    /* ���±�����AT_STATE���� */
    G63_IP_INITIAL,
    G63_IP_START,
    G63_IP_CONFIG,
    G63_IP_IND,
    G63_IP_GPRSACT,
    G63_IP_STATUS,
    G63_TCP_CONNECTING,
    G63_UDP_CONNECTING,
    G63_IP_CLOSE,
    G63_CONNECT_OK,
    G63_PDP_DEACT,
    
    G63_False = 0xFF        /* ʧ�� */
    
}G63_ResTypedef;

typedef struct G63_PriDataTypedef_
{
    /* �ַ��ֽ���� */
    uint8_t *ParamSplit[20];
    /* ָ�������ڴ˻������� */
    uint8_t InstructionBuff[60];
    /* �ź��� */
    uint8_t CSQ_Signal[4];
    uint8_t CSQ_Ber[4];
    /* �������ݴ洢 */
    uint8_t LastIP[20];
    uint8_t LastPort[10];
    uint8_t LastAPN[20];
}G63_PriDataTypedef;

typedef struct G63_UartTypedef_
{
    
    uint16_t(*Init)(uint16_t Baud);
    uint16_t(*Send)(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
    uint16_t(*Recv)(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout,uint8_t CLR);
    
}G63_UartTypedef;

typedef struct G63_DriverTypedef_
{
    G63_UartTypedef         Uart;
    G63_PriDataTypedef      PriData;
}G63_DriverTypedef;

extern G63_DriverTypedef G63_Driver;


uint16_t AT_Init(void);
uint16_t AT_TCP(uint8_t * IP,uint8_t * Port);
uint16_t AT_TCP_Close(void);
uint16_t AT_TCP_SendFinished(uint16_t Retry,uint16_t RetryInterval);
uint16_t AT_STATE(void);
uint16_t AT_TCP_Send(uint8_t * Data,uint16_t Len);
uint16_t AT_TCP_Recv(uint8_t * Data,uint16_t MaxLen,uint16_t Timeout,uint16_t * RxLen);
uint16_t AT_SetApn(uint8_t * APN);
uint16_t AT_CSQ(uint8_t * CSQ,uint8_t * Count);
uint16_t AT_WaitForPowerOnFinish(uint16_t TimeOut);
uint16_t AT_GetNetTime(uint8_t * TimeString);






