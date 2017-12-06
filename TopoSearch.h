
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
    Uint8  NodeType[MAX_NODE];//0:��ͨ�ڵ� 1:��������� 2:����������

    Uint8 monitor_end;//memory check end
    
    //////////////////////////  
    Uint8  Stat[MAX_NODE];//�ڵ���������������
    Uint8  Act[MAX_NODE]; //������������

    Uint8  Stat_[MAX_NODE];//�ڵ��������������� ������
    Uint8  Act_total;
    
	Uint8  flg_seterr;   //����������

}TopoSearch;

extern TopoSearch *newTopoSearch(Component *Parent,const char *Name);
extern int initTopoSearch(TopoSearch *Owner);
extern void  TopoSearchModule(TopoSearch *Owner);


#endif

