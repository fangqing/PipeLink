
// PipeServer.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "PipeServer.h"
#include "PipeServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CPipeServerDlg * m_Pdlg = nullptr;

// CPipeServerApp

BEGIN_MESSAGE_MAP(CPipeServerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPipeServerApp 构造

CPipeServerApp::CPipeServerApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CPipeServerApp 对象

CPipeServerApp theApp;


// CPipeServerApp 初始化

BOOL CPipeServerApp::InitInstance()
{
	CWinApp::InitInstance();

	InitPipe();

	CPipeServerDlg dlg;
	m_pMainWnd = &dlg;

	m_Pdlg = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{

	}
	else if (nResponse == IDCANCEL)
	{

	}
	else if (nResponse == -1)
	{
		
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	UnInitPipe();

	return FALSE;
}

