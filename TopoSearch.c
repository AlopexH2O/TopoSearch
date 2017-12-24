/*
    ģ�飺TopoSearch
    ���ܣ�
    ������1��һֱ���нڵ����״̬�жϣ�������pos��־
    ���ϣ�1���жϿ�����������·��������������״̬����act��־ act = �ڵ㵱ǰ���� & ֮ǰ���� & ������·�򿪹����� & װ������


        



*/



#include "Component.h"
#include "TopoSearch.h"
#include "sub.h"
#include <stdlib.h>
/* Local function declaration */
int initTopoSearch(TopoSearch *Owner);
void ClearCalInfo(TopoSearch* Owner);
void TopoSearchModule(TopoSearch *Owner);

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
	defineSignalIn((Component *)Owner,(void **)(&(Owner->Enable)),"Enable type=b level=2");
    for (i = 0; i < MAX_CONN; ++i){
		sprintf(str_temp,"inPos%d type=b level=2 ",i+1);
		defineSignalIn((Component *)Owner,(void **)(&(Owner->inPos[i])),str_temp);
 		sprintf(str_temp,"inFlt%d type=b level=2 ",i+1);
        defineSignalIn((Component *)Owner,(void **)(&(Owner->inFlt[i])),str_temp);
        
        sprintf(str_temp,"inCHK%d type=b level=2 ",i+1);
        defineSignalIn((Component *)Owner,(void **)(&(Owner->inCHK[i])),str_temp);
        sprintf(str_temp,"inBLK%d type=b level=2 ",i+1);
        defineSignalIn((Component *)Owner,(void **)(&(Owner->inBLK[i])),str_temp);

        sprintf(str_temp,"inIa%d type=i level=2 ",i+1);
        defineSignalIn((Component *)Owner,(void **)(&(Owner->inIa[i])),str_temp);
        sprintf(str_temp,"inIb%d type=i level=2 ",i+1);
        defineSignalIn((Component *)Owner,(void **)(&(Owner->inIb[i])),str_temp);
        sprintf(str_temp,"inIc%d type=i level=2 ",i+1);
        defineSignalIn((Component *)Owner,(void **)(&(Owner->inIc[i])),str_temp);
        sprintf(str_temp,"inP%d type=i level=2 ",i+1);
        defineSignalIn((Component *)Owner,(void **)(&(Owner->inP[i])),str_temp);

        sprintf(str_temp,"FromNode%d  type=cu min=0 max=16 default=0 option=1",i+1);
		defineParameter((Component *)Owner,(void *)&(Owner->FromNode[i]),str_temp);
        sprintf(str_temp,"EndNode%d  type=cu min=0 max=16 default=0 option=1",i+1);
        defineParameter((Component *)Owner,(void *)&(Owner->EndNode[i]), str_temp);

        sprintf(str_temp,"Iset%d  type=i min=0 max=30000 default=0 unit=A option=1",i+1);
        defineParameter((Component *)Owner,(void *)&(Owner->Iset[i]), str_temp);
        sprintf(str_temp,"Pset%d  type=i min=0 max=30000 default=0 unit=MW option=1",i+1);
        defineParameter((Component *)Owner,(void *)&(Owner->Pset[i]), str_temp);
        sprintf(str_temp,"Tset_Flt%d  type=i min=0 max=30000 default=0 unit=ms option=1",i+1);
        defineParameter((Component *)Owner,(void *)&(Owner->Tset_Flt[i]), str_temp);
    }

    sprintf(str_temp,"Node_num  type=cu min=0 max=16 default=0 option=1");
    defineParameter((Component *)Owner,(void *)&(Owner->Node_num), str_temp);
    sprintf(str_temp,"Conn_num  type=cu min=0 max=16 default=0 option=1");
    defineParameter((Component *)Owner,(void *)&(Owner->Conn_num), str_temp);
    sprintf(str_temp,"SET_POS_FD  type=b min=0 max=1 default=0 option=1");
    defineParameter((Component *)Owner,(void *)&(Owner->SET_POS_FD), str_temp);
    sprintf(str_temp,"SET_CONN_FD  type=b min=0 max=1 default=0 option=1");
    defineParameter((Component *)Owner,(void *)&(Owner->SET_CONN_FD), str_temp);

    for (i = 0; i < MAX_NODE; ++i){
  		sprintf(str_temp,"Type%d  type=cu min=0 max=2 default=0 option=1",i+1);
		defineParameter((Component *)Owner,(void *)&(Owner->Type[i]),str_temp);

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
    ClearCalInfo(Owner);
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
    int16 i, j, cnt_tmp;
    Uint8 index, index_other, ifrom, iend, chg;
    Uint8 flg, pHead, pEnd, flg_tmp, flg_tmp1, flg_tmp2;


    if(FLAG_QD.bit.Fqd_Run==0)  return;  

	//����װ�������Ƿ����
    Owner->flg_seterr = 0x0;
    if((Owner->Node_num > MAX_NODE)||(Owner->Conn_num > MAX_CONN))
        Owner->flg_seterr |= 0x1;
    for(i = 0;i < Owner->Conn_num;i++){
        if((Owner->FromNode[i] > Owner->Node_num)||(Owner->FromNode[i] <= 0))
            Owner->flg_seterr |= 0x1;
        if((Owner->EndNode[i] > Owner->Node_num)||(Owner->EndNode[i] <= 0))
            Owner->flg_seterr |= 0x1;
    }
    if(Owner->flg_seterr != 0){
        ClearCalInfo(Owner);
        device_status.bit.alm_own |= Owner->flg_seterr;
        return;        
    }
    //�ڵ�����򿪹ظ���Ϊ�㣬�򲻼�
    if((Owner->Node_num == 0) || (Owner->Conn_num == 0)){
        return;
    }

    //��ʼ����ռ���ռ�
    for (i = 0; i < MAX_NODE; ++i){
        Owner->stack[i] = 0;
        Owner->visited_Node[i] = 0;
        Owner->added_Node[i] = 0;
    }
    for(i = 0;i < MAX_CONN;i++){
        Owner->visited_Conn[i] = 0;
        Owner->Connected[i] = 0x0;
    }

    //��⿪��״̬�仯
    Owner->Flag_KG_CHG_All = 0x0;
    for(i = 0;i < Owner->Conn_num;i++){
        Owner->Connected_[i] = Owner->Connected[i];//����ǰһ�㿪������״̬
        Owner->Connected[i] = *Owner->inPos[i];
        //���ؼ��ޣ�Ĭ�Ͽ�������״̬Ϊ0
        if(*Owner->inCHK[i] != 0) Owner->Connected[i] = 0x0;
        
        Owner->Connected[i] = TSExt_Func(*Owner->inBLK[i], &Owner->T_inBLK[i], cnt_T2S);//���ع��ܱ���Ͷ�룬Ĭ��Ϊ1��չ��2s
        chg = (Owner->Connected_[i] ^ Owner->Connected[i]) & Owner->Connected_[i] & 0x1;//���ر�λ�Ҵ�ǰΪ����״̬
        Owner->Flag_KG_CHG[i] = TSExt_Func(chg, &Owner->T_Flag_KG_CHG, cnt_T5S);//���ر�λ�ź�չ��5s
        if(*Owner->inCHK[i] | *Owner->inBLK[i])
            Owner->Flag_KG_CHG[i] = 0x0;//���ؼ��޻��ܱ������򿪹�λ�ñ�λ��0
        Owner->Flag_KG_CHG_All |= Owner->Flag_KG_CHG[i];
    }
    //�����ر�λ����
    if(Owner->SET_POS_FD & FLAG_QD.bit.Fqd_Run){
        Owner->Flag_KG_FD = TSDlyExt_Func(Owner->Flag_KG_CHG_All, &Owner->T_Flag_KG_FD, 3, cnt_T100MS);
        FLAG_QD.bit.Fqd_SelfQD |= Owner->Flag_KG_FD;
    } else {
        Owner->Flag_KG_FD   = 0;
        Owner->T_Flag_KG_FD = 0;
    }

    //���˷�����������ڵ�����״̬
    pHead = 0;
    pEnd = 0;
    for(i = 0;i < Owner->Node_num;i++){
        if(Owner->Type[i] != 0x2) continue;
        if((Owner->visited_Node[i] | Owner->added_Node[i])!= 0) continue;
        Owner->stack[pEnd] = i;
        pEnd = (pEnd + 1) % MAX_NODE;
        Owner->added_Node[i] = 0x1;
        while(pHead != pEnd){
            index = Owner->stack[pHead];
            Owner->visited_Node[index] = 0x1;
            pHead = (pHead + 1) % MAX_NODE;
            for(j = 0;j < Owner->Conn_num;j++){
                if(Owner->Connected[j] == 0) continue;
                if(Owner->visited_Conn[j] != 0) continue;
                ifrom = Owner->FromNode[j] - 1;
                iend = Owner->EndNode[j] - 1;
                if((ifrom != index) && (iend != index)) continue;
				if(ifrom == index){
					index_other = iend;
				}else{
					index_other = ifrom;
				}
                if(Owner->added_Node[index_other] != 0) continue;//�ýڵ��Ѿ����ع��ˣ��Ͳ�������
                Owner->stack[pEnd] = index_other;
                pEnd = (pEnd + 1) % MAX_NODE;
                Owner->added_Node[index_other] = 0x1;
                Owner->visited_Conn[j] = 0x1;
            }
        }
    }

    //���ɽӵ�����仯״̬
    chg = 0x0;
    Owner->Flag_Stat_CHG_All = 0;
    for(i = 0;i < Owner->Node_num;i++){
        Owner->Stat_[i] = Owner->Stat[i];
        Owner->Stat[i] = (Owner->visited_Node[i] ^ 0x1) & 0x1;//����״̬Ϊ0x1
        //��������ڵ���״̬�仯
        if(Owner->Type[i] == 1){
            chg = (Owner->Stat[i] ^ Owner->Stat_[i]) & Owner->Stat[i] & 0x1;
            Owner->Flag_Stat_CHG[i] = TSExt_Func(chg, &Owner->T_Flag_Stat_CHG[i], cnt_T5S);
            Owner->Flag_Stat_CHG_All |= Owner->Flag_Stat_CHG[i];
        }
    }
    //�����������������߼�
    if(Owner->SET_CONN_FD & FLAG_QD.bit.Fqd_Run){
        Owner->Flag_Stat_FD = TSDlyExt_Func(Owner->Flag_Stat_CHG_All, &Owner->T_Flag_Stat_FD, 3, cnt_T100MS);
        FLAG_QD.bit.Fqd_SelfQD |= Owner->Flag_Stat_FD;
    } else {
        Owner->Flag_Stat_FD   = 0;
        Owner->T_Flag_Stat_FD = 0;
    }

    ////////////////////////////////���ϴ���ģ��////////////////////////////////////
    if(FLAG_QD.bit.Fqd_QD & device_status.bit.ZGNTR & (*Owner->Enable)){
        Owner->Flag_KG_Flt_All = 0x0;
        for(i = 0;i < Owner->Conn_num;i++){
            cnt_tmp = 0;
            flg_tmp1 = 0x0;
            flg_tmp2 = 0x0;
            if(*Owner->inIa[i] <= Owner->Iset) cnt_tmp++;
            if(*Owner->inIb[i] <= Owner->Iset) cnt_tmp++;
            if(*Owner->inIc[i] <= Owner->Iset) cnt_tmp++;
            if(cnt_tmp >= 2) flg_tmp1 = 0x1;
            if(abs(*Owner->inP[i]) <= Owner->Pset) flg_tmp2 = 0x1;
            //���㿪����·��բ��־
            flg_tmp = flg_tmp1 & flg_tmp2 & Owner->Flag_KG_CHG[i];
            if(*Owner->inCHK[i] | *Owner->inBLK[i]) flg_tmp = 0x0;
            Owner->Flag_KG_Flt[i] = TSDelay_Func(flg_tmp, &Owner->T_Flag_KG_Flt[i], Owner->Tset_Flt[i]);
            Owner->Flag_KG_Flt_All |= Owner->Flag_KG_Flt[i];
        }
        //����ڵ����������־
        flg_tmp = Owner->Flag_KG_Flt_All & Owner->Flag_KG_CHG_All & Owner->Flag_Stat_CHG_All;//���ر�λ&�ڵ����&��·������բ
        if(flg_tmp){
            for(i = 0;i < Owner->Node_num;i++){
                Owner->Act[i] = Owner->Flag_Stat_CHG[i];
                Owner->Act_total |= Owner->Act[i];
            }
        }
    }else{
        for(i = 0;i < MAX_NODE;i++){
            Owner->Act[i] = 0x0;
        }
        Owner->Act_total = 0x0;
        //���������
        for(i = 0;i < MAX_NODE;i++){
            Owner->Flag_KG_Flt[i] = 0x0;
            Owner->T_Flag_KG_Flt = 0;
        }
        Owner->Flag_KG_Flt_All = 0x0;
    }
	
    return;
}


void ClearCalInfo(TopoSearch* Owner){
    Uint8 i;
    for(i = 0;i < MAX_NODE;i++){
        Owner->Stat[i] = 0x0;
        Owner->Stat_[i] = 0x0;
        Owner->Act[i] = 0x0;
        Owner->visited_Node[i] = 0x0;
        Owner->stack[i] = 0x0;
        Owner->Flag_Stat_CHG[i] = 0x0;
        Owner->T_Flag_Stat_CHG[i] = 0;
    }
    for(i = 0;i < MAX_CONN;i++){
        Owner->visited_Conn[i] = 0x0;
        Owner->Connected[i] = 0x0;
        Owner->Connected_[i] = 0x0;
    }
    Owner->Act_total = 0x0;
    Owner->Flag_KG_CHG = 0x0;
    Owner->T_Flag_KG_CHG = 0;
    Owner->Flag_KG_FD = 0x0;
    Owner->T_Flag_KG_FD = 0;
    Owner->Flag_Stat_FD = 0x0;
    Owner->T_Flag_Stat_FD = 0;
    Owner->Flag_Conn_CHG = 0x0;
    //Owner->flg_seterr = 0x0;
    return;
}
