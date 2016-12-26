#include "includes.h"

uint16_t GPRSBufLen = 1;
char GPRSBuf[MAX_GPRSBUF_LEN] = {0};
//============================================================
//ȫ�ֱ�����IP PORT VPN ��Ҫ��ģ���ȡ��ÿ�ν��׶�Ҫ��ȡһ��
char TCP_PORT[7];
char TCP_IP[17];
char YL_APN[33];
char sig3[21] = {0};
//=============================================================
uint8_t  TransMode_FLAG = 0;
uint8_t  SIMCard_FLAG = 0;
extern volatile unsigned int M72D_RecT;
extern volatile char M72DRecTOut;
extern volatile unsigned int M72D_PwrT;
extern volatile char M72D_PwrTOut;
extern char  gcIsRunToMain;
extern unsigned char cFlagForOffLineSaleDisPlay;

// void Delay10ms(uint16_t time)
// {
// 	SetTimer1(time);
// 	while(CheckTimer1());
// 	
// 	return;
// }
static void Startdelay()
{
	int count = 5000;
	while(count--);
}
void M72D_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);// | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD
	
	//����GPRSģ�鹤��״̬���ţ���������PA8
	GPIO_InitStructure.GPIO_Pin = M72D_STUPin;					 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;		
	GPIO_Init( M72D_STUPort, &GPIO_InitStructure ); 
	//����GPRSģ��Ӳ���������������������
	GPIO_InitStructure.GPIO_Pin = M72D_RTSPin;					 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		
	GPIO_Init( M72D_RTSPort, &GPIO_InitStructure ); 	
	//����GPRSģ��Ӳ���������������������PA12
	GPIO_InitStructure.GPIO_Pin   = M72D_CTSPin;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;//GPIO_Mode_Out_PP;
	GPIO_Init(M72D_CTSPort, &GPIO_InitStructure);
	//����GPRSģ�鿪������PA11
	GPIO_InitStructure.GPIO_Pin   = M72D_PWRPin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(M72D_PWRPort, &GPIO_InitStructure);
	//���õ�Դʹ������PA1
	GPIO_InitStructure.GPIO_Pin   = M72D_ENPin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;//GPIO_Mode_IN_FLOATING;
	GPIO_Init(M72D_ENPort, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;//GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;//GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;//GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


//	GPIO_ResetBits( M72D_PWRPort, M72D_PWRPin ); //PWRKEY�����ϵ�����
	M72D_PWRKEY_E();//PWRKEY�����ϵ�����
}
/*
**�������ȷ��ģ��Ӧ���Ƿ���ȷ
**char *command:���͵�����
** char *reply��ģ����ȷ��Ӧ������
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
static int Comsend_back_Fail(char *command, char *reply)
{
	memset(GPRSBuf, 0, sizeof(GPRSBuf));  //��ǰ���
	
	Uart1ClrRevBuff();//
	Uart1ClrTxBuff();
	UartReset(M72D_Com);
//	ComSend(M72D_Com, (uint8_t *)command, strlen(command));
	Uart1WriteTxBuff((uint8_t *)command,strlen(command));/////
	SetTimer1(50);

	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
			return 1;
	}
	if( M72D_RxFinsh )
	{
		StopTimer1();
		M72D_RxFinsh = 0;
//		ComGetRxCount(M72D_Com, &GPRSBufLen);
//		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//		ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
		GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
		Uart1ClrRevBuff();//UartReset(M72D_Com);//ADD by zgy  ��մ���
		Uart1ClrTxBuff();
	}

	if( strcmp((char *)GPRSBuf , reply) ) //�ַ����Ƚϣ������˳�
	{
		memset(GPRSBuf, 0, sizeof(GPRSBuf));
	   return 1;
	} 
	 memset(GPRSBuf, 0, sizeof(GPRSBuf));
	 return 0; 
}
/////////////////////////////////////////////////////////////////////////////////////////////////

static int Comsend_back_Fail_APN(char *command, char *reply)
{
	memset(GPRSBuf, 0, sizeof(GPRSBuf));  //��ǰ���
	//M72D_RxFinsh = 0;
	Uart1ClrRevBuff();//
	Uart1ClrTxBuff();
	UartReset(M72D_Com);
//	ComSend(M72D_Com, (uint8_t *)command, strlen(command));
	Uart1WriteTxBuff((uint8_t *)command,strlen(command));
//	ComSend(M72D_Com, (uint8_t *)"\"", strlen("\""));
	Uart1WriteTxBuff((uint8_t *)"\"", strlen("\""));
//	ComSend(M72D_Com, (uint8_t *)YL_APN, strlen(YL_APN));
	Uart1WriteTxBuff((uint8_t *)YL_APN, strlen(YL_APN));
	//ComSend(M72D_Com, (uint8_t *)"\"", strlen("\""));
	Uart1WriteTxBuff((uint8_t *)"\"", strlen("\""));
//	ComSend(M72D_Com, (uint8_t *)"\n\r", strlen("\n\r"));
	Uart1WriteTxBuff((uint8_t *)"\n\r", strlen("\n\r"));

	SetTimer1(50);
	while( M72D_RxFinsh == 0 ){
		if( !CheckTimer1() )
			return 1;
	}
	StopTimer1();
	//M72D_RxFinsh = 0;
//	ComGetRxCount(M72D_Com, &GPRSBufLen);
//	if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//	ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
	GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
	Uart1ClrRevBuff();//UartReset(M72D_Com);//ADD by zgy  ��մ���
	Uart1ClrTxBuff();
	if( strcmp((char *)GPRSBuf , reply) ){ //�ַ����Ƚϣ������˳�	
		memset(GPRSBuf, 0, sizeof(GPRSBuf));
	   return 1;
	} 
	 memset(GPRSBuf, 0, sizeof(GPRSBuf));
	 return 0; 
}

/*
**����Ӧ������
**Fail:return 1
*/

int Adaptive_BD_Fail(void)
{	
	uint8_t BackBuf[10]={0x41, 0x54, 0x0D, 0x0D, 0x0A, 0x4F, 0x4B, 0x0D, 0x0A};
	char ATbuf[]={0x41, 0x54, 0x0D, 0x0A};
	uint8_t i=0;
	memset(GPRSBuf, 0, sizeof(GPRSBuf));	

	//M72D_RxFinsh = 0;
	Uart1ClrRevBuff();Uart1ClrTxBuff();//UartReset(M72D_Com);
//DisplayMess(48, "���ڳ�ʼ��GPRS11", 16);
//	ComSend(M72D_Com, (uint8_t *)ATbuf, 4);
 Uart1WriteTxBuff((uint8_t *)ATbuf, 4);
//DisplayMess(48, "���ڳ�ʼ��GPRS22", 16);	
	//ComSend(M72D_Com, (uint8_t *)AT, strlen(AT));
//	ComSend(3, (uint8_t *)AT, strlen(AT));//DEBUG
	SetTimer1(500);
	
	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
			return 1;
	}
	if( M72D_RxFinsh )
	{
		StopTimer1();
		M72D_RxFinsh = 0;
		
//		ComGetRxCount(M72D_Com, &GPRSBufLen);
//		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//		ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
		GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
		Uart1ClrRevBuff();Uart1ClrTxBuff();//
		UartReset(M72D_Com);//ADD by zgy  ��մ���
	}
//	ComSend(3, (uint8_t *)GPRSBuf, (GPRSBufLen));//DEBUG
	for(i=0; i<GPRSBufLen; i++)
	{
		if(GPRSBuf[i] != BackBuf[i])
		{
			memset(GPRSBuf, 0, sizeof(GPRSBuf));			
			return 1;
		}
			
	}
	
	//���������Զ��ϱ���ֱ���ӵ�
	for(i=0; i<3; i++)
	{
	
		M72D_RxFinsh = 0;
		SetTimer1(500);
		while( M72D_RxFinsh == 0 )
		{
			if( !CheckTimer1() )
			break;
		}
		if( M72D_RxFinsh )
		{
			StopTimer1();
			M72D_RxFinsh = 0;
//			ComGetRxCount(M72D_Com, &GPRSBufLen);
//			if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//			ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
			GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
			Uart1ClrRevBuff();Uart1ClrTxBuff();//
			UartReset(M72D_Com);//ADD by zgy  ��մ���
		}
		memset(GPRSBuf, 0, sizeof(GPRSBuf));//������� 

	}
	return 0;	//if( strcmp((uint8_t )GPRSBuf, "/r/nAT/r/n") )
}

/*
**���ô��ڲ�����Ϊ115200
*/
#if 0
static int SETBaudRate(void)
{
//	uint8_t GPRSBuf[MAX_GPRSBUF_LEN];
//	int AT_Comsend(char *command,char *rcv, char *reply,char *reply1,char at)
	
	if(Comsend_back_Fail(BaudRate,BackOK) != 0 )
		return 1;
	if(Comsend_back_Fail(SaveConfig,BackOK) != 0)
		return 1;
	
	return 0;
}
#endif
/*
**�ر�AT������Թ��ܣ������ڷ������͵�����
**Fail:return 1
*/
static int CloseEcho_Fail(void)
{
	uint8_t i=0;
	uint8_t ret = 1;
	char  TransBuf[100];

	memset(GPRSBuf, 0, sizeof(GPRSBuf));//��ǰ���  
	memset(TransBuf, 0, sizeof(TransBuf));//��ǰ���
	//M72D_RxFinsh = 0;
	Uart1ClrRevBuff();Uart1ClrTxBuff();//
	UartReset(M72D_Com);
	//ComSend(M72D_Com, (uint8_t *)CloseEcho, strlen(CloseEcho));
	Uart1WriteTxBuff((uint8_t *)CloseEcho, strlen(CloseEcho));
	SetTimer1(50);

	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
			return 1;
	}
	if( M72D_RxFinsh )
	{
		StopTimer1();
		M72D_RxFinsh = 0;
//		ComGetRxCount(M72D_Com, &GPRSBufLen);
//		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//		ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
		GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
		Uart1ClrRevBuff();Uart1ClrTxBuff();//
		UartReset(M72D_Com);//ADD by zgy  ��մ���
	}
//	ComSend(3, (uint8_t *)GPRSBuf, (GPRSBufLen));//DEBUG
	if( strcmp( (char *)GPRSBuf,  "ATE0\r\r\nOK\r\n") ) //�ַ����Ƚϣ������˳� TransBuf 
	{
		if( strcmp( (char *)GPRSBuf,  "\r\nOK\r\n") )
		{
			memset(GPRSBuf, 0, sizeof(GPRSBuf));
			return 1;
		}
		
	}  
	memset(GPRSBuf, 0, sizeof(GPRSBuf));//�������  
	 //�رտ��������Զ��ϱ�
	ret = Comsend_back_Fail(CloseURC, BackOK);
	if(ret == 1) //�ر�ʧ��
	{
		ClrDisplay();
	 	DisplayMess(0, "URC ģ�����ʧ��", 16);
		BEEP(1,3,1);
		//���������Զ��ϱ���ֱ���ӵ�,�����Ǳ��⴮������Ӧʧ�ܣ���������Ӧ֮������ϱ�����
		for(i=0; i<3; i++)
		{
		
				M72D_RxFinsh = 0;
			SetTimer1(50);
			while( M72D_RxFinsh == 0 )
			{
				if( !CheckTimer1() )
					break;
			}
			if( M72D_RxFinsh )
			{
				StopTimer1();
				M72D_RxFinsh = 0;
//				ComGetRxCount(M72D_Com, &GPRSBufLen);
//				if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//				ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
				GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
				Uart1ClrRevBuff();Uart1ClrTxBuff();//
				UartReset(M72D_Com);//ADD by zgy  ��մ���
		}
	}
		memset(GPRSBuf, 0, sizeof(GPRSBuf));//������� 

	}
	return 0;
}
#if 1
/*
**GPRS�ź�ǿ�Ȳ�ѯ
*/
int SignalIntensity_Inquire_Init(void)
{
	unsigned char sig2 = 0;
  int ret;
//	unsigned char iRet;
	char i;
//	extern unsigned char cFlagForOffLineSaleDisPlay;
	for(i=0;i<5;i++)
	{
  ret=AT_Comsend((char*)InquireCSQ,GPRSBuf);
	
  if(ret!=0)return ret;
#if 1
	if( GPRSBufLen ==  20)
	{

		GPRSBuf[8] = (GPRSBuf[8]& 0x0f) << 4;
		sig2 = (GPRSBuf[8]) | (GPRSBuf[9] & 0x0f) ;
		sig3[1] = sig2;
		sig3[2] = ((sig2>>4)&0x0f)*10;
		sig3[3] = sig2&0x0f;
		sig3[4] = sig3[2] + sig3[3];
		sig3[5] = ((sig2>>4)&0x0f)*10+ sig2&0x0f;

	}
	else if(GPRSBufLen == 19)
	{

		sig3[4]  =  GPRSBuf[8]&0x0f;
		 
	}
	
	if(0<sig3[4]<31)
	{
		break;	
	}
	Delay10ms(10);
  }
	if(i>=5) return LOWCSQ;
	#endif
	return 0;
}
#endif
int SIMCard_Inquire(void)
{
	int ret = 1;
	uint8_t i = 0;
// 	SignalIntensity_Inquire();
		if(SIMCard_FLAG == 0 )
	{
		SIMCard_FLAG = 1;
		ret = Comsend_back_Fail(PIN, PIN_OK);  //SIM��PIN���ѽ�
		if(ret == 1)
		{
		i=0;
			while( ret )
			{
				Delay10ms(100);
				i++;
				ret = Comsend_back_Fail(PIN, PIN_OK); 
				
				if( i==10 ) break;
			}
			if( i==10 ) return SIMFail;
    }			
			
		ret = Comsend_back_Fail(FindNet, FindNet_OK);
		if( ret == 1 ) 	//ȷ�������ɹ�
		{
			i=0;
			while( ret )
			{
				Delay10ms(100);
				i++;
				ret = Comsend_back_Fail(FindNet, FindNet_OK); 
				if(ret == 1) ret = Comsend_back_Fail(FindNet, FindNet_OK5); 
				if( i==30 ) break;
			}
			if( i==30 ) return NetFail;
		}
		ret = 	Comsend_back_Fail(GPRSAttach, GPRSAttach_OK);
		if( ret == 1 ) 	//��ѯGPRS���ųɹ�
		{
			i=0;
			while( ret )
			{
				Delay10ms(100);
				i++;
				ret = Comsend_back_Fail(GPRSAttach, GPRSAttach_OK); 
				
					if( i==30 ) break;
			}
			if( i==30 ) return NetFail;
		}
	}
	SignalIntensity_Inquire();
	return 0;
}

int Trans_ModeInit(void)
{
	if( Comsend_back_Fail(TransMode, BackOK) == 1 )
		return 1;
	if( Comsend_back_Fail(TransDatas, BackOK) == 1 )
		return 1;

	return 0;
}
/*
**�������ó�����ʽ����VPNΪ����ר��sdums.sd
**
*/
int Trans_Config(void)
{

  int ret;
  memset(GPRSBuf, 0, sizeof(GPRSBuf)); 
	ret=AT_Comsend((char *)"AT+QIFGCNT?\n\r",GPRSBuf);	
  if(ret!=0)return ret;
	if( strcmp((char *)GPRSBuf , "\r\n+QIFGCNT: 0,0\r\n\r\nOK\r\n") )
	{if( Comsend_back_Fail(SetFGCNT, BackOK) == 1 )
	return ModeFail;}	
	 memset(GPRSBuf, 0, sizeof(GPRSBuf)); 
	 ret=AT_Comsend((char *)"AT+QIMUX?\n\r",GPRSBuf);	
   if(ret!=0)return ret;
	 if( strcmp((char *)GPRSBuf , "\r\n+QIMUX: 0\r\n\r\nOK\r\n") )		
	 {if( Comsend_back_Fail(SetMUXIP, BackOK) == 1 )
	 return ModeFail;
	 }
	 memset(GPRSBuf, 0, sizeof(GPRSBuf)); 
	 ret=AT_Comsend((char *)"AT+QIMODE?\n\r",GPRSBuf);	
   if(ret!=0)return ret;
	 if( strcmp((char *)GPRSBuf , "\r\n+QIMODE: 1\r\n\r\nOK\r\n") )		
	 if( Comsend_back_Fail(TransMode, BackOK) == 1 )
	 return ModeFail;
	 memset(GPRSBuf, 0, sizeof(GPRSBuf)); 
	 ret=AT_Comsend((char *)"AT+QITCFG?\n\r",GPRSBuf);	
   if(ret!=0)return ret;
	 if( strcmp((char *)GPRSBuf , "\r\n+QITCFG: 3,2,1460,1\r\n\r\nOK\r\n") )		
		if( Comsend_back_Fail(TransDatas, BackOK) == 1 )
			return ModeFail;
		memset(GPRSBuf, 0, sizeof(GPRSBuf)); 
		ret=AT_Comsend((char *)"AT+QIDNSIP?\n\r",GPRSBuf);	
   if(ret!=0)return ret;
	 if( strcmp((char *)GPRSBuf , "\r\n+QIDNSIP: 0\r\n\r\nOK\r\n") )	
		if( Comsend_back_Fail(SetTCPIP, BackOK) == 1 )
			return ModeFail;
	
	if( Comsend_back_Fail_APN(SetUnionPayAPN, BackOK) == 1 )///ÿ�ζ���������
	return ModeFail;	
	return 0;
}
/*
**�������ò��Գ�����ʽ����VPNΪ����ר��sdums.sd
���Գ���VPNΪCMNET
*/
int Trans_Config_test(void)
{
	//ֻ����һ�δ��䷽ʽ�����ã����ڵڶ�������ʱMUXIP�޷�ͨ��

	if(TransMode_FLAG == 0)
	{
		TransMode_FLAG = 1;
		if( Comsend_back_Fail(SetFGCNT, BackOK) == 1 )
			return ModeFail;
		if( Comsend_back_Fail(SetAPN, BackOK) == 1 )
			return ModeFail;	
		if( Comsend_back_Fail(SetMUXIP, BackOK) == 1 )
			return ModeFail;
		if( Comsend_back_Fail(TransMode, BackOK) == 1 )
			return ModeFail;
		if( Comsend_back_Fail(TransDatas, BackOK) == 1 )
			return ModeFail;
		if( Comsend_back_Fail(SetTCPIP, BackOK) == 1 )
			return ModeFail;
	}
	return 0;
}


/*
**M72Dģ���ʼ��
**GPIO�����ڡ�����������������Ӧ���ػ���
*/
int M72D_SysInit(void)
{
	COMCONFIG com_config;

//	UartReset(M72D_Com);
	M72D_RxFinsh = 0;

	if(ReadStu() == SET){ //������ǰ�Ѿ��ϵ�ɹ� --W.F. 2016-1-14 10:43:05	
		return 0;
	}
	//GPIO��ʼ��
	M72D_GPIOInit();
	M72D_Enable();//��Դʹ��
	//���ڳ�ʼ��
	ComConfig((COMCONFIG *)&com_config, 115200, 8, 1, 0);
	ComClose( M72D_Com );
	ComOpen( M72D_Com ,&com_config );
//	//����ģ��
//	GPIO_ResetBits( M72D_PWRPort, M72D_PWRPin );	
//	Startdelay();//���ͳ���30msʱ��
	GPIO_SetBits( M72D_PWRPort, M72D_PWRPin );
	Startdelay();//���ͳ���30msʱ��
	StartGSTimer(130);
	while(M72D_PwrTOut == 0);
	StopGSTimer();
	StartGSTimer(210);
	while(ReadStu() == RESET)
	{
	
	 	if( !CheckSTimer() ) 
		{
			StopGSTimer();
		//	DisplayMess(32, "  ����ʧ��....  ", 16);//DEBUG
			return 1;	
		}		
	}
	Delay10ms(50);
	Uart1ClrRevBuff();Uart1ClrTxBuff();//UartReset(M72D_Com);

	GPIO_ResetBits( M72D_PWRPort, M72D_PWRPin );

	return 0;
}


static int TCP_Link(char *IP, char *PORT)
{
	memset(GPRSBuf, 0, sizeof(GPRSBuf));//��ǰ���  

//	ComSend(M72D_Com, (uint8_t *)TCPLink, strlen(TCPLink));
//	ComSend(M72D_Com, (uint8_t *)"\"", strlen("\""));		
//	ComSend(M72D_Com, (uint8_t *)IP, strlen(IP));
//	ComSend(M72D_Com, (uint8_t *)"\"", strlen("\""));	
//	ComSend(M72D_Com, (uint8_t *)",", strlen(","));
//	ComSend(M72D_Com, (uint8_t *)"\"", strlen("\""));		
//	ComSend(M72D_Com, (uint8_t *)PORT, strlen(PORT));
//	ComSend(M72D_Com, (uint8_t *)"\"", strlen("\""));	
// 	ComSend(M72D_Com, (uint8_t *)"\r\n", strlen("\r\n"));
	
	Uart1WriteTxBuff((uint8_t *)TCPLink, strlen(TCPLink));
	Uart1WriteTxBuff((uint8_t *)"\"", strlen("\""));	
	Uart1WriteTxBuff((uint8_t *)IP, strlen(IP));
	Uart1WriteTxBuff((uint8_t *)"\"", strlen("\""));
	Uart1WriteTxBuff((uint8_t *)",", strlen(","));
	Uart1WriteTxBuff((uint8_t *)"\"", strlen("\""));
	Uart1WriteTxBuff((uint8_t *)PORT, strlen(PORT));
	Uart1WriteTxBuff((uint8_t *)"\"", strlen("\""));
	Uart1WriteTxBuff((uint8_t *)"\r\n", strlen("\r\n"));
	
	M72D_RxFinsh = 0;
	SetTimer1(500);
	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
			return TCPFail;
	}
	if( M72D_RxFinsh )
	{
		StopTimer1();
		M72D_RxFinsh = 0;
//		ComGetRxCount(M72D_Com, &GPRSBufLen);
//		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//		ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
		GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
		Uart1ClrRevBuff();Uart1ClrTxBuff();//
		UartReset(M72D_Com);//ADD by zgy  ��մ���
	}
// 	ComSend(3, (uint8_t *)GPRSBuf, (GPRSBufLen));//DEBUG
	if( strcmp( (char *)GPRSBuf, BackOK) ) //�ַ����Ƚϣ������˳�
	{
		memset(GPRSBuf, 0, sizeof(GPRSBuf));//�������	
	   return CommandFail;
	}  
	memset(GPRSBuf, 0, sizeof(GPRSBuf));//�������  
	
	M72D_RxFinsh = 0;
	SetTimer1(7500); //�ȴ����Ϸ�����
	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
			return LinkFAIL;
	}
	if( M72D_RxFinsh )
	{
		StopTimer1();
		M72D_RxFinsh = 0;
//		ComGetRxCount(M72D_Com, &GPRSBufLen);
//		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//		ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
		GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
		Uart1ClrRevBuff();Uart1ClrTxBuff();//
		UartReset(M72D_Com);//ADD by zgy  ��մ���
	}
//	strcpy(TransBuf, GPRSBuf);
	//ComSend(3, (uint8_t *)GPRSBuf, (GPRSBufLen));//DEBUG
	if( (strcmp( (char *)GPRSBuf, ConnectOK) == 0) || (strcmp( (char *)GPRSBuf, Connect) == 0) ) //�ַ����Ƚϣ������˳�
	{
		memset(GPRSBuf, 0, sizeof(GPRSBuf));//�������	
	  return APP_SUCC;// LinkOK
	}  
	else if (strcmp( (char *)GPRSBuf, ConnectFAIL) == 0)
	{
		memset(GPRSBuf, 0, sizeof(GPRSBuf));//������� 
		return LinkFAIL; 	
	}
	else
	{
		memset(GPRSBuf, 0, sizeof(GPRSBuf));//�������  
	
		return LinkFAIL;			
	}
	
}

int Socket_Link(char *IP, char *PORT)
{
	int ret=1;
	uint8_t repeat=0;
	memset(GPRSBuf, 0, sizeof(GPRSBuf));
	//��ѯ����״̬
	//M72D_RxFinsh = 0;
	Uart1ClrRevBuff();Uart1ClrTxBuff();//
	UartReset(M72D_Com);
//	ComSend(M72D_Com, (uint8_t *)NetState, strlen(NetState));
	Uart1WriteTxBuff((uint8_t *)NetState, strlen(NetState));
	SetTimer1(50);
	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
			return TCPFail;
	}
	if( M72D_RxFinsh )
	{
		StopTimer1();
		M72D_RxFinsh = 0;
//		ComGetRxCount(M72D_Com, &GPRSBufLen);
//		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//		ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
		GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
		Uart1ClrRevBuff();Uart1ClrTxBuff();//
		UartReset(M72D_Com);//ADD by zgy  ��մ���
	}
// 	ComSend(3, (uint8_t *)GPRSBuf, strlen(GPRSBuf));//DEBUG
	memset(GPRSBuf, 0, sizeof(GPRSBuf));
	if( strcmp( (char *)GPRSBuf, "/r/nSTATE: IP INITIAL/r/n") == 0 )
	{
		memset(GPRSBuf, 0, sizeof(GPRSBuf));
	} 
	else if( strcmp( (char *)GPRSBuf, "/r/nSTATE: IP STATUS/r/n") == 0 )
	{
					 
		memset(GPRSBuf, 0, sizeof(GPRSBuf));
	} 
	else if( strcmp( (char *)GPRSBuf, "/r/nSTATE: IP CLOSE/r/n") == 0 )
	{
					 
		memset(GPRSBuf, 0, sizeof(GPRSBuf));
	} 
	//�����������״̬������Ҫ�ȹر�
	else if( strcmp( (char *)GPRSBuf, "/r/nSTATE: IP CONNECTING/r/n") == 0 )
	{
		memset(GPRSBuf, 0, sizeof(GPRSBuf));
//		M72D_RxFinsh = 0;
		Uart1ClrRevBuff();Uart1ClrTxBuff();//
		UartReset(M72D_Com);
//		ComSend(M72D_Com, (uint8_t *)IPClose, strlen(IPClose));
		Uart1WriteTxBuff((uint8_t *)IPClose, strlen(IPClose));
		SetTimer1(500);
		ret = 1;
		while( M72D_RxFinsh == 0 )
		{
			if( !CheckTimer1() )  //����ȴ�IP CLOSE����δ�ȵ�����ʱ�Ͽ���ǰGPRS����
			{
			    ret = Comsend_back_Fail(GPRSDeact, DeactOK);
				if(ret == 1)//GPRS�����޷��Ͽ�������ģ��
				{
					//�����������ģ�����
					return RESTART;
				}
				else if(ret == 0) break;	
			
			}
		}
		if( M72D_RxFinsh )
		{
			StopTimer1();
			M72D_RxFinsh = 0;
//			ComGetRxCount(M72D_Com, &GPRSBufLen);
//			if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//			ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
			GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
			Uart1ClrRevBuff();Uart1ClrTxBuff();//
			UartReset(M72D_Com);//ADD by zgy  ��մ���
				
			if( strcmp( (char *)GPRSBuf, CloseOK) == 0 )
			{			 
				memset(GPRSBuf, 0, sizeof(GPRSBuf));				
			}
			else return CloseFail;
		}	
	}
	ret = 1;
	ret = TCP_Link(IP, PORT);
	while((ret != APP_SUCC) && (repeat<3))
	{
		Delay10ms(100);
		ret = TCP_Link(IP, PORT);
		repeat++;
	}

	return ret;		
}
/*
**͸������ص�����ģʽ
*/
int Transparen2Command_Fail(void)
{
	memset(GPRSBuf, 0, sizeof(GPRSBuf));  //��ǰ���
	/*"+++"����ǰ1S������*/
	SetTimer1(100);
	while(CheckTimer1());
	StopTimer1();
	
//	M72D_RxFinsh = 0;
	Uart1ClrRevBuff();Uart1ClrTxBuff();//
	UartReset(M72D_Com);
//	ComSend(M72D_Com, (uint8_t *)TPlus, strlen(TPlus));
	Uart1WriteTxBuff((uint8_t *)TPlus, strlen(TPlus));
	//��+++������0.5S�������ݣ��������ĵȴ���������ʱ���ӳ���һ��
	SetTimer1(200);
	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
			return 1;
	}
	if( M72D_RxFinsh ){
		StopTimer1();
		M72D_RxFinsh = 0;
//		ComGetRxCount(M72D_Com, &GPRSBufLen);
//		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//		ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
		GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
		Uart1ClrRevBuff();Uart1ClrTxBuff();//
		UartReset(M72D_Com);//ADD by zgy  ��մ���
	}
// 	ComSend(3, (uint8_t *)GPRSBuf, strlen(GPRSBuf));//DEBUG
	if(strcmp((char *)GPRSBuf , BackOK)) //�ַ����Ƚϣ������˳�
	{
		memset(GPRSBuf, 0, sizeof(GPRSBuf));
	   return 1;
	} 
	 memset(GPRSBuf, 0, sizeof(GPRSBuf));
	 return 0; 

}
int Socket_Close(void)
{
	if(Comsend_back_Fail(IPClose, CloseOK) == 1 )
	return 1 ;
	
	return 0;	
}

int Shutdown(void)
{
	//
	if(ReadStu() == SET)//ģ�鴦�ڿ���״̬
	{
			Uart1ClrRevBuff();Uart1ClrTxBuff();//UartReset(M72D_Com);
			//�������ػ�ʧ�ܲ��ÿ������ػ�
			GPIO_ResetBits( M72D_PWRPort, M72D_PWRPin );	
			Startdelay();//���ͳ���30msʱ��
			GPIO_SetBits( M72D_PWRPort, M72D_PWRPin );
			SetTimer1(80);//����ʱ��0.6S~1S
			while( CheckTimer1());			
			GPIO_ResetBits( M72D_PWRPort, M72D_PWRPin );
			StopTimer1();

			//�ϱ��ػ���Ϣ
			M72D_RxFinsh = 0;
			//UartReset(M72D_Com);
			SetTimer1(500);

			while( M72D_RxFinsh == 0 )
			{
				if( !CheckTimer1() )
					return 1;
			}
			if( M72D_RxFinsh )
			{
				StopTimer1();
				M72D_RxFinsh = 0;
//				ComGetRxCount(M72D_Com, &GPRSBufLen);
//				if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//				ComRecv(M72D_Com, (uint8_t *)GPRSBuf, &GPRSBufLen);
				GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
				Uart1ClrRevBuff();Uart1ClrTxBuff();//
				UartReset(M72D_Com);//ADD by zgy  ��մ���
			}
    // 	ComSend(3, (uint8_t *)GPRSBuf, strlen(GPRSBuf));//DEBUG
		//	sprintf(TransBuf, "%d", GPRSBuf);//��uint8_tת��Ϊchar��
			if( strcmp((char *)GPRSBuf , NormalDown) ) //�ַ����Ƚϣ������˳�
			{
				if( strcmp((char *)GPRSBuf , NormalDownPDP) )
				{
				 memset(GPRSBuf, 0, sizeof(GPRSBuf));
				 return 1;
				}				
			} 
			 memset(GPRSBuf, 0, sizeof(GPRSBuf));

	}
	
	  return 0;	
}
/*****************************************************************
      �ⲿ���ò���
******************************************************************/
/*
**�����豸��ʼ��
*/

int NetworkDevices_init(void)
{
	if(M72D_SysInit())//Ӳ���豸��ʼ��
		return -1;
	
	return 0;
}
int NetworkDevicesGPIO_init(void)
{
	COMCONFIG com_config;
	M72D_RxFinsh = 0;
	
	M72D_GPIOInit();
	M72D_Enable();//��Դʹ��
	//���ڳ�ʼ��
	ComConfig((COMCONFIG *)&com_config, 115200, 8, 1, 0);
	ComClose( M72D_Com );
	ComOpen( M72D_Com ,&com_config );
//	//����ģ��
//	GPIO_ResetBits( M72D_PWRPort, M72D_PWRPin );	
//	Startdelay();//���ͳ���30msʱ��
	GPIO_SetBits( M72D_PWRPort, M72D_PWRPin );
	Startdelay();//���ͳ���30msʱ��
	StartGSTimer(230);
	while(M72D_PwrTOut == 0);
	StopGSTimer();
	StartGSTimer(310);
	while(ReadStu() == RESET)
	{
	
	 	if( !CheckSTimer() ) 
		{
			StopGSTimer();
		//	DisplayMess(32, "  ����ʧ��....  ", 16);//DEBUG
			return 1;	
		}		
	}
	GPIO_ResetBits( M72D_PWRPort, M72D_PWRPin );

	return 0;
}
/**
***Ϊ����Ӧ֮ǰWiFi�汾��Z03A���������Ԥ���ų������Ϊ��������ʹ��
**/
int CommPredel(char cType)
{
// 			memset(TCP_PORT,0,7);
// 	memset(TCP_IP,0,17);
// 	strcpy(TCP_PORT, IP);
// 	strcpy(TCP_IP, IP);
	
		char Protocol[4];

	memset(Protocol,0,4);
//	memset(TCP_PORT,0,7);
//	memset(TCP_IP,0,17);
	GetParam((char *)&stPosParam);

	memcpy(Protocol,"TCP",3);
	if (cType == 0)
	{
//		strcpy(TCP_IP,stPosParam.szCommIp1);
//		strcpy(TCP_PORT,stPosParam.szCommPort1);
//		strcpy(TCP_IP,MS_Param.serverIP);
//		strcpy(TCP_PORT,MS_Param.serverPort);
	}
	else if (cType == 1)
	{
//		strcpy(TCP_IP,stPosParam.szCommIp2);
//		strcpy(TCP_PORT,stPosParam.szCommPort2);
//		strcpy(TCP_IP,MS_Param.serverIP);
//		strcpy(TCP_PORT,MS_Param.serverPort);
	}


	
	return 1;
}
/*
*����socket����
*/
int Connnect(void)
{
	int ret = 1;
	ret = SIMCard_Inquire();
	if( ret == SIMFail || ret == NetFail )//SIM����ѯ
		return SIMFail;
	if( Trans_Config() == ModeFail )  //TCP��������
		return ModeFail;
//	DisplayMess(32, "���ڲ������Ժ�  ", 16);
	ret =  Socket_Link(TCP_IP, TCP_PORT);//��������
//	memset(TCP_PORT,0,7);
//	memset(TCP_IP,0,17);
//	DisplayMess(32, "    ���ųɹ�    ", 16);
	 return ret;
}

//�Ҷ�socket����
int CommHangupSocket(void)
{
	/*������ģʽת��Ϊ����ģʽ*/
//	DisplayMess(16, "    ��ʼ�Ҷ�    ", 16);
	if( Transparen2Command_Fail() ){
//		DisplayMess(16, "    �л�ʧ��    ", 16);
//		return 1;
	}
//	DisplayMess(16, "    �л����    ", 16);
	if( Socket_Close() ){//�ر�����
//		DisplayMess(16, "    �Ҷ�ʧ��    ", 16);
//		return 1;
	}
//	DisplayMess(16, "    �Ҷ����    ", 16);
	return 0;
}
/*
*�����豸��Դʧ��
*/
int NetworkDevices_ENDisable(void)
{

	NetworkDevices_Disable();
	M72D_Disable();	
	
	return 0;
}
//�豸�ػ�����
int NetworkDevices_Disable(void)
{
	TransMode_FLAG = 0;//����ֻ����һ�δ���ģʽ���ػ��ٴο��������������
	SIMCard_FLAG = 0;
	
	if(Shutdown()) 	return 1;
		
	return 0;
}
/*
**�����豸��ʼ���������������ӹ����еĴ�����Ϣ��ʾ
*/
int NetworkDevices_Wrong(int nRet)
{
	if ( (nRet == SIMFail) || (nRet == ModeFail) ) //W.F.  2015-5-11
		{
			ClrDisplay();
			DisplayMess(32,(unsigned char *)"��ѯSIM ��ʧ��",14);
			BEEP(3,2,1);
			Delay10ms(80);
			CommHangupSocket();
			return APP_FAIL;
		}
		else if( (nRet == TCPFail) || (nRet == LinkFAIL) || (nRet == CommandFail) ) //W.F.  2015-5-11
		{
			ClrDisplay();
			DisplayMess(32,(unsigned char *)"TCP ����ʧ��",13);
			BEEP(3,2,1);
			Delay10ms(80);
			CommHangupSocket();
			return APP_FAIL;
		}	
			return 0;
}
#if 0
//���̲���
int GPRS_test(void)
{
//	char button = 0;
	ClrDisplay();
	DisplayMess(0, "    ����ģ�����", 16);	
	M72D_SysInit();
  /* Add your application code here
     */
	StartGSTimer(100);//����ʱ�䳬��һ��
	while( M72D_PwrTOut == 0 );	
	StopGSTimer();
		//���벨��������Ӧ//���ﲻ���д��󷵻أ���Ϊ�ں����������Խ�������Ӧ
	Adaptive_BD_Fail();	
	if(CloseEcho_Fail())
	{
		DisplayMess(32, "  �ػ���ʧ��..  ", 16);//DEBUG
 		return 1;
	}
	if(SETBaudRate())
	{
		DisplayMess(32, "  ������ʧ��..  ", 16);//DEBUG
 		return 1;
	}	
	if(SIMCard_Inquire() == SIMFail) 
	{
		DisplayMess(32,"SIM ����ѯʧ��..",16);	
		BEEP(3,2,1);
		NetworkDevices_Disable();
		return 1;
	}
	if(Trans_Config_test() == ModeFail)
	{
		DisplayMess(32,"����ģʽ����ʧ��",16);
		BEEP(3,2,1);
    NetworkDevices_Disable();
		return 1;
	}

	if(Socket_Link("139.129.133.177", "80") != APP_SUCC)//
	{
		DisplayMess(32,"��������ʧ��....",16);
		BEEP(3,2,1);
		NetworkDevices_Disable();
		return 1;
	} 
	
	if( Transparen2Command_Fail() )
	{
		DisplayMess(32,"ģʽת��ʧ��....",16);	
		BEEP(3,2,1);
		NetworkDevices_Disable();
		return 1;
	}
			Delay10ms(200);
	
	if( Socket_Close() )
	{
		DisplayMess(32,"�Ͽ�����ʧ��....",16);	
		BEEP(3,2,1);
		NetworkDevices_Disable();
		return 1; 
	}
// 	Shutdown();
//	if( NetworkDevices_Disable() )
//	{
//		DisplayMess(32,"�ػ�ʧ�� .......",16);	
//		BEEP(3,2,1);
//		NetworkDevices_Disable();
//	  return 1;
//	}
	CloseXP100RF();
	ClrDisplay();
	DisplayMess(32, "GPRSģ����Գɹ�", 16);
	BEEP(1,3,1);
//	while(1)
//	{
//		button = GetButton();
//		if(button == 'N') 	return 0;
//		SignalIntensity_Inquire();
//		Startdelay();
//		Delay10ms(100);
//	}
	return 0;
}
/*
**GPRS�ź�ǿ�Ȳ�ѯ
*/
int SignalIntensity_Test(void)
{
	unsigned char sig2 = 0;
 	unsigned char iRet;
 	char szTempBuf[16+1] = {0};
	iRet =  Comsend_back_Fail(PIN, PIN_OK);
	if(ReadStu() == RESET || iRet == 1) //ģ��δ����//δ�忨
	{
		DisplayMess(0,"  ",2);
		Delay10ms(5);
		ClrRAMDisplay();
		Delay10ms(5);
		dzDisplayPicture2(0, 0, GPRSErr_signal);
		return 0;
	}
	memset(GPRSBuf, 0, sizeof(GPRSBuf));  
	//M72D_RxFinsh = 0;
	Uart1ClrRevBuff();Uart1ClrTxBuff();//UartReset(M72D_Com);
	//ComSend(M72D_Com, (uint8_t *)InquireCSQ, strlen(InquireCSQ));
	Uart1WriteTxBuff((uint8_t *)InquireCSQ, strlen(InquireCSQ));
	SetTimer1(50);

	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
			return 1;
	}
	if( M72D_RxFinsh )
	{
		StopTimer1();
		M72D_RxFinsh = 0;
		ComGetRxCount(M72D_Com, &GPRSBufLen);
		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
		ComRecv(M72D_Com, GPRSBuf, &GPRSBufLen);
	}
	
	if( GPRSBufLen ==  20)
	{
		memset(szTempBuf, 0, sizeof(szTempBuf));
		sprintf(szTempBuf, "������:%c        ", GPRSBuf[11]);
		DisplayMess(16, (unsigned char*)szTempBuf, 16);

		memset(szTempBuf, 0, sizeof(szTempBuf));
		sprintf(szTempBuf, "�ź���:%2.2s       ", &GPRSBuf[8]);
		DisplayMess(32, (unsigned char*)szTempBuf, 16);
		
//		DisplayMess(16, &GPRSBuf[8], 2);
//		DisplayMess(20, &GPRSBuf[11], 1);
//		errcode[0] = GPRSBuf[11]-0x30;
		GPRSBuf[8] = (GPRSBuf[8]& 0x0f) << 4;
		sig2 = (GPRSBuf[8]) | (GPRSBuf[9] & 0x0f) ;
		sig3[1] = sig2;
		sig3[2] = ((sig2>>4)&0x0f)*10;
		sig3[3] = sig2&0x0f;
		sig3[4] = sig3[2] + sig3[3];
		sig3[5] = ((sig2>>4)&0x0f)*10+ sig2&0x0f;

	}
	else if(GPRSBufLen == 19)
	{
		memset(szTempBuf, 0, sizeof(szTempBuf));
		sprintf(szTempBuf, "������:%c        ", GPRSBuf[10]);
		DisplayMess(16, (unsigned char*)szTempBuf, 16);

		memset(szTempBuf, 0, sizeof(szTempBuf));
		sprintf(szTempBuf, "�ź���:%c        ", GPRSBuf[8]);
		DisplayMess(32, (unsigned char*)szTempBuf, 16);
		
//		DisplayMess(16, &GPRSBuf[8], 1);
//		DisplayMess(20, &GPRSBuf[10], 1);
//		errcode[0] = (GPRSBuf[10])-0x30;
		sig3[4]  =  GPRSBuf[8]&0x0f;
		 
	}

	DisplayMess(0,"  ",2);
	Delay10ms(5);
	ClrRAMDisplay();
	Delay10ms(5);
	memset(szTempBuf, 0, sizeof(szTempBuf));
	if(sig3[4]<stPosParam.cMinSing || sig3[4]>31)
	{
		DisplayMess(48, "�źŲ�ܾ�����", 16);
		dzDisplayPicture2(0, 0, GPRSErr_signal);
	}
	else if(sig3[4] >=stPosParam.cMinSing && sig3[4] <= 15)
	{
		DisplayMess(48, "    �źŽϲ�    ", 16);		
		dzDisplayPicture2(0, 0, GPRS0_signal);
	}
	else if( sig3[4] >15 && sig3[4] < 20) 
	{
		DisplayMess(48,  "    �ź�һ��    ", 16);		
		dzDisplayPicture2(0, 0, GPRS1_signal);
	}
	else if( 20<= sig3[4] && sig3[4]<= 25 ) 
	{
		DisplayMess(48, "    �ź�����    ", 16);
		dzDisplayPicture2(0, 0, GPRS2_signal);
	}
	else if ( sig3[4] > 25 && sig3[4] <= 31 )
	{
		DisplayMess(48, "    �ź�����+    ", 16);
		dzDisplayPicture2(0, 0, GPRS3_signal);
	}

	return 0;
}

int CheckSignal(void)
{
 	unsigned char iRet;
 	char szTempBuf[16+1] = {0};
	int i = 0;
	int j = 0;

	j = 0;
	for(i=0; i<10; i++)
	{
		iRet =  Comsend_back_Fail(PIN, PIN_OK);
		if(ReadStu() == RESET || iRet == 1) //ģ��δ����//δ�忨
		{
			if(cFlagForOffLineSaleDisPlay == 0)
			{
				DisplayMess(0,"  ",2);
				Delay10ms(5);
				ClrRAMDisplay();
				Delay10ms(5);
				dzDisplayPicture2(0, 0, GPRSErr_signal);
			}

			Delay10ms(200);//ģ��δ����,�ȴ�2S�ٴ�ѭ��
			continue;
		}
		memset(GPRSBuf, 0, sizeof(GPRSBuf));  
		//M72D_RxFinsh = 0;
		Uart1ClrRevBuff();Uart1ClrTxBuff();//UartReset(M72D_Com);
//		ComSend(M72D_Com, (uint8_t *)InquireCSQ, strlen(InquireCSQ));
		Uart1WriteTxBuff((uint8_t *)InquireCSQ, strlen(InquireCSQ));
		SetTimer1(50);

		while( M72D_RxFinsh == 0 )
		{
			if( !CheckTimer1() )
			{
				return 1;
			}
		}
		if( M72D_RxFinsh )
		{
			StopTimer1();
			M72D_RxFinsh = 0;
			ComGetRxCount(M72D_Com, &GPRSBufLen);
			if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
			ComRecv(M72D_Com, GPRSBuf, &GPRSBufLen);
		}
		
		if( GPRSBufLen ==  20)
		{
			memset(szTempBuf, 0 ,sizeof(szTempBuf));
			memcpy(szTempBuf, &GPRSBuf[8], 2);

			if(atoi(szTempBuf) >= stPosParam.cMinSing && atoi(szTempBuf) < 32)
			{
				if( GPRSBuf[11] == '0' || j > 1)//�������ʻ����������źŴ�꣬�������ʶ���ɹ�
				{
					
					if(cFlagForOffLineSaleDisPlay == 0)
					{
						DisplayMess(0,"  ",2);
						Delay10ms(5);
						ClrRAMDisplay();
						Delay10ms(5);

						dzDisplayPicture2(0, 0, GPRS2_signal);
					}
					return APP_SUCC;
				}
				j++;
				i--;
			}
		}
		else if((stPosParam.cMinSing == 9) && (GPRSBufLen ==  19))//�ź�Ϊһλ��
		{
 			if((GPRSBuf[8]&0x0f) >= stPosParam.cMinSing )
 			{
 				if( GPRSBuf[10] == '0' || j > 1)//�������ʻ����������źŴ�꣬�������ʶ���ɹ�
 				{
	 				if(cFlagForOffLineSaleDisPlay == 0)
	 				{
						DisplayMess(0,"  ",2);
						Delay10ms(5);
						ClrRAMDisplay();
						Delay10ms(5);

						dzDisplayPicture2(0, 0, GPRS1_signal);
	 				}
					return APP_SUCC;
 				}
				j++;
				i--;
			}
		}

		if(cFlagForOffLineSaleDisPlay == 0)
		{
			DisplayMess(0,"  ",2);
			Delay10ms(5);
			ClrRAMDisplay();
			Delay10ms(5);
			dzDisplayPicture2(0, 0, GPRS0_signal);	
		}
		Delay10ms(100);//ģ��δ����,�ȴ�1S�ٴ�ѭ��
	}

	return APP_FAIL;
}
#endif
int GPRS_SignalIntensity_test1(void)
{
//	uint16_t len = 0;
//	char button = 0;
	int8_t err = 0;
//	uint8_t flag;
	uint8_t recdata[1024] = {0, 0,\
	9};
	char TxData[524];
//	char TxData1[97]={0x0,0x5B,0x60,0x06,0x01,0x0,0x0,0x60,0x31,0x0,0x31,0x08,0x42,0x08,0x00,\
//0x0,0x20,0x00,0x00,0x0,0xC0,0x0,0x16,0x0,0x06,0x02,0x33,0x32,0x33,0x32,0x30,0x32,0x39,\
//		0x36,0x31,0x30,0x35,0x32,0x39,0x30,0x30,0x35,0x33,0x33,0x31,0x30,0x31,0x34,0x39,0x00,\
//		0x11,0x0,0x0,0x07,0x68,0x0,0x40,0x0,0x29,0x53,0x65,0x71,0x75,0x65,0x6E,0x63,0x65,0x20,\
//0x4E,0x6F,0x31,0x36,0x33,0x31,0x33,0x37,0x42,0x35,0x4E,0x4C,0x30,0x30,0x32,0x33,0x39,\
//0x30,0x35,0x36,0x00,0x03,0x30,0x31,0x20,0x0D,0x0A,0x0D,0x0A}; 
//	unsigned char show[2];
  unsigned char show2[26];
	int nRet = 0;
  int sendlen = 0;
	int nRlen=0;
	int i;//j;//,k;//,errornum=0;
	sendlen=10;
	again:
	ClrDisplay();
	DisplayMess(0, "****�������****", 16);	

	DisplayMess(32, "GPRS��ʼ��...   ", 16);//DEBUG
	
  Set_APN("CMNET");
	
	nRet = GPRS_Init();////////////////////////////////
	if(nRet < 0)
	{
		switch(nRet)
		{

			case POWERTIMEOUT:
				DisplayMess(32, "  ģ�鿪��ʧ��..  ", 16);//DEBUG
				break;
			case LOWCSQ:
				DisplayMess(32, "      �ź�����..      ", 16);//DEBUG
				break;
			case ECOFALI:
				DisplayMess(32, "  �ػ���ʧ��..  ", 16);//DEBUG
				break;
			case BDFALI:
				DisplayMess(32, "  ����������ʧ��..  ", 16);//DEBUG
				break;
			case FINDNETFALI:
				DisplayMess(32,"SIM ����ѯʧ��..",16);	
				break;
			default:
				DisplayMess(32, "  ģ�鿪��ʧ��..  ", 16);//DEBUG
				break;
		}
		BEEP(3,2,1);
		Delay10ms(250);
		return APP_FAIL;
	}
	DisplayMess(32, "  ģ�鿪���ɹ�..  ", 16);//DEBUG
	
	nRet = GPRS_Check();/////////////////////////////////////////
	if(nRet < 0)
	{
		switch(nRet)
		{
			case POWERTIMEOUT:
				DisplayMess(32, "  ģ�鿪��ʧ��..  ", 16);//DEBUG
				break;
			case LOWCSQ:
				DisplayMess(32, "      �ź�����..      ", 16);//DEBUG
				break;
			case ECOFALI:
				DisplayMess(32, "  �ػ���ʧ��..  ", 16);//DEBUG
				break;
			case BDFALI:
				DisplayMess(32, "  ����������ʧ��..  ", 16);//DEBUG
				break;
			case FINDNETFALI:
				DisplayMess(32,"SIM ����ѯʧ��..",16);	
				break;
			default:
				DisplayMess(32, "  ģ�鿪��ʧ��..  ", 16);//DEBUG
				break;
		}
		BEEP(3,2,1);
		Delay10ms(250);
		return APP_FAIL;
	}	
	
	nRet = Connnect_Socket("220.180.239.212","8045");////////

	if(nRet != APP_SUCC)
	{
		if(nRet == NEEDINIT)//����Ҫ��ر�ģ��
		{
			Network_Disable();
		}
		DisplayMess(32,"  ��������ʧ��..",16);
		DisplayMess(48,"  ���Ժ�����....",16);
		BEEP(3,2,1);
		Delay10ms(250);
		return APP_FAIL;
	} 
	DisplayMess(32,"  �������ӳɹ�..",16);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////	

	
	//M72D_SysInit();
//	again:
//	DisplayMess(32, "                ", 16);//DEBUG
//	DisplayMess(0, "                 ", 16);		
//	if(GPRS_Init()!=0)
//	{
//		DisplayMess(32, "init err        ", 16);//DEBUG
//		Delay10ms(300);
//	  goto again;
//	}
	//SetIP_PORT("220.180.239.212","8045");
	
	  resend:
	  sendlen=sendlen+1;	
	  if(sendlen>500)sendlen=10;
		TxData[0]=sendlen/256;TxData[1]=sendlen%256;		
	  for(i=0;i<510;i++)
	  {
		if(i>255) TxData[i+2]=i-255;
		else      TxData[i+2]=i;
	  }
	DisplayMess(32, "                ", 16);//DEBUG
	DisplayMess(0, "                 ", 16);	
	if(Connnect_Socket("220.180.239.212","8045")!=0)
		{
		DisplayMess(32, "conn err        ", 16);//DEBUG
		Delay10ms(300);
	  goto again;
	  }
	
	//GPRS_Send_Recieve(TxData,sendlen, recdata,2038,60);
   GPRS_Send(TxData,sendlen);
	
	 nRlen=GPRS_Recieve((char*)recdata,1024,60);
		if(nRlen< 0)
	 {
	
		if(nRlen == CLOSED)
		{
			BEEP(3,2,1);
			DisplayMess(32, "����CLOSED...  ", 16);
			Delay10ms(300);
		}

		if(err == APP_TIMEOUT)
		{
			
			 BEEP(3,2,1);
			 DisplayMess(32, "���ճ�ʱ...     ", 16);
			 Delay10ms(300);
			
		}
		else
		{
	      BEEP(3,2,1);
				DisplayMess(32, "    ����ʧ��    ", 16);
        Delay10ms(300);
			 
		}
    goto resend;
	}
  /////////////////////////////////////////////////////////////////////////////	
	 PubHexToAsc((char*)&show2[0], (char*)&recdata[0], 16, 0);	
	//show2[0]=0x30;show2[1]=0x30;
		DisplayMess(0, "                 ", 16);	
	  Delay10ms(30);
		DisplayMess(0, show2, 16);	
		Delay10ms(150);
  ////////////////////////////////////////////////////////////////////////////	
	if(CommHangupSocketlink()==0)
	{		
		DisplayMess(32, "unlink  ok    ", 16);//DEBUG
		Delay10ms(300);
		goto resend;
	}
	if(NetworkDevices_Disable()!=0)
	{
		DisplayMess(32, "guanji  err    ", 16);//DEBUG
		Delay10ms(300);
	  goto again;
	}
	goto again;

}

/////////////////////////////////////////////20161221////////////////////////////////////////////////////////////////////
int GPRS_rst(void)
{
	
	M72D_Enable();//��Դʹ��
	Delay10ms(30);///�ȴ�300ms��Դ�ȶ�	
//	//����ģ��
	GPIO_SetBits( M72D_PWRPort, M72D_PWRPin );
	Startdelay();//
	StartGSTimer(130);
	while(M72D_PwrTOut == 0);
	StopGSTimer();
	StartGSTimer(210);
	while(ReadStu() == RESET)
	{
	   /*��ʱ����-1*/	
	 	if(!CheckSTimer() ) 
		{
			StopGSTimer();
			return POWERTIMEOUT;	
		}
    /*���ȡ����-2*/
			if(SaleScalCom())
			{
				return -2;
			}		
	}
	Delay10ms(50);
	Uart1ClrRevBuff();Uart1ClrTxBuff();//
	UartReset(M72D_Com);

	GPIO_ResetBits( M72D_PWRPort, M72D_PWRPin );
	return 0;
}
int Network_Disable(void)
{
//	TransMode_FLAG = 0;//����ֻ����һ�δ���ģʽ���ػ��ٴο��������������
//	SIMCard_FLAG = 0;
	
	if(Shutdown()) 	
	return PWOFFFALI;
	
// 	M72D_Disable();	////�������жϵ�Դ��һֱ���磬��ֹ�´��ϵ���ɵ�Դ����	
	return 0;
}

int GPRS_Check(void)
{
	int ret;
	 ///����ע��	
  ret=NETfind_Inquire();
	if(ret) 
	{		
	//	DisplayMess(32, "  ����ע��ʧ��  ", 16);//DEBUG
	//	BEEP(3,2,1);
   	NetworkDevices_Disable();
 		//return -3;
		return FINDNETFALI;
	}
	///��ѯ��������
	ret=SignalIntensity_Inquire_Init();
	
	if(ret) 	
	{		
	//	DisplayMess(32, "��ѯ��������ʧ��", 16);//DEBUG
	//	BEEP(3,2,1);
		
   	NetworkDevices_Disable();

		return LOWCSQ;
	}
	return 0;
}

int GPRS_Init(void)
{
	int ret;
	COMCONFIG com_config;	
	M72D_RxFinsh = 0;
	//GPIO��ʼ��
	M72D_GPIOInit();
	//���ڳ�ʼ��
	ComConfig((COMCONFIG *)&com_config, 115200, 8, 1, 0);
	ComClose( M72D_Com );
	ComOpen( M72D_Com ,&com_config );
	Uart1ClrRevBuff();Uart1ClrTxBuff();//
	Uart1ClrTxBuff();///
	UartReset(M72D_Com);////////////////////////////////////////////////
	if(ReadStu() == SET)
	{
   /* ��ʼ��֮ǰ���ж��Ƿ�״̬����Ϊ�ߣ�Ϊ�߿��ܴ���û�йػ����������ʱ����PWK��ػ� */ 
  Network_Disable();
	M72D_Disable();
	Delay10ms(50);	
	}
   ret=GPRS_rst();
   if(ret)return ret;
//  ���������ʱ�����⿪��ϵͳδ������ɣ����²�ѯ����ʧ��
 	StartGSTimer(150);
 	while( M72D_PwrTOut == 0 );	
 	StopGSTimer();
//  ���������Զ��ϱ���ֱ���ӵ�
  Uart1ClrRevBuff();Uart1ClrTxBuff();//
	UartReset(M72D_Com);
	Adaptive_BD_Fail();	
	///�ػ���
	if(CloseEcho_Fail())
	{
	//	DisplayMess(32, "  �ػ���ʧ��..  ", 16);//DEBUG
	//	BEEP(3,2,1);
   	NetworkDevices_Disable();
 		return ECOFALI;
	}
  ///���ò�����
	if(SETBD())
	{
		//DisplayMess(32, "  ����������ʧ��..  ", 16);//DEBUG
		//BEEP(3,2,1);
   	NetworkDevices_Disable();
 		return BDFALI;
	}

	return 0;
}



int CommHangupSocketlink(void)
{
	/*������ģʽת��Ϊ����ģʽ*/
//	DisplayMess(16, "    ��ʼ�Ҷ�    ", 16);
	if( Transparen2Command_Fail() ){
//		DisplayMess(16, "    �л�ʧ��    ", 16);
		return PPPFAIL;
	}
//	DisplayMess(16, "    �л����    ", 16);
	if( Socket_Close() ){//�ر�����
//		DisplayMess(16, "    �Ҷ�ʧ��    ", 16);
		return IPOFFFALI;
	}
//	DisplayMess(16, "    �Ҷ����    ", 16);
	return 0;
}

int SetIP_PORT(char *IP, char *PORT)
{
//	char TCP_PORT[7];
//  char TCP_IP[17];
//  char YL_APN[33];
	memset(TCP_PORT, 0, sizeof(TCP_PORT));
	memset(TCP_IP, 0, sizeof(TCP_IP));
	memcpy(TCP_PORT,PORT,strlen(PORT));
	memcpy(TCP_IP,IP,strlen(IP));
	
	
	
	//memcpy(YL_APN,stPosParam.apn,32);
	return 0;
	
}
int Set_APN(char *APN)
{
//	char TCP_PORT[7];
//  char TCP_IP[17];
//  char YL_APN[33];
	memset(YL_APN, 0, sizeof(YL_APN));

	memcpy(YL_APN,APN,strlen(APN));
	
	//if( Comsend_back_Fail_APN(SetUnionPayAPN, BackOK) == 1 )
	//return -3;		
	
	return 0;
}

int GET_Status( char *CSQ, char *Count)
{
	unsigned char sig2 = 0;
 	unsigned char iRet;
// 	char szTempBuf[19] = {0};
	int ret;
//	int i = 0;

	
	if(ReadStu() == RESET) 
	return NOTPWON;///δ�����˳�
	

	iRet =  Comsend_back_Fail(PIN, PIN_OK);
	if(iRet == 1) //ģ��δ����//δ�忨
	{
	//	Delay10ms(150);//ģ��δ����,�ȴ�2S�ٴ�ѭ��

		return SIMFail;
	}
	
	ret=AT_Comsend(InquireCSQ,GPRSBuf);
		
      if(ret!=0)return ERR;
		
	if( GPRSBufLen ==  20)
	{
//		memset(Count, 0, sizeof(Count));
//		GPRSBuf[11]=GPRSBuf[11]=0x30;
//		memcpy(Count,&GPRSBuf[11],1);
		*Count = GPRSBuf[11]-0x30;;

		
		GPRSBuf[8] = (GPRSBuf[8]& 0x0f) << 4;
		sig2 = (GPRSBuf[8]) | (GPRSBuf[9] & 0x0f) ;
		sig3[1] = sig2;
		sig3[2] = ((sig2>>4)&0x0f)*10;
		sig3[3] = sig2&0x0f;
		sig3[4] = sig3[2] + sig3[3];
		sig3[5] = ((sig2>>4)&0x0f)*10+ sig2&0x0f;
		
//		memset(CSQ, 0, sizeof(CSQ));
		*CSQ = sig3[4];
//		memcpy(CSQ,&,1);

		return 0;
	
	}
	else if(GPRSBufLen == 19)
	{
//		memset(Count, 0, sizeof(Count));
//		GPRSBuf[10]=GPRSBuf[10]=0x30;
//		memcpy(Count,&GPRSBuf[10],1);
		*Count = GPRSBuf[10]-0x30;;
		
	  	sig3[4]  =  GPRSBuf[8]&0x0f;	
		
//		memset(CSQ, 0, sizeof(CSQ));
//		memcpy(CSQ,&sig3[4],1);
		*CSQ = sig3[4];
		 return 0;
	}


	return ERR;
}

int Connnect_Socket(char *TCP_IP, char *TCP_PORT)
{
	int ret = 1;
	ret = GPRS_Check();
	if(ret != 0)
	{
		return NEEDINIT;
	}
	if( Trans_Config() == ModeFail )  //TCP��������
		
	return NEEDINIT;

	ret = Socket_Link(TCP_IP, TCP_PORT);//��������
	if(ret != APP_SUCC)
	{
		return NEEDINIT;
	}
	return 0;
}

int GPRS_Send(char *TxData,uint16_t sendlen)
{
		if(sendlen){
		Uart1ClrRevBuff();Uart1ClrTxBuff();//UartReset(M72D_Com);
	  //ComSend(WIFI_PORT, (uint8_t*)TxData, sendlen);//͸����������
		Uart1WriteTxBuff((uint8_t *)(uint8_t*)TxData, sendlen);	//͸����������
		}
		return 0;	
}

int GPRS_Recieve(char *Rxdata,uint16_t Maxlen,uint16_t timeout)
{
		uint16_t len = 0;
//	  char button = 0;
//	  int8_t err = 0;
//	  uint16_t  relen;
//	  uint8_t recdata[1024] = {0, 0};
		char *recdata;
//		uint16_t rxlen = 0;
	
		SetGPRSrevTimer(timeout*100);
		recdata=Rxdata;
//		SetGPRSrevTimer1(5);
//		while(1)
//		{
//			Delay10ms(3);
//			ComGetRxCount(WIFI_PORT, &len);//��ȡ���յ������ݳ���	
//			if(len>0)break;
//			
//			if(CheckGPRSrevTimer()){
//				return -1;
//			}
//			if(SaleScalCom())
//			{
//				StopGPRSrevTimer();
//				return -2;
//			}	
//			
//		}
//		SetGPRSrevTimer1(10);
//		while(1)
//		{
//			if(CheckGPRSrevTimer1()){
//				
//				ComGetRxCount(WIFI_PORT, &len);//��ȡ���յ������ݳ���	
//				if(len>0){
//					if((rxlen+len)>Maxlen){ len=Maxlen-rxlen;ComRecv(WIFI_PORT, recdata+rxlen, &len);rxlen+=len;break;}
//						ComRecv(WIFI_PORT, recdata+rxlen, &len);//��ȡ���յ�������
//						rxlen+=len;
//				}
//				else	break;
//			}
//			if(SaleScalCom())
//			{
//				StopGPRSrevTimer1();
//				StopGPRSrevTimer();
//				return -2;
//			}		
//			if(CheckGPRSrevTimer()){
//			StopGPRSrevTimer1();
//			return -1;
//			}
//		}
//		StopGPRSrevTimer1();
		
		
		
		while(!M72D_RxFinsh){
		//���ȴ���ʱ��ʱ���Ƿ�ʱ
		if(CheckGPRSrevTimer()){

////	/*		if(cFlagForOffLineSaleDisPlay == 0)
////			{
////				DisplayMess(48, "���ճ�ʱ1..     ", 16);
////				BEEP(2,2,1);
////			}*/
			return -1;
		}
		//���ȡ����
		if(SaleScalCom())
		{
			StopGPRSrevTimer();
			return -2;
		}		
	}
		StopGPRSrevTimer();
//	Delay10ms(50);	/////////////////////////////////////////
	M72D_RxFinsh = 0;
	
	//ComGetRxCount(WIFI_PORT, &len);//��ȡ���յ������ݳ���	
	
	len=Uart1ReadRevBuff((unsigned char*)recdata,Maxlen);
	
//	if(len>Maxlen) len=Maxlen;// return ERR;
	
	
	//err = ComRecv(WIFI_PORT, recdata, &len);//��ȡ���յ�������
	
	
//	if(err != API_SUCCESS)
//		
//	{
///*		if(cFlagForOffLineSaleDisPlay == 0)
//		{
//			DisplayMess(48, "    ����ʧ��1   ", 16);
//			BEEP(2,2,1);
//			Delay10ms(100);
//		}*/
//    return -3;
//	}
	
	if(memcmp(recdata, "CLOSED", 6) == 0 || memcmp(recdata+2, "CLOSED", 6) == 0 )
	{
//		  StopGPRSrevTimer();
//			DisplayMess(48, "    ����ʧ��2   ", 16);
//			BEEP(3,2,1);
//			Delay10ms(300);		
    		  return CLOSED;
	}

	if(memcmp(recdata, "+PDP", 4) == 0 || memcmp(recdata+2, "+PDP", 4) == 0 )
	{
//		  StopGPRSrevTimer();
//			DisplayMess(48, "    ����ʧ��3   ", 16);
//			BEEP(3,2,1);
//			Delay10ms(300);		
     		 return NEEDINIT;
	}

//	memcpy(Rxdata,recdata,len);
	
	return len;
}

//int GPRS_Send_Recieve(char *TxData,uint16_t sendlen, char *Rxdata,uint16_t Maxlen,uint16_t timeout)
//{
//	  uint16_t len = 0;
//	
////	  char button = 0;
//	  int8_t err = 0;
//	
//	  uint8_t recdata[1024] = {0, 0};
//	
//	  if(sendlen)
//	  ComSend(WIFI_PORT, (uint8_t*)TxData, sendlen);//͸����������

//	  SetRFTimer(timeout*100);
//		while(!M72D_RxFinsh){
//		//���ȴ���ʱ��ʱ���Ƿ�ʱ
//		if(!CheckRFTimer()){
//			
//	/*		if(cFlagForOffLineSaleDisPlay == 0)
//			{
//				DisplayMess(48, "���ճ�ʱ1..     ", 16);
//				BEEP(2,2,1);
//			}*/
//			return -1;
//		}
//		//���ȡ����
//		if(SaleScalCom())
//		{
//			return -2;
//		}		
//	}
//	Delay10ms(5);//Delay10ms(50);	/////////////////////////////////////////
//	M72D_RxFinsh = 0;
//	ComGetRxCount(WIFI_PORT, &len);//��ȡ���յ������ݳ���	
//	
//	if(len>Maxlen)  return ERR;
//	
//	memset(recdata,0,1024);//������ڽ��ջ����� 
//	
//	err = ComRecv(WIFI_PORT, recdata, &len);//��ȡ���յ�������
//	if(err != API_SUCCESS)
//	{
///*		if(cFlagForOffLineSaleDisPlay == 0)
//		{
//			DisplayMess(48, "    ����ʧ��1   ", 16);
//			BEEP(2,2,1);
//			Delay10ms(100);
//		}*/
//    return -3;
//	}
//	
//		if(memcmp(recdata, "CLOSED", 6) == 0 || memcmp(recdata+2, "CLOSED", 6) == 0 )
//	{
//	//	  StopRFTimer();
//	//		DisplayMess(48, "    ����ʧ��2   ", 16);
//	//		BEEP(3,2,1);
//	//		Delay10ms(300);		
//      return CLOSED;
//	}
//	
//	memcpy(Rxdata,recdata,len);
//	
//	return len;

//}

int AT_Comsend(char *command,char *rcv)
{
	memset(rcv, 0, sizeof(rcv)); 
  //memset(GPRSBuf, 0, sizeof(GPRSBuf));	
	Uart1ClrRevBuff();Uart1ClrTxBuff();
	UartReset(M72D_Com);
//	ComSend(M72D_Com, (uint8_t *)command, strlen(command));
	Uart1WriteTxBuff((uint8_t *)command, strlen(command));
	SetTimer1(50);

	while( M72D_RxFinsh == 0 )
	{
		if( !CheckTimer1() )
		return  APP_TIMEOUT;
	}
	if( M72D_RxFinsh )
	{
		StopTimer1();
		M72D_RxFinsh = 0;
//		ComGetRxCount(M72D_Com, &GPRSBufLen);
//		if(GPRSBufLen>MAX_GPRSBUF_LEN) GPRSBufLen=MAX_GPRSBUF_LEN-1;
//		ComRecv(M72D_Com, (uint8_t *)rcv, &GPRSBufLen);
		GPRSBufLen=Uart1ReadRevBuff((uint8_t *)GPRSBuf,MAX_GPRSBUF_LEN);
		return 0;
	}
  return 0;
}
//
 int SETBD(void)
{
	if(Comsend_back_Fail(BaudRate,BackOK) != 0 )
		return 1;
	if(Comsend_back_Fail(SaveConfig,BackOK) != 0)
		return 1;
	
	return 0;
}


int NETfind_Inquire(void)
{
	  int ret = 1;
	  uint8_t i = 0;

		ret = Comsend_back_Fail(PIN, PIN_OK);  //SIM��PIN���ѽ�
		if(ret == 1)
		{
		  i=0;
			while( ret )
			{
				Delay10ms(100);
				i++;
				ret = Comsend_back_Fail(PIN, PIN_OK); 
				
				if( i==10 ) break;
			}
			if( i==10 ) return SIMFail;
    }			
			
		ret = Comsend_back_Fail(FindNet, FindNet_OK);
		if( ret == 1 ) 	//ȷ�������ɹ�
		{
			i=0;
			while( ret )
			{
				Delay10ms(100);
				i++;
				ret = Comsend_back_Fail(FindNet, FindNet_OK); 
				if(ret == 1) ret = Comsend_back_Fail(FindNet, FindNet_OK5); 
				if( i==30 ) break;
			}
			if( i==30 ) return NetFail;
		}
		ret = 	Comsend_back_Fail(GPRSAttach, GPRSAttach_OK);
		if( ret == 1 ) 	//��ѯGPRS���ųɹ�
		{
			i=0;
			while( ret )
			{
				Delay10ms(100);
				i++;
				ret = Comsend_back_Fail(GPRSAttach, GPRSAttach_OK); 
				
					if( i==30 ) break;
			}
			if( i==30 ) return NetFail;
		}

		
	return 0;
}

