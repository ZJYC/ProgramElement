
#ifndef __N10_H__
#define __N10_H__

/*
****************************************************
*  文件名             : BT_BDE.h
*  作者               : --
*  版本               : V1.0
*  编写日期           : 2016--12--14--14--39--54
*  简介               : ！！C99标准！！
*  函数列表           : 
                        N10_OpsTypedef
                        N10_PriDataTypedef
                        N10_DriverTypedef
*  历史版本           : 
*****************************************************
*/

#include <stdio.h>
#include <string.h>
#include "includes.h"

/* 常用宏定义 */
#if 1

#define N10_True                 (0xff)
#define N10_False                (0x00)
#define N10_Locked               (0x01)
#define N10_Error                (0x02)
#define N10_DISCONNECTED         (0x03)
#define N10_RecvError            (0x04)
#define N10_RetryCost            (0x05)//重传次数用尽

#define N10_CHK_PARAM(x)         {if((uint32_t)x == 0)return N10_False;}
#define N10_CHK_BIT(val,bit)     ((val) & (1 << (bit)))
#define N10_SET_BIT(val,bit)     {(val) |= (1 << (bit));}
#define N10_CLR_BIT(val,bit)     {(val) &= ~(1 << (bit));}
#define N10_ABS(x)               ((x) > 0 ? (x):(-(x)))
#define N10_CHK_LEN(pxStr,Len)   {if(strlen((const char *)(pxStr)) > (Len))return N10_False;}

#define N10_FLAG_RxLock          (0x01)
#define N10_FLAG_CONNECTED       (0x02)
#define N10_FLAG_INITED          (0x04)

#endif

/* 蓝牙数据 */
typedef struct N10_PriDataTypedef_
{
    /* 当前角色 */
    uint8_t CurRole;
    /* 字符分解输出 */
    uint8_t *ParamSplit[6];
    /* 标志组 */
    uint16_t FlagGroup;
    /* 指令生成在此缓冲区中 */
    uint8_t InstructionBuff[60];
    
    uint8_t CSQ_Signal[4];
    uint8_t CSQ_Ber[4];
    
    uint8_t LastIP[20];
    uint8_t LastPort[20];
    uint8_t LastAPN[20];
    
    uint32_t Counter;
    
}N10_PriDataTypedef;
/* 串口驱动 */
typedef struct N10_UartTypedef_
{
    
    uint16_t(*Init)(uint16_t Baud);
    uint16_t(*Send)(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
    uint16_t(*Recv)(uint8_t * Data,uint16_t ExpectLen,uint16_t Timeout);
    
}N10_UartTypedef;
/* 蓝牙驱动 */
typedef struct N10_DriverTypedef_
{
    N10_UartTypedef          Uart;
    N10_PriDataTypedef   PriData;

    uint8_t(*TimingProcess)(uint16_t Period);
    
}N10_DriverTypedef;


extern N10_DriverTypedef N10_Driver;

#endif


