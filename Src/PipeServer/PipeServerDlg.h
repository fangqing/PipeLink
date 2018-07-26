
// PipeServerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include "EventHandler.h"

typedef CEventHandler<1> CNetServerHandler;

// CPipeServerDlg 对话框
class CPipeServerDlg : public CDialog, public CNetServerHandler
{
// 构造
public:
	CPipeServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PIPESERVER_DIALOG };
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

protected:
	void OnEstablish(HLINK)override;
	void OnClosed(HLINK)override;
	void OnProtocolError(HLINK) override;
	void OnReceiveData(HLINK hLink, const char* pData, unsigned int cbSize) override;
private:
	CListBox m_link_listbox;
	CEdit m_editInput;
	CListBox m_outPutList;
	CButton m_btn_Moniter;
	bool m_bMoniter;
	CSLock m_cs_event;
};
