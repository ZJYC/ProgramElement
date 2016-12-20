
#if 1

/* 定义滴答的最大值，必须为循环计数 */
#define TickFull    (1000000)
/* 获取两个滴答之间的间隔 */
#define GetInterval(Cur,Last)   ((Cur) > Last() ? ((Cur) - (Last)) : (TickFull - (Last) + (Cur)))

#endif


#if 1



#endif











