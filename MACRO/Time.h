
#if 1

/* 定义滴答的最大值，必须为循环计数 */
#define TickFull    (1000000)
/* 获取两个滴答之间的间隔 */
#define GetInterval(Cur,Last)   ((Cur) > Last() ? ((Cur) - (Last)) : (TickFull - (Last) + (Cur)))
/* 递增 */
#define TickAdd(val,add)            {\
                                    (val) += (add);\
                                    if((val) > TickFull)(val) = 0;\
                                    }
}
#endif


#if 1
/* 用于检查参数 */
#define BT_CHK_PARAM(x)   if((uint32_t)x == 0)while(1);
/* 用于检查某一位 */
#define BT_CHK_BIT(val,bit) ((val) & (1 << (bit)))
/* 用于设置某一位 */
#define BT_SET_BIT(val,bit) {(val) |= (1 << (bit));}
/* 用于清除某一位 */
#define BT_CLR_BIT(val,bit) {(val) &= ~(1 << (bit));}
/* 绝对值 */
#define BT_ABS(x)           ((x) > 0 ? (x):(-(x)))
/* 字符串比较--防止编译器警告 */
#define G63_STR_EQL(x,y)         (strcmp((const char *)(x),(const char *)(y)) == (0))
/* 字符串复制--防止编译器警告 */
#define G63_STR_CPY(x,y)         strcpy((char *)(x),(char *)(y))

#endif











