/*
****************************************************
*  文件名             : 
*  作者               : -5A4A5943-
*  版本               : 
*  编写日期           : 
*  简介               : 
*  函数列表           : 
*  历史版本           : 
*****************************************************
*/


/*头文件  */

#include "G60.h"



/*宏定义  */





/*变量声明*/





/*变量定义*/




/*函数声明*/

uint16_t G60_UartRecv(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
uint16_t G60_UartSend(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
uint16_t G60_UartInit(uint16_t Baud);
uint8_t AT_TCP_Close(void);

/*函数定义*/

/*
In:指令输入
Len:指令长度
Out:把指令分解输出到二维字符数组

输入：“SPP: ok idle\r\n\0”
输出：Out[0] = "SPP",Out[1] = "ok",Out[2] = "idle"
返回：分解出的子字符串个数
BUG:In 的开头不能为有意义字符
*/
static uint8_t prvSplitString(uint8_t * In,uint16_t Len,uint8_t ** Out,uint8_t IndexMax)
{
    //CharUseless：标明一个无用字符
    uint8_t i = 0,tIndex = 0,CharUseless = 0;
    /* 参数检查 */
    assert(In);
    assert(Out);
    assert(Len);
    //对字符串的长度做一下限定
    while(In[i] && i < Len)
    {
        //跳过这些个无用字符
        if(In[i] == '\r' || In[i] == '\n' || In[i] == ':' || In[i] == ' ' || In[i] == '+' || In[i] == ',')
        {
            In[i] = '\0';
            i ++;
            CharUseless = 0xff;
            /* 在这里限定子字符串的个数 */
            if(tIndex >= IndexMax)return tIndex;
            continue;
        }
        //存储此字段的地址
        if(CharUseless == 0xff){Out[tIndex++] = &In[i];CharUseless = 0x00;}
        i ++;
    }
    
    return tIndex;
}

/*
****************************************************
*  函数名         : prvSendInstruction
*  函数描述       : 
*  参数           : 
                        Instruction:指令
                        RetryCnt:重传次数
                        Timeout:超时
                        MatchIndex:匹配索引
                        Match:匹配项
*  返回值         : 
                        (uint16_t)G60_True            //成功
                        G60_DISCONNECTED    //连接断开
                        G60_RetryCost       //重试未果
                        
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
static uint8_t prvSendInstruction(
                                    uint8_t * Instruction,      /* 指令内容 */
                                    uint16_t RetryCnt,           /* 重试次数 */
                                    uint16_t RetryInterval,      /* 重试间隔 (单位：ms)*/
                                    uint16_t Timeout,           /* 回复超时时间 (单位：ms)*/
                                    int8_t MatchIndex,         /* 匹配索引 */
                                    uint8_t * Match             /* 匹配项 */
                                    )
{
    //uint32_t CurCounter = G60_Driver.PriData.Counter;
    /* 这里假定指令长度不可能超过100字节 */
    uint8_t RetryTemp = 0,**ParamSplitTemp = G60_Driver.PriData.ParamSplit;
    uint8_t * Buf = G60_Driver.PriData.InstructionBuff;
    uint16_t LenRecv = 0,i = 0;
    /* 检查参数 */
    assert(Instruction);
    if(MatchIndex >= 0)assert(Match);
    /* 发送指令 */
    G60_Driver.Uart.Send(Instruction,strlen((char *)Instruction),Timeout);
    for(;;)
    {
        /* 收到一帧信息 直接判断是否匹配Match */
        LenRecv = G60_Driver.Uart.Recv(Buf,0,Timeout);
        /* 如果收到数据 */
        if(LenRecv)
        {
            uint8_t i = 0;
            /* 分解字符串到ParamSplitTemp */
            uint8_t tIndex = prvSplitString(Buf,LenRecv,ParamSplitTemp,10);
            if(MatchIndex < 0)return G60_NeedNotMatch;
            /* 判断子字符串 */
            if(strcmp((const char *)ParamSplitTemp[MatchIndex],(const char *)Match) == 0 )
            {
                return (uint16_t)G60_True;
            }
            else
            {
                /* 检查是否含有敏感字符 */
                for(i = 0;i < tIndex;i ++)
                {
                    if(strcmp((const char *)ParamSplitTemp[i],"CLOSE") == 0 )
                    {
                        /* 关闭！严重问题 */
                        return G60_DISCONNECTED;
                    }
                }
                if(++RetryTemp < RetryCnt)
                {
                    /* 一段时间之后我们再次发送命令 */
                    Delay10ms(RetryInterval / 10);
                    G60_Driver.Uart.Send(Instruction,strlen((const char *)Instruction),Timeout);
                }
                else
                {
                    return G60_RetryCost;
                }
            }
        }
        /* 我们没有收到数据 */
        else
        {
            /* 重传 */
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
*  函数名         : AT_Init
*  函数描述       : 指令初始化
*  参数           : 
*  返回值         : 同prvSendInstruction
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint8_t AT_Init(void)
{
    uint8_t Res = 0;
    /* 关回显 */
    Res = prvSendInstruction((uint8_t*)"ATE0\r\n",6,300,300,0,(uint8_t*)"OK");
    if(Res != (uint16_t)G60_True)return (uint16_t)G60_EchoFail;
    /* 设置波特率 */
    Res = prvSendInstruction((uint8_t*)"AT+IPR=115200&W\r\n",6,300,300,0,(uint8_t*)"OK");
    if(Res != (uint16_t)G60_True)return (uint16_t)G60_SetBDFail;
    /* 确保 SIM 卡的 PIN 码已解 */
    Res = prvSendInstruction((uint8_t*)"AT+CPIN?\r\n",10,300,300,1,(uint8_t*)"READY");
    /* 确认找网成功 */
    Res = prvSendInstruction((uint8_t*)"AT+CREG?\r\n",2,300,300,2,(uint8_t*)"1");
    Res = prvSendInstruction((uint8_t*)"AT+CREG?\r\n",2,300,300,2,(uint8_t*)"5");
    /* 查询 GPRS 附着是否成功 */
    Res = prvSendInstruction((uint8_t*)"AT+CGATT?\r\n",6,300,300,1,(uint8_t*)"1");
    /* 将 Context 0 设为前台 Context。 */
    Res = prvSendInstruction((uint8_t*)"AT+QIFGCNT=0\r\n",6,300,300,0,(uint8_t*)"OK");
    /* 设置 GPRS 的 APN。 */
    Res = prvSendInstruction((uint8_t*)"AT+QICSGP=1,\"CMNET\"\r\n",6,300,300,0,(uint8_t*)"OK");
    /* 接收到数据后，输出提示:" +QIRDI: <id>,<sc>,<sid>"。 */
    Res = prvSendInstruction((uint8_t*)"AT+QINDI=1\r\n",6,300,300,0,(uint8_t*)"OK");
    /* 非透传模式 */
    Res = prvSendInstruction((uint8_t*)"AT+QIMODE=0\r\n",3,300,300,0,(uint8_t*)"OK");
    /* 单路连接 */
    Res = prvSendInstruction((uint8_t*)"AT+QIMUX=0\r\n",3,300,300,0,(uint8_t*)"OK");
    /* 启动发送数据回显 */
    Res = prvSendInstruction((uint8_t*)"AT+QISDE=1\r\n",6,300,300,0,(uint8_t*)"OK");
    return Res;
}

/*
****************************************************
*  函数名         : AT_TCP
*  函数描述       : 连接TCP
*  参数           : 
                    IP：
                    Port：
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/

uint8_t AT_TCP(uint8_t * IP,uint8_t * Port)
{
    uint8_t * Buff = G60_Driver.PriData.InstructionBuff,Res = 0,Len = 0,Retry = 6;
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit,WaitTime = 0;
    /* 检查数据 (发行版可以不带)*/
    assert(IP);
    assert(Port);
    /* 保存数据，方便重新连接 */
    strcpy(G60_Driver.PriData.LastIP,IP);
    strcpy(G60_Driver.PriData.LastPort,Port);
    
    ReConnect_1:
    /* 发送指令 */
    sprintf((char *)Buff,"AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",(const char *)IP,(const char *)Port);
    Res = prvSendInstruction(Buff,3,500,1000,0,(uint8_t*)"OK");
    /* 首先会返回“OK”以确认指令格式正确，一段时间之后返回“CONNECT OK”,说明连接成功 */
    
    /* 如果返回“ALREADY”说明已经连接成功，先断开，再重连 */
    if(Res != (uint16_t)G60_True)
    {
        if(strcmp((const char *)ParamSplitTemp[0],"ALREADY") == 0)
        {
            /* 断开重新连接 */
            AT_TCP_Close();
            if(--Retry)goto ReConnect_1;
            return (uint16_t)G60_False;
        }
    }
    
    /* 接收连接结果 */
    /* 10S无返回  即等效于失败 */
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
*  函数名         : AT_TCP_Close
*  函数描述       : 断开连接
*  参数           : 
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
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
*  函数名         : TCP_SendFinished
*  函数描述       : 确保数据发送完成
*  参数           : 
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
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
        /* 所有数据||已经被应答的数据||没被应答的数据 */
        All = atoi(ParamSplitTemp[1]);
        Acked = atoi(ParamSplitTemp[2]);
        NotSure = atoi(ParamSplitTemp[3]);
        
        /* 发送完成 */
        if(All == Acked && All != 0)
        {
            return (uint16_t)G60_True;
        }
        /* 连接断开 */
        if(All == Acked && All == 0)
        {
            /* 问题不小 */
            return (uint16_t)G60_DISCONNECTED;
        }
        /* 发送 */
        /* Retry = 10 ： 10S内数据没有发送完成，即确定失败 */
        if(--Retry == 0 )return (uint16_t)G60_RetryCost;
        Delay10ms(100);
    }
    return (uint16_t)G60_False;
}

/*
****************************************************
*  函数名         : AT_STATE
*  函数描述       : 获取模块当前状态
*  参数           : 
*  返回值         : 见 G60_ResTypedef
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t AT_STATE(void)
{
    uint8_t * Buff = G60_Driver.PriData.InstructionBuff,Res = 0;
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit;

    Res = prvSendInstruction("AT+QISTAT\r\n",6,500,300,-1,(uint8_t *)">");
    if(Res == (uint16_t)G60_NeedNotMatch)
    {
        /* 初始化 */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"INITIAL"))
        {
            return (uint16_t)G60_IP_INITIAL;
        }
        /* 启动任务 */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"START"))
        {
            return (uint16_t)G60_IP_START;
        }
        /* 配置场景 */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"CONFIG"))
        {
            return (uint16_t)G60_IP_CONFIG;
        }
        /* 激活 GPRS/CSD 场景中 */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"IND"))
        {
            return (uint16_t)G60_IP_IND;
        }
        /* 接收场景配置 */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"GPRSACT"))
        {
            return (uint16_t)G60_IP_GPRSACT;
        }
        /* 获得本地 IP 地址 */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"STATUS"))
        {
            return (uint16_t)G60_IP_STATUS;
        }
        /* TCP 连接中 */
        if(G60_STR_CMP(ParamSplitTemp[2],"TCP")     && G60_STR_CMP(ParamSplitTemp[3],"CONNECTING"))
        {
            return (uint16_t)G60_TCP_CONNECTING;
        }
        /* UDP 连接中 */
        if(G60_STR_CMP(ParamSplitTemp[2],"UDP")     && G60_STR_CMP(ParamSplitTemp[3],"CONNECTING"))
        {
            return (uint16_t)G60_UDP_CONNECTING;
        }
        /* TCP/UDP 连接关闭 */
        if(G60_STR_CMP(ParamSplitTemp[2],"IP")      && G60_STR_CMP(ParamSplitTemp[3],"CLOSE"))
        {
            return (uint16_t)G60_IP_CLOSE;
        }
        /* TCP/UDP 连接成功 */
        if(G60_STR_CMP(ParamSplitTemp[2],"CONNECT") && G60_STR_CMP(ParamSplitTemp[3],"OK"))
        {
            return (uint16_t)G60_CONNECT_OK;
        }
        /* GPRS/CSD 场景异常关闭 */
        if(G60_STR_CMP(ParamSplitTemp[2],"PDP")     && G60_STR_CMP(ParamSplitTemp[3],"DEACT"))
        {
            return (uint16_t)G60_PDP_DEACT;
        }
    }
    return (uint16_t)G60_True;
}

/*
****************************************************
*  函数名         : TCP_Send
*  函数描述       : 发送数据
*  参数           : 
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t TCP_Send(uint8_t * Data,uint16_t Len)
{
    uint8_t * Buff = G60_Driver.PriData.InstructionBuff,RxLen = 0,Res = 0,Retry = 8;
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit;
    /* 检查数据 */
    assert(Data);
    assert(Len);
    /* 确保我们在连接状态 */
    Res = AT_STATE();
    if(Res != (uint16_t)G60_CONNECT_OK)
    {
        /* 连接已经断开   我们会重新连接*/
        if(
                Res == (uint16_t)G60_PDP_DEACT  || 
                Res == (uint16_t)G60_IP_INITIAL || 
                Res == (uint16_t)G60_IP_CLOSE)
        {
            /* 首先需要确保数据正确 */
            assert((uint32_t)G60_Driver.PriData.LastIP);
            assert((uint32_t)G60_Driver.PriData.LastPort);
            Res = AT_TCP(G60_Driver.PriData.LastIP,G60_Driver.PriData.LastPort);
            /* 连接成功  开始发送数据 */
            if(Res == (uint16_t)G60_True) goto SendDataRetry;
            /* 无法连接 */
            return (uint16_t)G60_False;
        }
    }
    
    SendDataRetry:
    /* 交代要发送的字节数 并确保模块回复 ">" */
    sprintf((char *)Buff,"AT+QISEND=%d\r\n",Len);
    Res = prvSendInstruction(Buff,6,500,100,0,(uint8_t *)">");
    if(Res != (uint16_t)G60_True)
    {
        /* 模块没有回复 ">"，重传！ */
        if(--Retry){Delay10ms(50);goto SendDataRetry;}
    }
    /* 发送数据 */
    G60_Driver.Uart.Send(Data,Len,100);
    /* 我们先检测数据是否发送正确，在检测数据是否发送完成 
    RxLen >= Len说明模块除了回显数据之外，还回复了结果*/
    RxLen = G60_Driver.Uart.Recv(Buff,0,300);
    if(RxLen != 0 && RxLen >= Len)
    {
        uint16_t i = 0;
        for(i = 0;i < Len;i++)
        {
            if(Data[i] != Buff[i])
            {
                /* 模块回显的数据不对 */
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
    /* 失败重传 */
    if(--Retry)goto SendDataRetry;
    return (uint16_t)G60_False;
}

uint16_t TCP_Recv(uint8_t * Data,uint16_t MaxLen,uint16_t Timeout,uint16_t * RxLen)
{
    uint16_t LenRecv = 0,DataLen = 0,Retry = 10;
    /*2016--12--29--14--26--33(ZJYC): 范工说了，为提高空间利用率，我直接用他的缓冲区：Data   */ 
    /* 必须的让应用层的缓冲区超过1024，最好是1200 */
    uint8_t **ParamSplitTemp = G60_Driver.PriData.ParamSplit,*pData = 0;
    
    //assert(MaxLen >= 1200);
    
    while(1)
    {
        G60_Driver.Uart.Send("AT+QIRD=0,1,0,1024\r\n",strlen("AT+QIRD=0,1,0,1024\r\n"),Timeout);
        LenRecv = G60_Driver.Uart.Recv(Data,0,Timeout);
        prvSplitString(Data,LenRecv,ParamSplitTemp,5);
        
        if(G60_STR_CMP(ParamSplitTemp[0],"OK"))
        {
            /* 没有收到数据   等待1S后再查*/
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
            /* 出现错误 */
            *RxLen = 0;
            return (uint16_t)G60_DISCONNECTED;
        }
        /* 接收成功 */
        if(strcmp(ParamSplitTemp[0],"QIRD") == 0 && strcmp(ParamSplitTemp[3],"TCP") == 0)
        {
            uint16_t i = 0;
            DataLen = atoi(ParamSplitTemp[4]);
            
            while(ParamSplitTemp[4][i++] != '\n' && i < 2048);
            if(i >= 2048)return (uint16_t)G60_False;
            /* 数据首地址 */
            pData = (uint8_t *)&ParamSplitTemp[4][i];
            /* 复制数据 */
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
    
    /* 检查数据 */
    assert(APN);
    /* 保存数据 */
    strcpy(G60_Driver.PriData.LastAPN,APN);
    /* 发送命令 */
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





















