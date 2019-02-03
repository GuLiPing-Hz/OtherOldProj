// XGSQLiteTestDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "XGSQLite/MusicSQLObject.h"
// CXGSQLiteTestDlg dialog
class CXGSQLiteTestDlg : public CDialog
{
// Construction
public:
	CXGSQLiteTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_XGSQLITETEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_MusicName;
	CString m_MusicNamePY;
	int m_MusicNameCount;
	CString m_MusicSinger;
	CString m_MusicSingerPY;
	CComboBox m_CtrlMusicType;
	CComboBox m_CtrlMusicSingerType;
	CComboBox m_CtrlMusicLangType;
	BOOL m_IsSinger;
	afx_msg void OnBnClickedButton1();
	int m_InitCount;
	CListBox m_CtrlResultList;
	afx_msg void OnBnClickedRadioSong();
	afx_msg void OnBnClickedRadioSinger();
private:
	void SetCtrlStatus();

	void SearchMusic();

	void SearchSinger();
private:
	XGSQLITE::CMusicSQLObject *m_pMusicObject;
public:
	CEdit m_CtrlName;
	CEdit m_CtrlNamePY;
	CEdit m_CtrlNameCount;
	CEdit m_CtrlSinger;
	CEdit m_CtrlSingerPY;
	int m_iMusicId;
	afx_msg void OnBnClickedButton2();
	CComboBox m_CtrlMusicTopInfo;
	BOOL m_MusicNew;
	int m_PageIndex;
	int m_allpage;
	afx_msg void OnBnClickedBtnPre();
	afx_msg void OnBnClickedBtnNext();
	afx_msg void OnBnClickedCancel();
	int m_Debugtime;
	afx_msg void OnBnClickedBtnSingerinfo();
};
