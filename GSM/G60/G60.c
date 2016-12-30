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

#include "G60.h"



/*�궨��  */





/*��������*/





/*��������*/




/*��������*/

uint16_t G60_UartRecv(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
uint16_t G60_UartSend(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
uint16_t G60_UartInit(uint16_t Baud);
uint8_t AT_TCP_Close(void);

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
static uint8_t prvSplitString(uint8_t * In,uint16_t Len,uint8_t ** Out,uint8_t IndexMax)
{
    //CharUseless������һ�������ַ�
    uint8_t i = 0,tIndex = 0,CharUseless = 0;
    /* ������� */
    assert(In);
    assert(Out);
    assert(Len);
    //���ַ����ĳ�����һ���޶�
    while(In[i] && i < Len)
    {
        //������Щ�������ַ�
        if(In[i] == '\r' || In[i] == '\n' || In[i] == ':' || In[i] == ' ' || In[i] == '+' || In[i] == ',')
        {
            In[i] = '\0';
            i ++;
            CharUseless = 0xff;
            /* �������޶����ַ����ĸ��� */
            if(tIndex >= IndexMax)return tIndex;
            continue;
        }
        //�洢���ֶεĵ�ַ
        if(CharUseless == 0xff){Out[tIndex++] = &In[i];CharUseless = 0x00;}
        i ++;
    }
    
    return tIndex;
}

/*
****************************************************
*  ������         : prvSendInstruction
*  ��������       : 
*  ����           : 
                        Instruction:ָ��
                        RetryCnt:�ش�����
                        Timeout:��ʱ
                        MatchIndex:ƥ������
                        Match:ƥ����
*  ����ֵ         : 
                        (uint16_t)G60_True            //�ɹ�
                        G60_DISCONNECTED    //���ӶϿ�
                        G60_RetryCost       //����δ��
                        
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
static uint8_t prvSendInstruction(
                                    uint8_t * Instruction,      /* ָ������ */
                                    uint16_t RetryCnt,           /* ���Դ��� */
                                    uint16_t RetryInterval,      /* ���Լ�� (��λ��ms)*/
                                    uint16_t Timeout,           /* �ظ���ʱʱ�� (��λ��ms)*/
                                    int8_t MatchIndex,         /* ƥ������ */
                                    uint8_t * Match             /* ƥ���� */
                                    )
{
    //uint32_t CurCounter = G60_Driver.PriData.Counter;
    /* ����ٶ�ָ��Ȳ����ܳ���100�ֽ� */
    uint8_t RetryTemp = 0,**ParamSplitTemp = G60_Driver.PriData.ParamSplit;
    uint8_t * Buf = G60_Driver.PriData.InstructionBuff;
    uint16_t LenRecv = 0,i = 0;
    /* ������ */
    assert(Instruction);
    if(MatchIndex >= 0)assert(Match);
    /* ����ָ�� */
    G60_Driver.Uart.Send(Instruction,strlen((char *)Instruction),Timeout);
    for(;;)
    {
        /* �յ�һ֡��Ϣ ֱ���ж��Ƿ�ƥ��Match */
        LenRecv = G60_Driver.Uart.Recv(Buf,0,Timeout);
        /* ����յ����� */
        if(LenRecv)
        {
            uint8_t i = 0;
            /* �ֽ��ַ�����ParamSplitTemp */
            uint8_t tIndex = prvSplitString(Buf,LenRecv,ParamSplitTemp,10);
            if(MatchIndex < 0)return G60_NeedNotMatch;
            /* �ж����ַ��� */
            if(strcmp((const char *)ParamSplitTemp[MatchIndex],(const char *)Match) == 0 )
            {
                return (uint16_t)G60_True;
            }
            else
            {
                /* ����Ƿ��������ַ� */
                for(i = 0;i < tIndex;i ++)
                {
                    if(strcmp((const char *)ParamSplitTemp[i],"CLOSE") == 0 )
                    {
                        /* �رգ��������� */
                        return G60_DISCONNECTED;
                    }
                }
                if(++RetryTemp < RetryCnt)
                {
                    /* һ��ʱ��֮�������ٴη������� */
                    Delay10ms(RetryInterval / 10);
                    G60_Driver.Uart.Send(Instruction,strlen((const char *)Instruction),Timeout);
                }
                else
                {
                    return G60_RetryCost;
                }
            }
        }
        /* ����û���յ����� */
        else
        {
            /* �ش� */
            if(++RetryTemp < RetryCnt)
            {
                //CurCounter = G60_Driver.PriData.Counter;
                G60_Driver.Uart.Send(Instruction,strlen((const char *)Instruction),Timeout);
            }
            else
            {
                return G60_RetryCost;
            }
        }
        if(RetryTemp >= RetryCnt)return G60_RetryCost;
    }
}

/*
****************************************************
*  ������         : AT_Init
*  ��������       : ָ���ʼ��
*  ����           : 
*  ����ֵ         : ͬprvSendInstruction
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
uint8_t AT_Init(void)
{
    uint8_t Res = 0;
    /* �ػ��� */
    Res = prvSendInstruction((uint8_t*)"ATE0\r\n",6,300,300,0,(uint8_t*)"OK");
    if(Res != (uint16_t)G60_True)return (uint16_t)G60_EchoFail;
    /* ���ò����� */
    Res = prvSendInstruction((uint8_t*)"AT+IPR=115200&W\r\n",6,300,300,0,(uint8_t*)"OK");
    if(Res != (uint16_t)G60_True)return (uint16_t)G60_SetBDFail;
    /* ȷ�� SIM ���� PIN ���ѽ� */
    Res = prvSendInstruction((uint8_t*)"AT+CPIN?\r\n",10,300,300,1,(uint8_t*)"READY");
    /* ȷ�������ɹ� */
    Res = prvSendInstruction((uint8_t*)"AT+CREG?\r\n",2,300,300,2,(uint8_t*)"1");
    Res = prvSendInstruction((uint8_t*)"AT+CREG?\r\n",2,300,300,2,(uint8_t*)"5");
    /* ��ѯ GPRS �����Ƿ�ɹ� */
    Res = prvSendInstruction((uint8_t*)"AT+CGATT?\r\n",6,300,300,1,(uint8_t*)"1");
    /* �� Context 0 ��Ϊǰ̨ Context�� */
    Res = prvSendInstruction((uint8_t*)"AT+QIFGCNT=0\r\n",6,300,300,0,(uint8_t*)"OK");
    /* ���� GPRS �� APN�� */
    Res = prvSendInstruction((uint8_t*)"AT+QICSGP=1,\"CMNET\"\r\n",6,300,300,0,(uint8_t*)"OK");
    /* ���յ����ݺ������ʾ:" +QIRDI: <id>,<sc>,<sid>"�� */
    Res = prvSendInstruction((uint8_t*)"AT+QINDI=1\r\n",6,300,300,0,(uint8_t*)"OK");
    /* ��͸��ģʽ */
    Res = prvSendInstruction((uint8_t*)"AT+QIMODE=0\r\n",3,300,300,0,(uint8_t*)"OK");
    /* ��·���� */
    Res = prvSendInstruction((uint8_t*)"AT+QIMUX=0\r\n",3,300,300,0,(uint8_t*)"OK");
    /* �����������ݻ��� */
    Res = prvSendInstruction((uint8_t*)"AT+QISDE=1\r\n",6,300,300,0,(uint8_t*)"OK");
    return Res;
}

/*
****************************************************
*  ������         : AT_TCP
*  ��������       : ����TCP
*  ����           : 
                    IP��
                    Port��
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/

uint8_t AT_TCP(uint8_t * IP,uint8_t * Port)
{
    uint8_t * Buff = G60_Driver.PriData.InstructionBuff,Res = 0,Len = 0,Retry = 6;
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit,WaitTime = 0;
    /* ������� (���а���Բ���)*/
    assert(IP);
    assert(Port);
    /* �������ݣ������������� */
    strcpy(G60_Driver.PriData.LastIP,IP);
    strcpy(G60_Driver.PriData.LastPort,Port);
    
    ReConnect_1:
    /* ����ָ�� */
    sprintf((char *)Buff,"AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",(const char *)IP,(const char *)Port);
    Res = prvSendInstruction(Buff,3,500,1000,0,(uint8_t*)"OK");
    /* ���Ȼ᷵�ء�OK����ȷ��ָ���ʽ��ȷ��һ��ʱ��֮�󷵻ء�CONNECT OK��,˵�����ӳɹ� */
    
    /* ������ء�ALREADY��˵���Ѿ����ӳɹ����ȶϿ��������� */
    if(Res != (uint16_t)G60_True)
    {
        if(strcmp((const char *)ParamSplitTemp[0],"ALREADY") == 0)
        {
            /* �Ͽ��������� */
            AT_TCP_Close();
            if(--Retry)goto ReConnect_1;
            return (uint16_t)G60_False;
        }
    }
    
    /* �������ӽ�� */
    /* 10S�޷���  ����Ч��ʧ�� */
    Len = G60_Driver.Uart.Recv(Buff,0,10000);
    if(Len == 0) 
    {
        goto ReConnect_1;
    }
    else
    {
        prvSplitString(Buff,Len,ParamSplitTemp,10);
        if((strcmp((const char *)ParamSplitTemp[0],"CONNECT") == 0 ) && (strcmp((const char*)ParamSplitTemp[1],"OK") == 0))
        {
            return (uint16_t)G60_True;
        }
        else
        {
            AT_TCP_Close();
            goto ReConnect_1;
        }
    }
    return (uint16_t)G60_False;
}


/*
****************************************************
*  ������         : AT_TCP_Close
*  ��������       : �Ͽ�����
*  ����           : 
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/

uint8_t AT_TCP_Close(void)
{
    uint8_t Res = 0;
    Res = prvSendInstruction((uint8_t*)"AT+QICLOSE\r\n",6,500,300,1,(uint8_t*)"OK");
    Res = prvSendInstruction((uint8_t*)"AT+QIDEACT\r\n",6,500,300,1,(uint8_t*)"OK");
    return (uint16_t)G60_True;
}

/*
****************************************************
*  ������         : TCP_SendFinished
*  ��������       : ȷ�����ݷ������
*  ����           : 
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t TCP_SendFinished(void)
{
    uint8_t * Buff = G60_Driver.PriData.InstructionBuff,RxLen = 0,Res = 0,Retry = 10;
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit;
    uint16_t All = 0,Acked = 0,NotSure;
    
    while(1)
    {
        Res = prvSendInstruction((uint8_t *)"AT+QISACK\r\n",1,300,300,-1,(uint8_t *)"0");
        /* ��������||�Ѿ���Ӧ�������||û��Ӧ������� */
        All = atoi(ParamSplitTemp[1]);
        Acked = atoi(ParamSplitTemp[2]);
        NotSure = atoi(ParamSplitTemp[3]);
        
        /* ������� */
        if(All == Acked && All != 0)
        {
            return (uint16_t)G60_True;
        }
        /* ���ӶϿ� */
        if(All == Acked && All == 0)
        {
            /* ���ⲻС */
            return (uint16_t)G60_DISCONNECTED;
        }
        /* ���� */
        /* Retry = 10 �� 10S������û�з�����ɣ���ȷ��ʧ�� */
        if(--Retry == 0 )return (uint16_t)G60_RetryCost;
        Delay10ms(100);
    }
    return (uint16_t)G60_False;
}

/*
****************************************************
*  ������         : AT_STATE
*  ��������       : ��ȡģ�鵱ǰ״̬
*  ����           : 
*  ����ֵ         : �� G60_ResTypedef
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_STATE(void)
{
    uint8_t * Buff = G60_Driver.PriData.InstructionBuff,Res = 0;
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit;

    Res = prvSendInstruction("AT+QISTAT\r\n",6,500,300,-1,(uint8_t *)">");
    if(Res == (uint16_t)G60_NeedNotMatch)
    {
        /* ��ʼ�� */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"INITIAL"))
        {
            return (uint16_t)G60_IP_INITIAL;
        }
        /* �������� */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"START"))
        {
            return (uint16_t)G60_IP_START;
        }
        /* ���ó��� */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"CONFIG"))
        {
            return (uint16_t)G60_IP_CONFIG;
        }
        /* ���� GPRS/CSD ������ */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"IND"))
        {
            return (uint16_t)G60_IP_IND;
        }
        /* ���ճ������� */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"GPRSACT"))
        {
            return (uint16_t)G60_IP_GPRSACT;
        }
        /* ��ñ��� IP ��ַ */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"STATUS"))
        {
            return (uint16_t)G60_IP_STATUS;
        }
        /* TCP ������ */
        if(G60_STR_CMP(ParamSplitTemp[2],"TCP")     && G60_STR_CMP(ParamSplitTemp[3],"CONNECTING"))
        {
            return (uint16_t)G60_TCP_CONNECTING;
        }
        /* UDP ������ */
        if(G60_STR_CMP(ParamSplitTemp[2],"UDP")     && G60_STR_CMP(ParamSplitTemp[3],"CONNECTING"))
        {
            return (uint16_t)G60_UDP_CONNECTING;
        }
        /* TCP/UDP ���ӹر� */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"CLOSE"))
        {
            return (uint16_t)G60_IP_CLOSE;
        }
        /* TCP/UDP ���ӳɹ� */
        if(G60_STR_CMP(ParamSplitTemp[2],"CONNECT") && G60_STR_CMP(ParamSplitTemp[3],"OK"))
        {
            return (uint16_t)G60_CONNECT_OK;
        }
        /* GPRS/CSD �����쳣�ر� */
        if(G60_STR_CMP(ParamSplitTemp[2],"PDP")     && G60_STR_CMP(ParamSplitTemp[3],"DEACT"))
        {
            return (uint16_t)G60_PDP_DEACT;
        }
    }
    return (uint16_t)G60_True;
}

/*
****************************************************
*  ������         : TCP_Send
*  ��������       : ��������
*  ����           : 
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t TCP_Send(uint8_t * Data,uint16_t Len)
{
    uint8_t * Buff = G60_Driver.PriData.InstructionBuff,RxLen = 0,Res = 0,Retry = 8;
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit;
    /* ������� */
    assert(Data);
    assert(Len);
    /* ȷ������������״̬ */
    Res = AT_STATE();
    if(Res != (uint16_t)G60_CONNECT_OK)
    {
        /* �����Ѿ��Ͽ�   ���ǻ���������*/
        if(
                Res == (uint16_t)G60_PDP_DEACT  || 
                Res == (uint16_t)G60_IP_INITIAL || 
                Res == (uint16_t)G60_IP_CLOSE)
        {
            /* ������Ҫȷ��������ȷ */
            assert((uint32_t)G60_Driver.PriData.LastIP);
            assert((uint32_t)G60_Driver.PriData.LastPort);
            Res = AT_TCP(G60_Driver.PriData.LastIP,G60_Driver.PriData.LastPort);
            /* ���ӳɹ�  ��ʼ�������� */
            if(Res == (uint16_t)G60_True) goto SendDataRetry;
            /* �޷����� */
            return (uint16_t)G60_False;
        }
    }
    
    SendDataRetry:
    /* ����Ҫ���͵��ֽ��� ��ȷ��ģ��ظ� ">" */
    sprintf((char *)Buff,"AT+QISEND=%d\r\n",Len);
    Res = prvSendInstruction(Buff,6,500,100,0,(uint8_t *)">");
    if(Res != (uint16_t)G60_True)
    {
        /* ģ��û�лظ� ">"���ش��� */
        if(--Retry){Delay10ms(50);goto SendDataRetry;}
    }
    /* �������� */
    G60_Driver.Uart.Send(Data,Len,100);
    /* �����ȼ�������Ƿ�����ȷ���ڼ�������Ƿ������ 
    RxLen >= Len˵��ģ����˻�������֮�⣬���ظ��˽��*/
    RxLen = G60_Driver.Uart.Recv(Buff,0,300);
    if(RxLen != 0 && RxLen >= Len)
    {
        uint16_t i = 0;
        for(i = 0;i < Len;i++)
        {
            if(Data[i] != Buff[i])
            {
                /* ģ����Ե����ݲ��� */
                if(--Retry)goto SendDataRetry;
                return (uint16_t)G60_False;
            }
        }
        prvSplitString(&Buff[i],RxLen,ParamSplitTemp,10);
        if((strcmp((const char*)ParamSplitTemp[0],"SEND") == 0 ) && (strcmp((const char *)ParamSplitTemp[1],"OK") == 0))
        {
            return TCP_SendFinished();
        }
    }
    /* ʧ���ش� */
    if(--Retry)goto SendDataRetry;
    return (uint16_t)G60_False;
}

uint16_t TCP_Recv(uint8_t * Data,uint16_t MaxLen,uint16_t Timeout,uint16_t * RxLen)
{
    uint16_t LenRecv = 0,DataLen = 0,Retry = 10;
    /*2016--12--29--14--26--33(ZJYC): ����˵�ˣ�Ϊ��߿ռ������ʣ���ֱ�������Ļ�������Data   */ 
    /* �������Ӧ�ò�Ļ���������1024�������1200 */
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit,*pData = 0;
    
    //assert(MaxLen >= 1200);
    
    while(1)
    {
        G60_Driver.Uart.Send("AT+QIRD=0,1,0,1024\r\n",strlen("AT+QIRD=0,1,0,1024\r\n"),Timeout);
        LenRecv = G60_Driver.Uart.Recv(Data,0,Timeout);
        prvSplitString(Data,LenRecv,ParamSplitTemp,5);
        
        if(G60_STR_CMP(ParamSplitTemp[0],"OK"))
        {
            /* û���յ�����   �ȴ�1S���ٲ�*/
            if(--Retry)
            {
                Delay10ms(100);
                continue;
            }
            else
            {
                *RxLen = 0;
                return (uint16_t)G60_RetryCost;
            }
        }
        if(G60_STR_CMP(ParamSplitTemp[0],"ERROR"))
        {
            /* ���ִ��� */
            *RxLen = 0;
            return (uint16_t)G60_DISCONNECTED;
        }
        /* ���ճɹ� */
        if(strcmp(ParamSplitTemp[0],"QIRD") == 0 && strcmp(ParamSplitTemp[3],"TCP") == 0)
        {
            uint16_t i = 0;
            DataLen = atoi(ParamSplitTemp[4]);
            
            while(ParamSplitTemp[4][i++] != '\n' && i < 2048);
            if(i >= 2048)return (uint16_t)G60_False;
            /* �����׵�ַ */
            pData = (uint8_t *)&ParamSplitTemp[4][i];
            /* �������� */
            for(i = 0;i < DataLen;i ++)
            {
                Data[i] = pData[i];
            }
            *RxLen = DataLen;
            return (uint16_t)G60_True;
        }
        
    }
    return (uint16_t)G60_False;
}

uint16_t AT_SetApn(uint8_t * APN)
{
    uint8_t * Buff = G60_Driver.PriData.InstructionBuff;
    
    /* ������� */
    assert(APN);
    /* �������� */
    strcpy(G60_Driver.PriData.LastAPN,APN);
    /* �������� */
    sprintf(Buff,"AT+QICSGP=1,\"%s\"\r\n",APN);
    if((uint16_t)G60_True != prvSendInstruction(Buff,6,500,300,0,"OK"))return (uint16_t)G60_False;
    return (uint16_t)G60_True;
}

uint16_t AT_CSQ(uint8_t * CSQ,uint8_t * Count)
{
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit;
    uint8_t Res = 0;
    
    assert(CSQ);
    assert(Count);
    
    Res = prvSendInstruction((uint8_t*)"AT+CSQ\r\n",6,500,300,3,"OK");
    if(Res != (uint16_t)G60_True)return (uint16_t)G60_SignalFail;
    
    if((uint32_t)ParamSplitTemp[1] == 0 || (uint32_t)ParamSplitTemp[2] == 0)
    {
        return (uint16_t)G60_SignalFail;
    }
    
    *CSQ = (uint8_t)atoi((const char *)ParamSplitTemp[1]);
    *Count = (uint8_t)atoi((const char *)ParamSplitTemp[2]);
    
    if(*CSQ < 0 || *CSQ > 31)return (uint16_t)G60_SignalLow;
    
    Res = prvSendInstruction((uint8_t*)"AT+CPIN?\r\n",10,300,300,1,(uint8_t*)"READY");
    if(Res != (uint16_t)G60_True)return (uint16_t)G60_SIMFail;
    
    return G60_True;
}

G60_DriverTypedef G60_Driver = 
{
    {
        G60_UartInit,
        G60_UartSend,
        G60_UartRecv
    },
};





















