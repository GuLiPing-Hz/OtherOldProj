// TestSoundDlg.h : ͷ�ļ�
//

#pragma once
#include "sound/sound.h"
#include "afxcmn.h"

// CTestSoundDlg �Ի���
class CTestSoundDlg : public CDialog
{
// ����
public:
	CTestSoundDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CTestSoundDlg();

// �Ի�������
	enum { IDD = IDD_TESTSOUND_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:

	CGLSound*	m_sound;
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedStop();
	// ����
	CSliderCtrl m_ctrlSliderVol;
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	float m_fVol;
};
