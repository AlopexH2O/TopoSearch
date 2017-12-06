/*
元件：   TopoSearch
功能描述：拓扑分析模块

将来的工作：

备注：
	！

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

    //input信号注册
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
  
  if(FLAG_QD.bit.Fqd_Run==0)  return;  //没有上电运行，不进行处理
  
  if(Owner->NTopoSearch==0)      return;  //断面无效，直接返回
  
  Owner->Valid = 1;   //置断面有效
  Owner->Psec  = 0;
 	for(i=0; i<(int16)Owner->NTopoSearch; i++)
  {
  	temp16 = (*Owner->inRun[i]!=0)? (int16)(*Owner->inP[i]):0;
  	if(Owner->MAdd[i]!=0) temp16 = -temp16;
  	Owner->Psec += temp16; 	
  	Owner->Pk[i] = temp16;
  }
  
  if(*Owner->BlkP!=0) //外部PT，CT断线或其它原因造成的断面功率无效
  {
  	Owner->Psec=(int16)(Owner->PsDef);
  	Owner->Valid = 0;
  }
//存储断面功率    	
   	Owner->PdmStorage[store_ptr]=Owner->Psec;
   	Owner->VldStorage[store_ptr]= Owner->Valid;
//存储当前状态    	
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
    	Owner->Flthold[i] |= (*Owner->inFlt[i]);      //跳闸标志整组内保持
    	Owner->Sdbg[0] = (Owner->Run_[i])&(Owner->Flthold[i]);  //启动前投运且跳闸
    	Owner->Sdbg[1] = (Owner->Run_[i]^0x1);                  //启动前停运
    	Owner->Sdbg[2] &= Owner->Sdbg[0] | Owner->Sdbg[1];      //要求所有有效线路均满足
    	Owner->Sdbg[3] &=  Owner->Run_[i];   //所有线均投运
      Owner->Sdbg[4] |=  Owner->Run_[i];   //至少一条线投运
      Owner->Sdbg[5] |=  Owner->Flthold[i];//至少一条线跳闸
    }
 
   if((Owner->Valid_!=0)&&(*Owner->En!=0))  //断面有效，否则直接不判,且只动作1次
   {
   	 if(Owner->MTopoSearch==0x1)  
   	 {
   		Owner->Act = Owner->Sdbg[2]&Owner->Sdbg[3]&Owner->Sdbg[5];  //全部投运，全部跳闸
   	 }else
   	 {
   		Owner->Act = Owner->Sdbg[2]&Owner->Sdbg[4]&Owner->Sdbg[5]; //至少有一回线投运且跳闸
   	 }
   }   
   
   if((Owner->Act!=0)&&(Owner->ActLast==0))
   {
   	 Owner->Prtm = (int16)(*Owner->Prec);
   	 Owner->ActLast = Owner->Act;
   }
  //  device_status.bit.flg_act |=  Owner->Act; //动作标志
  }
 }  
 else
 {	
 	//计算事故前200ms断面功率和状态 
	    Owner->Psec_ = Owner->PdmStorage[read_ptr];         //断面功率
	    Owner->Valid_= Owner->VldStorage[read_ptr];        //断面有效标志
    	for(i=0;i<N_PSec;i++)
    	{
    		Owner->Pk_[i] =Owner->PowerStorage[read_ptr][i];	//200毫秒前的功率
    		Owner->Run_[i]=Owner->StateStorage[read_ptr][i];	//200毫秒前的投停标志
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







