/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: LinkListQueue.c
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: 用链表实现一个队列的功能
*Date:	   2019-06-22
*Author:   Caibiao Lee
*Version:  V1.1
*Others:
*History:
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "linklistqueue.h" 

pthread_mutex_t g_stLinkListMutex[LINK_LIST_MAX_COUNT];
static unsigned char gs_LinkListUseFlag[LINK_LIST_MAX_COUNT]={0};

int LinkListQueue_InitMutex(unsigned char u8QueueNum)
{
	if(u8QueueNum>=LINK_LIST_MAX_COUNT)
	{
		printf("%s %d input para  %d error \n",__FUNCTION__,__LINE__,u8QueueNum);
		return -1;	
	}

	if(0==gs_LinkListUseFlag[u8QueueNum])
	{
		printf("%s %d Link is not in use \n",__FUNCTION__,__LINE__);
		return -2;
	}
	
    pthread_mutex_init(&g_stLinkListMutex[u8QueueNum],NULL);

	return 0;
}

int LinkListQueue_ReleaseMutex(unsigned char u8QueueNum)
{
	if(u8QueueNum>=LINK_LIST_MAX_COUNT)
	{
		printf("%s %d input para  %d error \n",__FUNCTION__,__LINE__,u8QueueNum);
		return -1;	
	}

	if(0==gs_LinkListUseFlag[u8QueueNum])
	{
		printf("%s %d Link is not in use \n",__FUNCTION__,__LINE__);
		return -2;
	}

    pthread_mutex_destroy(&g_stLinkListMutex[u8QueueNum]);

	return 0;
}

int LinkListQueue_Lock(unsigned char u8QueueNum)
{
	if(u8QueueNum>=LINK_LIST_MAX_COUNT)
	{
		printf("%s %d input para  %d error \n",__FUNCTION__,__LINE__,u8QueueNum);
		return -1;	
	}

	if(0==gs_LinkListUseFlag[u8QueueNum])
	{
		printf("%s %d Link is not in use \n",__FUNCTION__,__LINE__);
		return -2;
	}

	pthread_mutex_lock(&g_stLinkListMutex[u8QueueNum]);

	return 0;
}

int LinkListQueue_Unlock(unsigned char u8QueueNum)
{
	if(u8QueueNum>=LINK_LIST_MAX_COUNT)
	{
		printf("%s %d input para  %d error \n",__FUNCTION__,__LINE__,u8QueueNum);
		return -1;	
	}

	if(0==gs_LinkListUseFlag[u8QueueNum])
	{
		printf("%s %d Link is not in use \n",__FUNCTION__,__LINE__);
		return -2;
	}

	pthread_mutex_unlock(&g_stLinkListMutex[u8QueueNum]);

	return 0;
}

/******************************************************** 
Function: LinkListQueue_Create	
Description: 创建链队列
Input:	
	s32Capacity 指定该线性表的大小
	u8QueueNum  需要创建的队列号
OutPut: none
Return: LIST_QUEUE_S*  返回创建的链队列
Others: 
Author: Caibiao Lee
Date:	2019-09-28
*********************************************************/
LIST_QUEUE_S *LinkListQueue_Create(int s32Capacity,unsigned char u8QueueNum)
{
	LIST_QUEUE_S *pstListQueue = NULL;

	if((s32Capacity<=0)||(u8QueueNum>=LINK_LIST_MAX_COUNT))
	{
		printf("%s %d input para error \n",__FUNCTION__,__LINE__);
		return NULL;
	}

	if(1==gs_LinkListUseFlag[u8QueueNum])
	{
		printf("%s %d LinkList Num=%d is already in use \n",
			__FUNCTION__,__LINE__,u8QueueNum);
		return NULL;
	}
	
	pstListQueue=(LIST_QUEUE_S *)malloc(sizeof(LIST_QUEUE_S));
	if(NULL!=pstListQueue)
	{
		pstListQueue->s32Length = 0;
		pstListQueue->s32Capacity = s32Capacity;
		gs_LinkListUseFlag[u8QueueNum] = 1;
		return pstListQueue;
	}
	else
	{
		return NULL;
	}
	
}

/******************************************************** 
Function: LinkListQueue_Destroy	
Description: 链队列清除
Input:	pstListQueue 链队列
OutPut: none
Return: 
Others: 0 成功；非0 失败
Author: Caibiao Lee
Date:	2019-09-28
*********************************************************/
int LinkListQueue_Destroy(LIST_QUEUE_S *pstListQueue)
{
	int i =0 ;
	LINK_DATA_S l_stLinkData;
	
	if(NULL==pstListQueue)
	{
		printf("%s %d input para error \n",__FUNCTION__,__LINE__);
		return -1;
	}

	if(0==gs_LinkListUseFlag[pstListQueue->s8QueueNum])
	{
		printf("%s %d Link List Num=%d is not in use \n",
			__FUNCTION__,__LINE__,pstListQueue->s8QueueNum);
		return -2;
	}

	for(i=0;i<pstListQueue->s32Length;i++)
	{
		LinkListQueue_Output(pstListQueue,&l_stLinkData);
	}
	
	gs_LinkListUseFlag[pstListQueue->s8QueueNum] = 0;
	
	return 0;
} 
 
/******************************************************** 
Function: LinkListQueue_Input	
Description: 元素进链队列
Input:	
	*pstListQueue 链队列
	stLinkData	  数据
OutPut: *pstListQueue
Return:
Others: 
Author: Caibiao Lee
Date:	2019-09-28
*********************************************************/
int LinkListQueue_Input(LIST_QUEUE_S *pstListQueue, LINK_DATA_S stLinkData)
{
	QUEUE_NODE_S *l_pstQueueNode = NULL;
	
	if(NULL==pstListQueue)
	{
		printf("%s %d input para error \n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	l_pstQueueNode=(QUEUE_NODE_S *)malloc(sizeof(QUEUE_NODE_S));
	if(NULL==l_pstQueueNode)
	{
		printf("%s %d malloc error \n",__FUNCTION__,__LINE__);
		return -2;
	}
	
	memcpy(&l_pstQueueNode->stData,&stLinkData,sizeof(LINK_DATA_S));

	l_pstQueueNode->stNext=NULL;
	
	LinkListQueue_Lock(pstListQueue->s8QueueNum);
	
	if(pstListQueue->stRear==NULL)
	{	
	    /**如果第一次插入则设置头指针和尾指针为l_pstQueueNode**/
		pstListQueue->stFront=pstListQueue->stRear=l_pstQueueNode;
	}else
	{
		/**链队列的尾部插入l_pstQueueNode**/
		pstListQueue->stRear->stNext=l_pstQueueNode;

		/**设置链队列的尾指针指向l_pstQueueNode**/
		pstListQueue->stRear=l_pstQueueNode;        
	}
	pstListQueue->s32Length++;
	
	LinkListQueue_Unlock(pstListQueue->s8QueueNum);
	
	return 0;
}
 
/******************************************************** 
Function: LinkListQueue_Output	
Description: 元素出链队列
Input:	
	*pstListQueue 链队列
	*stLinkData	  数据
OutPut: *pstListQueue，*pstLinkData
Return: 0 成功; 非0 失败
Others: 
Author: Caibiao Lee
Date:	2019-09-28
*********************************************************/
int LinkListQueue_Output(LIST_QUEUE_S *pstListQueue,LINK_DATA_S *pstLinkData)
{
	QUEUE_NODE_S *l_pstQueueNode=NULL;
	
	if((NULL==pstListQueue)||(NULL==pstLinkData))
	{
		printf("%s %d input para error \n",__FUNCTION__,__LINE__);
		return -1;
	};

	l_pstQueueNode = pstListQueue->stFront;
	if((NULL==pstListQueue->stRear)||(NULL==pstListQueue->stFront))
	{
		printf("%s %d error\n",__FUNCTION__,__LINE__);
		return -2;
	}

	LinkListQueue_Lock(pstListQueue->s8QueueNum);
	if(pstListQueue->stFront==pstListQueue->stRear)
	{
		pstListQueue->stFront=pstListQueue->stRear=NULL;
	}else
	{
		pstListQueue->stFront=pstListQueue->stFront->stNext;
	}

	pstListQueue->s32Length--;

	LinkListQueue_Unlock(pstListQueue->s8QueueNum);

	*pstLinkData=l_pstQueueNode->stData;
	memcpy((unsigned char*)pstLinkData,&l_pstQueueNode->stData,sizeof(LINK_DATA_S));
	free(l_pstQueueNode);
	
	return 0;
	
}
 
/******************************************************** 
Function: LinkListQueue_Printf	
Description: 打印输出链队列
Input:	
	*pstListQueue 链队列
OutPut: *pstListQueue
Return:
Others: 用于测试链队列使用
Author: Caibiao Lee
Date:	2019-09-28
*********************************************************/
int LinkListQueue_Printf(LIST_QUEUE_S *pstListQueue)
{
	LIST_QUEUE_S l_stQueueNode ;
 
 	if(NULL==pstListQueue)
	{
		printf("%s %d input para error \n",__FUNCTION__,__LINE__);
		return -1;
	};

	LinkListQueue_Lock(pstListQueue->s8QueueNum);
	l_stQueueNode.stFront = pstListQueue->stFront;
 
	if(pstListQueue->stFront==NULL || pstListQueue->stRear==NULL)
	{
		return -2;
	}
	
	while(pstListQueue->stFront!=NULL)
	{
		printf("%d\n",pstListQueue->stFront->stData.u32Len);
		pstListQueue->stFront=pstListQueue->stFront->stNext;
	}
	pstListQueue->stFront = l_stQueueNode.stFront;

	LinkListQueue_Unlock(pstListQueue->s8QueueNum);

	return 0;
}


int LinkListQueue_Debug(void)
{
	int l_s32Capacity = 0;
	unsigned char l_u8QueueNum = 0;
	LINK_DATA_S stTestData;	
	LIST_QUEUE_S* pstListQueue = NULL;

	l_s32Capacity = 20;
	l_u8QueueNum = 0;

	/**创建链队列**/
	pstListQueue = LinkListQueue_Create(l_s32Capacity,l_u8QueueNum);
	if(NULL==pstListQueue)
	{
		printf("%s %d create link list error \n",__FUNCTION__,__LINE__);
		return -1;
	}

	/**测试:空队列输出**/
	LinkListQueue_Output(pstListQueue,&stTestData);	
	printf("Out Put Data = %d  s32Capacity=%d s32Length=%d \n",stTestData.u32Len,
		pstListQueue->s32Capacity,pstListQueue->s32Length);

	/**添加元素到队列中去**/
	for(int i=0;i<10;i++)
	{
		stTestData.u32Len = i;
		LinkListQueue_Input(pstListQueue,stTestData);
	}
	
	/**打印队列中所有数据**/
	LinkListQueue_Printf(pstListQueue);

	/**出队列一个元素**/
	LinkListQueue_Output(pstListQueue,&stTestData);	
	printf("Out Put Data = %d  s32Capacity=%d s32Length=%d \n",stTestData.u32Len,
		pstListQueue->s32Capacity,pstListQueue->s32Length);

	/**出队列一个元素**/
	LinkListQueue_Output(pstListQueue,&stTestData);	
	printf("Out Put Data = %d  s32Capacity=%d s32Length=%d \n",stTestData.u32Len,
		pstListQueue->s32Capacity,pstListQueue->s32Length);

	/**出队列一个元素**/
	LinkListQueue_Output(pstListQueue,&stTestData);	
	printf("Out Put Data = %d  s32Capacity=%d s32Length=%d \n",stTestData.u32Len,
		pstListQueue->s32Capacity,pstListQueue->s32Length);

	/**插入队列一个元素**/
	stTestData.u32Len = 100;
	LinkListQueue_Input(pstListQueue,stTestData);

	/**打印队列中所有数据**/
	LinkListQueue_Printf(pstListQueue);

	/**测试创建一个已经存在的队列**/
	LinkListQueue_Create(l_s32Capacity,l_u8QueueNum);

	/**释放队列资源**/
	LinkListQueue_Destroy(pstListQueue);
 
	return 0;
}

 
