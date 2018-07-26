
// PipeClientDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "EventHandler.h"
#include "Guard.h"

typedef CEventHandler<2> CNetClientHandler;

class CPipeClientDlg : public CDialogEx,public CNetClientHandler
{
// ����
public:
	CPipeClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PIPECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();

	void OnEstablish(HLINK hlink) override;
	void OnClosed(HLINK hlink) override;
	void OnProtocolError(HLINK) override;
	void OnReceiveData(HLINK hLink, const char* pData, unsigned int cbSize) override;

private:
	CListBox m_link_listbox;
	CEdit m_editInput;
	CListBox m_outPutlist;
	HLINK m_hLink;
	CSLock m_cs_event;
};
