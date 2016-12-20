
typedef struct SP_
{
    数据
    [
        RB  Rx;
        RB  Tx;
        U8  RxFin;
        U8  TxFin;
        U8  RxEn;
        U8  TxEn;
    ]
    底层
    [
        (*FuncRx);
        (*FuncTx);
        (*FuncInit);
        
        (*RB_Write);
        (*RB_Read);
        
        (*Malloc);
        (*Free);
    ]
    规则
    [
        (*FuncTiming);
        (*FuncRxRule);
        (*FuncTxRule);
    ]
    容错/保障/自动
    [
        
    ]
    API
    [
        (*SP_Init);
        (*SP_Send);
        (*SP_Recv);
    ]
}






















