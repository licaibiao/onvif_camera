/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*FileName: rtp_client.h
*BlogAddr: https://blog.csdn.net/li_wen01
*Description: RTP 协议 
*Date:	   2019-10-05
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#ifndef __RTP_CLIENT_H__
#define __RTP_CLIENT_H__

#include "common.h"
      

/*****************************************************************************************
                        RTP标准结构体定义开始
******************************************************************************************/


/***************************************************************
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source (SSRC) identifier            |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|            contributing source (CSRC) identifiers             |
|                             ....                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*****************************************************************/ 
/**RTP 头结构**/
typedef struct 
{
    /** byte 0 **/
    unsigned char bit4CsrcLen:4;        /** expect 0 **/
    unsigned char bit1Extension:1;      /** expect 1, see RTP_OP below **/
    unsigned char bit1Padding:1;        /** expect 0 **/
    unsigned char bit1Version:2;        /** expect 2 **/
    /** byte 1 **/
    unsigned char bit7PayLoadType:7;    /** RTP_PAYLOAD_RTSP **/
    unsigned char bit1Marker:1;         /** expect 1 **/
    /** bytes 2,3 **/
    unsigned int u32SeqNum;             /** RTP sequence number **/     
    /** bytes 4-7 **/
    unsigned int u32TimeStamp;          /** RTP sequence number **/
    /** bytes 8-11 **/
    unsigned int u32Ssrc;               /**stream number is used here **/
}RTP_HEADER_S;

/***************** 
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
*****************/
/**网络抽象层单元头结构体**/
typedef struct 
{
    unsigned char bit5TYPE:5;
    unsigned char bit2NRI:2;
    unsigned char bit1F:1;        
}RTP_NALU_HEADER_S;

/****************
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
*****************/
/**分片包指示符**/
typedef struct 
{
	unsigned char Bit5TYPE:5;
	unsigned char BitNRI:2; 
	unsigned char BitF:1;              
}RTP_FU_INDICATOR_S; 


/******************
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|R|  Type   |
+---------------+
*******************/
/**分片包头结构**/
typedef struct 
{
	unsigned char Bit5TYPE:5;
	unsigned char Bit1R:1;
	unsigned char Bit1E:1;
	unsigned char Bit1S:1;    
}RTP_FU_HEADER_S;



/**组合包头长度结构体**/
typedef struct 
{   
    unsigned short u16AUHeaderLen;
}RTP_AU_HEADER_LEN_S;


/**组合包头结构体**/
typedef struct 
{   
    unsigned short u16AUHeader;
}RTP_AU_HEADER_S;


/*****************************************************************************************
                        RTP标准结构体定义结束
******************************************************************************************/



/*****************************************************************************************
                        自定义结构体开始
******************************************************************************************/
/**RTP帧类型**/
typedef enum 
{
    NAL_PACK = 0,    /**单包**/
    AP_PACK,         /**组合包**/
    FU_START_PACK,   /**分片包开始**/
    FU_MIDllE_PACK,  /***分片包中间包*/
    FU_END_PACK,     /**分片包结束包**/
    H264_SEI,
    H264_PPS,
    H264_SPS,
    AUDIO_AAC,
}RTP_FRAME_TYPE; 

/**RTP 数据包结构体**/
typedef struct
{
    unsigned char u8Version;      /**Version, 2 bits, MUST be 0x2**/
    unsigned char u8Padding;	  /**Padding bit, Padding MUST NOT be used**/
    unsigned char u8Extension;    /**Extension, MUST be zero**/
    unsigned char u8Cc;       	  /**CSRC count, normally 0 in the absence of RTP mixers**/ 
    unsigned char u8Marker;	      /**Marker bit**/
    unsigned char u8Pt;			  /**7 bits, Payload Type, dynamically established**/
    unsigned int  u32SeqNum;	  /**RTP sequence number, incremented by one for each sent packet**/
    unsigned int  u32TimeStamp;	  /**timestamp, 27 MHz for H.264**/
    unsigned int  u32Ssrc;        /**Synchronization Source, chosen randomly**/
    unsigned char * pu8Payload;   /**the payload including payload headers**/
    unsigned int  u32Paylen;      /**length of payload in bytes**/
}RTP_PACKET_S;


/**RTP 数据解包**/
typedef struct 
{
    unsigned char *pu8InputDataAddr;  /**输入数据包地址**/
    unsigned int   u32InputDataLen;   /**输入数据包长度**/
    RTP_FRAME_TYPE eFrameType;        /**输出数据包类型**/
    unsigned char  u8OutNaluType;     /***SPS PPS SEI等类型**/
    RTP_PACKET_S   stOutRTPPack;      /**输出RTP数据包解析后的信息**/
}RTP_UNPACK_S;

/**RTP 客户端数据接收情况**/
typedef struct
{
    unsigned char u8Count[2];          /**序列号计数，统计packNum 溢出的次数**/
    unsigned char u8PacketNum[2];      /**包数计数**/
    unsigned char u8Lost;              /**丢包次数**/
    unsigned int  u32LostPackNum;      /**丢包数**/
}RTP_RECV_INFO_S;

/*****************************************************************************************
                                 自定义结构体结束
******************************************************************************************/
int RTP_Client_Init(RTP_STATUS_S *pstRTPClient);
int RTP_Client_Release(RTP_STATUS_S *pstRTPClient);
int RTP_CLient_Session(RTP_STATUS_S *pstRTPClient);
int RTP_Client_GetOnePacketData(RTP_STATUS_S *pstRTPClient);


#endif



