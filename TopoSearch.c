
#include "Component.h"
#include "TopoSearch.h"
//#include "sub.h"
#include <stdlib.h>
/* Local function declaration */
int initTopoSearch(TopoSearch *Owner);
void TopoSearchModule(TopoSearch *Owner);
void cal_node_conn(NodeType* pnt);



INIT_CODE
TopoSearch *newTopoSearch(Component *Parent,const char *Name){
    TopoSearch *Owner;
    int16 i;
    char str_temp[128];

    Owner=(TopoSearch *)calloc(1,sizeof(TopoSearch));
    if (!Owner) return NULL;

    Owner->type_name="TopoSearch";
    Owner->parent=Parent;
    Owner->name=Name;
    Owner->InitComponent=initTopoSearch;

    defineComponent((Component *)Owner);

    //input
    for (i = 0; i < MAX_CONN; ++i){
		sprintf(str_temp,"inRun%d type=b level=2 ",i+1);
		defineSignalIn((Component *)Owner,(void **)(&(Owner->inRun[i])),str_temp);
 		sprintf(str_temp,"inFlt%d type=b level=2 ",i+1);
		defineSignalIn((Component *)Owner,(void **)(&(Owner->inFlt[i])),str_temp);

 		sprintf(str_temp,"From_Conn%d  type=i min=0 max=16 default=0 option=1",i+1);
		defineParameter((Component *)Owner,(void *)&(Owner->From_conn[i]),str_temp);
        sprintf(str_temp,"End_Conn%d  type=i min=0 max=16 default=0 option=1",i+1);
		defineParameter((Component *)Owner,(void *)&(Owner->End_Conn[i]), str_temp);
    }
    sprintf(str_temp,"NodeNum  type=i min=0 max=16 default=0 option=1");
    defineParameter((Component *)Owner,(void *)&(Owner->Node_num), str_temp);
    sprintf(str_temp,"ConnNum  type=i min=0 max=16 default=0 option=1");
    defineParameter((Component *)Owner,(void *)&(Owner->Conn_num), str_temp);

    for (i = 0; i < MAX_NODE; ++i){
  		sprintf(str_temp,"type%d  type=b min=0 max=2 default=0 option=1",i+1);
		defineParameter((Component *)Owner,(void *)&(Owner->type[i]),str_temp);

 		sprintf(str_temp,"Stat%d type=b level=2 ",i+1);
		defineSignalOut((Component *)Owner,(void *)&(Owner->Stat[i]),str_temp);
 		sprintf(str_temp,"Act%d type=b level=2 ",i+1);
		defineSignalOut((Component *)Owner,(void *)&(Owner->Act[i]),str_temp);
    }

    defineSignalOut((Component *)Owner,(void *)&(Owner->Act_total),	"Act_total  type=b level=2");
    defineSignalOut((Component *)Owner,(void *)&(Owner->flg_seterr),"flg_seterr type=b level=2");


    return Owner;
}

INIT_CODE
int initTopoSearch(TopoSearch *Owner)
{

 	int16 i,j;
    int16 ret = 0;
    ret = addDataToMonitor (&Owner->monitor_start,  &Owner->monitor_end);
    if  (ret != OK) {
        return ret;
    }
    //for (i = 0; i < MAX_CONN; ++i){
    	//*Owner->inRun[i] = 0;
    	//*Owner->inFlt[i] = 0;
    	//Owner->Conn[i] = 0;
    //}
    for (i = 0; i < MAX_NODE; ++i){
    	Owner->Stat[i] = 0;
    	Owner->Act[i] = 0;
        Owner->Stat_[i] = 0;
        Owner->stack[i] = 0;
        Owner->visited_Node[i] = 0;
    }
    for(i = 0;i < MAX_CONN;i++){
        Owner.visited_Conn[i] = 0;
    }
    Owner->Act_total = 0;
    Owner->flg_seterr = 0;

  addTask(2,TopoSearchModule,Owner);
  return 0;
}


/********************************************************************************
*         Input    :                                                *
*         Output   :                                          *
*         Function :                                       *
********************************************************************************/
RTM_CODE1
void  TopoSearchModule(TopoSearch *Owner)
{
    Uint16 temp16;
    int16 i, j, num_KG_checked;
    Uint8 index;

    //初始化清空计算量
    for (i = 0; i < MAX_NODE; ++i){
        Owner->stack[i] = 0;
        Owner->visited_Node[i] = 0;
    }
    for(i = 0;i < MAX_CONN;i++){
        Owner.visited_Conn[i] = 0;
        Owner->isConnected[i] = 0x0;
    }
    //获取开关状态
    for(i = 0;i < MAX_CONN;i++){
        if((*Owner->inRun[i] != 0x0) && (*Owner->inFlt[i] == 0x0)){
            Owner->isConnected[i] = 0x1;
        }
    }
    Owner->pHead = 0;
    Owner->pEnd = 0;
    num_KG_checked = 0;//开关遍历的个数
    for(i = 0;i < MAX_NODE;i++){
        if(Owner->type[i] != 0x2) continue;//非主网并网点，pass
        //检查是否已经遍历过
        if(Owner->visited_Node[i] != 0) continue;//很遗憾，这个节点已经遍历过了
        //hahaha,终于找到一个很单纯的节点
        Owner->stack[pEnd] = i;
        pEnd = (pEnd + 1) % MAX_NODE;//索引转移到下一个节点
        //开始广度遍历所有联系节点
        while(pHead != pEnd){
            index = Owner->stack[pHead];
            Owner->visited_Node[index] = 0x1;//已经不单纯了
            pHead = (pHead + 1) % MAX_NODE;//准星瞄准下一个目标
            //根据该点寻找其他相连节点
            for(j = 0;j < MAX_CONN;j++){
                if(isConnected[j] != 0) continue;//不好意思，这个开关已经被占领
                
            }



            for(j = 0;j < MAX_NODE;j++){
                if((temp16 & (0x1 << j)) == 0x0) continue;//该节点都没联系
                if(Owner->TPNode[j].visited != 0x0) continue;//很遗憾，这个老乡已经被打劫过了
                //hahah 找到了个单纯的，赶紧收入囊中
                Owner->stack[pEnd] = j;
                pEnd = (pEnd + 1) % MAX_NODE;
            }
        }
    }
    //跟主网相连的节点都遍历完了,开始看看有哪些孤家寡人
    Owner->Act = 0x0;
    for(i = 0;i < MAX_NODE;i++){
        Owner->Stat_[i] = Owner->Stat[i];//先看看上次一有没有遍历到
        Owner->Stat[i] = Owner->TPNode[i].visited;//被皇军拜访过得，肯定是要交皇粮的。
        //如果是突然变化的，肯定是个受害者
        if((Owner->Stat_[i] == 0x1) && (Owner->Stat[i] == 0x0)){
            Owner->Act[i] = 0x1;//变化了就置跳开标志，但只有一个计算点哎，过两天俺再想想
        } else {
            Owner->Act[i] = 0x0;
        }
        Owner->Act |= Owner->Act[i];//置总的标志啊
    }

    


       




}
