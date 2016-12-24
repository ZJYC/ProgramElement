

uint32_t SP_Pool[1024] = {0x00};
static HeapRegion_t xHeapRegions[] =
{
    {SP_Pool,4096},
    { NULL, 0 }
};
/*
****************************************************
*  函数名         : SP_Init
*  函数描述       : SP初始化
*  参数           : 
                    ID：每一个SP控制块用ID进行识别，后期会有用处
                    RxBuffSize：接收缓冲大小，暂时认为不能为0
                    TxBuffLen：发送缓冲长度，暂时认为不能为0
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
pSerialPortTypedef SP_Init(uint32_t ID,uint32_t RxBuffSize,uint32_t TxBuffLen)
{
    pSerialPortTypedef SerialPortTemp = 0x00;
    /* 暂时认为不能把缓冲区0 */
    BT_CHK_PARAM(RxBuffSize);
    BT_CHK_PARAM(TxBuffLen);
    
    MM_Ops.Init(xHeapRegions);
    
    SerialPortTemp = (pSerialPortTypedef)MM_Ops.Malloc(sizeof(SerialPortTypedef));
    memset((void*)SerialPortTemp,'\0',sizeof(SerialPortTypedef));

    SerialPortTemp.RxRB.buffer = MM_Ops.Malloc(RxBuffSize);
    memset((void*)SerialPortTemp.RxRB.buffer,'\0',RxBuffSize);
    SerialPortTemp.RxRB.size = RxBuffSize;
    SerialPortTemp.TxRB.buffer = MM_Ops.Malloc(TxBuffLen);
    memset((void*)SerialPortTemp.TxRB.buffer,'\0',TxBuffLen);
    SerialPortTemp.TxRB.size = TxBuffLen;
    
    SerialPortTemp->ID = ID;
    
    LowerLayerInit(SerialPortTemp);
    return SerialPortTemp;
}

/*
****************************************************
*  函数名         : SP_Send
*  函数描述       : 发送数据
*  参数           : 
                    pSerialPort：SP控制块
                    Data：数据地址
                    TxLen：发送数据长度
                    Timeout：超时（如果缓冲区满了，我们要等待直到有足够的空间来存储）
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t SP_Send(pSerialPortTypedef pSerialPort,uint8_t * Data,uint16_t TxLen,uint16_t Timeout)
{
    /* 执行参数检查 */
    BT_CHK_PARAM(pSerialPort);
    BT_CHK_PARAM(Data);
    /* 记录当前时间 */
    pSerialPort->LastTxTime = pSerialPort->Counter;
    TxWaitRB:
    /* 如果缓冲区足够，保存返回成功 */
    if(RB_FUNC.Available(pSerialPort->TxRB) >= TxLen)
    {
        RB_FUNC.Write(pSerialPort->TxRB,Data,TxLen);
        LowerLayerTxFirstByte(pSerialPort);
        pSerialPort->u.bit.TxEn = 1;
        return (uint16_t)Res_OK;
    }
    /* 缓存区并不充足，极有可能是正在发送中，等待缓冲区腾出空间 */
    while(GetInterval(pSerialPort->Counter,pSerialPort->LastTxTime) < Timeout)goto TxWaitRB;
    return (uint16_t)Res_TxTimeout;
}
/*
****************************************************
*  函数名         : LowerLayerTxFirstByte
*  函数描述       : 本函数需要用户自行实现,只在发送数据时调用一次，后续由中断控制
*  参数           : pSerialPort：控制块
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t LowerLayerTxFirstByte(pSerialPortTypedef pSerialPort)
{
    uint8_t Temp = 0;
    
    BT_CHK_PARAM(pSerialPort);
    
    if(pSerialPort->u.bit.TxEn == 1)
    {
        /* 当前没有字节流在发送 */
        if(pSerialPort->u.bit.Txing == 0)
        {
            /* 有数据存储在RB */
            if(RB_FUNC.Used(pSerialPort->TxRB) != 0)
            {
                /* 读取一字节数据 */
                RB_FUNC.Read(&pSerialPort->TxRB,&Temp,1);
                /* 发送数据 */
                UsartSendIT(uart1,&Temp,1);
                return (uint16_t)Res_OK;
            }
            /* 没有数据可以发送 */
            else
            {
                pSerialPort->u.bit.Txing = 0;
                pSerialPort->u.bit.TxEn = 0;
                return (uint16_t)Res_NoData2Send;
            }
        }
        else
        {
            return (uint16_t)Res_NowTxing;
        }
    }
    if(pSerialPort->u.bit.TxEn == 0)return (uint16_t)Res_SendNotAllowes;
}

/*
****************************************************
*  函数名         : SP_Recv
*  函数描述       : 接收数据
*  参数           : 
                    pSerialPort：控制块
                    Data：数据缓冲
                    MaxLen：期望接受长度
                    Timeout：超时
*  返回值         : 
                    Res_OK
                    Res_RxDataHaveRecv
                    Res_RxTimeout
                    Res_False
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t SP_Recv(pSerialPortTypedef pSerialPort,uint8_t * Data,uint16_t *MaxLen,uint16_t Timeout)
{
    BT_CHK_PARAM(pSerialPort);
    BT_CHK_PARAM(Data);
    pSerialPort->RxInterval = 30;
    
    /* 缓冲区里有数据等待取回 */
    if(pSerialPort->u.bit.RxFin == 1 || pSerialPort->u.bit.Rxing == 1)return (uint16_t)Res_RxDataHaveRecv;
    
    /* 如果指定了数据长度 并且 RB有足够的数据 */
    if((*MaxLen != 0) && RB_FUNC.Used(&pSerialPort->RxRB) >= *MaxLen)
    {
        RB_FUNC.Read(&pSerialPort->RxRB,Data,*MaxLen);
        
        return Res_OK;
    }
    /* RB显然没有足够的数据  但是调用者希望得到MaxLen长度*/
    if((*MaxLen != 0) && RB_FUNC.Used(&pSerialPort->RxRB) < *MaxLen)
    {
        LowerLayerPrapareRx(pSerialPort);
        /* 没有探测到字符流则等待Timeout */
        while(GetInterval(pSerialPort->Counter,pSerialPort->LastRxTime) < Timeout)
        {
            /* 探测到字符流 */
            if(pSerialPort->u.bit.Rxing == 1)
            {
                break;
            }
        }
        if(GetInterval(pSerialPort->Counter,pSerialPort->LastRxTime) >= Timeout)return (uint16_t)Res_RxTimeout;
        /* 字符流接收完成 */
        /* 这里我们认为字符流肯定会有结束 */
        //////////////////////////////////////
        /* 这里隐藏一很严重的隐患，程序会死在这里 */
        //////////////////////////////////////
        while(pSerialPort->u.bit.RxFin == 0);
        /* 依然没有收到调用者期望的数据长 */
        if(RB_FUNC.Used(&pSerialPort->RxRB) < *MaxLen)
        {
            return (uint16_t)Res_RxTimeout;
        }
        /* 符合调用者的要求 */
        else
        {
            RB_FUNC.Read(&pSerialPort->RxRB,Data,*MaxLen);
            return Res_OK;
        }
    }
    /* 调用者不指定接受长度 */
    if(*MaxLen == 0)
    {
        uint16_t RxLen = RB_FUNC.Used(&pSerialPort->RxRB);
        RB_FUNC.Read(&pSerialPort->RxRB,Data,RxLen);
        *MaxLen = RxLen;
        return Res_OK;
    }
    return Res_False;
}

/*
****************************************************
*  函数名         : LowerLayerPrapareRx
*  函数描述       : 开始准备接收
*  参数           : pSerialPort：控制块
*  返回值         : 
                    Res_OK：
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t LowerLayerPrapareRx(pSerialPortTypedef pSerialPort)
{
    /* 允许中断接收数据 */
    pSerialPort->u.bit.RxEn = 1;
    pSerialPort->u.bit.Rxing = 0;
    /* 记录时间 */
    pSerialPort->LastRxTime = pSerialPort->Counter;
    return Res_OK;
}


/*
****************************************************
*  函数名         : LowerLayerInit
*  函数描述       : 初始化底层接口
*  参数           : pSerialPort：控制块
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t LowerLayerInit(pSerialPortTypedef pSerialPort)
{
    UsartInit(pSerialPort);
}















