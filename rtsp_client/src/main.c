/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: main.c
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: RTSP øÕªß∂À≤‚ ‘≥Ã–Ú 
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

#define TEST_RTSP_URL	"rtsp://192.168.1.120:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"

int RTSP_Debug(void)
{
	int l_s32Ret = {0};
	RTSP_STATUS_S l_stRTSPClient={0};

	l_s32Ret = RTSP_Client_Init(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client Init error \n",__FUNCTION__,__LINE__);
		return -1;
	}

	/**for debug**/
	sprintf(l_stRTSPClient.arrs8RTSPUrl,"%s",TEST_RTSP_URL);
	sprintf(l_stRTSPClient.arrs8ServerIP,"%s","192.168.1.120");
	sprintf(l_stRTSPClient.stClientUser.arrs8Name,"%s","admin");
	sprintf(l_stRTSPClient.stClientUser.arrs8PassWord,"%s","0281BBbb");
	l_stRTSPClient.s32StreamType = MAINSTREAM;
	
	l_s32Ret = RTSP_Client_Session(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client Session error \n",__FUNCTION__,__LINE__);
		return -2;
	}

	l_s32Ret = RTSP_Client_OPTIONS(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client OPTIONS error \n",__FUNCTION__,__LINE__);
		return -3;
	}


	l_s32Ret = RTSP_Client_DESCRIBE(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client DESCRIBE error \n",__FUNCTION__,__LINE__);
		return -4;
	}


	l_s32Ret = RTSP_Client_SETUP(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client SETUP error \n",__FUNCTION__,__LINE__);
		return -5;
	}

	l_s32Ret = RTSP_Client_PLAY(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client PLAY error \n",__FUNCTION__,__LINE__);
		return -6;
	}
	
	sleep(1);
	l_s32Ret = RTSP_Client_TEARDOWN(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client TEARDOWN error \n",__FUNCTION__,__LINE__);
		return -7;
	}

	l_s32Ret = RTSP_Client_Release(&l_stRTSPClient);
	if(0!=l_s32Ret)
	{
		printf("%s %d RTSP Client Release error \n",__FUNCTION__,__LINE__);
		return -8;
	}	

	return 0;
}

int main(void)
{
	RTSP_Debug();	
	return 0;
}

