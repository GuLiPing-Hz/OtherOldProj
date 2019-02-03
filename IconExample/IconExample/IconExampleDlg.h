// IconExampleDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#define WM_MYICON (WM_USER+100)

namespace avhttp {
	class multi_download;
};
// CIconExampleDlg �Ի���
class CIconExampleDlg : public CDialog
{
// ����
public:
	CIconExampleDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CIconExampleDlg();
// �Ի�������
	enum { IDD = IDD_ICONEXAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	static DWORD CALLBACK DownloadThread(LPVOID pParam);
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
