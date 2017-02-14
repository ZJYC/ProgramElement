



typedef enum ResEnum_
{
    Res_False = 0,
    
    Res_Other = 1,
    
    Res_True = 0xFFFF
}ResEnum;

#define GetInf(Res)\
        ((Res) == Res_False?"Success": \
        ((Res) == Res_Other?"Failure": \
        ((Res) == Res_True?"NullPointer": \
        "Unknown")))























