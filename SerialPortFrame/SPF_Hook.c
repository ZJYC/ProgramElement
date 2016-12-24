
/*
****************************************************
*  函数名         : FuncRxIntHook
*  函数描述       : 接收中断调用此函数
*  参数           : pSerialPort：控制块
*  返回值         : 
                    Res_OK：
                    Res_RxNotAllowed：
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
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
        return (uint16_t)Res_OK;
    }
    return (uint16_t)Res_RxNotAllowed;
}

/*
****************************************************
*  函数名         : FuncTickHook
*  函数描述       : 滴答中断调用此函数
*  参数           : 
                    pSerialPort：控制块
                    Period：调用周期
*  返回值         : 
                    Res_OK
                    Res_AutoClosed
                    Res_RxInterval
                    
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t FuncTickHook(pSerialPortTypedef pSerialPort,uint16_t Period)
{
    BT_CHK_PARAM(pSerialPort);
    /* 递增计数值 */
    TickAdd(pSerialPort->Counter,Period);
    /* 字符流间隔到达指定长度 */
    if(pSerialPort->u.bit.Rxing == 1 && GetInterval(pSerialPort->Counter,pSerialPort->LastRxTime) > pSerialPort->RxInterval)
    {
        pSerialPort->u.bit.RxFin = 1;
        /* 标明字符流结束 */
        pSerialPort->u.bit.Rxing = 0;
        /* 自动关闭接收 */
        if(pSerialPort->u.bit.AutoCloseRx == 1)
        {
            pSerialPort->u.bit.RxEn = 0;
            return (uint16_t)Res_AutoClosed;
        }
        return (uint16_t)Res_RxInterval;
    }
    
    return (uint16_t)Res_OK;
}

/*
****************************************************
*  函数名         : FuncTxIntHook
*  函数描述       : 发送中断应调用此函数
*  参数           : pSerialPort：控制块
*  返回值         : 
                    Res_OK
                    Res_TxFinished
                    Res_SendNotAllowes
                    
*  作者           : -5A4A5943-
*  历史版本       : 
*****************************************************
*/
uint16_t FuncTxIntHook(pSerialPortTypedef pSerialPort)
{
    BT_CHK_PARAM(pSerialPort);
    /* 确保我们处于发送状态 */
    if(pSerialPort->u.bit.TxEn == 1)
    {
        /* 开始发送字节流 */
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
            /* 标明发送完成 */
            pSerialPort->u.bit.TxFin = 1;
            pSerialPort->u.bit.TxEn = 0;
            return (uint16_t)Res_TxFinished;
        }
    }
    else
    {
        return (uint16_t)Res_SendNotAllowes;
    }
}





















