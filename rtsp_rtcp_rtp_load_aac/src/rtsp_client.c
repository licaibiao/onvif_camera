/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: rtsp_client.c
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: RTSP 协议 
*Date:	   2019-06-22
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#include <string.h>
#include <stdlib.h>  
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rtsp_client.h"

#define BUF_LEN_1024_BYTE 	 1024	
#define RTSP_DEFAULT_PORT    554	
static int gs_s32CseqCount    = 1;

static unsigned short IPC_RTSP_GetPort(const char *const ps8Rtsp)
{
    char l_as8RTSP[128] = {0};
    char *l_ps8Buf = NULL;
    int i =0;
    unsigned int l_u16RtspPort = 0;
	
    if(NULL == ps8Rtsp)
    {   
        printf("[%s,%d] psTIPCDEV is NULL error\n ",__FILE__,__LINE__);
        return 0;
    }
    memcpy(l_as8RTSP,ps8Rtsp,strlen(ps8Rtsp));
	
    printf("[%s,%d]l_ps8Buf=%s\n",__FUNCTION__,__LINE__,l_as8RTSP);
    l_ps8Buf = strtok(l_as8RTSP,"//");
	
    printf("[%s,%d]l_ps8Buf=%s\n",__FUNCTION__,__LINE__,l_ps8Buf);
	
    if(NULL != l_ps8Buf)
    {
        l_ps8Buf = strtok(NULL,":");
    }
    if(NULL != l_ps8Buf)
    {
        l_ps8Buf = strtok(NULL,"/");
    }
    if(NULL != l_ps8Buf)
    {
        for(i = 0;i < 6 && l_ps8Buf[i] - '0' >= 0 && l_ps8Buf[i] - '0' <= 9;i++)
        {
            l_u16RtspPort *= 10;
            l_u16RtspPort += l_ps8Buf[i] - '0';
        }
    }
    return l_u16RtspPort;
}


static int IPC_RTSP_RecvSelect(int s32RtspFd,char *ps8buf,int s32Len)
{
    int l_s32Ret;
    fd_set l_s4Fdset;
    struct timeval l_stTimeout;
	
    FD_ZERO(&l_s4Fdset);
    FD_SET(s32RtspFd,&l_s4Fdset);
    l_stTimeout.tv_sec = 2;
    l_stTimeout.tv_usec = 0;
	
    l_s32Ret = select(s32RtspFd + 1,&l_s4Fdset,NULL,NULL,&l_stTimeout);
    if(l_s32Ret < 0)
    {
        printf("[%s,%d]slect error\n",__FUNCTION__,__LINE__);
    }
    else if(0 == l_s32Ret)
    {
        printf("[%s,%d] slect timeout\n",__FUNCTION__,__LINE__);
    }
    else
    {
       l_s32Ret = NET_SocketRecvData(s32RtspFd,ps8buf,s32Len); 
    }
    return l_s32Ret;
}

static int IPC_RTSP_CheckReply(char *ps8buf)
{
    char *p = NULL;
    p = strstr(ps8buf,"OK");
    if(NULL == p)
    {
        return -1;
    }
    return 0;
}

static int IPC_RTSP_HandleRTSPStr(RTSP_STATUS_S *pstRTSPClient)
{
    char l_arrs8Buf[256]={0};
    int l_s32Len;
	
    memcpy(l_arrs8Buf,pstRTSPClient->arrs8RTSPUrl,strlen(pstRTSPClient->arrs8RTSPUrl));
    l_s32Len = strlen(l_arrs8Buf)-1;
    while(l_s32Len >20)
    {
        if(' ' == l_arrs8Buf[l_s32Len] || '&' == l_arrs8Buf[l_s32Len])
        {
            l_arrs8Buf[l_s32Len] = '\0';
            bzero(pstRTSPClient->arrs8RTSPUrl,strlen(pstRTSPClient->arrs8RTSPUrl));
            memcpy(pstRTSPClient->arrs8RTSPUrl,l_arrs8Buf,strlen(l_arrs8Buf));
            break;
        }
        l_s32Len--;
    }
    return 0;
}


static void IPC_RTSP_SDPAnalyze(RTSP_STATUS_S *pstRTSPClient,char *ps8Buf)
{
    char *p = ps8Buf;
    char *tmp;
    int i,j;
	
	/*********************step 1:control***********************/
    for(i = 0;i< 3;i++)
    {
    	/**取出control的内容**/
        p = strstr(p,"a=control:");
        if(NULL == p)
            break;
		
		/**去掉*的行继续下行检测**/
        if(0 == strncmp(p,"a=control:*",11)) 
        {
            p++;
            continue;
        }
		/**检测control中是否存在rtsp**/
        else if(NULL != (tmp = strstr(p,"rtsp"))) 
        {
            while('\n' != *tmp && '\r' != *tmp && '\0' != *tmp)
            {
                tmp++;
            }
            *tmp = '\0';
            if(NULL != strstr(p,"track"))
            {
                j=0;
                p+=10;
                bzero(pstRTSPClient->stSDPPara.arrs8RTSPUrl,sizeof(pstRTSPClient->stSDPPara.arrs8RTSPUrl));
                while('\n' != *p && '\r' != *p && '\0' != *p)
                {
                    pstRTSPClient->arrs8RTSPUrl[j] = *(p++);
                    j++;
                }
                pstRTSPClient->stSDPPara.arrs8Control[0]='\0';
                bzero(pstRTSPClient->arrs8RTSPUrl,128);
                memcpy(pstRTSPClient->arrs8RTSPUrl,pstRTSPClient->stSDPPara.arrs8RTSPUrl,
					strlen(pstRTSPClient->stSDPPara.arrs8RTSPUrl));
                return ;
                
            }
            p = tmp +1;
            continue;
        }
        else
        {
            
            j=0;
             p+=10;
            while('\n' != *p && '\r' != *p && '\0' != *p)
            {
                pstRTSPClient->stSDPPara.arrs8Control[j] = *(p++);
                j++;
            }
            pstRTSPClient->stSDPPara.arrs8Control[j]='\0';
            break;
        }
    }
    p = ps8Buf;

    /********************************step 2:rtspurl******************************/
    p = strstr(p,"rtsp:");   /**出去rtsp**/
    if(NULL != p)
    {
         tmp = p;
         while(NULL != p && '\r' != *p && '\n' != *p && '\0' != *p)
         {
             p++;
         }
         if(NULL != p)
             *p = '\0';
         p--;
         while(' ' == *p)
         {
             p--;
         }
         if('/' ==  *p)
             *p = '\0';
         bzero(pstRTSPClient->stSDPPara.arrs8RTSPUrl,sizeof(pstRTSPClient->stSDPPara.arrs8RTSPUrl));
         memcpy(pstRTSPClient->stSDPPara.arrs8RTSPUrl,tmp,strlen(tmp));
    }

}

static int IPC_RTSP_ERRHandle(RTSP_STATUS_S *pstRTSPClient,char *ps8Buf)
{
    char *p;
    char *tmp;
    int i;
    if(NULL == strstr(ps8Buf,"401"))
    {
        return 0;
    }
    p = strstr(ps8Buf,"realm=");
    if(NULL == p)
    {
        return -1;
    }
    tmp = p+7;
    for(i = 0;NULL != tmp && '"' != tmp[i] && i < 32;i++)
    {
        pstRTSPClient->stSDPPara.arrs8Realm[i] = tmp[i];
    }
    p = strstr(ps8Buf,"nonce=");
    if(NULL == p)
    {
        return -1;
    }
    tmp = p+7;
    for(i = 0;NULL != tmp && '"' != tmp[i] && i < 128;i++)
    {
        pstRTSPClient->stSDPPara.arrs8Nonce[i] = tmp[i];
    }
    return 0;
}


static char *IPC_RTSP_StreamHandle(char *ps8Buf,int s32StreamType)
{
    char *sub = NULL;
    sub = strstr(ps8Buf,"subtype=");
    if(NULL != sub)
    {
        if(SUBSTREAM == s32StreamType)
        {
              *(sub + 8) = '1';
        }
        else if(MAINSTREAM == s32StreamType){
            *(sub + 8) = '0';
        }
        return ps8Buf;
    }
    sub = strstr(ps8Buf,"stream=");
    if(NULL != sub)
    {
        if(SUBSTREAM == s32StreamType)
        {
              *(sub + 7) = '1';
        }
        else if(MAINSTREAM == s32StreamType){
            *(sub + 7) = '0';
        }
        return ps8Buf;
    }
    else if(NULL != (sub = strstr(ps8Buf,"Channels/")) || NULL != (sub = strstr(ps8Buf,"channels/")))
    {
        sub += 9;
        while('/' != *sub && '\0' != *sub)
        {
            sub++;
        }
        if(SUBSTREAM == s32StreamType)
        {
            *(--sub) +=1;
        }
    }
    else if(NULL != (sub = strstr(ps8Buf,"stream")))
    {
        if((*(sub + 6) >= '0') && (*(sub + 6) <= '9')) 
        {
            if(SUBSTREAM == s32StreamType)
            {
                *(sub + 6) = '2';
            }
            else if(MAINSTREAM == s32StreamType)
            {
                *(sub + 6) = '1';
            }
        }
        return ps8Buf;
    }
    else if(NULL != (sub = strstr(ps8Buf,"/1/1")))
    {
        if(SUBSTREAM == s32StreamType)
        {
            sub+=3;
            *sub +=1;
        }
    }
    
    return ps8Buf;
}

static int IPC_RTSP_DESCRIBEHandle(RTSP_STATUS_S *stRTSPClient,char *ps8Buf,int s32Type)
{
    char l_arrs8Author[32]={0};
    unsigned char l_arru8Response[33]={0};
	
	bzero(ps8Buf,BUF_LEN_1024_BYTE);

    switch(s32Type)
    {
        case 0:
        {
			sprintf(ps8Buf,"DESCRIBE %s RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nAccept: application/h264\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount);
			break;
		}

        case 1:
        {
			COM_authorization_digest(stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord,
				stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,l_arru8Response,"DESCRIBE",
				stRTSPClient->arrs8RTSPUrl,NULL,0);
			sprintf(ps8Buf,"DESCRIBE %s RTSP/1.0\r\nCSeq: %d\r\nAuthorization: Digest username=\"%s\", realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\nUser-Agent: Caibiao_Lee\r\nAccept: application/sdp\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,stRTSPClient->stClientUser.arrs8Name,\
				stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,stRTSPClient->arrs8RTSPUrl,l_arru8Response);
			break;
		}

        case 2:
        {
			sprintf(ps8Buf,"%s:%s",stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord);
			COM_base64_bits_to_64((unsigned char *)l_arrs8Author,(unsigned char *)ps8Buf,strlen(ps8Buf));
			sprintf(ps8Buf,"DESCRIBE %s RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nAccept: application/sdp\r\nAuthorization: Basic %s\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,l_arrs8Author);			
			break;
		}

        default:
            s32Type = -1;
        break;
    }
	
    return s32Type;
}


static int IPC_RTSP_OPTIONSHandle(RTSP_STATUS_S *stRTSPClient,char *ps8buf,int s32Type)
{
	char l_s32Author[32] = {0};
	unsigned char l_s32Response[33] = {0};

	bzero(ps8buf,BUF_LEN_1024_BYTE);

	printf("s32Type =  %d \n",s32Type);
	
    switch(s32Type)
    {
        case 0:
        {
			sprintf(ps8buf,"OPTIONS %s RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount);
			break;
		}

        case 1:
        {
			COM_authorization_digest(stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord,\
				stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,l_s32Response,"OPTIONS",\
				stRTSPClient->arrs8RTSPUrl,NULL,0);
			sprintf(ps8buf,"OPTIONS %s RTSP/1.0\r\nCSeq: %d\r\nAuthorization: Digest username=\"%s\", realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,stRTSPClient->arrs8RTSPUrl,l_s32Response);
			break;
		}

        case 2:
        {
			sprintf(ps8buf,"%s:%s",stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord);
			printf("name = %s password = %s buf = %s \n",
				stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord,ps8buf);
			COM_base64_bits_to_64((unsigned char *)l_s32Author,(unsigned char *)ps8buf,strlen(ps8buf));
			sprintf(ps8buf,"OPTIONS %s RTSP/1.0\r\nCSeq: %d\r\nAuthorization: Basic %s\r\n\r\n",stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,l_s32Author);
			break;
		}

		default:
            s32Type = -1;
        break;
    }
    return s32Type;
	
}

static int IPC_RTSP_TEARDOWNHandle(RTSP_STATUS_S *stRTSPClient,char *ps8Buf,int s32tmp,char *pRTSPSessionId)
{
    char l_s32author[32] = {0};
    unsigned char l_arru8Response[33]={0};
	
	bzero(ps8Buf,BUF_LEN_1024_BYTE);
	
    switch(s32tmp)
    {
        case 0:
        {
			sprintf(ps8Buf,"TEARDOWN %s/ RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nSession: %s\r\nRange: npt=0.000-\r\n\r\n",stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,pRTSPSessionId);
			break;
		}
		
        case 1:
		{
			COM_authorization_digest(stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord,stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,l_arru8Response,"TEARDOWN",stRTSPClient->arrs8RTSPUrl,NULL,0);
			sprintf(ps8Buf,"TEARDOWN %s/ RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nAuthorization: Digest username=\"%s\", realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\nUser-Agent: Caibiao_Lee\r\nSession: %s\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,stRTSPClient->stClientUser.arrs8Name,\
				stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,stRTSPClient->arrs8RTSPUrl,\
				l_arru8Response,pRTSPSessionId);
			break;
		}
		
        case 2:
        {
			sprintf(ps8Buf,"%s:%s",stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord);
			COM_base64_bits_to_64((unsigned char *)l_s32author,(unsigned char *)ps8Buf,strlen(ps8Buf));
			sprintf(ps8Buf,"TEARDOWN %s/ RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nSession: %s\r\nRange: npt=0.000-\r\nAuthorization: Basic %s\r\n\r\n",
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,pRTSPSessionId,l_s32author);
			break;

		}

        default:
        {
			printf("%s,%d,switch is default\n",__FUNCTION__,__LINE__);
			break;
			s32tmp = -1;
		}
    }
    return s32tmp;
}

static int IPC_RTSP_SETUPHandle(RTSP_STATUS_S *stRTSPClient,char *ps8Buf,int s32Type,int localport)
{
	char l_arrs8Author[32] = {0};
	unsigned char l_arru8Response[33]={0};

	bzero(ps8Buf,BUF_LEN_1024_BYTE);
	
    switch(s32Type)
    {
        case 0:
        {
	        sprintf(ps8Buf,"SETUP %s/%s RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nTransport:RTP/AVP;unicast;client_port=%d-%d\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,stRTSPClient->stSDPPara.arrs8Control,\
				gs_s32CseqCount,localport,localport+1);
			break;
		}

/**for debug h264**/
#if 0        
        case 1:
        {
			COM_authorization_digest(stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord,stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,l_arru8Response,"SETUP",stRTSPClient->arrs8RTSPUrl,NULL,0);
			sprintf(ps8Buf,"SETUP %s/%s RTSP/1.0\r\nCSeq: %d\r\nAuthorization: Digest username=\"%s\", realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\nUser-Agent: Caibiao_Lee\r\nTransport:RTP/AVP;unicast;client_port=%d-%d\r\n\r\n",
				stRTSPClient->arrs8RTSPUrl,stRTSPClient->stSDPPara.arrs8Control,\
				gs_s32CseqCount,stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stSDPPara.arrs8Realm,\
				stRTSPClient->stSDPPara.arrs8Nonce,stRTSPClient->arrs8RTSPUrl,\
				l_arru8Response,localport,localport+1);
			
			break;
		}
#else
        case 1:
        {
            COM_authorization_digest(stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord,stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,l_arru8Response,"SETUP",stRTSPClient->arrs8RTSPUrl,NULL,0);
            sprintf(ps8Buf,"SETUP %s/trackID=1 RTSP/1.0\r\nCSeq: %d\r\nAuthorization: Digest username=\"%s\", realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"\r\nUser-Agent: Caibiao_Lee\r\nTransport:RTP/AVP;unicast;client_port=%d-%d\r\n\r\n",
                stRTSPClient->arrs8RTSPUrl,\
                gs_s32CseqCount,stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stSDPPara.arrs8Realm,\
                stRTSPClient->stSDPPara.arrs8Nonce,stRTSPClient->arrs8RTSPUrl,\
                l_arru8Response,localport,localport+1);
            
            break;
        }

#endif
        case 2:
        {
			sprintf(ps8Buf,"%s:%s",stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord);
			COM_base64_bits_to_64((unsigned char *)l_arrs8Author,(unsigned char *)ps8Buf,strlen(ps8Buf));
			sprintf(ps8Buf,"SETUP %s/%s RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nTransport:RTP/AVP;unicast;client_port=%d-%d\r\nAuthorization: Basic %s\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,stRTSPClient->stSDPPara.arrs8Control,\
				gs_s32CseqCount,localport,localport+1,l_arrs8Author);
			
			break;
		}
        default:
        {
			s32Type = -1;
			break;
		}
    }
	
    return s32Type;
}

static int IPC_RTSP_PLAYHandle(RTSP_STATUS_S *stRTSPClient,char *ps8Buf,int s32Type,char *RTSPSessionId)
{
    bzero(ps8Buf,BUF_LEN_1024_BYTE);
    char l_arrs8Author[32] = {0};
    unsigned char l_arru8Response[33]={0};
    switch(s32Type)
    {
        case 0:
        {
			sprintf(ps8Buf,"PLAY %s/ RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nSession: %s\r\nRange: npt=0.000-\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,RTSPSessionId);
			break;

		}
		case 1:
        {
			COM_authorization_digest(stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord,stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,l_arru8Response,"PLAY",stRTSPClient->arrs8RTSPUrl,NULL,0);
			sprintf(ps8Buf,"PLAY %s/ RTSP/1.0\r\nCSeq: %d\r\nAuthorization: Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\nUser-Agent: Caibiao_Lee\r\nSession: %s\nRange: npt=0.000-\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,stRTSPClient->stClientUser.arrs8Name,\
				stRTSPClient->stSDPPara.arrs8Realm,stRTSPClient->stSDPPara.arrs8Nonce,stRTSPClient->arrs8RTSPUrl,\
				l_arru8Response,RTSPSessionId);
			break;
		}
        case 2:
        {
			sprintf(ps8Buf,"%s:%s",stRTSPClient->stClientUser.arrs8Name,stRTSPClient->stClientUser.arrs8PassWord);
			COM_base64_bits_to_64((unsigned char *)l_arrs8Author,(unsigned char *)ps8Buf,strlen(ps8Buf));
			sprintf(ps8Buf,"PLAY %s/ RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: Caibiao_Lee\r\nSession: %s\r\nRange: npt=0.000-\r\nAuthorization: Basic %s\r\n\r\n",\
				stRTSPClient->arrs8RTSPUrl,gs_s32CseqCount,RTSPSessionId,l_arrs8Author);
			break;
		}
        default:
        {
			s32Type = -1;
			break;
		}
    }
	
    return s32Type;
}

int RTSP_Client_Init(RTSP_STATUS_S * stRTSPClient)
{
	memset((unsigned char*)stRTSPClient,0,sizeof(RTSP_STATUS_S));
	stRTSPClient->bRTSPState = false;
	return 0;
}

int RTSP_Client_Release(RTSP_STATUS_S * stRTSPClient)
{
	if(NULL==stRTSPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}

	if(stRTSPClient->s32SockFd > 0)
	{
		close(stRTSPClient->s32SockFd);
		stRTSPClient->s32SockFd = -1;
	}
	stRTSPClient->bRTSPState = false;
	
	return 0;
}

int RTSP_Client_Session(RTSP_STATUS_S *stRTSPClient)
{
	bool l_bRTSPState;
	unsigned short l_u16Port = 0;
	int l_s32Ret = 0;
	int l_s32RtspSockfd = -1;

	if(NULL==stRTSPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	l_bRTSPState    = stRTSPClient->bRTSPState;
	l_s32RtspSockfd = stRTSPClient->s32SockFd;

	if(true==stRTSPClient->bRTSPState)
	{
		printf("%s %d RTSP is already init error \n",__FUNCTION__,__LINE__);
		return -1;
	}

	if(l_s32RtspSockfd > 0)
	{
		close(l_s32RtspSockfd);
	}
	l_s32RtspSockfd = NET_SocketCreate(SOCK_STREAM);
	if(l_s32RtspSockfd < 0)
	{
		printf("%s %d:create socket is error \n",__FUNCTION__,__LINE__);
		return -3;
	}

	l_u16Port = IPC_RTSP_GetPort(stRTSPClient->arrs8RTSPUrl);
	l_u16Port = (l_u16Port>0)?(l_u16Port):(RTSP_DEFAULT_PORT);
	printf("RTSP Server IP=%s,ServerPort=%d \n",stRTSPClient->arrs8ServerIP,l_u16Port);

	l_s32Ret = NET_SocketConnect(l_s32RtspSockfd,stRTSPClient->arrs8ServerIP,l_u16Port);
    if(l_s32Ret < 0)
    {
        printf("%s %d :connect to Server error \n",__FUNCTION__,__LINE__);
        return -4;
    }
	
	stRTSPClient->s32SockFd = l_s32RtspSockfd;

	return 0;
}

int RTSP_Client_OPTIONS(RTSP_STATUS_S *pstRTSPClient)
{
	int i;
	int l_s32Ret;
	int l_s32RtspSocketFd;
	bool l_bRTSPState;
	char l_arrs8Buf[BUF_LEN_1024_BYTE];

	if(NULL==pstRTSPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}

	if(true==pstRTSPClient->bRTSPState)
	{
		printf("%s %d RTSP is already start  error \n",__FUNCTION__,__LINE__);
		return -1;
	}

	l_bRTSPState       = pstRTSPClient->bRTSPState;
	l_s32RtspSocketFd  = pstRTSPClient->s32SockFd;

	pstRTSPClient->stSDPPara.s32AuthorType = 0;
	bzero(l_arrs8Buf,BUF_LEN_1024_BYTE); 
	
	for(i=0;i< 2;i++)
	{ 
		IPC_RTSP_OPTIONSHandle(pstRTSPClient,l_arrs8Buf,i);
		l_s32Ret = NET_SocketSendData(l_s32RtspSocketFd,l_arrs8Buf,strlen(l_arrs8Buf));
		if(l_s32Ret < 0)
		{
			printf("%s,%d :Send Data error \n",__FUNCTION__,__LINE__);
			return -2;
		}
		
		printf("[%s %d:] Send data:\n%s\nl_s32Ret=%d\n",__FUNCTION__,__LINE__,l_arrs8Buf,l_s32Ret);

		bzero(l_arrs8Buf,BUF_LEN_1024_BYTE);
		l_s32Ret = IPC_RTSP_RecvSelect(l_s32RtspSocketFd,l_arrs8Buf,BUF_LEN_1024_BYTE);
		if(l_s32Ret <= 0)
		{
			printf("[%s %d:] RTSP recv data error \n",__FUNCTION__,__LINE__);
			gs_s32CseqCount++;
			continue;

		}else
		{
			printf("[%s %d:] Recv Data: %s \n",__FUNCTION__,__LINE__,l_arrs8Buf);
		}
		
		l_s32Ret = IPC_RTSP_CheckReply(l_arrs8Buf);
		if(0 != l_s32Ret)
		{
			printf("biao debug [%s %d:] l_s32Ret = %d  \n",__FUNCTION__,__LINE__,l_s32Ret);
			IPC_RTSP_ERRHandle(pstRTSPClient,l_arrs8Buf);
			gs_s32CseqCount++;
			continue;
		}else
		{
			printf("biao debug [%s %d:] recv OK \n",__FUNCTION__,__LINE__);
		}
		gs_s32CseqCount++;
		pstRTSPClient->stSDPPara.s32AuthorType = i;
		return 0;
	}
	return -1;

}

int RTSP_Client_DESCRIBE(RTSP_STATUS_S *pstRTSPClient)
{
	bool l_bRTSPState;
	char *l_ps8Sub;
	char l_arrs8Buf[BUF_LEN_1024_BYTE];
	int i;
	int l_s32Ret;
	int l_s32RtspsocketFd;

	if(NULL==pstRTSPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}

	if(true==pstRTSPClient->bRTSPState)
	{
		printf("%s %d RTSP is already start  error \n",__FUNCTION__,__LINE__);
		return -1;
	}

	l_bRTSPState  = pstRTSPClient->bRTSPState;
	l_s32RtspsocketFd = pstRTSPClient->s32SockFd;

	for(i=pstRTSPClient->stSDPPara.s32AuthorType;i < 2;i++)
	{
		IPC_RTSP_DESCRIBEHandle(pstRTSPClient,l_arrs8Buf,i); 
        l_ps8Sub = COM_get_substringstart(l_arrs8Buf,"subtype=");
        if(NULL != l_ps8Sub)
        {
          l_ps8Sub += 8;
        }
		
        if(SUBSTREAM==pstRTSPClient->s32StreamType)
        {
              *l_ps8Sub = '1';
        }
        else if(MAINSTREAM== pstRTSPClient->s32StreamType)
		{
            *l_ps8Sub = '0';
        }
		
        l_s32Ret = NET_SocketSendData(l_s32RtspsocketFd,l_arrs8Buf,strlen(l_arrs8Buf));
        if(l_s32Ret < 0)
        {
            printf("%s %d: Send Data Error \n",__FUNCTION__,__LINE__);
            return -2;
        }
		
		printf("[%s %d:] Send data:\n%s\nl_s32Ret=%d\n",__FUNCTION__,__LINE__,l_arrs8Buf,l_s32Ret);

        bzero(l_arrs8Buf,BUF_LEN_1024_BYTE);

        l_s32Ret = IPC_RTSP_RecvSelect(l_s32RtspsocketFd,l_arrs8Buf,BUF_LEN_1024_BYTE);
        if(l_s32Ret <= 0)
        {
            printf("%s %d : Recv Data Error \n",__FUNCTION__,__LINE__);
			gs_s32CseqCount++;
			return -3;
        }else
		{
			printf("[%s %d:] Recv Data: %s \n",__FUNCTION__,__LINE__,l_arrs8Buf);
		}

        l_s32Ret = IPC_RTSP_CheckReply(l_arrs8Buf);
        if(0 != l_s32Ret)
        {
            IPC_RTSP_ERRHandle(pstRTSPClient,l_arrs8Buf);
			gs_s32CseqCount++;
			return -4;
        }
		else
		{
			printf("biao debug [%s %d:] recv OK \n",__FUNCTION__,__LINE__);
		}
        IPC_RTSP_SDPAnalyze(pstRTSPClient,l_arrs8Buf);
        gs_s32CseqCount++;
        pstRTSPClient->stSDPPara.s32AuthorType = i;
        return 0;
    }

    return -5;
}

int RTSP_Client_SETUP(RTSP_STATUS_S *pstRTSPClient)
{
	bool l_bRTSPState;
	char l_arrs8RTSPSessionId[16];
	char l_arrs8RTPPort[12];
	char l_arrs8Buf[BUF_LEN_1024_BYTE];
	char *l_ps8Sub;
	char l_s8Flag = ';';
	char l_arrs8SPFlagStr[] = "server_port=";
	int i;
	int l_s32Len;
	int localport = RTP_CLIENT_PORT;
	int l_s32Ret;
	int l_s32RtspsocketFd;
	char *l_ps8RTSPSessionId;
   
	if(NULL==pstRTSPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}

	if(true==pstRTSPClient->bRTSPState)
	{
		printf("%s %d RTSP is already start  error \n",__FUNCTION__,__LINE__);
		return -1;
	}

	l_bRTSPState       = pstRTSPClient->bRTSPState;
	l_s32RtspsocketFd  = pstRTSPClient->s32SockFd;
	l_ps8RTSPSessionId = pstRTSPClient->arrs8SessionId;

	for(i=pstRTSPClient->stSDPPara.s32AuthorType;i <3;i++)
	{
        IPC_RTSP_StreamHandle(pstRTSPClient->arrs8RTSPUrl,pstRTSPClient->s32StreamType);
        IPC_RTSP_SETUPHandle(pstRTSPClient,l_arrs8Buf,i,localport);
 
        l_s32Ret = NET_SocketSendData(l_s32RtspsocketFd,l_arrs8Buf,strlen(l_arrs8Buf));
        if(l_s32Ret < 0)
        {
            printf("%s %d: Send Data Error \n",__FUNCTION__,__LINE__);
            return -2;
        }

		printf("[%s %d:] Send data:\n%s\nl_s32Ret=%d\n",__FUNCTION__,__LINE__,l_arrs8Buf,l_s32Ret);
        bzero(l_arrs8Buf,BUF_LEN_1024_BYTE);
        l_s32Ret = IPC_RTSP_RecvSelect(l_s32RtspsocketFd,l_arrs8Buf,BUF_LEN_1024_BYTE);
        if(l_s32Ret <= 0)
        {
            printf("%s %d : Recv Data Error \n",__FUNCTION__,__LINE__);
			gs_s32CseqCount++;
			return -3;
        }else
		{
			printf("[%s %d:] Recv Data: %s \n",__FUNCTION__,__LINE__,l_arrs8Buf);
		}

        l_s32Ret = IPC_RTSP_CheckReply(l_arrs8Buf);
        if(0 != l_s32Ret)
        {
			gs_s32CseqCount++;
			return -4;
        }
		else
		{
			printf("biao debug [%s %d:] recv OK \n",__FUNCTION__,__LINE__);
		}
        l_ps8Sub = COM_get_substringstart(l_arrs8Buf,"Session:");
        if(NULL != l_ps8Sub)
        {
          l_ps8Sub += 8;
        }
        while(NULL != l_ps8Sub && ' ' == *l_ps8Sub)
        {
            l_ps8Sub++;
        }

        bzero(l_arrs8RTSPSessionId,16);

        COM_get_begingstring(l_ps8Sub,l_arrs8RTSPSessionId,l_s8Flag);
        
        l_ps8Sub = COM_get_substringstart(l_arrs8Buf,l_arrs8SPFlagStr);
        if(NULL != l_ps8Sub)
        {
          l_ps8Sub += 12;
        }
        bzero(l_arrs8RTPPort,12);
        COM_get_begingstring(l_ps8Sub,l_arrs8RTPPort,'-');
		bzero(pstRTSPClient->arrs8SessionId,16);
		sprintf(pstRTSPClient->arrs8SessionId,"%s",l_arrs8RTSPSessionId);
		bzero(pstRTSPClient->arrs8SerRTPPort,12); /**RTP 为偶数**/
        bzero(pstRTSPClient->arrs8SerRTCPPort,12);/**RTCP 端口号为奇数**/
		sprintf(pstRTSPClient->arrs8SerRTCPPort,"%s",l_arrs8RTPPort);
		l_s32Len = strlen(pstRTSPClient->arrs8SerRTCPPort);
		l_s32Len = l_s32Len -1;
        strncpy(pstRTSPClient->arrs8SerRTPPort,pstRTSPClient->arrs8SerRTCPPort,sizeof(pstRTSPClient->arrs8SerRTPPort)); 
        pstRTSPClient->arrs8SerRTCPPort[l_s32Len] = pstRTSPClient->arrs8SerRTPPort[l_s32Len]+1;






        gs_s32CseqCount++;
        return 0;
    }
    return -5;

}
int RTSP_Client_PLAY(RTSP_STATUS_S *pstRTSPClient)
{
	char l_arrs8Buf[BUF_LEN_1024_BYTE];
	char *l_ps8RTSPSessionId;
	int i;
	int l_s32Ret;
	int l_s32RtspsocketFd;
	bool l_bRTSPState;

	if(NULL==pstRTSPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}

	if(true==pstRTSPClient->bRTSPState)
	{
		printf("%s %d RTSP is already start  error \n",__FUNCTION__,__LINE__);
		return -2;
	}

	l_bRTSPState       = pstRTSPClient->bRTSPState;
	l_s32RtspsocketFd  = pstRTSPClient->s32SockFd;
	l_ps8RTSPSessionId = pstRTSPClient->arrs8SessionId;
	
	for(i=pstRTSPClient->stSDPPara.s32AuthorType;i < 3;i++)
	{
		 IPC_RTSP_StreamHandle(pstRTSPClient->arrs8RTSPUrl,pstRTSPClient->s32StreamType);
		 IPC_RTSP_PLAYHandle(pstRTSPClient,l_arrs8Buf,i,l_ps8RTSPSessionId);
		 l_s32Ret = NET_SocketSendData(l_s32RtspsocketFd,l_arrs8Buf,strlen(l_arrs8Buf));
		 if(l_s32Ret < 0)
		 {
			 printf("%s %d:Send Data Error \n",__FUNCTION__,__LINE__);
			 return -3;
		 }
		 
		 printf("[%s %d:] Send data:\n%s\nl_s32Ret=%d\n",__FUNCTION__,__LINE__,l_arrs8Buf,l_s32Ret);

		 bzero(l_arrs8Buf,BUF_LEN_1024_BYTE);
		 l_s32Ret = IPC_RTSP_RecvSelect(l_s32RtspsocketFd,l_arrs8Buf,BUF_LEN_1024_BYTE);
		 if(l_s32Ret <= 0)
		 {
			 printf("%s %d Recv Select Error\n",__FUNCTION__,__LINE__);

			gs_s32CseqCount++;
			return -4;
		 }else
		 {
			printf("[%s %d:] Recv Data: %s \n",__FUNCTION__,__LINE__,l_arrs8Buf);
		 }

		 l_s32Ret = IPC_RTSP_CheckReply(l_arrs8Buf);
		 if(0 != l_s32Ret)
		 {
			 gs_s32CseqCount++;
			 return -5;
		 }
		 else
		 {
			 printf("biao debug [%s %d:] recv OK \n",__FUNCTION__,__LINE__);
		 }

		 gs_s32CseqCount++;

		 pstRTSPClient->bRTSPState = true;
	 
		 return 0;
	 }
	 return -1;

}

int RTSP_Client_TEARDOWN(RTSP_STATUS_S *pstRTSPClient)
{
    char l_arrs8Buf[BUF_LEN_1024_BYTE];
    int l_s32Ret;
    int i;
    int l_s32RtspsocketFd;
    char *l_ps8RTSPSessionId;

	if(NULL==pstRTSPClient)
	{
		printf("%s %d input para error\n",__FUNCTION__,__LINE__);
		return -1;
	}
    
    if((pstRTSPClient->s32SockFd<0)||(false==pstRTSPClient->bRTSPState))
    {
        printf("%s %d RTCP Status Error \n",__FUNCTION__,__LINE__);
        return -2;
    }

	l_s32RtspsocketFd  = pstRTSPClient->s32SockFd;
	l_ps8RTSPSessionId = pstRTSPClient->arrs8SessionId;

    bzero(l_arrs8Buf,BUF_LEN_1024_BYTE);    

    for(i=pstRTSPClient->stSDPPara.s32AuthorType;i<3;i++)
    {
        IPC_RTSP_StreamHandle(pstRTSPClient->arrs8RTSPUrl,pstRTSPClient->s32StreamType);
        IPC_RTSP_TEARDOWNHandle(pstRTSPClient,l_arrs8Buf,i,l_ps8RTSPSessionId);
        l_s32Ret = NET_SocketSendData(l_s32RtspsocketFd,l_arrs8Buf,strlen(l_arrs8Buf));
        if(l_s32Ret < 0)
        {
            printf("%s %d:Send Data Error \n",__FUNCTION__,__LINE__);
            return -2;
        }

		printf("[%s %d:] Send data:\n%s\nl_s32Ret=%d\n",__FUNCTION__,__LINE__,l_arrs8Buf,l_s32Ret);
		
        bzero(l_arrs8Buf,BUF_LEN_1024_BYTE);
        l_s32Ret = IPC_RTSP_RecvSelect(l_s32RtspsocketFd,l_arrs8Buf,BUF_LEN_1024_BYTE);
        if(l_s32Ret <= 0)
        {
            printf("%s %d: Recv Select Error\n",__FUNCTION__,__LINE__);
            return -3;
        }else
        {
			printf("[%s %d:] Recv Data: %s \n",__FUNCTION__,__LINE__,l_arrs8Buf);
		}
		
        l_s32Ret = IPC_RTSP_CheckReply(l_arrs8Buf);
        if(0 != l_s32Ret)
        {
            printf("%s %d Check Replay Error \n",__FUNCTION__,__LINE__);
            return -4;
        }
        gs_s32CseqCount++;

		pstRTSPClient->bRTSPState = true;
        return 0;
    }
	
    return -1;
}


