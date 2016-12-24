

uint32_t SP_Pool[1024] = {0x00};
static HeapRegion_t xHeapRegions[] =
{
    {SP_Pool,4096},
    { NULL, 0 }
};

pSerialPortTypedef SP_Init(uint32_t ID)
{
    pSerialPortTypedef SerialPortTemp = 0x00;
    
    MM_Ops.Init(xHeapRegions);
    
    SerialPortTemp = (pSerialPortTypedef)MM_Ops.Malloc(sizeof(SerialPortTypedef));
    memset((void*)SerialPortTemp,'\0',sizeof(SerialPortTypedef));

    SerialPortTemp.RxRB.buffer = MM_Ops.Malloc(1024);
    memset((void*)SerialPortTemp.RxRB.buffer,'\0',1024);
    SerialPortTemp.RxRB.size = 1024;
    SerialPortTemp.TxRB.buffer = MM_Ops.Malloc(1024);
    memset((void*)SerialPortTemp.TxRB.buffer,'\0',1024);
    SerialPortTemp.TxRB.size = 1024;
    
    SerialPortTemp->ID = ID;
    
    LowerLayerInit(SerialPortTemp);
    return SerialPortTemp;
}

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
*  函数名         : 
*  函数描述       : 本函数需要用户自行实现,只在发送数据时调用一次，后续由中断控制
*  参数           : 
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t LowerLayerTxFirstByte(pSerialPortTypedef pSerialPort)
{
    uint8_t Temp = 0;
    
    BT_CHK_PARAM(pSerialPort);
    
    if(pSerialPort->u.bit.Txing == 0 && pSerialPort->u.bit.TxEn == 1)
    {
        /* 以使得中断发送得以继续 */
        pSerialPort->u.bit.Txing = 1;
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
            return (uint16_t)Res_NoData2Send;
        }
    }
    if(pSerialPort->u.bit.TxEn == 0)return (uint16_t)Res_SendNotAllowes;
    return (uint16_t)Res_NowTxing;
}
/*
****************************************************
*  函数名         : 
*  函数描述       : 发送中断应调用此函数
*  参数           : 
*  返回值         : 
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t FuncTxIntHook(pSerialPortTypedef pSerialPort)
{
    BT_CHK_PARAM(pSerialPort);
    /* 确保我们处于发送状态 */
    if(pSerialPort->u.bit.Txing == 1 && pSerialPort->u.bit.TxEn == 1)
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
            pSerialPort->u.bit.TxFin = 0;
            pSerialPort->u.bit.TxEn = 0;
            return (uint16_t)Res_TxFinished;
        }
    }
    if(pSerialPort->u.bit.TxEn == 0)return (uint16_t)Res_SendNotAllowes;
    return (uint16_t)Res_NowTxing;
}

uint16_t FuncTickHook(pSerialPortTypedef pSerialPort,uint16_t Period)
{
    BT_CHK_PARAM(pSerialPort);
    /* 递增计数值 */
    TickAdd(pSerialPort->Counter,Period);
    
    if(pSerialPort->u.bit.RxEn == 1 && GetInterval(pSerialPort->Counter,pSerialPort->LastRxTime) > pSerialPort->RxInterval)
    {
        pSerialPort->u.bit.RxFin = 1;
        if(pSerialPort->u.bit.AutoCloseRx == 1)pSerialPort->u.bit.RxEn = 0;
    }
    
    return (uint16_t)Res_OK;
}

uint16_t SP_Recv(pSerialPortTypedef pSerialPort,uint8_t * Data,uint16_t MaxLen,uint16_t Timeout)
{
    uint16_t i = 0;
    
    BT_CHK_PARAM(pSerialPort);
    BT_CHK_PARAM(Data);
    pSerialPort->RxInterval = 30;
    
    /* 缓冲区里有数据等待取回 */
    if(pSerialPort->u.bit.RxFin == 1)return (uint16_t)Res_RxDataHaveRecv;
    
    /* 如果指定了数据长度 并且 RB有足够的数据 */
    if((MaxLen != 0) && RB_FUNC.Used(&pSerialPort->RxRB) >= MaxLen)
    {
        RB_FUNC.Read(&pSerialPort->RxRB,Data,MaxLen);
        
        return MaxLen;
    }
    /* RB显然没有足够的数据  但是调用者希望得到MaxLen长度*/
    if((MaxLen != 0) && RB_FUNC.Used(&pSerialPort->RxRB) < MaxLen)
    {
        if(LowerLayerPrapareRx(pSerialPort) == (uint16_t)Res_OK)
        {
            /* 没有探测到字符流则等待Timeout */
            while(GetInterval(pSerialPort->Counter,pSerialPort->LastRxTime) < Timeout)
            {
                /* 探测到字符流 */
                if(pSerialPort->u.bit.Rxing == 1)
                {
                    break;
                }
            }
            if(pSerialPort->u.bit.Rxing == 1)
            {
                /* 依然没有收到调用者期望的数据长 */
                if(RB_FUNC.Used(&pSerialPort->RxRB) < MaxLen)
                {
                    return (uint16_t)Res_RxTimeout;
                }
                /* 符合调用者的要求 */
                else
                {
                    RB_FUNC.Read(&pSerialPort->RxRB,Data,MaxLen);
                    return MaxLen;
                }
            }
            if(pSerialPort->u.bit.Rxing == 0)return (uint16_t)Res_RxTimeout;
        }
        else
        {
            /* 别人正在接收？ */
        }
    }
    
    
}

uint16_t LowerLayerPrapareRx(pSerialPortTypedef pSerialPort)
{
    if(pSerialPort->u.bit.RxEn == 1)return (uint16_t)Res_NowRxing;
    /* 允许中断接收数据 */
    pSerialPort->u.bit.RxEn = 1;
    pSerialPort->u.bit.Rxing = 0;
    /* 记录时间 */
    pSerialPort->LastRxTime = pSerialPort->Counter;
    return Res_OK;
}

uint16_t FuncRxIntHook(pSerialPortTypedef pSerialPort)
{
    uint8_t Temp = 0;
    /* 首先确保允许接收 */
    if(pSerialPort->u.bit.RxEn == 1)
    {
        /* 刷新计数值 */
        pSerialPort->LastRxTime = pSerialPort->Counter;
        /* 开始接收字符流 */
        pSerialPort->u.bit.Rxing = 1;
        
        UsartRxData(&Temp);
        
        RB_FUNC.Write(&pSerialPort->RxRB,Temp,1);
        
    }
}

















