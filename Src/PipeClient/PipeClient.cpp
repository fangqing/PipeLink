
// PipeClient.cpp : ����Ӧ�ó��������Ϊ��
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


// CPipeClientApp ����

CPipeClientApp::CPipeClientApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CPipeClientApp ����

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

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

