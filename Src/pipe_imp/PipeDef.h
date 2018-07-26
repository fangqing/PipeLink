#pragma once
#define  IOREQ_RECV		0x01
#define  IOREQ_SEND		0x02
#define  IOREQ_ACCEPT	0x04   //Server接受Client的连接

#define  PIPE_TIMEOUT 2000

#define  IOBUF_MAXSIZE	1024
#define	 BUFSIZE		128

#define EVT_ESTABLISH	1	//连接
#define EVT_CLOSED		2	//销毁
#define EVT_RECEIVEDATA 3	//接收数据
#define EVT_ERRPROTOCOL 4   //传输数据出错

#define STATUS_RECV_BEGIN		0x0001		//开始接收
#define STATUS_RECV_MIDDLE		0x0002		//正在接收
#define STATUS_RECV_END			0x0003		//接收完成

typedef unsigned long HLINK;
typedef void(*EventCallBack)(void* pParam1, void* pParam2);

struct  LinkEvent
{
	unsigned int nEvent;
	HLINK hLink;
};

struct LinkRecvDataParam
{
	unsigned int nStatus;	//接收数据状态
	char* pRecvData;		//接收数据Buf
	unsigned int cbSize;	//该数据包字节大小
	unsigned int nIndex;	//当前接收字节数
};

struct SOvlpIOContext
{
	OVERLAPPED ovlp;
	BYTE io_type;
	HANDLE hPipe;
	DWORD dwBytes;  //读写的字节数
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
