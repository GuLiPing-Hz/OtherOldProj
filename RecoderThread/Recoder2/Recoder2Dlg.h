// Recoder2Dlg.h : 头文件
//

#pragma once
#include "wave/WaveUnit.h"
#include "wave/MixerDev.h"
#include "afxwin.h"

#include <vector>
#include <map>

// typedef std::vector<MIXERCAPS>	VECTMIXERCAPS;
// typedef std::vector<MIXERLINE>		VECTMIXERLINE;
// typedef std::map<std::string,UINT>	MAPSTRDEVID;
// CRecoder2Dlg 对话框
class CRecoder2Dlg : public CDialog
{
// 构造
public:
	CRecoder2Dlg(CWnd* pParent = NULL);	// 标准构造函数
	~CRecoder2Dlg();

// 对话框数据
	enum { IDD = IDD_RECODER2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	BOOL GetLineInfo(LPMIXERLINE pmxl, DWORD dwDstType, DWORD dwSrcType);
	BOOL GetLineControl(LPMIXERCONTROL pmxc, LPMIXERLINE pmxl, DWORD dwType);
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRecodestart();
private:
	CWaveUnit	m_wavRecoder1;
	CWaveUnit  m_wavRecoder2;
	bool						m_brecodeRunning1;
	bool						m_brecodeRunning2;
	UINT					m_uMxId; //mixer的ID
	HMIXER				m_hmx; //

	VECTMIXERCAPS		m_vectMixerCaps;
	VECTMIXERLINE			m_vectMixerLine;
	VECTMIXERLINE			m_vectMixerLine2;
	MAPSTRDEVID		m_mapMixerCaps;
	char								m_szIniFile[_MAX_PATH];
	char								m_currentDir[260];
public:
	afx_msg void OnBnClickedRecodestop();
	CComboBox m_ctrlDevName1;
	CComboBox m_ctrlDevName2;
	CComboBox m_ctrlDevUnit1;
	CComboBox m_ctrlDevUnit2;
	afx_msg void OnCbnSelchangeDevname1();
	afx_msg void OnCbnSelchangeDevname2();

private:
	bool ctrlSelectChangeDevName(CComboBox & comboBoxSel,CComboBox& comboBoxUnit,CComboBox & comboBoxCha,int nIndex);
	void analysisDevUnit(/*out*/DWORD &nRecordFrom,int n);
public:
	CComboBox m_ctrlDevUnit3;
private:
	BOOL m_bmultiWaveIn;
public:
	afx_msg void OnBnClickedMultiplewavein();
private:
	CButton m_ctrlMulWaveIn;
public:
	afx_msg void OnBnClickedBdebug();
};
