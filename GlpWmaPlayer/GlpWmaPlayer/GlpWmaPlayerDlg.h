// GlpWmaPlayerDlg.h : 头文件
//

#pragma once
#include <string>
#include "ManualGraph.h"

// CGlpWmaPlayerDlg 对话框
class CGlpWmaPlayerDlg : public CDialog
{
// 构造
public:
	CGlpWmaPlayerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_GLPWMAPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	std::wstring m_file;
	CManualGraph m_cMG;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpenfile();
private:
	std::wstring GetMoviePath();
public:
	afx_msg void OnBnClickedBplay();
	afx_msg void OnBnClickedBpause();
	afx_msg void OnBnClickedBstop();
	afx_msg void OnBnClickedBswitchaudio();
};
