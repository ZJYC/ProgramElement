

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





/*变量定义*/

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
    union  XXXX
    {
        struct XXX
        {
            uint32_t RxFin  :   1;/* 接收完成标志 */
            uint32_t TxFin  :   1;/* 发送完成标志 */
            uint32_t RxEn   :   1;/* 允许接收数据 */
            uint32_t TxEn   :   1;/* 允许发送数据 */
            uint32_t Retried:   1;/* 表明发生重传 */
        }bit;
        uint32_t bits;
    }u;
}SerialPortTypedef,*pSerialPortTypedef;


uint16_t LowerLayerRx(pSerialPortTypedef pSerialPort);
/* 发送数据，返回发送的数据长度 */
uint16_t LowerLayerTx(pSerialPortTypedef pSerialPort);
/* 串口初始化 */
uint16_t LowerLayerInit(pSerialPortTypedef pSerialPort,);
/* 回调函数 */
uint16_t FuncTick(pSerialPortTypedef pSerialPort,uint16_t Period);
uint16_t FuncRxByteHook(pSerialPortTypedef pSerialPort);
uint16_t FuncTxByteHook(pSerialPortTypedef pSerialPort);
/* 容错/保障/自动 */
uint16_t AutoAdopt(pSerialPortTypedef pSerialPort);
/* 用户接口 */
pSerialPortTypedef SP_Init(void);
uint16_t SP_Send(pSerialPortTypedef pSerialPort,uint8_t * Data,uint16_t Len);
uint16_t SP_Recv(pSerialPortTypedef pSerialPort,uint8_t * Data,uint16_t Len);




/*变量声明*/





/*函数声明*/



















































