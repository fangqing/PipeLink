#include "stdafx.h"
#include "PipeLink.h"
#include "AllocContextMgr.h"
#include <algorithm>

extern bool g_listen_switch;
extern HANDLE g_listening_pipe;
extern HANDLE g_hIOCP;
extern EventCallBack g_pfnIOCB;

CPipeLink::CPipeLink()
{
	m_hPipe = INVALID_HANDLE_VALUE;
	m_pfnIOCB = nullptr;
	m_pRecv = new CDataBuf;
	pipeBroken = false;

	m_nMaxPacketSize = 1024 * 1024 * 10;		
	m_nMaxAllocContextCnt = m_nMaxPacketSize * 2 /IOBUF_MAXSIZE;

	InitPacketHead();
	m_bInvalidPacket = FALSE;
}

CPipeLink::~CPipeLink()
{
	if (m_pRecv != NULL)
	{
		delete m_pRecv;
		m_pRecv = NULL;
	}
	CGuardLock<CSLock> guard(&m_csSend);
	CSingletonAllocSend::Instance()->FreeContextList(m_lsSend);
	m_lsSend.clear();
}

CSLock CPipeLink::m_csLink;
CMapPipeHandleLink CPipeLink::m_mapLink;

CPipeLinkPtr CPipeLink::CreatePipeLink()
{
	CGuardLock<CSLock> guard(&m_csLink);
	static int nLink = 0;
	HLINK hLink = (++nLink) % 0x3fffffff;
	CPipeLinkPtr pLinkPtr(new CPipeLink());
	pLinkPtr->m_hLink = hLink;
	m_mapLink[hLink] = pLinkPtr;
	return pLinkPtr;
}

BOOL CPipeLink::FindLink(HLINK hLink, CPipeLinkPtr& pLink)
{
	CGuardLock<CSLock> guard(&m_csLink);
	CMapPipeHandleLink::iterator it = m_mapLink.find(hLink);
	if (it != m_mapLink.end())
	{
		pLink = m_mapLink[hLink];
		return TRUE;
	}
	return FALSE;
}

void CPipeLink::FreeAllLink()
{
	CGuardLock<CSLock> guard(&m_csLink);
	m_mapLink.clear();
}

void CPipeLink::AttachPipe(HANDLE hpipe)
{
	assert(m_hPipe == INVALID_HANDLE_VALUE);
	m_hPipe = hpipe;
	pipeBroken = false;
	//将m_hPipe绑定到iocp上,这样与m_hPipe 相关的读写操作就可以异步回调
	CreateIoCompletionPort(m_hPipe, g_hIOCP, (DWORD)IOCompletionProc, 0);
}

DWORD CPipeLink::Accept()
{
	DWORD dwFlag;
	GetNamedPipeInfo(m_hPipe, &dwFlag,NULL,NULL,NULL);
	assert((dwFlag&PIPE_SERVER_END) == PIPE_SERVER_END);

	SOvlpAcceptContext* pContext = CSingletonAllocAccept::Instance()->GetContext();
	pContext->Reset();
	InitContext(pContext);
	pContext->io_type = IOREQ_ACCEPT;
	BOOL bconnect = ConnectNamedPipe(pContext->hPipe, &pContext->ovlp);
	DWORD dwError = GetLastError();
	switch (dwError)
	{
	case ERROR_IO_PENDING:
		break;
	case ERROR_PIPE_CONNECTED:
		//若此时已经connected,则不会收到OnAccept()回调
		SendLinkEvent(EVT_ESTABLISH);
		StartAsyncRead();
		break;
	default:
		break;
	}
	return dwError;
}

void  CPipeLink::Accepted()
{
	SendLinkEvent(EVT_ESTABLISH);
	StartAsyncRead();
}

void CPipeLink::StartAsyncRead()
{
	CheckPipeState();
	DWORD bytesToRead = IOBUF_MAXSIZE;

	SOvlpRecvContext* pContext = CSingletonAllocRecv::Instance()->GetContext();
	InitContext(pContext);
	pContext->io_type = IOREQ_RECV;
	pContext->dwBytes = bytesToRead;
	DWORD dwRead;
	if (!ReadFile(pContext->hPipe, pContext->data, bytesToRead, &dwRead, &pContext->ovlp))
	{
		DWORD dwError = GetLastError();
		switch (dwError)
		{
		case ERROR_BROKEN_PIPE:
		case ERROR_PIPE_NOT_CONNECTED:
			pipeBroken = true;
			break;
		default:
			break;
		}
	}

	if (pipeBroken)
		Destroy();
}

DWORD CPipeLink::CheckPipeState()
{
	DWORD bytes;
	if (PeekNamedPipe(m_hPipe, NULL, 0, NULL, &bytes, NULL))
	{
		return bytes;
	}
	else
	{
		pipeBroken = true;
	}
	return 0;
}


void CPipeLink::StartAsyncSend()
{
	CGuardLock<CSLock> guard(&m_csSend);

	SOvlpSendContext* pNext = NULL;
	if (!m_lsSend.empty())
	{
		pNext = *m_lsSend.begin();
		m_lsSend.erase(m_lsSend.begin());
	}
	if (pNext != NULL)
	{
		DWORD dwRet = IssueSend(pNext);
		if (dwRet != 0 && dwRet != ERROR_IO_PENDING)
		{
			Destroy();
			return;
		}
	}
}
void CPipeLink::SendData(const char* pData, unsigned int cbSize)
{
	if (cbSize > m_nMaxPacketSize || cbSize == 0)
		return;
	BuildPacket(pData, cbSize);
	StartAsyncSend();
}

void CPipeLink::SendLinkEvent(int nEvent)
{
	if (m_pfnIOCB)
	{
		LinkEvent evt;
		evt.hLink = m_hLink;
		evt.nEvent = nEvent;
		(*m_pfnIOCB)(&evt, 0);
	}
}

void CPipeLink::SendRecvDataStatus(int nStatus)
{
	if (m_pfnIOCB)
	{
		LinkEvent evt;
		evt.hLink = m_hLink;
		evt.nEvent = EVT_RECEIVEDATA;
		
		LinkRecvDataParam param;
		param.nStatus = nStatus;
		param.pRecvData = m_pRecv->m_pszBuf;
		param.nIndex = m_pRecv->m_nIndex;
		param.cbSize = m_pRecv->m_cbBuf;
		(*m_pfnIOCB)(&evt, &param);
	}
}

void CPipeLink::Destroy()
{
	m_csLink.Lock();
	int  removed = m_mapLink.erase(m_hLink);
	m_csLink.Unlock();

	if (removed>0 && m_hPipe != INVALID_HANDLE_VALUE)
	{
		DWORD dwFlag;
		GetNamedPipeInfo(m_hPipe, &dwFlag, NULL, NULL, NULL);
		if ((dwFlag&PIPE_SERVER_END)== PIPE_SERVER_END)
			DisconnectNamedPipe(m_hPipe);
		CloseHandle(m_hPipe);
		SendLinkEvent(EVT_CLOSED);
		m_hPipe = INVALID_HANDLE_VALUE;
	}
}

void CPipeLink::InitPacketHead()
{
	m_nBreakIndex = 0;
	m_bHeadBreak = FALSE;
	m_bHeadRecvOK = FALSE;
	memset(&m_packHead, 0, sizeof(SPacketHead));
}

bool CPipeLink::BuildPacket(const char* pData, unsigned int nLen)
{
	CGuardLock<CSLock> guard(&m_csSend);
	int needContextCount = (nLen + PACKET_HEAD_LENGTH) / IOBUF_MAXSIZE + 1;

	//alloc context
	BOOL bAllocFailed = FALSE;
	CListSendContext lsAlloc;
	for (int i = 0; i<needContextCount; ++i)
	{
		SOvlpSendContext* pAlloc = CSingletonAllocSend::Instance()->GetContext();
		if (pAlloc == NULL)
		{
			bAllocFailed = TRUE;
			break;
		}
		lsAlloc.push_back(pAlloc);
	}
	if (bAllocFailed)
	{
		CSingletonAllocSend::Instance()->FreeContextList(lsAlloc);
		return false;
	}

	//copy data to context
	char* pCurPos = (char*)pData;
	int nLeft = nLen;
	//put the head info in first Context
	SOvlpSendContext *pFirstContext = *lsAlloc.begin();
	lsAlloc.pop_front();
	pFirstContext->Reset();
	InitContext(pFirstContext);
	SPacketHead* pHead = (SPacketHead*)pFirstContext->data;
	pHead->headTag = PACK_MAGIC_NUMBER;

	
	pHead->dwPacketLen = nLen;
	pFirstContext->dwBytes += PACKET_HEAD_LENGTH ;

	SOvlpSendContext* pContext = pFirstContext;
	while (nLeft > 0)
	{
		if (pContext == NULL)
		{
			pContext = *lsAlloc.begin();
			lsAlloc.pop_front();
			pContext->Reset();
			InitContext(pContext);
		}

		char *pDataStart = (char*)pContext->data + pContext->dwBytes;
		int nCopy = min(nLeft, (int)(IOBUF_MAXSIZE - pContext->dwBytes));
		if (nCopy > 0)
		{
			memcpy(pDataStart, pCurPos, nCopy);
			pCurPos += nCopy;
			pContext->dwBytes += nCopy;
			nLeft -= nCopy;
		}
		if (pContext->dwBytes == IOBUF_MAXSIZE ||nLeft == 0 || nCopy == 0)
		{
			m_lsSend.push_back(pContext);
			pContext = NULL;
		}
	}

	CSingletonAllocSend::Instance()->FreeContextList(lsAlloc);
	return true;
}

void CPipeLink::OnSend(SOvlpSendContext* pContext, int errorCode, DWORD numberOfBytesWritten)
{
	if (pContext != NULL)
	{
		CSingletonAllocSend::Instance()->FreeContext(pContext);
	}

	switch (errorCode) {
	case ERROR_SUCCESS:
	case ERROR_OPERATION_ABORTED:
		StartAsyncSend();
		break;
	default:
		Destroy();
		break;
	}
}
void CPipeLink::OnRecv(SOvlpRecvContext* pContext, int errorCode, DWORD numberOfBytesRead)
{
	CGuardLock<CSLock> guard(&m_csRecv);
	switch (errorCode)
	{
	case ERROR_SUCCESS:
		break;
	case ERROR_MORE_DATA:
		// This is not an error. We're connected to a message mode
		// pipe and the message didn't fit into the pipe's system
		// buffer. We will read the remaining data in the next call.
		break;
	case ERROR_BROKEN_PIPE:
	case ERROR_PIPE_NOT_CONNECTED:
		pipeBroken = true;
		break;
	default:
		pipeBroken = true;
		break;
	}

	if (pipeBroken)
	{
		CSingletonAllocRecv::Instance()->FreeContext(pContext);
		Destroy();
		return;
	}

	if (numberOfBytesRead > 0)
	{
		ProcessRecevData((char*)pContext->data, numberOfBytesRead);
	}
	CSingletonAllocRecv::Instance()->FreeContext(pContext);
	StartAsyncRead();
}

void CPipeLink::HandleRecvEnd()
{
	if (m_pRecv->m_pszBuf == NULL ||
		m_pRecv->m_cbBuf <= 0 ||
		m_pRecv->m_nIndex <= 0)
		return;


	//将完整的包发到上层
	SendRecvDataStatus(STATUS_RECV_END);
	//重新初始化PacketHead
	InitPacketHead();
	m_pRecv->Free();
}

 void CPipeLink::ProcessRecevData(char* pdata, int cbsize)
{
	char* pRecv = pdata;
	int nRecv = cbsize;

	if (!m_bHeadRecvOK)
		SendRecvDataStatus(STATUS_RECV_BEGIN);			//接收开始

	int nLeft = nRecv;
	int nProcessed = 0;
	int nIndex = 0;
	while (1)
	{
		if (UnPacket(pRecv + nIndex, nLeft, nProcessed))
		{
			//此时接收到一个完整的包
			HandleRecvEnd();
		}
		else
		{
			if (m_bInvalidPacket)
				break;

			if (m_bHeadRecvOK &&m_pRecv->m_nIndex > 0)
				SendRecvDataStatus(STATUS_RECV_MIDDLE);	
		}

		nLeft -= nProcessed;
		nIndex += nProcessed;
		if (nLeft <= 0 || nIndex >= nRecv)
			break;
	}
}

BOOL CPipeLink::UnPacket(char* pData, int nLeft, int& nProcessed)
{
	BOOL bInvalidPacket = FALSE;
	nProcessed = 0;
	if (m_pRecv->m_pszBuf == NULL)
	{
		if (nLeft < PACKET_HEAD_LENGTH - m_nBreakIndex || m_bHeadBreak)
		{
			char* pHeadBreakPos = (char*)&m_packHead + m_nBreakIndex;
			int nBreakWanted = min(nLeft, PACKET_HEAD_LENGTH - m_nBreakIndex);
			memcpy(pHeadBreakPos, pData, nBreakWanted);
			m_nBreakIndex += nBreakWanted;
			m_bHeadBreak = TRUE;
			if (m_nBreakIndex >= PACKET_HEAD_LENGTH)
			{
				m_nBreakIndex = 0;
				m_bHeadBreak = FALSE;
				m_bHeadRecvOK = TRUE;

				bInvalidPacket = ProcessRecvHead();
			}

			nProcessed += nBreakWanted;
			if (bInvalidPacket)
				ProcessRecvInvalidPkt();
			return FALSE;
		}

		if (!m_bHeadRecvOK)
		{
			int nHeadNeed = min(nLeft, PACKET_HEAD_LENGTH);
			memcpy(&m_packHead, pData, nHeadNeed);

			if (nHeadNeed < PACKET_HEAD_LENGTH)
			{
				m_bHeadBreak = TRUE;
				m_nBreakIndex = nHeadNeed;
				nProcessed += nHeadNeed;
				return FALSE;
			}

			bInvalidPacket = ProcessRecvHead();

			nProcessed += nHeadNeed;
			if (bInvalidPacket)
				ProcessRecvInvalidPkt();
			return FALSE;
		}
	}

	char* pCopy = m_pRecv->m_pszBuf + m_pRecv->m_nIndex;
	int nWanted = m_pRecv->m_cbBuf - m_pRecv->m_nIndex;
	if (nWanted > nLeft)
	{
		memcpy(pCopy, pData, nLeft);
		m_pRecv->m_nIndex += nLeft;
		nProcessed += nLeft;
		return FALSE;
	}
	else
	{
		memcpy(pCopy, pData, nWanted);
		m_pRecv->m_nIndex += nWanted;
		nProcessed += nWanted;
		return TRUE;
	}
	return FALSE;
}

BOOL CPipeLink::ProcessRecvHead()
{
	BOOL bInvalidPacket = FALSE;

	m_nBreakIndex = 0;
	m_bHeadBreak = FALSE;
	m_bHeadRecvOK = TRUE;

	if (m_packHead.headTag == PACK_MAGIC_NUMBER)
	{
		if (m_packHead.dwPacketLen < m_nMaxPacketSize && m_packHead.dwPacketLen > 0)
			m_pRecv->Alloc(m_packHead.dwPacketLen);
		else
			bInvalidPacket = TRUE;
	}
	else
	{
		bInvalidPacket = TRUE;
	}
	return bInvalidPacket;
}

void CPipeLink::ProcessRecvInvalidPkt()
{
	InitPacketHead();
	m_pRecv->Free();
	SendLinkEvent(EVT_ERRPROTOCOL);
	m_bInvalidPacket = TRUE;
	Destroy();
}

//此时管道建立成功
void CPipeLink::OnAccept(SOvlpAcceptContext* pContext, int errorCode, DWORD numberOfBytes)
{
	CSingletonAllocAccept::Instance()->FreeContext(pContext);
	
	if (errorCode == ERROR_OPERATION_ABORTED) //正在监听中的连接被取消了
	{
		Destroy();
	}
	else if (errorCode == ERROR_SUCCESS)  //连接被接受了
	{
		SendLinkEvent(EVT_ESTABLISH);
		//让管道开始接收数据
		StartAsyncRead();
	}	
}

void CPipeLink::InitContext(SOvlpIOContext* pContext)
{
	pContext->hLink = GetHLink();
	pContext->hPipe = GetPipeHandle();
	pContext->dwBytes = 0;
}

void CPipeLink::SetCallback(EventCallBack pfnIOCB)
{
	m_pfnIOCB = pfnIOCB;
}

void CPipeLink::IOCompletionProc(DWORD dwStatus, DWORD dwBytes, LPOVERLAPPED pOvlp)
{
	if (pOvlp == NULL)
		return;

	SOvlpIOContext* pContext = CONTAINING_RECORD(pOvlp, SOvlpIOContext, ovlp);
	SOvlpRecvContext* pRecv = (SOvlpRecvContext*)pContext;
	SOvlpSendContext* pSend = (SOvlpSendContext*)pContext;
	SOvlpAcceptContext* pAccept = (SOvlpAcceptContext*)pContext;

	CPipeLinkPtr pLink;
	if (!HLink2PLink(pContext->hLink, pLink))
	{
		if (pContext->io_type == IOREQ_SEND)
			CSingletonAllocSend::Instance()->FreeContext(pSend);
		else if (pContext->io_type == IOREQ_RECV)
			CSingletonAllocRecv::Instance()->FreeContext(pRecv);
		else if (pContext->io_type == IOREQ_ACCEPT)
			CSingletonAllocAccept::Instance()->FreeContext(pAccept);
		return;
	}

	pContext->dwBytes = dwBytes;
	if (pContext->io_type == IOREQ_SEND)
		pLink->OnSend(pSend, dwStatus, dwBytes);
	else if (pContext->io_type == IOREQ_RECV)
		pLink->OnRecv(pRecv, dwStatus, dwBytes);
	else if (pContext->io_type == IOREQ_ACCEPT)
		pLink->OnAccept(pAccept, dwStatus, dwBytes);
}

DWORD CPipeLink::IssueSend(SOvlpSendContext* pContext)
{
	DWORD dwBytes = pContext->dwBytes;
	pContext->io_type = IOREQ_SEND;
	DWORD dwWritten;
	WriteFile(pContext->hPipe, pContext->data, pContext->dwBytes, &dwWritten, &(pContext->ovlp));
	return GetLastError();
}

