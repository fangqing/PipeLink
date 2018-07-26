#pragma once

class CDataBuf
{
public:
	CDataBuf() : m_pszBuf(NULL), m_cbBuf(0), m_nIndex(0), m_bAttach(FALSE)
	{}
	CDataBuf(char* pBuf, unsigned int nLen) : 
		m_pszBuf(pBuf),  m_cbBuf(nLen), m_nIndex(0), 
		m_bAttach(TRUE)
	{}
	~CDataBuf() {	Free();  }

	char* Alloc(unsigned int cbSize) 
	{
		m_pszBuf = new char[cbSize];
		if (m_pszBuf != NULL)
		{
			memset(m_pszBuf, 0, cbSize);
			m_cbBuf = cbSize;
			m_nIndex = 0;
		}
		return m_pszBuf;
	}
	void Free() 
	{
		if (!m_bAttach && m_pszBuf != NULL)
		{
			delete [] m_pszBuf;
			m_pszBuf = NULL;
			m_cbBuf = m_nIndex = 0;
		}
	}
	void CloneBuf(char* pBuf, unsigned int nLen)
	{
		Free();
		memcpy(Alloc(nLen), pBuf, nLen);
	}

	char* m_pszBuf;
	unsigned int m_cbBuf;
	unsigned int m_nIndex;
	BOOL m_bAttach;
};
