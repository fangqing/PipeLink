
// PipeServer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPipeServerApp: 
// �йش����ʵ�֣������ PipeServer.cpp
//

class CPipeServerApp : public CWinApp
{
public:
	CPipeServerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPipeServerApp theApp;