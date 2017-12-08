
#ifndef _TOPOSEARCH_H
#define _TOPOSEARCH_H

//#include "uapcSlave.h"
#include "Const.h"
#include "Global.h"

#define  MAX_NODE  16
#define  MAX_CONN  16

typedef struct {
    #include "BaseComponent.h"

    Uint8 monitor_start;						//version,memory check head

    //input
    Uint8 *inRun[MAX_CONN];
    Uint8 *inFlt[MAX_CONN];

    //parameter
    Uint8    type[MAX_NODE];//0:normal node 1:Conn_node of GF  2:Conn_node of Grid
    Uint8    From_Conn[MAX_CONN];
    Uint8    End_Conn[MAX_CONN];
    int16    Node_num;//节点个数
    int16    Conn_num;//开关个数

    Uint8 monitor_end;//memory check end
    //////////////////////////
    Uint8  Stat[MAX_NODE];//the connection with Grid
    Uint8  Act[MAX_NODE]; //the disconnecting sign with Grid

    Uint8  Stat_[MAX_NODE];//bak of Stat
    Uint8  Act_total;

    Uint8 visited_Conn[MAX_CONN];
    Uint8 visited_Node[MAX_NODE];
    Uint8 isConnected[MAX_CONN];

    Uint8 stack[MAX_NODE];
    Uint8 pHead;//stack head point
    Uint8 pEnd; //stack end  point

	Uint8  flg_seterr;   //err in setting 

}TopoSearch;

extern TopoSearch *newTopoSearch(Component *Parent,const char *Name);
extern int initTopoSearch(TopoSearch *Owner);
extern void  TopoSearchModule(TopoSearch *Owner);


#endif
