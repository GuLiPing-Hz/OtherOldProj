// IconExampleDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#define WM_MYICON (WM_USER+100)

namespace avhttp {
	class multi_download;
};
// CIconExampleDlg 对话框
class CIconExampleDlg : public CDialog
{
// 构造
public:
	CIconExampleDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CIconExampleDlg();
// 对话框数据
	enum { IDD = IDD_ICONEXAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	static DWORD CALLBACK DownloadThread(LPVOID pParam);
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT onMyIcon(WPARAM wParam,LPARAM lParam);
	void exitNotifyIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnIconExit();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CEdit m_ctrlUrl;
	CProgressCtrl m_ctrlProgress;
	avhttp::multi_download* m_pMDHelp;
	afx_msg void OnBnClickedButton1();
	CString m_strUrl;
};
