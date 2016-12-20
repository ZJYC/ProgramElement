

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

uint16_t SP_Send(pSerialPortTypedef pSerialPort,uint8_t * Data,uint16_t Len)
{
    if(RB_FUNC.Remain(pSerialPort->TxRB) >= Len)
    {
        RB_FUNC.Write(pSerialPort->TxRB,Data,Len);
        LowerLayerTx(pSerialPort);
        return 0;
    }
    return 1;
}

























