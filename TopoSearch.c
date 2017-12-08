
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

    //��ʼ����ռ�����
    for (i = 0; i < MAX_NODE; ++i){
        Owner->stack[i] = 0;
        Owner->visited_Node[i] = 0;
    }
    for(i = 0;i < MAX_CONN;i++){
        Owner.visited_Conn[i] = 0;
        Owner->isConnected[i] = 0x0;
    }
    //��ȡ����״̬
    for(i = 0;i < MAX_CONN;i++){
        if((*Owner->inRun[i] != 0x0) && (*Owner->inFlt[i] == 0x0)){
            Owner->isConnected[i] = 0x1;
        }
    }
    Owner->pHead = 0;
    Owner->pEnd = 0;
    num_KG_checked = 0;//���ر����ĸ���
    for(i = 0;i < MAX_NODE;i++){
        if(Owner->type[i] != 0x2) continue;//�����������㣬pass
        //����Ƿ��Ѿ�������
        if(Owner->visited_Node[i] != 0) continue;//���ź�������ڵ��Ѿ���������
        //hahaha,�����ҵ�һ���ܵ����Ľڵ�
        Owner->stack[pEnd] = i;
        pEnd = (pEnd + 1) % MAX_NODE;//����ת�Ƶ���һ���ڵ�
        //��ʼ��ȱ���������ϵ�ڵ�
        while(pHead != pEnd){
            index = Owner->stack[pHead];
            Owner->visited_Node[index] = 0x1;//�Ѿ���������
            pHead = (pHead + 1) % MAX_NODE;//׼����׼��һ��Ŀ��
            //���ݸõ�Ѱ�����������ڵ�
            for(j = 0;j < MAX_CONN;j++){
                if(isConnected[j] != 0) continue;//������˼����������Ѿ���ռ��
                
            }



            for(j = 0;j < MAX_NODE;j++){
                if((temp16 & (0x1 << j)) == 0x0) continue;//�ýڵ㶼û��ϵ
                if(Owner->TPNode[j].visited != 0x0) continue;//���ź�����������Ѿ�����ٹ���
                //hahah �ҵ��˸������ģ��Ͻ���������
                Owner->stack[pEnd] = j;
                pEnd = (pEnd + 1) % MAX_NODE;
            }
        }
    }
    //�����������Ľڵ㶼��������,��ʼ��������Щ�¼ҹ���
    Owner->Act = 0x0;
    for(i = 0;i < MAX_NODE;i++){
        Owner->Stat_[i] = Owner->Stat[i];//�ȿ����ϴ�һ��û�б�����
        Owner->Stat[i] = Owner->TPNode[i].visited;//���ʾ��ݷù��ã��϶���Ҫ�������ġ�
        //�����ͻȻ�仯�ģ��϶��Ǹ��ܺ���
        if((Owner->Stat_[i] == 0x1) && (Owner->Stat[i] == 0x0)){
            Owner->Act[i] = 0x1;//�仯�˾���������־����ֻ��һ������㰥�������찳������
        } else {
            Owner->Act[i] = 0x0;
        }
        Owner->Act |= Owner->Act[i];//���ܵı�־��
    }

    


       




}
