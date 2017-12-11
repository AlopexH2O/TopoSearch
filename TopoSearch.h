
#ifndef _TOPOSEARCH_H
#define _TOPOSEARCH_H

//#include "uapcSlave.h"
#include "Const.h"
#include "Global.h"

#define  MAX_NODE  16
#define  MAX_CONN  16

typedef struct{
    int16 Ia;
    int16 Ib;
    int16 Ic;
    int16 P;
}POWERELE;

typedef struct {
    #include "BaseComponent.h"

    Uint8 monitor_start;						//version,memory check head

    //input
	Uint8 *Enable;
    Uint8 *inFlt[MAX_CONN];
	Uint8 *inCHK[MAX_CONN];//线路开关间隔检修
    Uint8 *inBLK[MAX_CONN];//线路开关间隔功能闭锁
    int16 *inIa[MAX_CONN];
    int16 *inIb[MAX_CONN];
    int16 *inIc[MAX_CONN];
    int16 *inP[MAX_CONN];
    Uint8 *inPos[MAX_CONN];//开关状态标志
    //Uint8 *FD;//外部启动标志

    //parameter
    Uint8 SET_KG_FD;//是否开关变位启动
    Uint8 SET_CONN_FD;//是否光伏并网点联网状态变化启动
    Uint8 Node_num;//节点数目
    Uint8 Conn_num;//开关数目
    Uint8 Type[MAX_NODE];//0:normal node 1:Conn_node of GF  2:Conn_node of Grid
    Uint8 FromNode[MAX_CONN];
    Uint8 EndNode[MAX_CONN];
    int16 Iset[MAX_CONN];//电流判投运定值
    int16 Pset[MAX_CONN];//功率判投运定值
    int16 Tset[MAX_CONN];//电气判投停延时定值

    Uint8 monitor_end;//memory check end
    
    
    //////////////////////////
    Uint8 Stat[MAX_NODE];//the disconnection with Grid
    Uint8 Act[MAX_NODE]; //the disconnecting sign with Grid
    Uint8 Stat_[MAX_NODE];//bak of Stat
    Uint8 Act_total;

    //内部计算变量
    Uint8 visited_Conn[MAX_CONN];
    Uint8 visited_Node[MAX_NODE];
    Uint8 Connected[MAX_CONN];
    Uint8 Connected_[MAX_CONN];

    Uint8 stack[MAX_NODE];
    Uint8 Flag_KG_Flt_All;
    Uint8 Flag_KG_CHG_All;
    Uint8 Flag_KG_Flt[MAX_CONN];
    Uint8 Flag_KG_CHG[MAX_CONN];//Flag of Conn change from 1 to 0
    int16 T_Flag_KG_CHG[MAX_CONN];
    int16 T_Flag_KG_Flt[MAX_CONN];
    Uint8 Flag_KG_FD;
    int16 T_Flag_KG_FD;
    Uint8 Flag_Stat_CHG[MAX_NODE];
    int16 T_Flag_Stat_CHG[MAX_NODE];
    Uint8 Flag_Stat_FD;
    int16 T_Flag_Stat_FD;
    Uint8 Flag_Conn_CHG;
    int16 T_inBLK[MAX_CONN];
    Uint8 Flag_QD_HOLD;
    //int16 T_Flag_Conn_CHG;

	Uint8 flg_seterr;   //err in setting 

}TopoSearch;

extern TopoSearch *newTopoSearch(Component *Parent,const char *Name);
extern int initTopoSearch(TopoSearch *Owner);
extern void  TopoSearchModule(TopoSearch *Owner);


#endif
