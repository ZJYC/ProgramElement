
typedef struct SP_
{
    ����
    [
        RB  Rx;
        RB  Tx;
        U8  RxFin;
        U8  TxFin;
        U8  RxEn;
        U8  TxEn;
    ]
    �ײ�
    [
        (*FuncRx);
        (*FuncTx);
        (*FuncInit);
        
        (*RB_Write);
        (*RB_Read);
        
        (*Malloc);
        (*Free);
    ]
    ����
    [
        (*FuncTiming);
        (*FuncRxRule);
        (*FuncTxRule);
    ]
    �ݴ�/����/�Զ�
    [
        
    ]
    API
    [
        (*SP_Init);
        (*SP_Send);
        (*SP_Recv);
    ]
}






















