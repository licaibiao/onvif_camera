/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: main.c
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: RTSP RTCP RTP 客户端测试程序 
*Date:	   2019-09-22
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "rtsp_client.h"
#include "rtp_client.h"
#include "rtcp_client.h"

#define SERVER_IP_ADDR  "192.168.0.120"

#define TEST_RTSP_URL	"rtsp://192.168.0.120:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"

/******************************************************** 
Function:    RTSP_RTCP_RTP_Debug  
Description: RTSP,RTCP,RTP 测试函数
Input:  none
OutPut: none
Return: 0 成功，非0失败
Others: 
Author: Caibiao Lee
Date:   2019-10-05
*********************************************************/
int RTSP_RTCP_RTP_Debug(void)
{
	int l_s32Ret = 0;
    int l_s32Res = 0;
	RTSP_STATUS_S l_stRTSPClient = {0};
    RTCP_STATUS_S l_stRTCPClient = {0};
    RTP_STATUS_S  l_stRTPClient  = {0};

	l_s32Ret = RTSP_Client_Init(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client Init error \n",__FUNCTION__,__LINE__);
		return -1;
	}

	/**for debug**/
	sprintf(l_stRTSPClient.arrs8RTSPUrl,"%s",TEST_RTSP_URL);
	sprintf(l_stRTSPClient.arrs8ServerIP,"%s",SERVER_IP_ADDR);
	sprintf(l_stRTSPClient.stClientUser.arrs8Name,"%s","admin");
	sprintf(l_stRTSPClient.stClientUser.arrs8PassWord,"%s","0281BBbb");
	l_stRTSPClient.s32StreamType = MAINSTREAM;
	
	l_s32Ret = RTSP_Client_Session(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client Session error \n",__FUNCTION__,__LINE__);
		l_s32Ret = -2;
        goto ERROR;
	}

	l_s32Ret = RTSP_Client_OPTIONS(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client OPTIONS error \n",__FUNCTION__,__LINE__);
		l_s32Ret = -3;
        goto ERROR;

	}


	l_s32Ret = RTSP_Client_DESCRIBE(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client DESCRIBE error \n",__FUNCTION__,__LINE__);
		l_s32Ret = -4;
        goto ERROR;

	}


	l_s32Ret = RTSP_Client_SETUP(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client SETUP error \n",__FUNCTION__,__LINE__);
		l_s32Ret = -5;
        goto ERROR;

	}

	l_s32Ret = RTSP_Client_PLAY(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client PLAY error \n",__FUNCTION__,__LINE__);
		l_s32Ret = -6;
        goto ERROR;
	}

    printf("\n\n");
    printf("biao debug  SerRTCPPort = %s \n",l_stRTSPClient.arrs8SerRTCPPort);
    printf("biao debug  SerRTPPort  = %s \n",l_stRTSPClient.arrs8SerRTPPort);
    printf("\n\n");

    /*******************************************
                    RTCP Debug 
    *******************************************/
    l_s32Ret = RTCP_Client_Init(&l_stRTCPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTCP Client Init error \n",__FUNCTION__,__LINE__);
		l_s32Ret = -7;
        goto ERROR;

	}

    /**测试程序，强制赋值**/
    l_stRTCPClient.u32SerRTCPPort    = atoi(l_stRTSPClient.arrs8SerRTCPPort);
    l_stRTCPClient.u32ClientRTCPPort = RTP_CLIENT_PORT+1;
	sprintf(l_stRTCPClient.arrs8ServerIP,"%s",SERVER_IP_ADDR);
	l_stRTSPClient.s32StreamType = MAINSTREAM;
    
    l_s32Ret = RTCP_Client_Session(&l_stRTCPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTCP Client Session error \n",__FUNCTION__,__LINE__);
		l_s32Ret = -9;
        goto ERROR;
	}

    /*******************************************
                    RTP Debug 
    *******************************************/
    l_s32Ret=RTP_Client_Init(&l_stRTPClient);
    if(0!=l_s32Ret)
    {
        printf("%s %d RTP Client Init error \n",__FUNCTION__,__LINE__);
        l_s32Ret = -10;
        goto ERROR;
    }

    l_stRTPClient.u32SerRTPPort    = atoi(l_stRTSPClient.arrs8SerRTPPort);
    l_stRTPClient.u32ClientRTPPort = RTP_CLIENT_PORT;
    sprintf(l_stRTPClient.arrs8ServerIP,"%s",SERVER_IP_ADDR);

    
    l_s32Ret=RTP_CLient_Session(&l_stRTPClient);
    if(0!=l_s32Ret)
    {
        printf("%s %d RTP Client Session error \n",__FUNCTION__,__LINE__);
        l_s32Ret = -11;
        goto ERROR;
    }

    printf("biao debug %s %d \n",__FUNCTION__,__LINE__);    
    l_s32Ret=RTP_Client_GetOnePacketData(&l_stRTPClient);
    if(0!=l_s32Ret)
    {
        printf("%s %d RTCP Client Session error \n",__FUNCTION__,__LINE__);
        l_s32Ret = -12;
        goto ERROR;
    }

ERROR:
    l_s32Ret = RTSP_Client_TEARDOWN(&l_stRTSPClient);
    if(0!=l_s32Ret)
    {
        printf("%s %d RTSP Client TEARDOWN error \n",__FUNCTION__,__LINE__);
        l_s32Ret = -10;
        goto ERROR;
    
    }
    
	l_s32Res = RTSP_Client_Release(&l_stRTSPClient);
	if(0!=l_s32Res)
	{
		printf("%s %d RTSP Client Release error \n",__FUNCTION__,__LINE__);
	}	


    l_s32Res = RTCP_Client_Release(&l_stRTCPClient);
	if(0!=l_s32Res)
	{
		printf("%s %d RTSP Client Release error \n",__FUNCTION__,__LINE__);
	}	

    l_s32Res = RTP_Client_Release(&l_stRTPClient);
	if(0!=l_s32Res)
	{
		printf("%s %d RTSP Client Release error \n",__FUNCTION__,__LINE__);
	}	

	return l_s32Ret;
    
}

int main(void)
{
	RTSP_RTCP_RTP_Debug();	
	return 0;
}

