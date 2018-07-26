#pragma once
#include "PipeDef.h"
#include "Guard.h"
template <int TYPE = 0>
class CEventHandler
{
public:
	CEventHandler() { m_pThis = this; }
	~CEventHandler() { m_pThis = NULL; }
	virtual void OnEstablish(HLINK) = 0;
	virtual void OnClosed(HLINK) = 0;
	virtual void OnReceiveData(HLINK hLink, const char* pData, unsigned int cbSize) = 0;
	virtual void OnProtocolError(HLINK) = 0;
	static void IOCallbackFunc(void* pParam1, void* pParam2);
	static HLINK PipeCreate();
private:
	static CEventHandler<TYPE>* m_pThis;
};

template <int TYPE>
CEventHandler<TYPE>* CEventHandler<TYPE>::m_pThis = NULL;

template <int TYPE>
HLINK CEventHandler<TYPE>::PipeCreate()
{
	return ::PipeCreate(CEventHandler::IOCallbackFunc);
}

template <int TYPE>
void CEventHandler<TYPE>::IOCallbackFunc(void* pParam1, void* pParam2)
{
	//be carefutl::this function is not thread-safe
	if (m_pThis == NULL)
		return;

	LinkEvent *pEvent = (LinkEvent*)pParam1;
	LinkRecvDataParam* pRecv = (LinkRecvDataParam*)pParam2;
	__try
	{
		switch (pEvent->nEvent)
		{
		case EVT_ESTABLISH:
			m_pThis->OnEstablish(pEvent->hLink);
			break;
		case EVT_CLOSED:
			m_pThis->OnClosed(pEvent->hLink);
			break;
		case EVT_RECEIVEDATA:
			if (pRecv->nStatus == STATUS_RECV_END)
				m_pThis->OnReceiveData(pEvent->hLink, pRecv->pRecvData, pRecv->cbSize);
			break;
		case EVT_ERRPROTOCOL:
			m_pThis->OnProtocolError(pEvent->hLink);
			break;
		default:
			break;
		}
	}
	__except (EXCEPTION_CONTINUE_SEARCH)
	{

	}
}
