#pragma once
#include "Guard.h"

template <typename T, typename LOCK = CSLock>
class CSingletonHandler
{
public:
	static T* Instance()
	{
		if (m_pInstance == NULL)
		{
			CGuardLock<LOCK> Guard(&m_Lock);
			if (m_pInstance == NULL)
				m_pInstance = new T;
		}
		return m_pInstance;
	}
	static void Release()
	{
		if (m_pInstance != NULL)
		{
			CGuardLock<LOCK> Guard(&m_Lock);
			if (m_pInstance != NULL)
			{
				delete m_pInstance;
				m_pInstance = NULL;
			}
		}
	}
private:
	CSingletonHandler();
	~CSingletonHandler();
	
	CSingletonHandler(const CSingletonHandler& singletonhandler);
	CSingletonHandler& operator=(const CSingletonHandler& singletonhandler);

	static T* m_pInstance;
	static LOCK m_Lock;
};

template <typename T, typename LOCK>
T* CSingletonHandler<T, LOCK>::m_pInstance = NULL;

template <typename T, typename LOCK>
LOCK CSingletonHandler<T, LOCK>::m_Lock;