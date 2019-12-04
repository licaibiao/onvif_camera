/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: rtcp_client.c
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: RTCP 协议 
*Date:	   2019-9-22
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#include "rtcp_client.h"

#define RTCP_BUF_LEN_BYTE	512

/******************************************************** 
Function:	 RTCP_Client_Init  
Description: 
Input:	pstRTCPClient
OutPut: pstRTCPClient
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:	2019-9-22
*********************************************************/
int RTCP_Client_Init(RTCP_STATUS_S *pstRTCPClient)
{
	if(NULL==pstRTCPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	memset(pstRTCPClient,0,sizeof(RTCP_STATUS_S));
	pstRTCPClient->s32SockFd = -1;
	pstRTCPClient->bRTCPState =false;
	return 0;
}

/******************************************************** 
Function:	 RTCP_Client_Release  
Description: 
Input:	pstRTCPClient
OutPut: pstRTCPClient
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:	2019-9-22
*********************************************************/
int RTCP_Client_Release(RTCP_STATUS_S *pstRTCPClient)
{
	if(NULL==pstRTCPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}
    
    if(pstRTCPClient->s32SockFd>0)
    {
        NET_SocketClose(pstRTCPClient->s32SockFd);
        pstRTCPClient->s32SockFd = -1;
    }

	pstRTCPClient->bRTCPState =false;
	return 0;
}

/******************************************************** 
Function:	 RTCP_Client_SendHeader  
Description: 
Input:	pstRTCPClient
OutPut: pstRTCPClient
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:	2019-9-22
*********************************************************/
int RTCP_Client_SendHeader(RTCP_STATUS_S *pstRTCPClient)
{
	char l_arrs8Buf[32]={0};
	int l_s32Ret;
	int l_s32SocketFd;

	if(NULL==pstRTCPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	l_s32SocketFd = pstRTCPClient->s32SockFd;

	l_arrs8Buf[0] = 0xce;
	l_arrs8Buf[1] = 0xfa;
	l_arrs8Buf[2] = 0xed;
	l_arrs8Buf[3] = 0xfe;
	l_s32Ret = NET_SocketSendData(l_s32SocketFd,l_arrs8Buf,strlen(l_arrs8Buf));
	if(l_s32Ret < 0)
	{
		printf("%s %d Send Data Error \n",__FUNCTION__,__LINE__);
		return -2;
	}
	
	return 0;
}

/******************************************************** 
Function:	 RTCP_Client_Session  
Description: 
Input:	pstRTCPClient
OutPut: pstRTCPClient
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:	2019-9-22
*********************************************************/
int RTCP_Client_Session(RTCP_STATUS_S *pstRTCPClient)
{
	int l_s32SocketFd;
	int l_s32Ret;
	unsigned int l_u32Cport;
	unsigned int l_u32Sport;
	bool l_bRTCPState;
	
	if(NULL==pstRTCPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}

	l_s32SocketFd = pstRTCPClient->s32SockFd;
	l_u32Sport     = pstRTCPClient->u32SerRTCPPort;
	l_u32Cport    = pstRTCPClient->u32ClientRTCPPort;
	l_bRTCPState  = pstRTCPClient->bRTCPState;
	
	if(true==l_bRTCPState)
	{
		printf("%s,%d RTCP is already start\n",__FUNCTION__,__LINE__);
		return -2;
	}
	
	if(l_s32SocketFd > 0)
	{
		NET_SocketClose(l_s32SocketFd);
	}
	
	l_s32SocketFd = NET_SocketCreate(SOCK_DGRAM);
	if(l_s32SocketFd < 0)
	{
		printf("%s,%d create error %d\n",__FUNCTION__,__LINE__,l_s32SocketFd);
		return -4;
	}
	
	printf("%s,%d,l_s32SocketFd = %d,l_s32Cport = %d,l_s8Sport = %d,Server IP = %s\n",__FUNCTION__,__LINE__,
		l_s32SocketFd,l_u32Cport,l_u32Sport,pstRTCPClient->arrs8ServerIP);
	
	l_s32Ret = NET_SocketBind(l_s32SocketFd,l_u32Cport);
	if(l_s32Ret < 0)
	{
		printf("%s %d Bind Error \n",__FUNCTION__,__LINE__);
		return -5;
	}
	
	l_s32Ret = NET_SocketConnect(l_s32SocketFd,pstRTCPClient->arrs8ServerIP,l_u32Sport);
	if(l_s32Ret < 0)
	{
		printf("%s %d Connet Error \n",__FUNCTION__,__LINE__);
		return -6;
	}

	pstRTCPClient->bRTCPState = true;
	pstRTCPClient->s32SockFd  = l_s32SocketFd;
	RTCP_Client_SendHeader(pstRTCPClient);
	
	return 0;
}

/******************************************************** 
Function:	 RTCP_Client_RecvData  
Description: 
Input:	s32SocketFd,ps8buf,len
OutPut: ps8buf,
Return: the len of recv from socket
Others:
Author: Caibiao Lee
Date:	2019-9-22
*********************************************************/
int RTCP_Client_RecvData(int s32SocketFd,char *ps8buf,int len)
{
	int l_s32Ret;
	l_s32Ret = NET_SocketRecvData(s32SocketFd,ps8buf,len); 
	if(l_s32Ret < 0)
	{
		printf("%s %d :Recv Data Error \n",__FUNCTION__,__LINE__);
		return -1;
	}
	return l_s32Ret;
}


/******************************************************** 
Function:	 RTCP_Client_SendData  
Description: 
Input:	s32SocketFd,ps8buf,len
OutPut: none
Return:  0: success，none 0:error
Others:
Author: Caibiao Lee
Date:	2019-9-22
*********************************************************/
int RTCP_Client_SendData(int s32SocketFd,char *rtcpbuf,RTP_RECV_INFO_S *Info)
{
#if 0
	char *count;
	char *pnum;
	char flag;
	unsigned short srlen;
	char senderssrc[4]={0};

	char ntpt[4]={0};
	char rtpt[4]={0};
	int lostpacknum;
	int l_s32Ret;

	flag = rtcpbuf[0];
	srlen = rtcpbuf[2];
	srlen <<= 8;
	srlen |= rtcpbuf[3];

	senderssrc[0] = rtcpbuf[4];
	senderssrc[1] = rtcpbuf[5];
	senderssrc[2] = rtcpbuf[6];
	senderssrc[3] = rtcpbuf[7];
	
	if(!(flag & 0x80))
	{
		printf("sender is not need\n");
		return -1;
	}
	if(200 == rtcpbuf[1])
	{
		ntpt[0] = rtcpbuf[8];
		ntpt[1] = rtcpbuf[9];
		ntpt[2] = rtcpbuf[10];
		ntpt[3] = rtcpbuf[11];

		rtpt[0] = rtcpbuf[12];
		rtpt[1] = rtcpbuf[13];
		rtpt[2] = rtcpbuf[14];
		rtpt[3] = rtcpbuf[15];
	   
	}

	count = Info->count;
	pnum = Info->pnum;
	lostpacknum = Info->lostpacknum;


	bzero(rtcpbuf,128);

	rtcpbuf[0] = (2 << 6) | (0 << 5) | 1;
	rtcpbuf[1] = 201;

				 
	rtcpbuf[3] = 7;//32bit为单位的rtcp包长度
	rtcpbuf[4] = 0x16;//同步源
	rtcpbuf[5] = 0xa3;
	rtcpbuf[6] = 0x12;
	rtcpbuf[7] = 0xf3;
   
   /*rtp发送的同步源*/
	rtcpbuf[8] = senderssrc[0];
	rtcpbuf[9] = senderssrc[1];
	rtcpbuf[10] = senderssrc[2];
	rtcpbuf[11] = senderssrc[3];

		  
	rtcpbuf[12] = 0;//包的丢失率
	//rtcp.lost = 0;

	rtcpbuf[13] = ((lostpacknum >> 16) & 0xff);//丢失包的累计数
	rtcpbuf[14] = ((lostpacknum>>8)&0xff);
	rtcpbuf[15] = (lostpacknum & 0xff);

	rtcpbuf[16] = count[0];//循环的次数
	rtcpbuf[17] = count[1];

	rtcpbuf[18] = pnum[0]; //RTP接收到的最大的序列号，
	rtcpbuf[19] = pnum[1];

	rtcpbuf[20] = 0x00;   //间隔的抖动
	rtcpbuf[21] = 0x00;
	rtcpbuf[22] = 0x01;
	rtcpbuf[23] = 0x55;

	rtcpbuf[24] = ntpt[0];	//最后的sr时标
	rtcpbuf[25] = ntpt[1];
	rtcpbuf[26] = rtpt[2];
	rtcpbuf[27] = rtpt[3];

	rtcpbuf[28] = 0x00;  //收到sr到发送rr之间的时间差=(rrtime-srtime)*66536
	rtcpbuf[29] = 0x02;
	rtcpbuf[30] = 0xc2;
	rtcpbuf[31] = 0x4f;

	rtcpbuf[32] = 0x0;
	rtcpbuf[32] |= 0x1;

	rtcpbuf[33] = 202;

	rtcpbuf[34] = 0x00;
	rtcpbuf[35] = 0x04;
	
	
	rtcpbuf[36] = 0x16;
	rtcpbuf[37] = 0xa3;
	rtcpbuf[38] = 0x12;
	rtcpbuf[39] = 0xf3;
	
	rtcpbuf[40] = 1;
	rtcpbuf[41] = 0x07;
	rtcpbuf[42] = 0x64;
	rtcpbuf[43] = 0x65;
	rtcpbuf[44] = 0x6c;
	rtcpbuf[45] = 0x6c;
	rtcpbuf[46] = 0x2d;
	rtcpbuf[47] = 0x50;
	rtcpbuf[48] = 0x43;
	
	rtcpbuf[49] = 0x00;
	rtcpbuf[50] = 0x00;
	rtcpbuf[51] = 0x00;

	l_s32Ret = NET_SocketSendData(s32SocketFd,rtcpbuf,52); 
	if(l_s32Ret < 0)
	{
		printf("%s %d:Send Data error",__FUNCTION__,__LINE__);
		return -2;
	}
	return 0;

#endif    
}


