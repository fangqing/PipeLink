
// PipeClient.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "PipeClient.h"
#include "PipeClientDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPipeClientApp

BEGIN_MESSAGE_MAP(CPipeClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPipeClientApp 构造

CPipeClientApp::CPipeClientApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CPipeClientApp 对象

CPipeClientApp theApp;


BOOL CPipeClientApp::InitInstance()
{
	CWinApp::InitInstance();

	InitPipe();
	CPipeClientDlg dlg;
	m_pMainWnd = &dlg;
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

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

