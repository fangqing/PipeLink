
// PipeClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PipeClient.h"
#include "PipeClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPipeClientDlg �Ի���

CPipeClientDlg::CPipeClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PIPECLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPipeClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_link_listbox);
	DDX_Control(pDX, IDC_EDIT2, m_editInput);
	DDX_Control(pDX, IDC_LIST2, m_outPutlist);
}

BEGIN_MESSAGE_MAP(CPipeClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CPipeClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CPipeClientDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CPipeClientDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CPipeClientDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CPipeClientDlg ��Ϣ�������

BOOL CPipeClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

void CPipeClientDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPipeClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPipeClientDlg::OnClosed(HLINK hLink)
{
	CGuardLock<CSLock> guard(&m_cs_event);
	WCHAR szData[20] = { 0 };
	swprintf_s(szData, 20, _T("link:%u"), (DWORD)hLink);

	int index = m_link_listbox.FindString(-1, szData);
	if (index != LB_ERR)
		m_link_listbox.DeleteString(index);
}

void CPipeClientDlg::OnEstablish(HLINK hLink)
{
	TCHAR szData[20] = { 0 };
	swprintf_s(szData, 10, _T("link:%u"), (DWORD)hLink);
	int itemIndex = m_link_listbox.AddString(szData);
	m_link_listbox.SetItemData(itemIndex, (DWORD_PTR)hLink);

}



void CPipeClientDlg::OnProtocolError(HLINK)
{

}

void CPipeClientDlg::OnReceiveData(HLINK hLink, const char* pData, unsigned int cbSize)
{
	CGuardLock<CSLock> guard(&m_cs_event);

	int bufCount = cbSize / sizeof(WCHAR) + 20;
	WCHAR *pszBuf = new WCHAR[bufCount];
	ZeroMemory(pszBuf, bufCount * sizeof(WCHAR));
	swprintf_s(pszBuf, bufCount, _T("hlink(%d) recv:%s"), hLink, (const WCHAR*)pData);
	m_outPutlist.AddString(pszBuf);
	delete []pszBuf;
}


void CPipeClientDlg::OnBnClickedButton1()
{
	HLINK link = PipeCreate();
	if (!PipeConnnect(link))
	{
		PipeClose(link);
		::MessageBox(GetSafeHwnd(), _T("connect failed"), NULL, MB_OK);
	}
}
 
void CPipeClientDlg::OnBnClickedButton2()
{
	int index = m_link_listbox.GetCurSel();
	if (index != LB_ERR)
	{
		HLINK link = m_link_listbox.GetItemData(index);
		PipeClose(link);
	}
}

void CPipeClientDlg::OnBnClickedButton3()
{
	//����
	int index = m_link_listbox.GetCurSel();
	if (index != LB_ERR)
	{
		HLINK linkIndex = m_link_listbox.GetItemData(index);
		int textLen = ::GetWindowTextLengthW(m_editInput.GetSafeHwnd());
		WCHAR *pszBuf = new WCHAR[textLen + 1];
		ZeroMemory(pszBuf, (textLen + 1) * sizeof(WCHAR));
		::GetWindowTextW(m_editInput.GetSafeHwnd(), pszBuf, textLen+1);
		PipeSend(linkIndex, (char*)pszBuf, (textLen + 1) * sizeof(WCHAR));
		delete[]pszBuf;
	}
}

void CPipeClientDlg::OnBnClickedButton4()
{
	m_outPutlist.ResetContent();
}

