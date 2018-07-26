#pragma once
#include <Windows.h>
class CSLock
{
public:
	CSLock()
	{
		InitializeCriticalSection(&m_sect);
	}
	~CSLock()
	{
		DeleteCriticalSection(&m_sect);
	}
	operator CRITICAL_SECTION*() { return &m_sect; }
	
	void Lock()
	{
		EnterCriticalSection(&m_sect);
	}
	void Unlock()
	{
		LeaveCriticalSection(&m_sect);
	}
private:
	CRITICAL_SECTION m_sect;
};

template <typename LOCK>
class CGuardLock
{
public:
	explicit CGuardLock(LOCK* p) : m_pLock(p) { m_pLock->Lock(); }
	~CGuardLock() { m_pLock->Unlock(); }
private:
	CGuardLock(const CGuardLock&);
	CGuardLock& operator=(const CGuardLock&);
	LOCK* m_pLock;
};

template<bool> struct ComplieTimeError;
template<> struct ComplieTimeError<false> {};
#define STATIC_CHECK(expr) (ComplieTimeError<(expr) != 0>())

template <class T, class U>
class CConversion
{
	typedef char Small;
	class Big { char dummy[2]; };
	static Small Test(U);
	static Big Test(...);
	static T MakeT();
public:
	enum { exists = sizeof(Test(MakeT())) == sizeof(Small) };
	enum { exists2Way = exists && CConversion<U, T>::exists };
	enum { sameType = false };
};

template <class T>
class CConversion<T, T>
{
public:
	enum { exists = 1, exists2Way = 1, sameType = 1 };
};


#define SUPERSUBCLASS(T, U) \
	(CConversion<const U*, const T*>::exists && \
	!CConversion<const U*, const void*>::sameType)
#define INHERITSCLASS(T, U) \
	(CConversion<const U*, const T*>::exists || \
	 CConversion<const T*, const U*>::exists)
#define SUPERSUBCLASS_STRICT(T, U) \
	(SUPERSUBCLASS(T, U) && \
	!CConversion<const T, const U>::sameType)

template <class T>
class CRefCountPtr 
{
public:
	CRefCountPtr() : m_pObj(0), m_pRef(0) {}
	explicit CRefCountPtr(T* p) : m_pObj(p)
	{
		m_pRef = new long;
		*m_pRef = 1;
	}
	CRefCountPtr(const CRefCountPtr<T>& rcf) : m_pObj(rcf.m_pObj), m_pRef(rcf.m_pRef)
	{
		if (m_pRef)
			InterlockedIncrement(m_pRef);
	}
	
	template <class U>
	CRefCountPtr(const CRefCountPtr<U>& rcf) : 
			m_pObj(reinterpret_cast<T*>(rcf.Obj())), m_pRef(rcf.Ref())
	{
		STATIC_CHECK(!INHERITSCLASS(T, U));
		if (m_pRef)
			InterlockedIncrement(m_pRef);
	}

	~CRefCountPtr() 
	{
		Release();
	}
	
	CRefCountPtr<T>& operator=(const CRefCountPtr<T>& rcf)
	{
		Release();
		m_pObj = rcf.m_pObj;
		m_pRef = rcf.m_pRef;
		InterlockedIncrement(m_pRef);
		return *this;
	}

	BOOL operator==(const CRefCountPtr<T>& rcf) const
	{
		if (m_pObj && rcf.m_pObj && m_pRef && rcf.m_pRef && 
			m_pObj == rcf.m_pObj && m_pRef == rcf.m_pRef)
			return TRUE;
		return FALSE;
	}

	template <class U>
	CRefCountPtr<T>& operator=(const CRefCountPtr<U>& rcf)
	{
		STATIC_CHECK(!INHERITSCLASS(T, U));
		Release();
		m_pObj = dynamic_cast<T*>(rcf.m_pObj);
		if (m_pObj)
		{
			m_pRef = rcf.m_pRef;
			InterlockedIncrement(m_pRef);
		}
		return *this;
	}

	T* operator->() const { return m_pObj; } 
	T& operator*() const { return *m_pObj; }
	T* Obj() const { return m_pObj; }
	long* Ref() const { return m_pRef; }

private:
	void Release()
	{
		if (m_pRef && *m_pRef != 0)
		{
			if (InterlockedDecrement(m_pRef) == 0)
			{
				if (m_pObj)
					delete m_pObj;
				delete m_pRef;
			}
		}
	}

private:
	T* m_pObj;
	long* m_pRef;
};

template <class T>
BOOL operator==(const CRefCountPtr<T>& left, const CRefCountPtr<T>& right)
{
	return left.operator ==(right);
}