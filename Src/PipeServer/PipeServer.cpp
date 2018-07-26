
// PipeServer.cpp : ����Ӧ�ó��������Ϊ��
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


// CPipeServerApp ����

CPipeServerApp::CPipeServerApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CPipeServerApp ����

CPipeServerApp theApp;


// CPipeServerApp ��ʼ��

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

