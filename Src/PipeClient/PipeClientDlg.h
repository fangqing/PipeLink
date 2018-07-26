
// PipeClientDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "EventHandler.h"
#include "Guard.h"

typedef CEventHandler<2> CNetClientHandler;

class CPipeClientDlg : public CDialogEx,public CNetClientHandler
{
// 构造
public:
	CPipeClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PIPECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
