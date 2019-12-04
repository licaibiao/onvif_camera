/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: rtcp_client.h
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: RTCP –≠“È 
*Date:	   2019-9-22
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#ifndef _RTCP_CLIENT_H_
#define _RTCP_CLIENT_H_

#include "common.h"
#include "rtp_client.h"

int RTCP_Client_Init(RTCP_STATUS_S *pstRTCPClient);
int RTCP_Client_Release(RTCP_STATUS_S *pstRTCPClient);
int RTCP_Client_SendHeader(RTCP_STATUS_S *pstRTCPClient);
int RTCP_Client_Session(RTCP_STATUS_S *pstRTCPClient);
int RTCP_Client_RecvData(int s32SocketFd,char *ps8buf,int len);
int RTCP_Client_SendData(int s32SocketFd,char *rtcpbuf,RTP_RECV_INFO_S *Info);

#endif


