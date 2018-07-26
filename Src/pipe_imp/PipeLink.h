#pragma once

//author:fangqing0827@126.com
//https://github.com/fangqing/pipe_iocp

#include <unordered_map>
#include <list>
#include <assert.h>
#include "Guard.h"
#include "PipeApi.h"
#include "DataBuf.h"

class CPipeLink;
typedef CRefCountPtr<CPipeLink> CPipeLinkPtr;
typedef std::unordered_map <HLINK, CPipeLinkPtr> CMapPipeHandleLink;
typedef std::list<SOvlpSendContext*> CListSendContext;

class CPipeLink
{
public:
	CPipeLink();
	~CPipeLink();

public:
	static BOOL FindLink(HLINK hLink, CPipeLinkPtr& pLink);
	static void FreeAllLink();
	static CPipeLinkPtr CreatePipeLink();
private:
	static CSLock m_csLink;
	static CMapPipeHandleLink m_mapLink;
public:
	operator HLINK() const { return m_hLink; }
	inline HLINK  GetHLink() { return m_hLink; }
	inline HANDLE GetPipeHandle() { 
		assert(m_hPipe != INVALID_HANDLE_VALUE);
		return m_hPipe; 
	}

	//attach a pipe handle and bind it to a iocp
	void AttachPipe(HANDLE hpipe);
	
	//only called by server side
	DWORD Accept();
	//called by client side 
	void Accepted();
	void StartAsyncRead();
	void StartAsyncSend();

	void SendData(const char* pData, unsigned int cbSize);
	void InitContext(SOvlpIOContext* pContext);
	void Destroy();
	void SetCallback(EventCallBack pfnIOCB);
	bool hasBroken() { return pipeBroken; }
private:
	void SendLinkEvent(int nEvent);
	void SendRecvDataStatus(int nStatus);
	void OnSend(SOvlpSendContext* pContext, int errorCode, DWORD numberOfBytesWritten);
	void OnRecv(SOvlpRecvContext* pContext, int errorCode, DWORD numberOfBytesRead);
	//OnAccept only be called in server side
	void OnAccept(SOvlpAcceptContext* pContext, int errorCode, DWORD numberOfBytes);
	DWORD CheckPipeState();
	
	bool BuildPacket(const char* pData, unsigned int cbSize);
	void ProcessRecevData(char* pdata, int cbsize);
	BOOL UnPacket(char* pData, int nLeft, int& nProcessed);
	void InitPacketHead();
	BOOL ProcessRecvHead();
	void ProcessRecvInvalidPkt();
	void HandleRecvEnd();

	static DWORD IssueSend(SOvlpSendContext* pContext);
	static void IOCompletionProc(DWORD dwStatus, DWORD dwBytes, LPOVERLAPPED pOvlp);
protected:
	EventCallBack m_pfnIOCB;
	HLINK m_hLink;
	HANDLE m_hPipe; 
	CSLock m_csRecv;
	CSLock m_csSend;
	CDataBuf* m_pRecv;
	CListSendContext m_lsSend;
	bool pipeBroken;

	SPacketHead m_packHead;
	int m_nBreakIndex;
	BOOL m_bHeadBreak;
	BOOL m_bHeadRecvOK;
	BOOL m_bInvalidPacket;

	unsigned int m_nMaxPacketSize;
	unsigned int m_nMaxAllocContextCnt;
};

inline BOOL HLink2PLink(HLINK hLink, CPipeLinkPtr& pLink)
{
	return CPipeLink::FindLink(hLink, pLink);
}