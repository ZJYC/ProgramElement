/*
****************************************************
*  文件名             : 
*  作者               : --
*  版本               : 
*  编写日期           : 
*  简介               : 
*  函数列表           : 
*  历史版本           : 
*****************************************************
*/


/*头文件  */

#include "G63.h"

/*宏定义  */
/*变量声明*/
/*变量定义*/
/*函数声明*/

uint16_t G63_UartRecv(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout,uint8_t CLR);
uint16_t G63_UartSend(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
uint16_t G63_UartInit(uint16_t Baud);
uint16_t AT_TCP_Close(void);
uint16_t AT_CREG_CGATT(uint16_t Retry,uint16_t RetryInterval);
uint16_t AT_EnterCommandMode(void);
uint16_t AT_EnterDataMode(void);

/*函数定义*/

/*
****************************************************
*  函数名         : prvDoesNeedShield
*  函数描述       : 查看字符是否属于数组UselessChar
*  参数           : Char：待对比字符
*  返回值         : 找到返回(uint16_t)G63_True，否则返回(uint16_t)G63_False
*  作者           : --
*  历史版本       : 
*****************************************************
*/
static uint16_t prvWithinUselessChar(uint8_t Char)
{
    /* 此处标明无用字符，方便处理 */
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
*  函数名         : prvSplitString
*  函数描述       : 分解字符串
*  参数           : 
                    In:指令输入
                    Len:指令长度
                    Out:把指令分解输出到二维字符数组
                    IndexMax：分解出几个字符串
                    
                    举例：
                    
                    输入：“\r\n+CREG: 0,1\r\n\r\nOK\r\n”
                    输出： 
                            Out[0] = "CREG",
                            Out[1] = "0",
                            Out[2] = "1",
                            Out[3] = "OK".
                    返回：分解出的子字符串个数
                    BUG:In 的开头不能为有意义字符
*  返回值         : 
*  作者           : --
*  历史版本       : 
*****************************************************
*/
static uint8_t prvSplitString(uint8_t * In,uint16_t Len,uint8_t ** Out,uint8_t IndexMax)
{
    //CharUseless：标明一个无用字符
    uint8_t i = 0,tIndex = 0,CharUseless = 0;
    /* 参数检查 */
    G63_Asert(In);
    G63_Asert(Out);
    G63_Asert(Len);
    //对字符串的长度做一下限定
    while(In[i] && i < Len)
    {
        //跳过这些个无用字符
        if(prvWithinUselessChar(In[i]) == (uint16_t)G63_True)
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
                        RetryInterval:重试间隔(单位：ms)
                        Timeout:超时(单位：ms)
                        MatchIndex:匹配索引(小于0表示不进行匹配)
                        Match:匹配项
*  返回值         : 
                        G63_True            //成功
                        G63_DISCONNECTED    //连接断开
                        G63_RetryCost       //重试未果
                        
*  作者           : --
*  历史版本       : 
*****************************************************
*/
static uint8_t prvSendInstruction(
                                    uint8_t * Instruction,          /* 指令内容 */
                                    uint16_t RetryCnt,              /* 重试次数 */
                                    uint16_t RetryInterval,         /* 重试间隔 (单位：ms)*/
                                    uint16_t Timeout,               /* 回复超时时间 (单位：ms)*/
                                    int8_t MatchIndex,              /* 匹配索引 */
                                    uint8_t * Match                 /* 匹配项 */
                                    )
{
    uint8_t RetryTemp = 0,**ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    uint8_t * Buf = G63_Driver.PriData.InstructionBuff;
    uint16_t LenRecv = 0;
    /* 检查参数 */
    G63_Asert(Instruction);
    if(MatchIndex >= 0)G63_Asert(Match);
    /* 发送指令 */
    G63_Driver.Uart.Send(Instruction,strlen((char *)Instruction),Timeout);
    for(;;)
    {
        /* 收到一帧信息 直接判断是否匹配Match */
        LenRecv = G63_Driver.Uart.Recv(Buf,0,Timeout,0);
        /* 如果收到数据 */
        if(LenRecv)
        {
            uint8_t i = 0;
            /* 分解字符串到ParamSplitTemp */
            uint8_t tIndex = prvSplitString(Buf,LenRecv,ParamSplitTemp,10);
            /* 用户不希望本函数比较Match项 */
            if(MatchIndex < 0)return G63_NeedNotMatch;
            /* 判断子字符串 */
            if(G63_STR_EQL(ParamSplitTemp[MatchIndex],Match))
            {
                return (uint16_t)G63_True;
            }
            else
            {
                /* 检查是否含有敏感字符 */
                for(i = 0;i < tIndex;i ++)
                {
                    if(strcmp((const char *)ParamSplitTemp[i],"CLOSE") == 0 )
                    {
                        /* 关闭！严重问题 */
                        return G63_DISCONNECTED;
                    }
                }
                if(++RetryTemp < RetryCnt)
                {
                    /* 一段时间之后我们再次发送命令 */
                    Delay10ms(RetryInterval / 10);
                    G63_Driver.Uart.Send(Instruction,strlen((const char *)Instruction),Timeout);
                }
                else
                {
                    return G63_RetryCost;
                }
            }
        }
        /* 我们没有收到数据 */
        else
        {
            /* 重传 */
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
*  函数名         : AT_Init
*  函数描述       : 指令初始化
*  参数           : 
*  返回值         : 同prvSendInstruction
                    特殊：
                    G63_EchoFail：关回显失败
                    G63_SetBDFail：设置波特率失败
*  作者           : --
*  历史版本       : 
*****************************************************
*/
uint16_t AT_Init(void)
{
    /* ##在此函数前模块已经开机## */
    uint16_t Res = 0;
    /* 关回显 */
    Res = prvSendInstruction((uint8_t*)"ATE0\r\n",30,200,300,0,(uint8_t*)"OK");
    if(Res != (uint16_t)G63_True)return (uint16_t)G63_EchoFail;
    /* 设置波特率 */
    Res = prvSendInstruction((uint8_t*)"AT+IPR=115200&W\r\n",6,200,300,0,(uint8_t*)"OK");
    if(Res != (uint16_t)G63_True)return (uint16_t)G63_SetBDFail;
    /* 确保 SIM 卡的 PIN 码已解 */
    /* 此部分大约耗时 9S，循环 6*2 = 12S*/
    Res = prvSendInstruction((uint8_t*)"AT+CPIN?\r\n",60,200,300,1,(uint8_t*)"READY");
    /* 确认找网成功 */
    /* 查询 GPRS 附着是否成功 */
    /* 此部分大约耗时 7S，循环 5*2 = 10S */
    Res = AT_CREG_CGATT(50,200);
    /* 将 Context 0 设为前台 Context。 */
    Res = prvSendInstruction((uint8_t*)"AT+QIFGCNT=0\r\n",6,100,300,0,(uint8_t*)"OK");
    /* 设置 GPRS 的 APN。 */
    Res = prvSendInstruction((uint8_t*)"AT+QICSGP=1,\"CMNET\"\r\n",6,100,300,0,(uint8_t*)"OK");
    /* 接收到数据后，输出提示:" +QIRDI: <id>,<sc>,<sid>"。 */
    Res = prvSendInstruction((uint8_t*)"AT+QINDI=1\r\n",6,100,300,0,(uint8_t*)"OK");
    /* 非透传模式 */
    Res = prvSendInstruction((uint8_t*)"AT+QIMODE=0\r\n",3,100,300,0,(uint8_t*)"OK");
    /* 单路连接 */
    Res = prvSendInstruction((uint8_t*)"AT+QIMUX=0\r\n",3,100,300,0,(uint8_t*)"OK");
    /* 启动发送数据回显 */
    Res = prvSendInstruction((uint8_t*)"AT+QISDE=1\r\n",6,100,300,0,(uint8_t*)"OK");
    return Res;
}

/*
****************************************************
*  函数名         : AT_TCP
*  函数描述       : 连接TCP
*  参数           : 
                    IP：IP地址，字符串形式：“255.255.255.255”
                    Port：端口号，字符串形式：“6666”
*  返回值         : 
*  作者           : --
*  历史版本       : 
*****************************************************
*/

uint16_t AT_TCP(uint8_t * IP,uint8_t * Port)
{
    uint8_t * Buff = G63_Driver.PriData.InstructionBuff,Len = 0,Retry = 6;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    uint16_t Res = 0;
    /* 检查数据 (发行版可以不带)*/
    G63_Asert(IP);
    G63_Asert(Port);
    /* 保存数据，方便重新连接 */
    if((uint32_t)G63_Driver.PriData.LastIP != (uint32_t)IP)G63_STR_CPY(G63_Driver.PriData.LastIP,IP);
    if((uint32_t)G63_Driver.PriData.LastPort != (uint32_t)IP)G63_STR_CPY(G63_Driver.PriData.LastPort,Port);
    
    ReConnect_1:
    /* 发送指令 */
    sprintf((char *)Buff,"AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",(const char *)IP,(const char *)Port);
    Res = prvSendInstruction(Buff,3,500,1000,0,(uint8_t*)"OK");
    /* 首先会返回“OK”以确认指令格式正确，一段时间之后返回“CONNECT OK”,说明连接成功 */
    
    /* 如果返回“ALREADY”说明已经连接成功，先断开，再重连 */
    if(Res != (uint16_t)G63_True)
    {
        if(strcmp((const char *)ParamSplitTemp[0],"ALREADY") == 0)
        {
            /* 断开重新连接 */
            AT_TCP_Close();
            if(--Retry)goto ReConnect_1;
            return (uint16_t)G63_False;
        }
    }
    
    /* 接收连接结果 */
    /* 5S无返回  即等效于失败 */
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
*  函数名         : AT_CREG_CGATT
*  函数描述       : 等待直到找网和GPRS附着成功
*  参数           : 
                    Retry：重试次数（非零正数）
                    RetryInterval:重试间隔（可以为0）
*  返回值         : 
*  作者           : --
*  历史版本       : 
*****************************************************
*/
uint16_t AT_CREG_CGATT(uint16_t Retry,uint16_t RetryInterval)
{
    //uint16_t Res = (uint16_t)G63_False;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    
    G63_Asert(Retry);
    /* RetryInterval可以为0 */
    
    /* 确认找网成功 */
    for(;;)
    {
        prvSendInstruction((uint8_t*)"AT+CREG?\r\n",1,300,300,-1,(uint8_t*)"1");
        if(G63_STR_EQL(ParamSplitTemp[2],"1") || G63_STR_EQL(ParamSplitTemp[2],"5"))
        {
            /* 找网成功 */
            break;
        }
        else
        {
            /* 重试 */
            if(--Retry == 0)
            {
                return (uint16_t)G63_False;
            }
            Delay10ms(RetryInterval / 10);
        }
    }
    /* 查询 GPRS 附着是否成功 */
    for(;;)
    {
        prvSendInstruction((uint8_t*)"AT+CGATT?\r\n",1,300,300,-1,(uint8_t*)"1");
        if(G63_STR_EQL(ParamSplitTemp[1],"1") || G63_STR_EQL(ParamSplitTemp[1],"5"))
        {
            /* 附着成功 */
            break;
        }
        else
        {
            /* 重试 */
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
*  函数名         : AT_TCP_Close
*  函数描述       : 断开连接
*  参数           : 
*  返回值         : 
*  作者           : --
*  历史版本       : 实测断开连接还没有失败过
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
*  函数名         : AT_TCP_SendFinished
*  函数描述       : 确保数据发送完成
                    模块回复“+QISACK: 500,300,200”说明数据总量为500，
                    成功发送300，还有200没成功（ACK）
                    当模块回复“+QISACK: 500,500,0”说明数据全部传送完成
                    当模块回复“+QISACK: 0,0,0”说明连接断开
*  参数           : 
*  返回值         : 
                    G63_True：数据发送完成
                    G63_DISCONNECTED：连接已断开
                    G63_RetryCost：重试次数耗尽
                    G63_False：不可能返回这个
*  作者           : --
*  历史版本       : 
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
        /* 所有数据+已经被应答的数据+没被应答的数据 */
        /* Maybe a BUG:atoi的最大值 */
        All = atoi((const char *)ParamSplitTemp[1]);
        Acked = atoi((const char *)ParamSplitTemp[2]);
        //NotSure = atoi((const char *)ParamSplitTemp[3]);
        
        /* 发送完成 */
        if(All == Acked && All != 0)
        {
            return (uint16_t)G63_True;
        }
        /* 连接断开 */
        if(All == Acked && All == 0)
        {
            /* 问题不小 */
            return (uint16_t)G63_DISCONNECTED;
        }
        if(--Retry == 0 )return (uint16_t)G63_RetryCost;
        Delay10ms(RetryInterval / 10);
    }
    //return (uint16_t)G63_False;
}

/*
****************************************************
*  函数名         : AT_STATE
*  函数描述       : 获取模块当前状态
*  参数           : 
*  返回值         : 见 G63_ResTypedef
*  作者           : --
*  历史版本       : 
*****************************************************
*/
uint16_t AT_STATE(void)
{
    //uint8_t * Buff = G63_Driver.PriData.InstructionBuff;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit,Retry = 4;
    uint16_t Res = 0;
    
    /* 部分情况下单片机不能正确接收，故此处会重复获取 */
    RepeatGetState:
    
    Res = prvSendInstruction("AT+QISTAT\r\n",6,500,300,-1,(uint8_t *)">");
    if(Res == (uint16_t)G63_NeedNotMatch)
    {
        /* 初始化 */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"INITIAL"))
        {
            return (uint16_t)G63_IP_INITIAL;
        }
        /* 启动任务 */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"START"))
        {
            return (uint16_t)G63_IP_START;
        }
        /* 配置场景 */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"CONFIG"))
        {
            return (uint16_t)G63_IP_CONFIG;
        }
        /* 激活 GPRS/CSD 场景中 */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"IND"))
        {
            return (uint16_t)G63_IP_IND;
        }
        /* 接收场景配置 */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"GPRSACT"))
        {
            return (uint16_t)G63_IP_GPRSACT;
        }
        /* 获得本地 IP 地址 */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"STATUS"))
        {
            return (uint16_t)G63_IP_STATUS;
        }
        /* TCP 连接中 */
        if(G63_STR_EQL(ParamSplitTemp[2],"TCP")     && G63_STR_EQL(ParamSplitTemp[3],"CONNECTING"))
        {
            return (uint16_t)G63_TCP_CONNECTING;
        }
        /* UDP 连接中 */
        if(G63_STR_EQL(ParamSplitTemp[2],"UDP")     && G63_STR_EQL(ParamSplitTemp[3],"CONNECTING"))
        {
            return (uint16_t)G63_UDP_CONNECTING;
        }
        /* TCP/UDP 连接关闭 */
        if(G63_STR_EQL(ParamSplitTemp[2],"IP")      && G63_STR_EQL(ParamSplitTemp[3],"CLOSE"))
        {
            return (uint16_t)G63_IP_CLOSE;
        }
        /* TCP/UDP 连接成功 */
        if(G63_STR_EQL(ParamSplitTemp[2],"CONNECT") && G63_STR_EQL(ParamSplitTemp[3],"OK"))
        {
            return (uint16_t)G63_CONNECT_OK;
        }
        /* GPRS/CSD 场景异常关闭 */
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
*  函数名         : AT_TCP_Send
*  函数描述       : 发送数据
                    我们打开了发送数据回显，此处需要一个最大为1200的数组接收数据回显
*  参数           : 
                    Data:数据地址
                    Len:待发送数据长度
*  返回值         : G63_False：失败
                    G63_True：成功
*  作者           : --
*  历史版本       : 
*****************************************************
*/
uint16_t AT_TCP_Send(uint8_t * Data,uint16_t Len)
{
    uint8_t * Buff = G63_Driver.PriData.InstructionBuff,Retry = 8,ReTransfer = 3;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit;
    uint8_t Temp[1200] = {0x00};
    uint16_t Res = 0,RxLen = 0;
    /* 检查数据 */
    G63_Asert(Data);
    G63_Asert(Len);
    RetransferLoop:
    /* 确保我们在连接状态 */
    Res = AT_STATE();
    if(Res != (uint16_t)G63_CONNECT_OK)
    {
        /* 连接已经断开   我们会重新连接*/
        if(
                Res == (uint16_t)G63_PDP_DEACT  || 
                Res == (uint16_t)G63_IP_INITIAL || 
                Res == (uint16_t)G63_IP_CLOSE   ||
                Res == (uint16_t)G63_False)
        {
            /* 首先需要确保数据正确 */
            G63_Asert((uint8_t)G63_Driver.PriData.LastIP);
            G63_Asert((uint8_t)G63_Driver.PriData.LastPort);
            Res = AT_TCP(G63_Driver.PriData.LastIP,G63_Driver.PriData.LastPort);
            /* 连接成功  开始发送数据 */
            if(Res == (uint16_t)G63_True) goto SendDataRetry;
            /* 无法连接 */
            return (uint16_t)G63_False;
        }
    }
    SendDataRetry:
    /* 交代要发送的字节数 并确保模块回复 ">" */
    sprintf((char *)Buff,"AT+QISEND=%d\r\n",Len);
    Res = prvSendInstruction(Buff,6,500,100,0,(uint8_t *)">");
    if(Res != (uint16_t)G63_True)
    {
        /* 模块没有回复 ">"，重传！ */
        if(--Retry){Delay10ms(30);goto SendDataRetry;}
    }
    /* 发送数据 */
    G63_Driver.Uart.Send(Data,Len,100);
    /* 我们先检测数据是否发送正确，在检测数据是否发送完成 
    RxLen >= Len说明模块除了回显数据之外，还回复了结果*/
    RxLen = G63_Driver.Uart.Recv(Temp,0,1000,0);
    if(RxLen != 0 && RxLen >= Len)
    {
        uint16_t i = 0;
        for(i = 0;i < Len;i++)
        {
            if(Data[i] != Temp[i])
            {
                /* 模块回显的数据不对 */
                if(--Retry)goto SendDataRetry;
                return (uint16_t)G63_False;
            }
        }
        /* 模块回复“SEND OK”说明模块已经收到数据，但不标明数据发送成功 */
        prvSplitString(&Temp[i],RxLen,ParamSplitTemp,10);
        if(G63_STR_EQL(ParamSplitTemp[0],"SEND") && G63_STR_EQL(ParamSplitTemp[1],"OK"))
        {
            /* 或许可以加入自适应功能 */
            //Res =  AT_TCP_SendFinished(150,200);
            
            //if(Res == (uint16_t)G63_True)return (uint16_t)G63_True;
            //if(--ReTransfer)goto RetransferLoop;
            //return (uint16_t)G63_False;
			return (uint16_t)G63_True;
        }
    }
    /* 失败重传 */
    if(--Retry)goto SendDataRetry;
    return (uint16_t)G63_False;
}

/*
****************************************************
*  函数名         : AT_TCP_Recv
*  函数描述       : 从模块接收数据
*  参数           : 
                    Data：数据缓冲
                    MaxLen：最大长度
                    Timeout：超时
                    RxLen：实际接受长度
*  返回值         : 
                    G63_RetryCost：重试次数用尽
                    G63_DISCONNECTED：连接断开
                    G63_True：成功
                    G63_False：失败
*  作者           : --
*  历史版本       : 
*****************************************************
*/
uint16_t AT_TCP_Recv(uint8_t * Data,uint16_t MaxLen,uint16_t Timeout,uint16_t * RxLen)
{
    uint16_t LenRecv = 0,DataLen = 0,Retry = 50,TimeLen = 0;
    /*2016--12--29--14--26--33(ZJYC): 范工说了，为提高空间利用率，我直接用他的缓冲区：Data   */ 
    /* 必须的让应用层的缓冲区超过1024，最好是1200 */
    //uint8_t Temp[1100] = {0x00};
    uint8_t *Temp = Data;
    uint8_t **ParamSplitTemp = G63_Driver.PriData.ParamSplit,*pData = 0;
    
    G63_Asert(Data);
    G63_Asert(RxLen);
    
    while(1)
    {
        /* 模块会将数据返回，所以此处的缓冲区应足够大 >= 1200B */
        //G63_Driver.Uart.Send("AT+QIRD=0,1,0,1024\r\n",strlen("AT+QIRD=0,1,0,1024\r\n"),Timeout);
        LenRecv = G63_Driver.Uart.Recv(Temp,0,Timeout,1);
        /* 接口返回-1 */
        if(LenRecv == 0xFFFF)continue;
        prvSplitString(Temp,LenRecv,ParamSplitTemp,4);
        /* 接收成功 */
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
                /* 简单的定位，有一定的危险 */
                /* Maybe a BUG */
                while(ParamSplitTemp[4][i++] != '\n' && i < 2048);
                if(i >= 2048)
                {
                    return (uint16_t)G63_False;
                }
                /* 数据首地址 */
                pData = (uint8_t *)&ParamSplitTemp[4][i];
                /* 复制数据 */
                for(i = 0;i < DataLen;i ++)
                {
                    Data[i] = pData[i];
                }
                *RxLen = DataLen;
                return (uint16_t)G63_True;
            }
        }
        TimeLen += Timeout * 10;
        /* 超时30S退出 */
        if(TimeLen > 30 * 1000)return (uint16_t)G63_RetryCost;
        
    }
    //return (uint16_t)G63_False;
}

/*
****************************************************
*  函数名         : AT_SetApn
*  函数描述       : 设置APN
*  参数           : APN：例如“CMNET”
*  返回值         : 
*  作者           : --
*  历史版本       : 实测还没失败过
*****************************************************
*/
uint16_t AT_SetApn(uint8_t * APN)
{
    uint8_t * Buff = G63_Driver.PriData.InstructionBuff;
    /* 检查数据 */
    G63_Asert(APN);
    /* 保存数据 */
    G63_STR_CPY(G63_Driver.PriData.LastAPN,APN);
    /* 发送命令 */
    sprintf((char *)Buff,"AT+QICSGP=1,\"%s\"\r\n",APN);
    if((uint16_t)G63_True != prvSendInstruction(Buff,6,500,300,0,(uint8_t *)"OK"))return (uint16_t)G63_False;
    return (uint16_t)G63_True;
}
/*
****************************************************
*  函数名         : AT_CSQ
*  函数描述       : 获取信号量以及CPIN状态
*  参数           : 
                    CSQ：获得的信号量地址：*CSQ = 26；
                    Count：获得的误码率地址：*Count = 58;
*  返回值         : 
                    G63_SignalFail：获取信号量失败
                    G63_SignalLow：信号不正常
                    G63_SIMFail：SIM卡查询失败
*  作者           : --
*  历史版本       : 
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
    /* 误码率未知或不可测 */
    if(*Count == 99)*Count = 0;
    
    if(*CSQ > 31)return (uint16_t)G63_SignalLow;
    
    Res = prvSendInstruction((uint8_t*)"AT+CPIN?\r\n",10,300,300,1,(uint8_t*)"READY");
    if(Res != (uint16_t)G63_True)return (uint16_t)G63_SIMFail;
    
    return G63_True;
}
/*
****************************************************
*  函数名         : AT_GetNetTime
*  函数描述       : 获取网络时间
*  参数           : 
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
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
            /* 年 月 日 时 分 秒*/
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
*  函数名         : AT_WaitForPowerOnFinish
*  函数描述       : 等待回显成功
*  参数           : TimeOut:超时时间
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t AT_WaitForPowerOnFinish(uint16_t TimeOut)
{
    /* 模块开机回显信息 */
    static uint8_t PowerOnInf[] = {0x7E,0x00,0x00,0x00,0x00,0x08,0x00,0xFE,0x0,0x7E};
    uint16_t LenRecv = 0,i = 0;
    uint8_t * Buff = G63_Driver.PriData.InstructionBuff;
    
    LenRecv = G63_Driver.Uart.Recv(Buff,0,TimeOut,0);
    
    if(LenRecv)
    {
        /* 去掉干扰0x00 */
        while(Buff[i] == 0x00 && i < 15)i ++;
        if(i >= 15)return (uint16_t)G63_False;
        /* 此处的比较不是必要的 */
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




















