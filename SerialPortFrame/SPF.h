

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

#include "heap_5.h"
#include "ringbuffer.h"
#include "Time.h"

/*宏定义  */

/* 定义滴答的最大值，必须为循环计数 */
#define TickFull    (1000000)
/* 获取两个滴答之间的间隔 */
#define GetInterval(Cur,Last)   ((Cur) > Last() ? ((Cur) - (Last)) : (TickFull - (Last) + (Cur)))
/* 递增 */
#define TickAdd(val,add)            {\
                                    (val) += (add);\
                                    if((val) > TickFull)(val) = 0;\
                                    }
/* 用于检查参数 */
#define BT_CHK_PARAM(x)   if((uint32_t)x == 0)while(1);


/*变量定义*/

typedef struct SP_ResTypedef_
{
    Res_OK = 0,
    Res_NoData2Send = 1,
    Res_SendNotAllowes = 2,
    Res_NowTxing = 3,
    Res_TxFinished = 4,
    Res_TxTimeout = 5,
    Res_NowRxing = 6,
    Res_RxTimeout = 7,
    Res_RxDataHaveRecv = 8,
    
    Res_Reserve = 1000
}SP_ResTypedef;

typedef struct SerialPortTypedef_
{
    uint32_t    ID;
    //////////////////////////////////////////////////////////////////////
    /* 接收与发送缓冲 */
    Ringbuff_t  RxRB;
    Ringbuff_t  TxRB;
    uint32_t    LastRxTime;
    uint32_t    LastTxTime;
    uint32_t    RxInterval;
    union  
    {
        struct 
        {
            uint32_t RxFin  :   1;/* 接收完成标志 */
            uint32_t TxFin  :   1;/* 发送完成标志 */
            uint32_t RxEn   :   1;/* 允许接收数据 */
            uint32_t TxEn   :   1;/* 允许发送数据 */
            uint32_t Retried:   1;/* 表明发生重传 */
            uint32_t Txing  :   1;/* 正在发送 */
            //uint32_t Rxing  :   1;/* 正在接收中 */
            uint32_t AutoCloseRx:1;/* 自动关闭接收 */
        }bit;
        uint32_t bits;
    }u;
    uint32_t Counter;
}SerialPortTypedef,*pSerialPortTypedef;

/*  */
uint16_t LowerLayerRx(pSerialPortTypedef pSerialPort);
/* 发送数据，返回发送的数据长度 */
uint16_t LowerLayerTx(pSerialPortTypedef pSerialPort);
/* 串口初始化 */
uint16_t LowerLayerInit(pSerialPortTypedef pSerialPort,);
/* 回调函数 */

/* 定时回调 */
uint16_t FuncTickHook(pSerialPortTypedef pSerialPort,uint16_t Period);
/* 接收中断回调 */
uint16_t FuncRxIntHook(pSerialPortTypedef pSerialPort);
/* 发送中断回调 */
uint16_t FuncTxIntHook(pSerialPortTypedef pSerialPort);
/* 容错/保障/自动 */
uint16_t AutoAdopt(pSerialPortTypedef pSerialPort);
/* 用户接口 */
pSerialPortTypedef SP_Init(void);
uint16_t SP_Send(pSerialPortTypedef pSerialPort,uint8_t * Data,uint16_t Len);
uint16_t SP_Recv(pSerialPortTypedef pSerialPort,uint8_t * Data,uint16_t Len);




/*变量声明*/





/*函数声明*/



















































