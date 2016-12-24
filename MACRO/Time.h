
#if 1

/* ����δ�����ֵ������Ϊѭ������ */
#define TickFull    (1000000)
/* ��ȡ�����δ�֮��ļ�� */
#define GetInterval(Cur,Last)   ((Cur) > Last() ? ((Cur) - (Last)) : (TickFull - (Last) + (Cur)))
/* ���� */
#define TickAdd(val,add)            {\
                                    (val) += (add);\
                                    if((val) > TickFull)(val) = 0;\
                                    }
}
#endif


#if 1
/* ���ڼ����� */
#define BT_CHK_PARAM(x)   if((uint32_t)x == 0)while(1);
/* ���ڼ��ĳһλ */
#define BT_CHK_BIT(val,bit) ((val) & (1 << (bit)))
/* ��������ĳһλ */
#define BT_SET_BIT(val,bit) {(val) |= (1 << (bit));}
/* �������ĳһλ */
#define BT_CLR_BIT(val,bit) {(val) &= ~(1 << (bit));}
/* ����ֵ */
#define BT_ABS(x)           ((x) > 0 ? (x):(-(x)))


#endif











