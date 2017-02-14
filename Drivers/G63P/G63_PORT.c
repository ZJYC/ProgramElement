
#include "G63.h"


uint16_t G63_UartInit(uint16_t Baud)
{
    //M72D_SysInit();
    return 0;
}

uint16_t G63_UartSend(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout)
{
    G63_Asert(Data);
    G63_Asert(ExpectLen);
    /* 准备 */
    //Uart1ClrRevBuff();
    //Uart1ClrTxBuff();
    //UartReset(M72D_Com);
    /* 发送 */
    Uart1WriteTxBuff((uint8_t *)Data,ExpectLen);
    return 0;
}

uint16_t x = 0x00;

uint16_t G63_UartRecv(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout,uint8_t CLR)
{
    uint16_t RxLen = 0;
    
    G63_Asert(Data);
	/*  */
	/*
	if(M72D_RxFinsh)
	{
		M72D_RxFinsh = 0;
		RxLen = Uart1ReadRevBuff((uint8_t *)Data,MAX_GPRSBUF_LEN);
		if(RxLen == 0)
		{
			;
		}
		else
		{
			return RxLen;
		}
	}
	*/
    /* 10ms定时器 */
    SetTimer1(Timeout/10);    
    while( M72D_RxFinsh == 0 )
    {
        if( !CheckTimer1() )return 0xFFFF;
    }
    if( M72D_RxFinsh )
    {
        StopTimer1();
        M72D_RxFinsh = 0;
        x = RxLen = Uart1ReadRevBuff((uint8_t *)Data,MAX_GPRSBUF_LEN);
        return RxLen;
    }
    return -1;
}

//建立连接  IP地址、端口
int Connnect_Socket(char *TCP_IP, char *TCP_PORT)
{
    uint16_t Res = AT_TCP((uint8_t *)TCP_IP,(uint8_t *)TCP_PORT);
    if(Res == (uint16_t)G63_True)return 0;
    if(Res == (uint16_t)G63_False)return NEEDINIT;
    
    return NEEDINIT;
}
//获取状态 信号量、误码率
int GET_Status(char *CSQ, char *Count)
{
    uint8_t Res = AT_CSQ((uint8_t *)CSQ,(uint8_t *)Count);
    
    if(Res == G63_True)return 0;
    if(Res == G63_SignalFail)return ERR;
    if(Res == G63_SIMFail)return SIMFail;
    
    return ERR;
}

int GPRS_Check(void)
{
    uint8_t CSQ = 0,Count = 0,Res = 0;
    Res = AT_CSQ(&CSQ,&Count);
    
    if(Res == G63_True)return 0;
    //if(Res == G63_SignalFail)return ERR;
    if(Res == G63_SIMFail)return FINDNETFALI;
    if(Res == G63_SignalLow)return LOWCSQ;
    
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
    uint16_t Res = 0;
    /* 来自M72D */
    M72D_RxFinsh = 0;
    M72D_GPIOInit();
    ComConfig((COMCONFIG *)&com_config, 115200, 8, 1, 0);
    ComClose( M72D_Com );
    ComOpen( M72D_Com ,&com_config );
    Uart1ClrRevBuff();Uart1ClrTxBuff();
    Uart1ClrTxBuff();
    UartReset(M72D_Com);
    M72D_Disable();
    Delay10ms(30);
    M72D_Enable();
    //AT_WaitForPowerOnFinish(3000);
    Delay10ms(20);
    M72D_PWRKEY_D();
    AT_WaitForPowerOnFinish(4000);
    Delay10ms(30);
    M72D_PWRKEY_E();
    /* 我的 */
    Res = AT_Init();
    
    if(Res == (uint16_t)G63_True)return 0;
    if(Res == (uint16_t)G63_EchoFail)return ECOFALI;
    if(Res == (uint16_t)G63_SetBDFail)return BDFALI;
    //if(Res == (uint16_t)G63_SetBDFail)return POWERTIMEOUT;
    return -2;
}
//接收函数  接收缓冲区 缓冲区可接收最大长度，超时时间
int GPRS_Recieve(char *Rxdata,uint16_t Maxlen,uint16_t timeout)
{
    uint16_t RxLen = 0;
    uint8_t Res = 0;
    Res = AT_TCP_Recv((uint8_t *)Rxdata,Maxlen,timeout,&RxLen);
    
    if(Res == G63_RetryCost)return -1;
    if(Res == G63_True)return (int)RxLen;
    if(Res == G63_False)return -1;
    return -1;
}
//发送函数  发送缓冲区 发送长度
int GPRS_Send(char *TxData,uint16_t sendlen)
{
    uint8_t Res = AT_TCP_Send((uint8_t *)TxData,sendlen);
    if(Res == (uint16_t)G63_False)return -1;
    return 0;
}

int CommHangupSocket(void)
{
    uint16_t Res = AT_TCP_Close();
    if(Res == (uint16_t)G63_True)return 0;
    return -1;
}

int Network_Disable(void)
{
    M72D_PWRKEY_D();
    AT_WaitForPowerOnFinish(4000);
    Delay10ms(30);
    M72D_PWRKEY_E();
    Delay10ms(30);
    M72D_Disable();
    return 0;
}

int GET_Nettime( char *TIME)
{
    uint16_t Res = AT_GetNetTime((uint8_t *)TIME);
    
    if(Res == (uint16_t)G63_True)return APP_SUCC;
    if(Res == (uint16_t)G63_False)return ERR;
    
    return ERR;
}















