#pragma once

//packet type
#define  IOREQ_RECV		0x01   //recv data
#define  IOREQ_SEND		0x02   //send data
#define  IOREQ_ACCEPT	0x04   //Server accept connection from client

#define  PIPE_TIMEOUT 2000

#define  IOBUF_MAXSIZE	1024
#define	 BUFSIZE		128

//event type
#define EVT_ESTABLISH	1	//pipe establish
#define EVT_CLOSED		2	//pipe close
#define EVT_RECEIVEDATA 3	//pipe receive data
#define EVT_ERR_PROTOCOL 4  //error ocurr when unpack data with custom protocol

//status type
#define STATUS_RECV_BEGIN		0x0001		
#define STATUS_RECV_MIDDLE		0x0002		
#define STATUS_RECV_END			0x0003		

typedef unsigned long HLINK;
typedef void(*EventCallBack)(void* pParam1, void* pParam2);

struct  LinkEvent
{
	unsigned int nEvent;
	HLINK hLink;
};

struct LinkRecvDataParam
{
	unsigned int nStatus;	//link status
	char* pRecvData;		//buffer for receiveing data 
	unsigned int cbSize;	//buffer size
	unsigned int nIndex;	//how much bytes has used in the buffer
};

struct SOvlpIOContext
{
	OVERLAPPED ovlp;
	BYTE io_type;
	HANDLE hPipe;
	DWORD dwBytes;  
	HLINK hLink;
};

struct  SOvlpSendContext :public SOvlpIOContext
{
	BYTE data[IOBUF_MAXSIZE];
	SOvlpSendContext()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(SOvlpSendContext));
	}
};

struct SOvlpRecvContext : public SOvlpIOContext
{
	BYTE data[IOBUF_MAXSIZE];
	SOvlpRecvContext()
	{
		Reset();
	}
	void Reset()
	{
		memset(this, 0, sizeof(SOvlpRecvContext));
	}
};

struct SOvlpAcceptContext : public SOvlpIOContext
{
	BYTE data[IOBUF_MAXSIZE];
	SOvlpAcceptContext()
	{
		Reset();
	}
	void Reset()
	{
		memset(this, 0, sizeof(SOvlpAcceptContext));
	}
};

#define PACK_MAGIC_NUMBER   0x12345678
 
struct SPacketHead
{
	DWORD headTag;				//maybe a magic number: PACK_MAGIC_NUMBER
	DWORD dwPacketLen;
	SPacketHead()
	{
		memset(this, 0, sizeof(SPacketHead));
	}
};
const int PACKET_HEAD_LENGTH = 8;
