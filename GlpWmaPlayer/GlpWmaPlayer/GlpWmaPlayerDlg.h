// GlpWmaPlayerDlg.h : ͷ�ļ�
//

#pragma once
#include <string>
#include "ManualGraph.h"

// CGlpWmaPlayerDlg �Ի���
class CGlpWmaPlayerDlg : public CDialog
{
// ����
public:
	CGlpWmaPlayerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_GLPWMAPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	std::wstring m_file;
	CManualGraph m_cMG;
	// ���ɵ���Ϣӳ�亯��
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
