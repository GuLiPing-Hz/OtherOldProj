// TestSoundDlg.h : 头文件
//

#pragma once
#include "sound/sound.h"
#include "afxcmn.h"

// CTestSoundDlg 对话框
class CTestSoundDlg : public CDialog
{
// 构造
public:
	CTestSoundDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CTestSoundDlg();

// 对话框数据
	enum { IDD = IDD_TESTSOUND_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:

	CGLSound*	m_sound;
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedStop();
	// 音量
	CSliderCtrl m_ctrlSliderVol;
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	float m_fVol;
};
