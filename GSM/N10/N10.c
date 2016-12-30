/*
****************************************************
*  �ļ���             : 
*  ����               : -5A4A5943-
*  �汾               : 
*  ��д����           : 
*  ���               : 
*  �����б�           : 
*  ��ʷ�汾           : 
*****************************************************
*/


/*ͷ�ļ�  */

#include "N10.h"


/*�궨��  */





/*��������*/





/*��������*/





/*��������*/


/*
In:ָ������
Len:ָ���
Out:��ָ��ֽ��������ά�ַ�����

���룺��SPP: ok idle\r\n\0��
�����Out[0] = "SPP",Out[1] = "ok",Out[2] = "idle"
���أ��ֽ�������ַ�������
BUG:In �Ŀ�ͷ����Ϊ�������ַ�
*/
static uint8_t prvSplitString(uint8_t * In,uint16_t Len,uint8_t ** Out)
{
    //CharUseless������һ�������ַ�
    uint8_t i = 0,tIndex = 0,CharUseless = 0;
    
    N10_CHK_PARAM(In);
    N10_CHK_PARAM(Out);
    N10_CHK_PARAM(Len);  /* ������ô����0�أ��ǲ��� */
    //���ַ����ĳ�����һ���޶�
    while(In[i] && i < Len)
    {
        //������4�������ַ�
        if(In[i] == '\r' || In[i] == '\n' || In[i] == ':' || In[i] == ' ' || In[i] == '+' || In[i] == ',')
        {
            In[i] = '\0';
            i ++;
            CharUseless = 0xff;
            continue;
        }
        //�洢���ֶεĵ�ַ
        if(CharUseless == 0xff){Out[tIndex++] = &In[i];CharUseless = 0x00;}
        if(tIndex > 6)return N10_False;
        i ++;
    }
    
    return tIndex;
}

/*
****************************************************
*  ������         : 
*  ��������       : 
*  ����           : 
                        Instruction:ָ��
                        RetryCnt:�ش�����
                        Timeout:��ʱ
                        MatchIndex:ƥ������
                        Match:ƥ����
*  ����ֵ         : 
                        N10_True
                        N10_RecvError
                        N10_DISCONNECTED
                        N10_RetryCost
                        
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
static uint8_t prvSendInstruction(uint8_t * Instruction,uint8_t RetryCnt,uint16_t Timeout,uint8_t MatchIndex,uint8_t * Match)
{
    uint32_t CurCounter = N10_Driver.PriData.Counter;
    /* ����ٶ�ָ��Ȳ����ܳ���100�ֽ� */
    uint8_t Retry = 0,**ParamSplitTemp = N10_Driver.PriData.ParamSplit;
    uint8_t * Buf = N10_Driver.PriData.InstructionBuff;
    uint16_t LenRecv = 0,i = 0;
    /* ������ */
    N10_CHK_PARAM(Instruction);
    N10_CHK_PARAM(Match);
    /* ����ָ�� */
    N10_Driver.Uart.Send(Instruction,strlen(Instruction),Timeout);
    
    for(;;)
    {
        /* �յ�һ֡��Ϣ ֱ���ж��Ƿ�ƥ��Match */
        LenRecv = N10_Driver.Uart.Recv(Buf,0,Timeout);
        /* ����յ����� */
        if(LenRecv)
        {
            uint8_t tIndex = prvSplitString(Buf,LenRecv,ParamSplitTemp);
            /* ����ƥ������Ƿ���true */
            if(strcmp(ParamSplitTemp[MatchIndex],Match) == 0 )
            {
                return N10_True;
            }
            /* �����ж��Ƿ��յ��ˡ�ERROR���� */
            for(i = 0;i < tIndex;i ++)
            {
                if(strcmp(ParamSplitTemp[tIndex],"ERROR") == 0)
                {
                    return N10_RecvError;
                }
                if(strcmp(ParamSplitTemp[tIndex],"$MYURCCLOSE") == 0)
                {
                    /* �����������⣡�����ӶϿ� */
                    N10_CLR_BIT(N10_Driver.PriData.FlagGroup,N10_FLAG_CONNECTED);
                    return N10_DISCONNECTED;
                }
            }
        }
        /* ����û���յ����� */
        else
        {
            /* �ش� */
            if(++Retry < RetryCnt)
            {
                CurCounter = N10_Driver.PriData.Counter;
                N10_Driver.Uart.Send(Instruction,strlen(Instruction),Timeout);
            }
            else
            {
                return N10_False;
            }
        }
        if(Retry >= RetryCnt)return N10_RetryCost;
    }
}

/*
****************************************************
*  ������         : 
*  ��������       : ���SIM���Ƿ����
*  ����           : 
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
 
uint8_t AppDetectCCID(void)
{
    uint8_t Res = prvSendInstruction("AT+CCID\r\n",2,400,2,"OK");
    return Res;
}

uint8_t AppATE0(void)
{
    uint8_t Res = prvSendInstruction("ATE0\r\n",10,500,0,"OK");
    return Res;
}

uint8_t AppCAPS(void)
{
    uint8_t Res = prvSendInstruction("AT+CPAS\r\n",2,500,1,"0");
    return Res;
}

uint8_t AppCREG(void)
{
    uint8_t Res = prvSendInstruction("AT+CREG?\r\n",2,500,2,"1");
    if(Res == N10_False)Res = prvSendInstruction("AT+CREG?\r\n",2,500,2,"5");
    return Res;
}

uint8_t AppCGREG(void)
{
    uint8_t Res = prvSendInstruction("AT+CGREG?\r\n",2,500,2,"1");
    if(Res == N10_False)Res = prvSendInstruction("AT+CREG?\r\n",2,500,2,"5");
    return Res;
}

uint8_t AppCSQ(void)
{
    uint8_t Res = prvSendInstruction("AT+CSQ\r\n",2,500,3,"OK");
    if(Res == N10_True)
    {
        strcpy(N10_Driver.PriData.CSQ_Signal,N10_Driver.PriData.ParamSplit[1]);
        strcpy(N10_Driver.PriData.CSQ_Ber,N10_Driver.PriData.ParamSplit[2]);
        return N10_True;
    }
    return Res;
}

uint8_t AppSetAPN(uint8_t * APN)
{
    uint8_t Res = 0;
    uint8_t * Buff = 0;
    
    N10_CHK_PARAM(APN);
    
    Buff = N10_Driver.PriData.InstructionBuff;
    sprintf(Buff,"AT$MYNETCON=0,\"APN\",\"%s\"\r\n",APN);
    Res = prvSendInstruction(Buff,4,1000,0,"OK");
    return Res;
}

uint8_t AppSetIP_Port(uint8_t * IP,uint8_t * Port)
{
    uint8_t * Buff = N10_Driver.PriData.InstructionBuff;
    uint8_t Res = prvSendInstruction("AT$MYNETACT=0,1\r\n",4,3000,0,"OK");
    
    N10_CHK_PARAM(IP);
    N10_CHK_PARAM(Port);
    /* �����Ա����� */
    strcpy(N10_Driver.PriData.LastIP,IP);
    strcpy(N10_Driver.PriData.LastPort,Port);
    
    if(Res == N10_True)
    {
        sprintf(Buff,"AT$MYNETSRV=0,0,0,0,\"%s:%s\"\r\n",IP,Port);
        Res = prvSendInstruction(Buff,2,500,0,"OK");
    }
    return Res;
}

uint8_t AppConnect(void)
{
    uint8_t Res = prvSendInstruction("AT$MYNETOPEN=0\r\n",1,11000,0,"CONNECT");
    /* ������ */
    if(Res == N10_True)N10_SET_BIT(N10_Driver.PriData.FlagGroup,N10_FLAG_CONNECTED);
    return Res;
}

uint16_t AppSend(uint8_t * Data,uint16_t Len,uint8_t * Res)
{
    uint8_t * Buff = N10_Driver.PriData.InstructionBuff;
    uint8_t **ParamSplitTemp = (uint8_t **)N10_Driver.PriData.ParamSplit;
    uint16_t LenRecv = 0;
    
    N10_CHK_PARAM(Data);
    N10_CHK_PARAM(Len);
    /* ȷ������������״̬ */
    if(N10_CHK_BIT(N10_Driver.PriData.FlagGroup,N10_FLAG_CONNECTED))
    {
        sprintf(Buff,"AT$MYNETWRITE=0,%d\r\n",Len);
        *Res = prvSendInstruction(Buff,4,300,0,"$MYNETWRITE");
        /* ���Ƿ������� */
        if(*Res == N10_True)
        {
            N10_Driver.Uart.Send(Data,Len,1000);
            N10_Driver.Uart.Send("\r\n",2,1000);
            /* ������Len������֮��ģ��ظ�OK������������� */
            LenRecv = N10_Driver.Uart.Recv(Buff,0,500);
            if(LenRecv)
            {
                prvSplitString(Buff,LenRecv,ParamSplitTemp);
                /* ����ƥ������Ƿ���true */
                if(strcmp(ParamSplitTemp[0],"OK") == 0 )
                {
                    return Len;
                }
            }
        }
    }
    return 0;
}

uint16_t AppBaud(uint8_t *Baud)
{
    uint8_t Res = 0;
    uint8_t * Buff = N10_Driver.PriData.InstructionBuff;
    sprintf(Buff,"AT+IPR=%s",Baud);
    Res = prvSendInstruction(Buff,4,1000,0,"OK");
    return Res;
}

uint16_t AppRecv(uint8_t * Data,uint16_t MaxLen,uint16_t Timeout,uint8_t * Res)
{
    uint8_t * LenStr = 0,*pData = 0;
    uint16_t ValLen = 0,i = 0,LenRecv = 0;
    
    N10_CHK_PARAM(Data);
    N10_CHK_PARAM(Res);
    
    *Res = N10_Driver.Uart.Send("AT$MYNETREAD=0,2048",strlen("AT$MYNETREAD=0,2048"),1000);
    if(*Res == N10_True)
    {
        LenRecv = N10_Driver.Uart.Recv(Data,MaxLen,Timeout);
        if(LenRecv)
        {
            /* �ҵ����� */
            while((Data[i ++] != ',') && i < 2048);
            LenStr = &Data[i];
            /* �ѳ��ȱ�����pData */
            while(1)
            {
                if(Data[i] == '\r' && Data[i+1] == '\n')
                {
                    Data[i] = 0;
                    Data[i + 1] = 0;
                    pData = &Data[i + 2];
                    break;
                }
                i++;
            }
            /* �ַ�����intת�� */
            ValLen = atoi(LenStr);
            if(ValLen > 2048)return 0;
            /*2016--12--23--10--07--03(ZJYC): ����˵������ճ���Len���������Щ���������   */ 
            if(ValLen > MaxLen)ValLen = MaxLen;
            /* �������� */
            for(i = 0;i < ValLen;i ++)
            {
                Data[i] = pData[i];
            }
            return ValLen;
        }
    }
    return 0;
}

uint16_t AppClose()
{
    uint8_t Res = prvSendInstruction("AT$MYNETCLOSE=0\r\n",4,300,0,"$MYNETCLOSE");
    return Res;
}

uint16_t AppInit()
{
    uint8_t Res = 0;
    
    N10_CLR_BIT(N10_Driver.PriData.FlagGroup,N10_FLAG_CONNECTED);
    Res = AppATE0();if(Res != N10_True)return Res;
    Res = AppCREG();if(Res != N10_True)return Res;
    Res = AppCGREG();if(Res != N10_True)return Res;
    Res = AppDetectCCID();if(Res != N10_True)return Res;
    Res = AppBaud("115200");if(Res != N10_True)return Res;
    
    return Res;
}
/*
****************************************************
*  ������         : 
*  ��������       : 
*  ����           : 
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
static uint8_t N10_TimingProcess(uint16_t Period)
{
    N10_CHK_PARAM(Period);
    
    N10_Driver.PriData.Counter += Period;
    if(N10_Driver.PriData.Counter > 100000)N10_Driver.PriData.Counter = 0;
    /* �ޤ�10������ ��Ҫ���ʤ󣿣� */
    if(N10_Driver.PriData.Counter % (Period * 10) == 0)
    {
        /* �յ�һ֡���� */
        //if(N10_Driver.Uart.Recv(Buf,&LenRecv,0) != 0)
        //{
        //    /* ��������Ҫ�ж�����͸�����ݻ������� */
        //    if(Buf[0] == 'S' && Buf[0] == 'P' &&Buf[0] == 'P' &&Buf[0] == ':')
        //    {
        //        
        //    }
        //}
    }
}
/*
****************************************************
*  ������         : 
*  ��������       : �����б�Ҫ֪�����ǵȴ��˶೤ʱ��
*  ����           : 
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
static uint32_t GetDelayed(uint32_t ConstCounter)
{
    if(ConstCounter <= N10_Driver.PriData.Counter)return (N10_Driver.PriData.Counter - ConstCounter);
    return 100000 - ConstCounter + N10_Driver.PriData.Counter;
}


static uint16_t N10_UartInit(uint16_t Baud)
{
    /*2016--12--26--09--00--30(ZJYC):    */ 
    COMCONFIG com_config;
    ComConfig((COMCONFIG *)&com_config, 115200, 8, 1, 0);
    ComClose( M72D_Com );
    ComOpen( M72D_Com ,&com_config );
    M72D_RxFinsh = 0;
}
static uint16_t N10_UartSend(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout)
{
    UartReset(M72D_Com);
    ComSend(M72D_Com, (uint8_t *)Data, strlen(Data));
}
static uint16_t N10_UartRecv(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout)
{
    uint16_t GPRSBufLen = 0;
    /* ����������������N10_False */
    if(N10_CHK_BIT(N10_Driver.PriData.FlagGroup,N10_FLAG_RxLock))return N10_False;
    /* ���� �� �̰߳�ȫ */
    N10_SET_BIT(N10_Driver.PriData.FlagGroup,N10_FLAG_RxLock);
    /* ���²��֣���������Ҳ���������ӣ��� */
    M72D_RxFinsh = 0;
    SetTimer1(Timeout);
    while( M72D_RxFinsh == 0 )
    {
        if( !CheckTimer1() )return 0;
    }
    if( M72D_RxFinsh )
    {
        StopTimer1();
        M72D_RxFinsh = 0;
        ComGetRxCount(M72D_Com, &GPRSBufLen);
        if(ExpectLen != 0 && GPRSBufLen>ExpectLen) GPRSBufLen=ExpectLen-1;
        ComRecv(M72D_Com, Data, &GPRSBufLen);
        UartReset(M72D_Com);
        return GPRSBufLen;
    }
    /* ������� */
    N10_CLR_BIT(N10_Driver.PriData.FlagGroup,N10_FLAG_RxLock);
    return 0;
}

N10_DriverTypedef N10_Driver = 
{
    {
        N10_UartInit,
        N10_UartSend,
        N10_UartRecv
    },
    {
        0x00
    },
    N10_TimingProcess,
};








