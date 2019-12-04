/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: common.c
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: 
*Date:	   2019-06-22
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include "common.h"

static const char base64digits[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BAD     -1  
#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)  
static const signed char base64val[] = 
{
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
    52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
    15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
    BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
    41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};

void COM_base64_bits_to_64(unsigned char *out, const unsigned char *in, int inlen)
{
    for (; inlen >= 3; inlen -= 3)
    {
        *out++ = base64digits[in[0] >> 2];
        *out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
        *out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
        *out++ = base64digits[in[2] & 0x3f];
        in += 3;
    }

    if (inlen > 0)
    {
        unsigned char fragment;

        *out++ = base64digits[in[0] >> 2];
        fragment = (in[0] << 4) & 0x30;

        if (inlen > 1)
            fragment |= in[1] >> 4;

        *out++ = base64digits[fragment];
        *out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
        *out++ = '=';
    }

    *out = '\0';
}

int COM_base64_64_to_bits(char *out, const char *in)
{
    int len = 0;
    register unsigned char digit1, digit2, digit3, digit4;

    if (in[0] == '+' && in[1] == ' ')
        in += 2;
    if (*in == '\r')
        return(0);

    do {
        digit1 = in[0];
        if (DECODE64(digit1) == BAD)
            return(-1);
        digit2 = in[1];
        if (DECODE64(digit2) == BAD)
            return(-1);
        digit3 = in[2];
        if (digit3 != '=' && DECODE64(digit3) == BAD)
            return(-1);
        digit4 = in[3];
        if (digit4 != '=' && DECODE64(digit4) == BAD)
            return(-1);
        in += 4;
        *out++ = (DECODE64(digit1) << 2) | (DECODE64(digit2) >> 4);
        ++len;
        if (digit3 != '=')
        {
            *out++ = ((DECODE64(digit2) << 4) & 0xf0) | (DECODE64(digit3) >> 2);
            ++len;
            if (digit4 != '=')
            {
                *out++ = ((DECODE64(digit3) << 6) & 0xc0) | DECODE64(digit4);
                ++len;
            }
        }
    } while (*in && *in != '\r' && digit4 != '=');

    return (len);
}

int COM_authorization_digest(char *name,char *password,char *realm,char *nonce,\
      unsigned char *response,char *meth,char *addr,char *qop,int cntnonce)
{
	char tmp[33]={0};
	char buf[33]={};
	char url[33]={};
	unsigned char swp[256]={};
	unsigned char md[16];
	int i;
	int len;

	sprintf((char *)swp,"%s:%s:%s",name,realm,password);
	len = strlen((char *)swp);
	MD5(swp,len,md);
	bzero(buf,33);
	for(i = 0;i < 16;i++)
	{
	    sprintf(&buf[i * 2],"%02x",md[i]);
	    //strcat(buf,tmp);
	}
	bzero(swp,sizeof(swp));
	bzero(md,sizeof(md));
	sprintf((char *)swp,"%s:%s",meth,addr); 
	len = strlen((char *)swp);
	MD5(swp,len,md);
	bzero(url,33);
	for(i = 0;i < 16;i++)
	{
	    sprintf(&url[i*2],"%02x",md[i]);
	    //strcat(url,tmp);
	}
	bzero(swp,sizeof(swp));
	bzero(md,sizeof(md));
	if(NULL != qop)
	{
	    char *ClientNonce = HTTP_CNONCE;
	    char count[10] = {0};
	    for(i = 0;i < 8;i++)
	    {
	        count[7-i] =(0xff & ((0x1 & (cntnonce >> i)) + 30));
	    }
	    sprintf((char *)swp,"%s:%s:%s:%s:%s:%s",buf,nonce,count,ClientNonce,qop,url);
	}
	else
	{

	    sprintf((char *)swp,"%s:%s:%s",buf,nonce,url); 
	}
	len = strlen((char *)swp);
	MD5(swp,len,md);

	bzero(response,33);
	bzero(tmp,33);
	int j;
	for(i = 0,j=0;i < 16;i++,j = i * 2)
	{
	    sprintf(&tmp[j],"%02x",md[i]);
	}
	memcpy(response,tmp,33);

    return 0;
}

char *COM_get_substringstart(char *str,char *sub)
{
    char *strp;
    char *tmp = str;
    char *subp=sub;
    if(NULL == str || NULL == sub)
    {
        printf("%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    while('\0' != *tmp && '\0' != *subp)
    {
        strp = tmp;
        subp = sub;
        while('\0' != *subp)
        {
            if(*strp != *subp)
            {
                tmp++;
                break;
            }
            strp++;
            subp++;
        }
    }
    return tmp;
}

char *COM_get_begingstring(char *src,char *dest,char flag)
{
    char *srcp = src;
    char *destp = dest;
    
    if(NULL == srcp || NULL == destp)
    {
        printf("%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    while('\0' != *srcp && flag != *srcp && '\n' != *srcp)
    {
        *destp = *srcp;
         destp++;
         srcp++;
    }
    return dest;
}

char *COM_get_localIP(char *IP)
{
	int socket_fd;

	struct ifreq *ifr;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;
	int i;
	socket_fd = socket(AF_INET,SOCK_DGRAM,0);
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	ioctl(socket_fd,SIOCGIFCONF,&conf);
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;
	for(i=0;i<num;i++)
	{
	struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

	ioctl(socket_fd,SIOCGIFFLAGS,ifr);
	if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
	{
	  sprintf(IP,"%s",inet_ntoa(sin->sin_addr));
	}
	ifr++;
	if(NULL != strstr(IP,"192.168.20."))
	{
	    break;
	}

	}
	return IP;
}


void COM_genrate_digest(char *pwddigest_out,char *pwd,char *nonc, char *time)  
{

    const unsigned char *tdist;
    unsigned char dist[1024] = {0};
    char tmp[1024] = {0};
    unsigned char bout[1024] = {0};

    if(NULL == pwddigest_out || NULL == pwd || NULL == nonc || NULL == time)
    {  
        printf("%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
        return ;
    }

    strcpy(tmp,nonc);
    COM_base64_64_to_bits((char*)bout, tmp);

    bzero(tmp,1024);

    sprintf(tmp,"%s%s%s",bout,time,pwd);


    SHA1((const unsigned char*)tmp,strlen((const char*)tmp),dist);


    tdist = dist;

    memset(bout,0x0,1024);

    COM_base64_bits_to_64(bout,tdist,(int)strlen((const char*)tdist));

    strcpy((char *)pwddigest_out,(const char*)bout);

}

int NET_SocketCreate(int s32Type)
{
    int l_s32Sockfd = 0;
    l_s32Sockfd = socket(AF_INET,s32Type,0);
    if(l_s32Sockfd < 0)
    {
        printf("%s %d :create socket is faild\n",__FUNCTION__,__LINE__);
        return -1;
    }
    return l_s32Sockfd;
}

int NET_SocketConnect(int s32SocketFd,char *pstIP,int s32Port)
{
	int l_s32Ret;
	int l_s32error;

	int l_s32RecvBuf = 32*1024;		//默认设置为32K
	int l_s32SendBuf = 32*1024;     //默认设置为32K
	fd_set l_stRset;
	fd_set l_stWset;
	socklen_t l_s32Len;
	struct timeval l_stTimeval;
	struct sockaddr_in l_srSaddr;
	static int flags = 0;
	
	FD_ZERO(&l_stRset);
	FD_SET(s32SocketFd,&l_stRset);
	l_stWset = l_stRset;
	l_stTimeval.tv_sec  = 10;
	l_stTimeval.tv_usec = 0;

    bzero(&l_srSaddr,sizeof(struct sockaddr_in));
    l_srSaddr.sin_family = AF_INET;
    l_srSaddr.sin_port = htons(s32Port);
    l_srSaddr.sin_addr.s_addr = inet_addr(pstIP);
	setsockopt(s32SocketFd,SOL_SOCKET,SO_RCVBUF,(const char*)&l_s32RecvBuf,sizeof(int));
	setsockopt(s32SocketFd,SOL_SOCKET,SO_SNDBUF,(const char*)&l_s32SendBuf,sizeof(int));

	flags = fcntl(s32SocketFd,F_GETFD, 0); 
	fcntl(s32SocketFd,F_SETFD, flags|O_NONBLOCK);

    l_s32Ret = connect(s32SocketFd,(struct sockaddr *)&l_srSaddr,sizeof(struct sockaddr_in));
    if(l_s32Ret < 0)
    {
        perror("connect server:");
        printf("connect is faild:%s,%d,%d,%s,%d\n",__FILE__,__LINE__,s32SocketFd,pstIP,s32Port);
        return -1;
    }
    if((l_s32Ret = select(s32SocketFd+1, &l_stRset, &l_stWset, NULL,&l_stTimeval)) <= 0)
    {
        printf("TCP CONNECT: over time...\n");
        close(s32SocketFd);
        return -1;
    }
    if(FD_ISSET(s32SocketFd,&l_stRset) || FD_ISSET(s32SocketFd,&l_stWset) )
    {
        l_s32Len = sizeof(l_s32error);
        if(getsockopt(s32SocketFd,SOL_SOCKET,SO_ERROR,&l_s32error,&l_s32Len) <0)
        {
            printf("TCP CONNECT: error...\n");
            close(s32SocketFd);
            return -1;
        }
    }
    return 0;
}

int NET_SocketSendData(int s32SocketFd,char *ps8buf,int s32Size)
{
     int l_s32Ret;
	 if(NULL == ps8buf)
	 {
          printf("%s,%d buf is NULL\n",__FILE__,__LINE__);
          return -1;
	 }
     l_s32Ret = send(s32SocketFd,ps8buf,s32Size,0);
     if( l_s32Ret < 0)
     {
          printf("%s,%s,%d,SendFalse:%d\n",__FILE__,__FUNCTION__,__LINE__,s32SocketFd);
          return -1;
     }
     
     return l_s32Ret;
}

int NET_SocketRecvData(int s32SocketFd,void  *ps8buf,int s32Len)
{
	int l_s32Count = 3;
    ssize_t s32Size;
	while(l_s32Count-->0)
	{
		s32Size = recv(s32SocketFd,ps8buf,s32Len,0);
		if(s32Size==0)
		{
			usleep(300000);
			continue;
		}
		else if(s32Size < 0)
		{
			perror("recv err :");
			printf("[%s,%d:] s32SocketFd = %d, s32Size = %d\n",__FUNCTION__,__LINE__,s32SocketFd,(int)s32Size);
			return -1;
		}else if(s32Size > 0)
		{
			break;
		}
	}

    return s32Size; 
}


int NET_SocketClose(int s32SocketFd)
{
    close(s32SocketFd);

	return 0;
}

int NET_SocketBind(int s32SocketFd,int localport)
{
    int l_s32Ret;
    char IP[32]={0};
    struct sockaddr_in client;
    unsigned int value = 0x1;
    bzero(&client,sizeof(struct sockaddr_in));
    
    COM_get_localIP(IP);

    client.sin_family = AF_INET;

    client.sin_port = htons(localport);

    client.sin_addr.s_addr = inet_addr(IP);

    printf("bind===========%s,localport:%d,l_s32SocketFd:%d\n",IP,localport,s32SocketFd);
    l_s32Ret = setsockopt(s32SocketFd,SOL_SOCKET,SO_REUSEADDR,(void *)&value,sizeof(value)); 
    if(l_s32Ret < 0)
    {
        printf("%s,%s,%d,l_s32Ret:%d\n",__FILE__,__FUNCTION__,__LINE__,l_s32Ret);
        return -1;
    }

    l_s32Ret = bind(s32SocketFd,(struct sockaddr *)&client,sizeof(struct sockaddr_in));
    if(l_s32Ret < 0)
    {
        printf("%s,%s,%d,l_s32Ret:%d\n",__FILE__,__FUNCTION__,__LINE__,l_s32Ret);
        return -1;
    }
	
    return 0;
}

