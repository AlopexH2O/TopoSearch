/*
Ԫ����   TopoSearch
�������������˷���ģ��

�����Ĺ�����

��ע��
	��

*/


#include "Component.h"
#include "TopoSearch.h"
//#include "sub.h"
#include <stdlib.h>
/* Local function declaration */
int initTopoSearch(TopoSearch *Owner);
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

    //input�ź�ע��
    for (i = 0; i < MAX_CONN; ++i){
		sprintf(str_temp,"inRun%d type=b level=2 ",i+1); 	
		defineSignalIn((Component *)Owner,(void **)(&(Owner->inRun[i])),str_temp);		    
 		sprintf(str_temp,"inFlt%d type=b level=2 ",i+1); 	
		defineSignalIn((Component *)Owner,(void **)(&(Owner->inFlt[i])),str_temp);
 
 		sprintf(str_temp,"Conn%d  type=i min=0 max=65535 default=0 option=1",i+1); 	
		defineParameter((Component *)Owner,(void *)&(Owner->Conn[i]),str_temp);	
    }
    
    for (i = 0; i < MAX_NODE; ++i){
  		sprintf(str_temp,"NodeType%d  type=b min=0 max=2 default=0 option=1",i+1); 	
		defineParameter((Component *)Owner,(void *)&(Owner->NodeType[i]),str_temp);	
 
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
    if  (ret != OK)
    {
        return ret;
    }

   Owner->Psec  = 0;
   Owner->Psec_ = 0;	
   Owner->Valid = 1;	
   Owner->Valid_= 1;	      
   Owner->Act   = 0;  
   Owner->ActLast = 0; 
   Owner->Sdbg[0] = 0;  Owner->Sdbg[1] = 0;
   Owner->Sdbg[2] = 0;  Owner->Sdbg[3] = 0;    
   Owner->Sdbg[4] = 0;  Owner->Sdbg[5] = 0; 
    
   for(i =0; i<N200msNum; i++)
   {
   	 for(j=0;j<N_PSec;j++) 
   	 { 
   	 	Owner->PowerStorage[i][j]=0; 
   	 	Owner->StateStorage[i][j]=0;
   	 	Owner->Flthold[j] = 0;
   	 }
     Owner->PdmStorage[i] = 0;
     Owner->VldStorage[i] = 0; 	  
   }    
    
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
  int16 temp16;
  int16 i;
  
  if(FLAG_QD.bit.Fqd_Run==0)  return;  //û���ϵ����У������д���
  
  if(Owner->NTopoSearch==0)      return;  //������Ч��ֱ�ӷ���
  
  Owner->Valid = 1;   //�ö�����Ч
  Owner->Psec  = 0;
 	for(i=0; i<(int16)Owner->NTopoSearch; i++)
  {
  	temp16 = (*Owner->inRun[i]!=0)? (int16)(*Owner->inP[i]):0;
  	if(Owner->MAdd[i]!=0) temp16 = -temp16;
  	Owner->Psec += temp16; 	
  	Owner->Pk[i] = temp16;
  }
  
  if(*Owner->BlkP!=0) //�ⲿPT��CT���߻�����ԭ����ɵĶ��湦����Ч
  {
  	Owner->Psec=(int16)(Owner->PsDef);
  	Owner->Valid = 0;
  }
//�洢���湦��    	
   	Owner->PdmStorage[store_ptr]=Owner->Psec;
   	Owner->VldStorage[store_ptr]= Owner->Valid;
//�洢��ǰ״̬    	
    for(i=0;i<N_PSec;i++)
    {
    	Owner->StateStorage[store_ptr][i]=(Uint8)*Owner->inRun[i];
    	Owner->PowerStorage[store_ptr][i]=Owner->Pk[i];
    }
////////////////////////////////////////////////////////////      
 if(FLAG_QD.bit.Fqd_QD)
 { 	
 	if(device_status.bit.ZGNTR)
 	{
    Owner->Sdbg[0] = 0;  Owner->Sdbg[1] = 0;
    Owner->Sdbg[2] = 1;  Owner->Sdbg[3] = 1;    
    Owner->Sdbg[4] = 0;  Owner->Sdbg[5] = 0;      
    for(i = 0; i<(int16)Owner->NTopoSearch; i++)
    {
    	Owner->Flthold[i] |= (*Owner->inFlt[i]);      //��բ��־�����ڱ���
    	Owner->Sdbg[0] = (Owner->Run_[i])&(Owner->Flthold[i]);  //����ǰͶ������բ
    	Owner->Sdbg[1] = (Owner->Run_[i]^0x1);                  //����ǰͣ��
    	Owner->Sdbg[2] &= Owner->Sdbg[0] | Owner->Sdbg[1];      //Ҫ��������Ч��·������
    	Owner->Sdbg[3] &=  Owner->Run_[i];   //�����߾�Ͷ��
      Owner->Sdbg[4] |=  Owner->Run_[i];   //����һ����Ͷ��
      Owner->Sdbg[5] |=  Owner->Flthold[i];//����һ������բ
    }
 
   if((Owner->Valid_!=0)&&(*Owner->En!=0))  //������Ч������ֱ�Ӳ���,��ֻ����1��
   {
   	 if(Owner->MTopoSearch==0x1)  
   	 {
   		Owner->Act = Owner->Sdbg[2]&Owner->Sdbg[3]&Owner->Sdbg[5];  //ȫ��Ͷ�ˣ�ȫ����բ
   	 }else
   	 {
   		Owner->Act = Owner->Sdbg[2]&Owner->Sdbg[4]&Owner->Sdbg[5]; //������һ����Ͷ������բ
   	 }
   }   
   
   if((Owner->Act!=0)&&(Owner->ActLast==0))
   {
   	 Owner->Prtm = (int16)(*Owner->Prec);
   	 Owner->ActLast = Owner->Act;
   }
  //  device_status.bit.flg_act |=  Owner->Act; //������־
  }
 }  
 else
 {	
 	//�����¹�ǰ200ms���湦�ʺ�״̬ 
	    Owner->Psec_ = Owner->PdmStorage[read_ptr];         //���湦��
	    Owner->Valid_= Owner->VldStorage[read_ptr];        //������Ч��־
    	for(i=0;i<N_PSec;i++)
    	{
    		Owner->Pk_[i] =Owner->PowerStorage[read_ptr][i];	//200����ǰ�Ĺ���
    		Owner->Run_[i]=Owner->StateStorage[read_ptr][i];	//200����ǰ��Ͷͣ��־
    	}
    Owner->Act = 0;      Owner->ActLast = 0;
    Owner->Sdbg[0] = 0;  Owner->Sdbg[1] = 0;
    Owner->Sdbg[2] = 0;  Owner->Sdbg[3] = 0;    
    Owner->Sdbg[4] = 0;  Owner->Sdbg[5] = 0;     
    Owner->Prtm    = 0;
    
    for(i = 0; i<N_PSec; i++)
    {
    	Owner->Flthold[i] = 0;
    } 
  }
}







