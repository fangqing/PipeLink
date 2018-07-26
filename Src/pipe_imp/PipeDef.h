#pragma once
#define  IOREQ_RECV		0x01
#define  IOREQ_SEND		0x02
#define  IOREQ_ACCEPT	0x04   //Server����Client������

#define  PIPE_TIMEOUT 2000

#define  IOBUF_MAXSIZE	1024
#define	 BUFSIZE		128

#define EVT_ESTABLISH	1	//����
#define EVT_CLOSED		2	//����
#define EVT_RECEIVEDATA 3	//��������
#define EVT_ERRPROTOCOL 4   //�������ݳ���

#define STATUS_RECV_BEGIN		0x0001		//��ʼ����
#define STATUS_RECV_MIDDLE		0x0002		//���ڽ���
#define STATUS_RECV_END			0x0003		//�������

typedef unsigned long HLINK;
typedef void(*EventCallBack)(void* pParam1, void* pParam2);

struct  LinkEvent
{
	unsigned int nEvent;
	HLINK hLink;
};

struct LinkRecvDataParam
{
	unsigned int nStatus;	//��������״̬
	char* pRecvData;		//��������Buf
	unsigned int cbSize;	//�����ݰ��ֽڴ�С
	unsigned int nIndex;	//��ǰ�����ֽ���
};

struct SOvlpIOContext
{
	OVERLAPPED ovlp;
	BYTE io_type;
	HANDLE hPipe;
	DWORD dwBytes;  //��д���ֽ���
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
