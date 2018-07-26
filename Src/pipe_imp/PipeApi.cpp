#include "stdafx.h"
#include "PipeApi.h"
#include "PipeLink.h"
#include <list>
#include <vector>
#include "AllocContextMgr.h"


//change the pipe name to you want
#define  PIPE_NAME  _T("\\\\.\\pipe\\hello_world")

HANDLE g_hIOCP = NULL;
std::vector<HANDLE> g_ayTPHandle;

EventCallBack g_pfnIOCB = NULL;
typedef void(*IOCPCallBack)(DWORD dwStatus, DWORD dwBytes, LPOVERLAPPED pOvlp);

DWORD WINAPI IOCPThreadProc(LPVOID lpParam)
{
	LPOVERLAPPED pOvlp = NULL;
	DWORD dwStatus = 0, dwBytes = 0;
	DWORD pfnCompletionKey = NULL;
	HANDLE hIOCP = (HANDLE)lpParam;
	while (1)
	{
		if (GetQueuedCompletionStatus(hIOCP, &dwBytes, &pfnCompletionKey, &pOvlp, INFINITE))
			dwStatus = ERROR_SUCCESS;
		else
			dwStatus = GetLastError();

		if (pfnCompletionKey == NULL)
			break;

		((IOCPCallBack)pfnCompletionKey)(dwStatus, dwBytes, pOvlp);
	}
	return -1;
}

void  InitPipe()
{
	CSingletonAllocSend::Instance();
	CSingletonAllocRecv::Instance();
	CSingletonAllocAccept::Instance();

	g_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int nThreadCount = si.dwNumberOfProcessors * 2;
	for (int i = 0; i<nThreadCount; i++)
	{
		DWORD dwThreadId = 0;
		HANDLE hThread = CreateThread(NULL, 0, IOCPThreadProc, (LPVOID)g_hIOCP, 0, &dwThreadId);
		g_ayTPHandle.push_back(hThread);
	}
}

void UnInitPipe()
{
	CPipeLink::FreeAllLink();

	int nThreadCount = g_ayTPHandle.size();
	for (int i = 0; i<nThreadCount; i++)
		PostQueuedCompletionStatus(g_hIOCP, 0, 0, 0);
	WaitForSingleObject(g_hIOCP, INFINITE);
	for (int i = 0; i < nThreadCount; i++)
	{
		WaitForSingleObject(g_ayTPHandle[i], INFINITE);
		CloseHandle(g_ayTPHandle[i]);
	}
	g_ayTPHandle.clear();

	CSingletonAllocSend::Instance()->FreeAllContext();
	CSingletonAllocRecv::Instance()->FreeAllContext();
	CSingletonAllocAccept::Instance()->FreeAllContext();
	CSingletonAllocSend::Release();
	CSingletonAllocRecv::Release();
	CSingletonAllocAccept::Release();
}

bool PipeSend(HLINK hLink, const char* pData, unsigned int cbSize)
{
	CPipeLinkPtr pLink;
	if (HLink2PLink(hLink, pLink))
	{
		pLink->SendData(pData, cbSize);
		return true;
	}
	return false;
}


void PipeClose(HLINK hLink)
{
	CPipeLinkPtr pLink;
	if (HLink2PLink(hLink, pLink))
	{
		pLink->Destroy();
		return ;
	}
}

HLINK PipeCreate(EventCallBack event_cb)
{
	CPipeLinkPtr plinkPtr = CPipeLink::CreatePipeLink();
	plinkPtr->SetCallback(event_cb);
	return plinkPtr->GetHLink();
}

bool  PipeListen(HLINK hLink)
{
	CPipeLinkPtr plinkPtr;
	if (!HLink2PLink(hLink, plinkPtr))
	{
		return false;
	}

	HANDLE hPipeServer = CreateNamedPipe(
		PIPE_NAME,					// pipe name 
		PIPE_ACCESS_DUPLEX |      // read/write access 
		FILE_FLAG_OVERLAPPED,     // overlapped mode 
		PIPE_TYPE_BYTE |       // message-type pipe 
		PIPE_READMODE_BYTE |   // message read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // unlimited instances 
		BUFSIZE,					// output buffer size 
		BUFSIZE,					// input buffer size 
		PIPE_TIMEOUT,             // client time-out 
		NULL);                    // default security attributes

	if (hPipeServer == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	plinkPtr->AttachPipe(hPipeServer);
	DWORD dwret = plinkPtr->Accept();
	return (!plinkPtr->hasBroken());
}

bool  PipeConnnect(HLINK hLink)
{
	CPipeLinkPtr plinkPtr;
	if (!HLink2PLink(hLink, plinkPtr))
	{
		return false;
	}
	HANDLE hPipeCient;
	while (1)  //loop until one connection accepted by the server
	{
		hPipeCient = CreateFile(
			PIPE_NAME,
			GENERIC_READ | GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			FILE_FLAG_OVERLAPPED,// FILE_FLAG_OVERLAPPED
			NULL);          // no template file 

		if (hPipeCient != INVALID_HANDLE_VALUE) 
			break;
		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			return false;
		}
		if (!WaitNamedPipe(PIPE_NAME, PIPE_TIMEOUT))
		{
			return false;
		}
	}
	plinkPtr->AttachPipe(hPipeCient);
	plinkPtr->Accepted();
	return true;
}