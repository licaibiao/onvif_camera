/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: rtsp_client.h
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: RTSP –≠“È 
*Date:	   2019-06-22
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#ifndef _RTSP_H_
#define _RTSP_H_

#include "common.h"

int RTSP_Client_Init(RTSP_STATUS_S * stRTSPClient);
int RTSP_Client_Release(RTSP_STATUS_S * stRTSPClient);
int RTSP_Client_Session(RTSP_STATUS_S *stRTSPClient);

int RTSP_Client_OPTIONS(RTSP_STATUS_S *pstRTSPClient);
int RTSP_Client_DESCRIBE(RTSP_STATUS_S *pstRTSPClient);
int RTSP_Client_SETUP(RTSP_STATUS_S *pstRTSPClient);
int RTSP_Client_PLAY(RTSP_STATUS_S *pstRTSPClient);
int RTSP_Client_TEARDOWN(RTSP_STATUS_S *pstRTSPClient);

#endif

