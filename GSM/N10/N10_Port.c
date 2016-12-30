
#include "N10.h"
#include "includes.h"

int PortPatchAction(uint8_t Res);
//建立连接  IP地址、端口
int Connnect_Socket(char *TCP_IP, char *TCP_PORT);
//获取状态 信号量、误码率
int GET_Status(char *CSQ, char *Count);
//设置APN  APN值字符串
int Set_APN(char *APN);
//GPRS初始化
int GPRS_Init(void);
//接收函数  接收缓冲区 缓冲区可接收最大长度，超时时间
int GPRS_Recieve(char *Rxdata,uint16_t Maxlen,uint16_t timeout);
//发送函数  发送缓冲区 发送长度
int GPRS_Send(char *TxData,uint16_t sendlen);

int Connnect_Socket(char *TCP_IP, char *TCP_PORT)
{
    uint8_t Res = 0;
    int PatchRes = 0;
    Res = AppSetIP_Port(TCP_IP,TCP_PORT);
    if(Res == N10_True)
    {
        Res = AppConnect();
        if(Res == N10_True)
        {
            return 0;
        }
    }
    PatchRes =  PortPatchAction(Res);
    if(PatchRes != 0)return PPPFAIL;
    return PPPFAIL;
}

int GET_Status(char *CSQ, char *Count)
{
    uint8_t Res = AppCSQ();
    if(Res == N10_True)
    {
        CSQ = N10_Driver.PriData.CSQ_Signal;
        Count = N10_Driver.PriData.CSQ_Ber;
        return 0;
    }
    return ERR;
}

int Set_APN(char *APN)
{
    int PatchRes = 0;
    uint8_t Res = AppSetAPN(APN);
    PatchRes = PortPatchAction(Res);
    if(PatchRes != 0)return ERR;
    return 0;
}

int GPRS_Init(void)
{
    uint8_t Res = 0;
    int PatchRes = 0;
    /*2016--12--26--08--54--31(ZJYC): 打开串口   */ 
    /*2016--12--26--08--54--34(ZJYC):    */ 
    Res = AppInit();
    PatchRes = PortPatchAction(Res);
    if(PatchRes != 0)return ERR;
    return 0;
}

int GPRS_Recieve(char *Rxdata,uint16_t Maxlen,uint16_t timeout)
{
    uint8_t Res = 0;
    uint16_t RecvLen = AppRecv(Rxdata,Maxlen,timeout,&Res);
    int PatchRes = PortPatchAction(Res);
    return PatchRes;
}

int GPRS_Send(char *TxData,uint16_t sendlen)
{
    uint8_t Res = 0;
    int PatchRes = 0;
    AppSend(TxData,sendlen,&Res);
    PatchRes = PortPatchAction(Res);
    return PatchRes;
}

int PortPatchAction(uint8_t Res)
{
    switch(Res)
    {
        case N10_True:{return 0;}
        case N10_False:{break;}
        case N10_Locked:{break;}
        case N10_Error:{break;}
        /* 如果断开连接了，驱动会尝试重新连接，如果再失败，没办法了 */
        case N10_DISCONNECTED:
        {
            AppConnect();
            if(N10_CHK_BIT(N10_Driver.PriData.FlagGroup,N10_FLAG_CONNECTED))
            {
                /* 问题得以解决，返回0 */
                return 0;
            }
            else
            {
                return NEEDINIT;
            }
            break;
        }
        case N10_RecvError:{break;}
        case N10_RetryCost:{break;}
        default:break;
    }
    return Res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*                               以下部分来源于M72D_Driver.h                                       */
//////////////////////////////////////////////////////////////////////////////////////////////////////












