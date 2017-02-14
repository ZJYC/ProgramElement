/*
****************************************************
*  �ļ���             : 
*  ����               : --
*  �汾               : 
*  ��д����           : 
*  ���               : 
*  �����б�           : 
*  ��ʷ�汾           : 
*****************************************************
*/


/*ͷ�ļ�  */

#include "G63.h"

/*�궨��  */
/*��������*/
/*��������*/
/*��������*/

uint16_t G63_UartRecv(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout,uint8_t CLR);
uint16_t G63_UartSend(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
uint16_t G63_UartInit(uint16_t Baud);
uint16_t AT_TCP_Close(void);
uint16_t AT_CREG_CGATT(uint16_t Retry,uint16_t RetryInterval);
uint16_t AT_EnterCommandMode(void);
uint16_t AT_EnterDataMode(void);

/*��������*/

/*
****************************************************
*  ������         : prvDoesNeedShield
*  ��������       : �鿴�ַ��Ƿ���������UselessChar
*  ����           : Char�����Ա��ַ�
*  ����ֵ         : �ҵ�����(uint16_t)G63_True�����򷵻�(uint16_t)G63_False
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
static uint16_t prvWithinUselessChar(uint8_t Char)
{
    /* �˴����������ַ������㴦�� */
    const uint8_t UselessChar[] = {'\r','\n',':',' ','+',',','\"','/',};
    uint16_t i = 0;
    
    for(i = 0;i < sizeof(UselessChar)/sizeof(UselessChar[0]); i++)
    {
        if(Char == UselessChar[i])return (uint16_t)G63_True;
    }
    
    return (uint16_t)G63_False;
}

/*
****************************************************
*  ������         : prvSplitString
*  ��������       : �ֽ��ַ���
*  ����           : 
                    In:ָ������
                    Len:ָ���
                    Out:��ָ��ֽ��������ά�ַ�����
                    IndexMax���ֽ�������ַ���
                    
                    ������
                    
                    ���룺��\r\n+CREG: 0,1\r\n\r\nOK\r\n��
                    ����� 
                            Out[0] = "CREG",
                            Out[1] = "0",
                            Out[2] = "1",
                            Out[3] = "OK".
                    ���أ��ֽ�������ַ�������
                    BUG:In �Ŀ�ͷ����Ϊ�������ַ�
*  ����ֵ         : 
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
static uint8_t prvSplitString(uint8_t * In,uint16_t Len,uint8_t ** Out,uint8_t IndexMax)
{
    //CharUseless������һ�������ַ�
    uint8_t i = 0,tIndex = 0,CharUseless = 0;
    /* ������� */
    G63_Asert(In);
    G63_Asert(Out);
    G63_Asert(Len);
    //���ַ����ĳ�����һ���޶�
    while(In[i] && i < Len)
    {
        //������Щ�������ַ�
        if(prvWithinUselessChar(In[i]) == (uint16_t)G63_True)
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
                        RetryInterval:���Լ��(��λ��ms)
                        Timeout:��ʱ(��λ��ms)
                        MatchIndex:ƥ������(С��0��ʾ������ƥ��)
                        Match:ƥ����
*  ����ֵ         : 
                        G63_True            //�ɹ�
                        G63_DISCONNECTED    //���ӶϿ�
                        G63_RetryCost       //����δ��
                        
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
static uint8_t prvSendInstruction(
                                    uint8_t * Instruction,          /* ָ������ */
                                    uint16_t RetryCnt,              /* ���Դ��� */
                                    uint16_t RetryInterval,         /* ���Լ�� (��λ��ms)*/
                                    uint16_t Timeout,               /* �ظ���ʱʱ�� (��λ��ms)*/
                                    int8_t MatchIndex,              /* ƥ������ */
                                    uint8_t * Match                 /* ƥ���� */
                                    )
{
    uint8_t RetryTemp = 0,**ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    uint8_t * Buf = G63_Driver.PriData.InstructionBuff;
    uint16_t LenRecv = 0;
    /* ������ */
    G63_Asert(Instruction);
    if(MatchIndex >= 0)G63_Asert(Match);
    /* ����ָ�� */
    G63_Driver.Uart.Send(Instruction,strlen((char *)Instruction),Timeout);
    for(;;)
    {
        /* �յ�һ֡��Ϣ ֱ���ж��Ƿ�ƥ��Match */
        LenRecv = G63_Driver.Uart.Recv(Buf,0,Timeout,0);
        /* ����յ����� */
        if(LenRecv)
        {
            uint8_t i = 0;
            /* �ֽ��ַ�����ParamSplitTemp */
            uint8_t tIndex = prvSplitString(Buf,LenRecv,ParamSplitTemp,10);
            /* �û���ϣ���������Ƚ�Match�� */
            if(MatchIndex < 0)return G63_NeedNotMatch;
            /* �ж����ַ��� */
            if(G63_STR_EQL(ParamSplitTemp[MatchIndex],Match))
            {
                return (uint16_t)G63_True;
            }
            else
            {
                /* ����Ƿ��������ַ� */
                for(i = 0;i < tIndex;i ++)
                {
                    if(strcmp((const char *)ParamSplitTemp[i],"CLOSE") == 0 )
                    {
                        /* �رգ��������� */
                        return G63_DISCONNECTED;
                    }
                }
                if(++RetryTemp < RetryCnt)
                {
                    /* һ��ʱ��֮�������ٴη������� */
                    Delay10ms(RetryInterval / 10);
                    G63_Driver.Uart.Send(Instruction,strlen((const char *)Instruction),Timeout);
                }
                else
                {
                    return G63_RetryCost;
                }
            }
        }
        /* ����û���յ����� */
        else
        {
            /* �ش� */
            if(++RetryTemp < RetryCnt)
            {
                G63_Driver.Uart.Send(Instruction,strlen((const char *)Instruction),Timeout);
            }
            else
            {
                return G63_RetryCost;
            }
        }
        if(RetryTemp >= RetryCnt)return G63_RetryCost;
    }
}

/*
****************************************************
*  ������         : AT_Init
*  ��������       : ָ���ʼ��
*  ����           : 
*  ����ֵ         : ͬprvSendInstruction
                    ���⣺
                    G63_EchoFail���ػ���ʧ��
                    G63_SetBDFail�����ò�����ʧ��
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_Init(void)
{
    /* ##�ڴ˺���ǰģ���Ѿ�����## */
    uint16_t Res = 0;
    /* �ػ��� */
    Res = prvSendInstruction((uint8_t*)"ATE0\r\n",30,200,300,0,(uint8_t*)"OK");
    if(Res != (uint16_t)G63_True)return (uint16_t)G63_EchoFail;
    /* ���ò����� */
    Res = prvSendInstruction((uint8_t*)"AT+IPR=115200&W\r\n",6,200,300,0,(uint8_t*)"OK");
    if(Res != (uint16_t)G63_True)return (uint16_t)G63_SetBDFail;
    /* ȷ�� SIM ���� PIN ���ѽ� */
    /* �˲��ִ�Լ��ʱ 9S��ѭ�� 6*2 = 12S*/
    Res = prvSendInstruction((uint8_t*)"AT+CPIN?\r\n",60,200,300,1,(uint8_t*)"READY");
    /* ȷ�������ɹ� */
    /* ��ѯ GPRS �����Ƿ�ɹ� */
    /* �˲��ִ�Լ��ʱ 7S��ѭ�� 5*2 = 10S */
    Res = AT_CREG_CGATT(50,200);
    /* �� Context 0 ��Ϊǰ̨ Context�� */
    Res = prvSendInstruction((uint8_t*)"AT+QIFGCNT=0\r\n",6,100,300,0,(uint8_t*)"OK");
    /* ���� GPRS �� APN�� */
    Res = prvSendInstruction((uint8_t*)"AT+QICSGP=1,\"CMNET\"\r\n",6,100,300,0,(uint8_t*)"OK");
    /* ���յ����ݺ������ʾ:" +QIRDI: <id>,<sc>,<sid>"�� */
    Res = prvSendInstruction((uint8_t*)"AT+QINDI=1\r\n",6,100,300,0,(uint8_t*)"OK");
    /* ��͸��ģʽ */
    Res = prvSendInstruction((uint8_t*)"AT+QIMODE=0\r\n",3,100,300,0,(uint8_t*)"OK");
    /* ��·���� */
    Res = prvSendInstruction((uint8_t*)"AT+QIMUX=0\r\n",3,100,300,0,(uint8_t*)"OK");
    /* �����������ݻ��� */
    Res = prvSendInstruction((uint8_t*)"AT+QISDE=1\r\n",6,100,300,0,(uint8_t*)"OK");
    return Res;
}

/*
****************************************************
*  ������         : AT_TCP
*  ��������       : ����TCP
*  ����           : 
                    IP��IP��ַ���ַ�����ʽ����255.255.255.255��
                    Port���˿ںţ��ַ�����ʽ����6666��
*  ����ֵ         : 
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/

uint16_t AT_TCP(uint8_t * IP,uint8_t * Port)
{
    uint8_t * Buff = G63_Driver.PriData.InstructionBuff,Len = 0,Retry = 6;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    uint16_t Res = 0;
    /* ������� (���а���Բ���)*/
    G63_Asert(IP);
    G63_Asert(Port);
    /* �������ݣ������������� */
    if((uint32_t)G63_Driver.PriData.LastIP != (uint32_t)IP)G63_STR_CPY(G63_Driver.PriData.LastIP,IP);
    if((uint32_t)G63_Driver.PriData.LastPort != (uint32_t)IP)G63_STR_CPY(G63_Driver.PriData.LastPort,Port);
    
    ReConnect_1:
    /* ����ָ�� */
    sprintf((char *)Buff,"AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",(const char *)IP,(const char *)Port);
    Res = prvSendInstruction(Buff,3,500,1000,0,(uint8_t*)"OK");
    /* ���Ȼ᷵�ء�OK����ȷ��ָ���ʽ��ȷ��һ��ʱ��֮�󷵻ء�CONNECT OK��,˵�����ӳɹ� */
    
    /* ������ء�ALREADY��˵���Ѿ����ӳɹ����ȶϿ��������� */
    if(Res != (uint16_t)G63_True)
    {
        if(strcmp((const char *)ParamSplitTemp[0],"ALREADY") == 0)
        {
            /* �Ͽ��������� */
            AT_TCP_Close();
            if(--Retry)goto ReConnect_1;
            return (uint16_t)G63_False;
        }
    }
    
    /* �������ӽ�� */
    /* 5S�޷���  ����Ч��ʧ�� */
    Len = G63_Driver.Uart.Recv(Buff,0,5000,0);
    if(Len == 0) 
    {
        if(--Retry)goto ReConnect_1;
    }
    else
    {
        prvSplitString(Buff,Len,ParamSplitTemp,10);
        if(G63_STR_EQL(ParamSplitTemp[0],"CONNECT") && G63_STR_EQL(ParamSplitTemp[1],"OK"))
        {
            return (uint16_t)G63_True;
        }
        else
        {
            AT_TCP_Close();
            if(--Retry)goto ReConnect_1;
            return (uint16_t)G63_False;
        }
    }
    return (uint16_t)G63_False;
}

/*
****************************************************
*  ������         : AT_CREG_CGATT
*  ��������       : �ȴ�ֱ��������GPRS���ųɹ�
*  ����           : 
                    Retry�����Դ���������������
                    RetryInterval:���Լ��������Ϊ0��
*  ����ֵ         : 
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_CREG_CGATT(uint16_t Retry,uint16_t RetryInterval)
{
    //uint16_t Res = (uint16_t)G63_False;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    
    G63_Asert(Retry);
    /* RetryInterval����Ϊ0 */
    
    /* ȷ�������ɹ� */
    for(;;)
    {
        prvSendInstruction((uint8_t*)"AT+CREG?\r\n",1,300,300,-1,(uint8_t*)"1");
        if(G63_STR_EQL(ParamSplitTemp[2],"1") || G63_STR_EQL(ParamSplitTemp[2],"5"))
        {
            /* �����ɹ� */
            break;
        }
        else
        {
            /* ���� */
            if(--Retry == 0)
            {
                return (uint16_t)G63_False;
            }
            Delay10ms(RetryInterval / 10);
        }
    }
    /* ��ѯ GPRS �����Ƿ�ɹ� */
    for(;;)
    {
        prvSendInstruction((uint8_t*)"AT+CGATT?\r\n",1,300,300,-1,(uint8_t*)"1");
        if(G63_STR_EQL(ParamSplitTemp[1],"1") || G63_STR_EQL(ParamSplitTemp[1],"5"))
        {
            /* ���ųɹ� */
            break;
        }
        else
        {
            /* ���� */
            if(--Retry == 0)
            {
                return (uint16_t)G63_False;
            }
            Delay10ms(RetryInterval / 10);
        }
    }
    return (uint16_t)G63_True;
}


/*
****************************************************
*  ������         : AT_TCP_Close
*  ��������       : �Ͽ�����
*  ����           : 
*  ����ֵ         : 
*  ����           : --
*  ��ʷ�汾       : ʵ��Ͽ����ӻ�û��ʧ�ܹ�
*****************************************************
*/
uint16_t AT_TCP_Close(void)
{
    prvSendInstruction((uint8_t*)"AT+QICLOSE\r\n",6,500,300,1,(uint8_t*)"OK");
    prvSendInstruction((uint8_t*)"AT+QIDEACT\r\n",6,500,300,1,(uint8_t*)"OK");
    return (uint16_t)G63_True;
}

/*
****************************************************
*  ������         : AT_TCP_SendFinished
*  ��������       : ȷ�����ݷ������
                    ģ��ظ���+QISACK: 500,300,200��˵����������Ϊ500��
                    �ɹ�����300������200û�ɹ���ACK��
                    ��ģ��ظ���+QISACK: 500,500,0��˵������ȫ���������
                    ��ģ��ظ���+QISACK: 0,0,0��˵�����ӶϿ�
*  ����           : 
*  ����ֵ         : 
                    G63_True�����ݷ������
                    G63_DISCONNECTED�������ѶϿ�
                    G63_RetryCost�����Դ����ľ�
                    G63_False�������ܷ������
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_TCP_SendFinished(uint16_t Retry,uint16_t RetryInterval)
{
    //uint8_t * Buff = G63_Driver.PriData.InstructionBuff,RxLen = 0;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    uint32_t All = 0,Acked = 0;
    
    while(1)
    {
        prvSendInstruction((uint8_t *)"AT+QISACK\r\n",1,300,300,-1,(uint8_t *)"0");
        /* ��������+�Ѿ���Ӧ�������+û��Ӧ������� */
        /* Maybe a BUG:atoi�����ֵ */
        All = atoi((const char *)ParamSplitTemp[1]);
        Acked = atoi((const char *)ParamSplitTemp[2]);
        //NotSure = atoi((const char *)ParamSplitTemp[3]);
        
        /* ������� */
        if(All == Acked && All != 0)
        {
            return (uint16_t)G63_True;
        }
        /* ���ӶϿ� */
        if(All == Acked && All == 0)
        {
            /* ���ⲻС */
            return (uint16_t)G63_DISCONNECTED;
        }
        if(--Retry == 0 )return (uint16_t)G63_RetryCost;
        Delay10ms(RetryInterval / 10);
    }
    //return (uint16_t)G63_False;
}

/*
****************************************************
*  ������         : AT_STATE
*  ��������       : ��ȡģ�鵱ǰ״̬
*  ����           : 
*  ����ֵ         : �� G63_ResTypedef
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_STATE(void)
{
    //uint8_t * Buff = G63_Driver.PriData.InstructionBuff;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit,Retry = 4;
    uint16_t Res = 0;
    
    /* ��������µ�Ƭ��������ȷ���գ��ʴ˴����ظ���ȡ */
    RepeatGetState:
    
    Res = prvSendInstruction("AT+QISTAT\r\n",6,500,300,-1,(uint8_t *)">");
    if(Res == (uint16_t)G63_NeedNotMatch)
    {
        /* ��ʼ�� */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"INITIAL"))
        {
            return (uint16_t)G63_IP_INITIAL;
        }
        /* �������� */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"START"))
        {
            return (uint16_t)G63_IP_START;
        }
        /* ���ó��� */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"CONFIG"))
        {
            return (uint16_t)G63_IP_CONFIG;
        }
        /* ���� GPRS/CSD ������ */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"IND"))
        {
            return (uint16_t)G63_IP_IND;
        }
        /* ���ճ������� */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"GPRSACT"))
        {
            return (uint16_t)G63_IP_GPRSACT;
        }
        /* ��ñ��� IP ��ַ */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"STATUS"))
        {
            return (uint16_t)G63_IP_STATUS;
        }
        /* TCP ������ */
        if(G63_STR_EQL(ParamSplitTemp[2],"TCP")     && G63_STR_EQL(ParamSplitTemp[3],"CONNECTING"))
        {
            return (uint16_t)G63_TCP_CONNECTING;
        }
        /* UDP ������ */
        if(G63_STR_EQL(ParamSplitTemp[2],"UDP")     && G63_STR_EQL(ParamSplitTemp[3],"CONNECTING"))
        {
            return (uint16_t)G63_UDP_CONNECTING;
        }
        /* TCP/UDP ���ӹر� */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"CLOSE"))
        {
            return (uint16_t)G63_IP_CLOSE;
        }
        /* TCP/UDP ���ӳɹ� */
        if(G63_STR_EQL(ParamSplitTemp[2],"CONNECT") && G63_STR_EQL(ParamSplitTemp[3],"OK"))
        {
            return (uint16_t)G63_CONNECT_OK;
        }
        /* GPRS/CSD �����쳣�ر� */
        if(G63_STR_EQL(ParamSplitTemp[2],"PDP")     && G63_STR_EQL(ParamSplitTemp[3],"DEACT"))
        {
            return (uint16_t)G63_PDP_DEACT;
        }
    }
    if(--Retry)goto RepeatGetState;
    return (uint16_t)G63_False;
}

/*
****************************************************
*  ������         : AT_TCP_Send
*  ��������       : ��������
                    ���Ǵ��˷������ݻ��ԣ��˴���Ҫһ�����Ϊ1200������������ݻ���
*  ����           : 
                    Data:���ݵ�ַ
                    Len:���������ݳ���
*  ����ֵ         : G63_False��ʧ��
                    G63_True���ɹ�
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_TCP_Send(uint8_t * Data,uint16_t Len)
{
    uint8_t * Buff = G63_Driver.PriData.InstructionBuff,Retry = 8,ReTransfer = 3;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    uint8_t Temp[1200] = {0x00};
    uint16_t Res = 0,RxLen = 0;
    /* ������� */
    G63_Asert(Data);
    G63_Asert(Len);
    RetransferLoop:
    /* ȷ������������״̬ */
    Res = AT_STATE();
    if(Res != (uint16_t)G63_CONNECT_OK)
    {
        /* �����Ѿ��Ͽ�   ���ǻ���������*/
        if(
                Res == (uint16_t)G63_PDP_DEACT  || 
                Res == (uint16_t)G63_IP_INITIAL || 
                Res == (uint16_t)G63_IP_CLOSE   ||
                Res == (uint16_t)G63_False)
        {
            /* ������Ҫȷ��������ȷ */
            G63_Asert((uint8_t)G63_Driver.PriData.LastIP);
            G63_Asert((uint8_t)G63_Driver.PriData.LastPort);
            Res = AT_TCP(G63_Driver.PriData.LastIP,G63_Driver.PriData.LastPort);
            /* ���ӳɹ�  ��ʼ�������� */
            if(Res == (uint16_t)G63_True) goto SendDataRetry;
            /* �޷����� */
            return (uint16_t)G63_False;
        }
    }
    SendDataRetry:
    /* ����Ҫ���͵��ֽ��� ��ȷ��ģ��ظ� ">" */
    sprintf((char *)Buff,"AT+QISEND=%d\r\n",Len);
    Res = prvSendInstruction(Buff,6,500,100,0,(uint8_t *)">");
    if(Res != (uint16_t)G63_True)
    {
        /* ģ��û�лظ� ">"���ش��� */
        if(--Retry){Delay10ms(30);goto SendDataRetry;}
    }
    /* �������� */
    G63_Driver.Uart.Send(Data,Len,100);
    /* �����ȼ�������Ƿ�����ȷ���ڼ�������Ƿ������ 
    RxLen >= Len˵��ģ����˻�������֮�⣬���ظ��˽��*/
    RxLen = G63_Driver.Uart.Recv(Temp,0,1000,0);
    if(RxLen != 0 && RxLen >= Len)
    {
        uint16_t i = 0;
        for(i = 0;i < Len;i++)
        {
            if(Data[i] != Temp[i])
            {
                /* ģ����Ե����ݲ��� */
                if(--Retry)goto SendDataRetry;
                return (uint16_t)G63_False;
            }
        }
        /* ģ��ظ���SEND OK��˵��ģ���Ѿ��յ����ݣ������������ݷ��ͳɹ� */
        prvSplitString(&Temp[i],RxLen,ParamSplitTemp,10);
        if(G63_STR_EQL(ParamSplitTemp[0],"SEND") && G63_STR_EQL(ParamSplitTemp[1],"OK"))
        {
            /* ������Լ�������Ӧ���� */
            //Res =  AT_TCP_SendFinished(150,200);
            
            //if(Res == (uint16_t)G63_True)return (uint16_t)G63_True;
            //if(--ReTransfer)goto RetransferLoop;
            //return (uint16_t)G63_False;
			return (uint16_t)G63_True;
        }
    }
    /* ʧ���ش� */
    if(--Retry)goto SendDataRetry;
    return (uint16_t)G63_False;
}

/*
****************************************************
*  ������         : AT_TCP_Recv
*  ��������       : ��ģ���������
*  ����           : 
                    Data�����ݻ���
                    MaxLen����󳤶�
                    Timeout����ʱ
                    RxLen��ʵ�ʽ��ܳ���
*  ����ֵ         : 
                    G63_RetryCost�����Դ����þ�
                    G63_DISCONNECTED�����ӶϿ�
                    G63_True���ɹ�
                    G63_False��ʧ��
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_TCP_Recv(uint8_t * Data,uint16_t MaxLen,uint16_t Timeout,uint16_t * RxLen)
{
    uint16_t LenRecv = 0,DataLen = 0,Retry = 50,TimeLen = 0;
    /*2016--12--29--14--26--33(ZJYC): ����˵�ˣ�Ϊ��߿ռ������ʣ���ֱ�������Ļ�������Data   */ 
    /* �������Ӧ�ò�Ļ���������1024�������1200 */
    //uint8_t Temp[1100] = {0x00};
    uint8_t *Temp = Data;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit,*pData = 0;
    
    G63_Asert(Data);
    G63_Asert(RxLen);
    
    while(1)
    {
        /* ģ��Ὣ���ݷ��أ����Դ˴��Ļ�����Ӧ�㹻�� >= 1200B */
        //G63_Driver.Uart.Send("AT+QIRD=0,1,0,1024\r\n",strlen("AT+QIRD=0,1,0,1024\r\n"),Timeout);
        LenRecv = G63_Driver.Uart.Recv(Temp,0,Timeout,1);
        /* �ӿڷ���-1 */
        if(LenRecv == 0xFFFF)continue;
        prvSplitString(Temp,LenRecv,ParamSplitTemp,4);
        /* ���ճɹ� */
        if(G63_STR_EQL(ParamSplitTemp[0],"QINDI"))
        {
            G63_Driver.Uart.Send("AT+QIRD=0,1,0,1024\r\n",strlen("AT+QIRD=0,1,0,1024\r\n"),Timeout);
            LenRecv = G63_Driver.Uart.Recv(Temp,0,Timeout * 10,1);
            if(LenRecv == 0xFFFF)return (uint16_t)G63_False;
            prvSplitString(Temp,LenRecv,ParamSplitTemp,5);
            if(G63_STR_EQL(ParamSplitTemp[0],"QIRD")/* && G63_STR_EQL(ParamSplitTemp[3],"TCP")*/)
            {
                uint16_t i = 0;
                DataLen = atoi((const char *)ParamSplitTemp[4]);
                /* �򵥵Ķ�λ����һ����Σ�� */
                /* Maybe a BUG */
                while(ParamSplitTemp[4][i++] != '\n' && i < 2048);
                if(i >= 2048)
                {
                    return (uint16_t)G63_False;
                }
                /* �����׵�ַ */
                pData = (uint8_t *)&ParamSplitTemp[4][i];
                /* �������� */
                for(i = 0;i < DataLen;i ++)
                {
                    Data[i] = pData[i];
                }
                *RxLen = DataLen;
                return (uint16_t)G63_True;
            }
        }
        TimeLen += Timeout * 10;
        /* ��ʱ30S�˳� */
        if(TimeLen > 30 * 1000)return (uint16_t)G63_RetryCost;
        
    }
    //return (uint16_t)G63_False;
}

/*
****************************************************
*  ������         : AT_SetApn
*  ��������       : ����APN
*  ����           : APN�����硰CMNET��
*  ����ֵ         : 
*  ����           : --
*  ��ʷ�汾       : ʵ�⻹ûʧ�ܹ�
*****************************************************
*/
uint16_t AT_SetApn(uint8_t * APN)
{
    uint8_t * Buff = G63_Driver.PriData.InstructionBuff;
    /* ������� */
    G63_Asert(APN);
    /* �������� */
    G63_STR_CPY(G63_Driver.PriData.LastAPN,APN);
    /* �������� */
    sprintf((char *)Buff,"AT+QICSGP=1,\"%s\"\r\n",APN);
    if((uint16_t)G63_True != prvSendInstruction(Buff,6,500,300,0,(uint8_t *)"OK"))return (uint16_t)G63_False;
    return (uint16_t)G63_True;
}
/*
****************************************************
*  ������         : AT_CSQ
*  ��������       : ��ȡ�ź����Լ�CPIN״̬
*  ����           : 
                    CSQ����õ��ź�����ַ��*CSQ = 26��
                    Count����õ������ʵ�ַ��*Count = 58;
*  ����ֵ         : 
                    G63_SignalFail����ȡ�ź���ʧ��
                    G63_SignalLow���źŲ�����
                    G63_SIMFail��SIM����ѯʧ��
*  ����           : --
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_CSQ(uint8_t * CSQ,uint8_t * Count)
{
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    uint8_t Res = 0;
    
    G63_Asert(CSQ);
    G63_Asert(Count);
    
    Res = prvSendInstruction((uint8_t*)"AT+CSQ\r\n",6,500,300,3,(uint8_t *)"OK");
    if(Res != (uint16_t)G63_True)return (uint16_t)G63_SignalFail;
    if((uint32_t)ParamSplitTemp[1] == 0 || (uint32_t)ParamSplitTemp[2] == 0)
    {
        return (uint16_t)G63_SignalFail;
    }
    
    *CSQ = (uint8_t)atoi((const char *)ParamSplitTemp[1]);
    *Count = (uint8_t)atoi((const char *)ParamSplitTemp[2]);
    /* ������δ֪�򲻿ɲ� */
    if(*Count == 99)*Count = 0;
    
    if(*CSQ > 31)return (uint16_t)G63_SignalLow;
    
    Res = prvSendInstruction((uint8_t*)"AT+CPIN?\r\n",10,300,300,1,(uint8_t*)"READY");
    if(Res != (uint16_t)G63_True)return (uint16_t)G63_SIMFail;
    
    return G63_True;
}
/*
****************************************************
*  ������         : AT_GetNetTime
*  ��������       : ��ȡ����ʱ��
*  ����           : 
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_GetNetTime(uint8_t * TimeString)
{
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    //uint8_t * Buff = G63_Driver.PriData.InstructionBuff;
    uint16_t Res = (uint16_t)G63_True;
    
    G63_Asert(TimeString);
    
    //for(i = 0;i < sizeof(NtpServerAddr)/sizeof(NtpServerAddr[0]);i ++)
    {
        //sprintf(Buff,"AT+QNTP=\"%s\"\r\n",NtpServerAddr[i]);
        //if(LenRecv == (uint16_t)G63_True)Res = prvSendInstruction(Buff,6,500,300,0,"OK");
        //else continue;
        
        if(Res == (uint16_t)G63_True)Res = prvSendInstruction("AT+QNITZ=1\r\n",6,500,300,0,"OK");
        else return (uint16_t)G63_False;
        
        if(Res == (uint16_t)G63_True)Res = prvSendInstruction("AT+CTZU=3\r\n",6,500,300,0,"OK");
        else return (uint16_t)G63_False;
        
        if(Res == (uint16_t)G63_True)Res = prvSendInstruction("AT+CCLK?\r\n",6,500,1000,8,"OK");
        else return (uint16_t)G63_False;
        
        if(G63_STR_EQL(ParamSplitTemp[0],"CCLK"))
        {
            /* �� �� �� ʱ �� ��*/
            G63_STR_CPY(&TimeString[0],ParamSplitTemp[1]);
            G63_STR_CPY(&TimeString[2],ParamSplitTemp[2]);
            G63_STR_CPY(&TimeString[4],ParamSplitTemp[3]);
            G63_STR_CPY(&TimeString[6],ParamSplitTemp[4]);
            G63_STR_CPY(&TimeString[8],ParamSplitTemp[5]);
            G63_STR_CPY(&TimeString[10],ParamSplitTemp[6]);
            return (uint16_t)G63_True;
        }
    }
    return (uint16_t)G63_False;
}

/*
****************************************************
*  ������         : AT_WaitForPowerOnFinish
*  ��������       : �ȴ����Գɹ�
*  ����           : TimeOut:��ʱʱ��
*  ����ֵ         : 
*  ����           : -5A4A5943-
*  ��ʷ�汾       : 
*****************************************************
*/
uint16_t AT_WaitForPowerOnFinish(uint16_t TimeOut)
{
    /* ģ�鿪��������Ϣ */
    static uint8_t PowerOnInf[] = {0x7E,0x00,0x00,0x00,0x00,0x08,0x00,0xFE,0x0,0x7E};
    uint16_t LenRecv = 0,i = 0;
    uint8_t * Buff = G63_Driver.PriData.InstructionBuff;
    
    LenRecv = G63_Driver.Uart.Recv(Buff,0,TimeOut,0);
    
    if(LenRecv)
    {
        /* ȥ������0x00 */
        while(Buff[i] == 0x00 && i < 15)i ++;
        if(i >= 15)return (uint16_t)G63_False;
        /* �˴��ıȽϲ��Ǳ�Ҫ�� */
        if(memcmp((void*)PowerOnInf,(void*)&Buff[i],sizeof(PowerOnInf)) == 0)
        {
            return (uint16_t)G63_True;
        }
    }
    return (uint16_t)G63_False;
}

G63_DriverTypedef G63_Driver = 
{
    {
        G63_UartInit,
        G63_UartSend,
        G63_UartRecv
    },
};




















