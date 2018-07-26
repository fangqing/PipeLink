#pragma once
#include "PipeDef.h"
#include "SingletonHandler.h"
#include <list>
template <typename T>
class CAllocContextMgr
{
public:
	typedef std::list<T*> CListTPtr;

	CAllocContextMgr()
	{ 
		m_nAllocContextCnt = 0;
	}
	~CAllocContextMgr() { FreeAllContext(); }

	T* GetContext()
	{
		CGuardLock<CSLock> guard(&m_csAlloc);
		T* pContext = NULL;
		if (m_lsAlloc.empty())
		{
			for (unsigned int i = 0; i < AllocContextUnit; ++i)
			{
				m_lsAlloc.push_front(new T);
				m_nAllocContextCnt++;
			}
		}

		if (!m_lsAlloc.empty())
		{
			pContext = *m_lsAlloc.begin();
			m_lsAlloc.erase(m_lsAlloc.begin());
		}
		else
			pContext = NULL;

		return pContext;	
	}

	void FreeContext(T* pContext)
	{
		if (pContext == NULL)
			return;
		pContext->Reset();
		CGuardLock<CSLock> guard(&m_csAlloc);
		m_lsAlloc.push_front(pContext);

		pContext = *m_lsAlloc.begin();
		m_lsAlloc.erase(m_lsAlloc.begin());
		m_nAllocContextCnt--;
		delete pContext;
	}

	void FreeContextList(CListTPtr& ls)
	{
		CGuardLock<CSLock> guard(&m_csAlloc);
		T* pContext = NULL;
		CListTPtr::iterator it = ls.begin();
		while (it!= ls.end())
		{
			pContext = *it;
			if (pContext)
			{
				pContext->Reset();
				m_lsAlloc.push_front(pContext);
			}
			it++;
		}
		ls.clear();
	}

	void FreeAllContext()
	{
		CGuardLock<CSLock> guard(&m_csAlloc);
		while (!m_lsAlloc.empty())
		{
			T* pContext = *m_lsAlloc.begin();
			m_lsAlloc.erase(m_lsAlloc.begin());
			delete pContext;
		}
	}

private:
	CSLock m_csAlloc;
	CListTPtr m_lsAlloc;
	unsigned int m_nAllocContextCnt;
	static const unsigned int AllocContextUnit = 10;
};

typedef CAllocContextMgr<SOvlpSendContext> CAllocSendContextMgr;
typedef CAllocContextMgr<SOvlpRecvContext> CAllocRecvContextMgr;
typedef CAllocContextMgr<SOvlpAcceptContext> CAllocAcceptContextMgr;

typedef CSingletonHandler<CAllocSendContextMgr> CSingletonAllocSend;
typedef CSingletonHandler<CAllocRecvContextMgr> CSingletonAllocRecv;
typedef CSingletonHandler<CAllocAcceptContextMgr> CSingletonAllocAccept;
