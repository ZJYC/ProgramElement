
#include "G60.h"


uint16_t G60_UartInit(uint16_t Baud)
{
    //M72D_SysInit();
    return 0;
}

uint16_t G60_UartSend(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout)
{
    assert(Data);
    assert(ExpectLen);
    /* 准备 */
    Uart1ClrRevBuff();
    Uart1ClrTxBuff();
    UartReset(M72D_Com);
    /* 发送 */
    Uart1WriteTxBuff((uint8_t *)Data,ExpectLen);
    return 0;
}

uint16_t G60_UartRecv(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout)
{
    uint16_t RxLen = 0;
    
    assert(Data);
    //assert(ExpectLen);
    //M72D_RxFinsh = 0;
   //Uart1ClrRevBuff();
    //UartReset(M72D_Com);
    /* 10ms定时器 */
    SetTimer1(Timeout/10);    
    while( M72D_RxFinsh == 0 )
    {
        if( !CheckTimer1() )return -1;
    }
    if( M72D_RxFinsh )
    {
        StopTimer1();
        M72D_RxFinsh = 0;
        RxLen = Uart1ReadRevBuff((uint8_t *)Data,MAX_GPRSBUF_LEN);
        Uart1ClrRevBuff();//UartReset(M72D_Com);//ADD by zgy  清空串口
        //Uart1ClrTxBuff();
        UartReset(M72D_Com);
        return RxLen;
    }
    return -1;
}

//建立连接  IP地址、端口
int Connnect_Socket(char *TCP_IP, char *TCP_PORT)
{
    uint8_t Res = AT_TCP((uint8_t *)TCP_IP,(uint8_t *)TCP_PORT);
    if(Res == (uint16_t)G60_True)return 0;
    if(Res == (uint16_t)G60_False)return NEEDINIT;
    
    return NEEDINIT;
}
//获取状态 信号量、误码率
int GET_Status(char *CSQ, char *Count)
{
    uint8_t Res = AT_CSQ((uint8_t *)CSQ,(uint8_t *)Count);
    
    if(Res == G60_True)return 0;
    if(Res == G60_SignalFail)return ERR;
    if(Res == G60_SIMFail)return SIMFail;
    
    return ERR;
}

int GPRS_Check(void)
{
    uint8_t CSQ = 0,Count = 0,Res = 0;
    Res = AT_CSQ(&CSQ,&Count);
    
    if(Res == G60_True)return 0;
    //if(Res == G60_SignalFail)return ERR;
    if(Res == G60_SIMFail)return FINDNETFALI;
    if(Res == G60_SignalLow)return LOWCSQ;
    
    return LOWCSQ;
}

//设置APN  APN值字符串
int Set_APN(char *APN)
{
    AT_SetApn((uint8_t *)APN);
    return 0;
}
//GPRS初始化
int GPRS_Init(void)
{
    COMCONFIG com_config;
    uint8_t Res = 0;
    /* 来自M72D */
    M72D_RxFinsh = 0;
    M72D_GPIOInit();
    ComConfig((COMCONFIG *)&com_config, 115200, 8, 1, 0);
    ComClose( M72D_Com );
    ComOpen( M72D_Com ,&com_config );
    Uart1ClrRevBuff();Uart1ClrTxBuff();
    Uart1ClrTxBuff();
    UartReset(M72D_Com);
    
    //上电-等待回显G63P到底有没有这个功能？？
    
    /* 我的 */
    Res = AT_Init();
    
    if(Res == (uint16_t)G60_True)return 0;
    if(Res == (uint16_t)G60_EchoFail)return ECOFALI;
    if(Res == (uint16_t)G60_SetBDFail)return BDFALI;
    //if(Res == (uint16_t)G60_SetBDFail)return POWERTIMEOUT;
    return -2;
}
//接收函数  接收缓冲区 缓冲区可接收最大长度，超时时间
int GPRS_Recieve(char *Rxdata,uint16_t Maxlen,uint16_t timeout)
{
    uint16_t RxLen = 0;
    uint8_t Res = 0;
    Res = TCP_Recv((uint8_t *)Rxdata,Maxlen,timeout,&RxLen);
    
    if(Res == G60_RetryCost)return -1;
    if(Res == G60_True)return (int)RxLen;
    if(Res == G60_False)return -1;
    
}
//发送函数  发送缓冲区 发送长度
int GPRS_Send(char *TxData,uint16_t sendlen)
{
    uint8_t Res = TCP_Send((uint8_t *)TxData,sendlen);
    if(Res == (uint16_t)G60_False)return -1;
    return 0;
}

int CommHangupSocket(void)
{
    uint8_t Res = AT_TCP_Close();
    if(Res == (uint16_t)G60_True)return 0;
    return -1;
}

int Network_Disable(void)
{
    //return PWOFFFALI;
    return 0;
}

















