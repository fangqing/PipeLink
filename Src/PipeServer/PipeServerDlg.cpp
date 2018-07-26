
// PipeServerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PipeServer.h"
#include "PipeServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPipeServerDlg �Ի���


#define MAX_CONNECTION 10
CPipeServerDlg::CPipeServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PIPESERVER_DIALOG, pParent)
{
	m_bMoniter = false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPipeServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_link_listbox);
	DDX_Control(pDX, IDC_EDIT1, m_editInput);
	DDX_Control(pDX, IDC_LIST2, m_outPutList);
	DDX_Control(pDX, IDC_BUTTON1, m_btn_Moniter);
}

BEGIN_MESSAGE_MAP(CPipeServerDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CPipeServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPipeServerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CPipeServerDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CPipeServerDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CPipeServerDlg ��Ϣ�������

BOOL CPipeServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPipeServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPipeServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPipeServerDlg::OnClosed(HLINK hLink)
{
	CGuardLock<CSLock> guard(&m_cs_event);
	TCHAR szData[20] = { 0 };
	swprintf_s(szData, 10, _T("link:%u"), (DWORD)hLink);
	int index = m_link_listbox.FindString(-1, szData);
	if (index != LB_ERR)
		m_link_listbox.DeleteString(index);

	PipeListen(PipeCreate());
}

void CPipeServerDlg::OnEstablish(HLINK hLink)
{
	TCHAR szData[20] = { 0 };
	swprintf_s(szData, 10, _T("link:%u"), (DWORD)hLink);
	int itemIndex = m_link_listbox.AddString(szData);
	m_link_listbox.SetItemData(itemIndex, (DWORD_PTR)hLink);
}

void CPipeServerDlg::OnReceiveData(HLINK hLink, const char* pData, unsigned int cbSize)
{
	CGuardLock<CSLock> guard(&m_cs_event);

	int bufCount = cbSize / sizeof(WCHAR)+20;
	WCHAR *pszBuf = new WCHAR[bufCount];
	swprintf_s(pszBuf, bufCount, _T("hlink(%d) recv:%s"), hLink, (const WCHAR*)pData);
	m_outPutList.AddString(pszBuf);
	delete []pszBuf;
}


void CPipeServerDlg::OnBnClickedButton1()
{
	if (!m_bMoniter)
	{
		//MAX_CONNECTION clients can connect to server at the same time
		for (int i = 0; i < MAX_CONNECTION; i++)
		{
			PipeListen(PipeCreate());
		}
		m_btn_Moniter.SetWindowText(_T("started"));
		m_btn_Moniter.EnableWindow(FALSE);
		m_bMoniter = true;
	}
}


void CPipeServerDlg::OnBnClickedButton2()
{
	int index = m_link_listbox.GetCurSel();
	if (index != LB_ERR)
	{
		HLINK link = m_link_listbox.GetItemData(index);
		PipeClose(link);
	}
}

void CPipeServerDlg::OnBnClickedButton3()
{
	int index = m_link_listbox.GetCurSel();
	if (index != LB_ERR)
	{
		HLINK linkIndex = m_link_listbox.GetItemData(index);
		int textLen = ::GetWindowTextLengthW(m_editInput.GetSafeHwnd());
		WCHAR *pszBuf = new WCHAR[textLen+1];
		ZeroMemory(pszBuf, (textLen +1) * sizeof(WCHAR));
		::GetWindowTextW(m_editInput.GetSafeHwnd(), pszBuf, textLen);
		PipeSend(linkIndex, (char*)pszBuf, (textLen + 1) * sizeof(WCHAR));
	}
}

void CPipeServerDlg::OnProtocolError(HLINK)
{

}

void CPipeServerDlg::OnBnClickedButton4()
{
	m_outPutList.ResetContent();
}
