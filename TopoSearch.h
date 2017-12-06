
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
    Uint16 Conn[MAX_CONN];
    Uint8  NodeType[MAX_NODE];//0:普通节点 1:光伏并网点 2:主网并网点

    Uint8 monitor_end;//memory check end
    
    //////////////////////////  
    Uint8  Stat[MAX_NODE];//节点跟主网的连接情况
    Uint8  Act[MAX_NODE]; //光伏线跳入孤网

    Uint8  Stat_[MAX_NODE];//节点跟主网的连接情况 备份用
    Uint8  Act_total;
    
	Uint8  flg_seterr;   //配置有问题

}TopoSearch;

extern TopoSearch *newTopoSearch(Component *Parent,const char *Name);
extern int initTopoSearch(TopoSearch *Owner);
extern void  TopoSearchModule(TopoSearch *Owner);


#endif

