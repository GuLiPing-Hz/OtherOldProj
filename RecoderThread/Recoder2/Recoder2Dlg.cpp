// Recoder2Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Recoder2.h"
#include "Recoder2Dlg.h"
#include "glputil.h"
#include <exception>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CRecoder2Dlg �Ի���




CRecoder2Dlg::CRecoder2Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecoder2Dlg::IDD, pParent)
	,m_brecodeRunning1(false)
	,m_brecodeRunning2(false)
	, m_bmultiWaveIn(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CRecoder2Dlg::~CRecoder2Dlg()
{
	mixerClose(m_hmx);
}

void CRecoder2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVNAME1, m_ctrlDevName1);
	DDX_Control(pDX, IDC_DEVNAME2, m_ctrlDevName2);
	DDX_Control(pDX, IDC_DEVUNIT1, m_ctrlDevUnit1);
	DDX_Control(pDX, IDC_DEVUNIT2, m_ctrlDevUnit2);
	DDX_Control(pDX, IDC_DEVUNIT3, m_ctrlDevUnit3);
	DDX_Check(pDX, IDC_MULTIPLEWAVEIN, m_bmultiWaveIn);
	DDX_Control(pDX, IDC_MULTIPLEWAVEIN, m_ctrlMulWaveIn);
}

BEGIN_MESSAGE_MAP(CRecoder2Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CRecoder2Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RECODESTART, &CRecoder2Dlg::OnBnClickedRecodestart)
	ON_BN_CLICKED(IDC_RECODESTOP, &CRecoder2Dlg::OnBnClickedRecodestop)
	ON_CBN_SELCHANGE(IDC_DEVNAME1, &CRecoder2Dlg::OnCbnSelchangeDevname1)
	ON_CBN_SELCHANGE(IDC_DEVNAME2, &CRecoder2Dlg::OnCbnSelchangeDevname2)
	ON_BN_CLICKED(IDC_MULTIPLEWAVEIN, &CRecoder2Dlg::OnBnClickedMultiplewavein)
	ON_BN_CLICKED(IDC_BDEBUG, &CRecoder2Dlg::OnBnClickedBdebug)
END_MESSAGE_MAP()


// CRecoder2Dlg ��Ϣ�������

BOOL CRecoder2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	int nsize = sizeof(m_currentDir);
	ZeroMemory(m_currentDir,nsize);
	GetCurrentDirectory(nsize,m_currentDir);
	sprintf(m_szIniFile,"%s\\config.ini",m_currentDir);//���������ļ�

// 	char logFile[260] = {0};
// 	sprintf(logFile,"%s\\dx.log",m_currentDir);//������־�ļ�
// 	initLogFile(logFile);

	CMixerDev::EnumMixerDevIdUnit(m_mapMixerCaps,m_vectMixerCaps);

	MAPSTRDEVID::iterator i_map;
	int i = 0;
	for(i_map=m_mapMixerCaps.begin();i_map!=m_mapMixerCaps.end();i_map++,i++)
	{
		char dev_name[256] = {0};
		sprintf(dev_name,"MIXERDEV%d",i);
		//Ini_SetString(m_szIniFile,"MIXER",dev_name,(i_map->first).c_str());
		m_ctrlDevName1.AddString((i_map->first).c_str());
		m_ctrlDevName2.AddString((i_map->first).c_str());		
	}
// 	CMixerDev::EnumMixerDev(m_vectMixerCaps);
// 	VECTMIXERCAPS::iterator i_vect;
// 	UINT i = 0;
// 	for (i_vect=m_vectMixerCaps.begin();i_vect!=m_vectMixerCaps.end();i_vect++,i++)
// 	{
// 		char dev_name[256] = {0};
// 		if(!CMixerDev::EnumMixerDevUnit(m_vectMixerLine2,i))
// 		{
// 			continue;
// 		}
// 		sprintf(dev_name,"MIXERDEV%d",i);
// 		Ini_SetString(m_szIniFile,"MIXER",dev_name,(*i_vect).szPname);
// 		m_ctrlDevName1.AddString((*i_vect).szPname);
// 		m_ctrlDevName2.AddString((*i_vect).szPname);
// 		std::string strDevName = (*i_vect).szPname;
// 		m_mapMixerCaps.insert(std::pair<std::string,UINT>(strDevName,i));
// 		VECTMIXERLINE::iterator j_vect;
// 		int j = 0;
// 		for(j_vect=m_vectMixerLine2.begin();j_vect!=m_vectMixerLine2.end();j_vect++)
// 		{
// 			char devunit_name[256] = {0};
// 			sprintf(devunit_name,"%s_MIXERDEVUNIT%d",dev_name,j);
// 			Ini_SetString(m_szIniFile,"MIXER",devunit_name,(*j_vect).szName);
// 			j++;
// 		}
// 	}

	//CMixerDev::EnumMixerDev(m_vectMixerCaps);
// 	int nSize = m_vectMixerCaps.size();
// 	for(int i=0;i<nSize;i++)
// 	{
// 		m_ctrlDevName1.AddString(m_vectMixerCaps[i].szPname);
// 		m_ctrlDevName2.AddString(m_vectMixerCaps[i].szPname);
// 	}
	
	::EnableWindow(m_ctrlDevUnit3.m_hWnd,FALSE);
// 	::EnableWindow(m_ctrlDevName1.m_hWnd,TRUE);
// 	::EnableWindow(m_ctrlDevName2.m_hWnd,TRUE);
// 	::EnableWindow(m_ctrlDevUnit1.m_hWnd,FALSE);
// 	::EnableWindow(m_ctrlDevUnit2.m_hWnd,FALSE);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_wavRecoder1.SetWaveInConfig(2,44100,16);
	m_wavRecoder2.SetWaveInConfig(2,22050,16);


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CRecoder2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CRecoder2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CRecoder2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CRecoder2Dlg::OnBnClickedOk()
{
	OnOK();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CRecoder2Dlg::analysisDevUnit(/*out*/DWORD &nRecordFrom,int n)
{
	nRecordFrom=(DWORD)-1;
	if (n >= m_vectMixerLine.size())
	{
		return ;
	}
	nRecordFrom = m_vectMixerLine[n].dwComponentType;
// 	if (strstr(str,"��·"))
// 	{
// 		nRecordFrom = RECORDFROMLINE;
// 	}
// 	else if (strstr(str,"��˷�"))
// 	{
// 		nRecordFrom = RECORDFROMMICPHONE;
// 	}
// 	else if (strstr(str,"������") || strstr(str,"����"))
// 	{
// 		nRecordFrom = RECORDFROMSTEREO;
// 	}
// 	else
// 	{
// 		MessageBox("���ܽ�������","Recoder");
// 	}
}

void CRecoder2Dlg::OnBnClickedRecodestart()
{
	// TODO: Add your control notification handler code here
	// 	MIXERLINE_COMPONENTTYPE_SRC_LINE
	// 	MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE
	// 	MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER
	if (m_brecodeRunning1 || m_brecodeRunning2)
	{
		return ;
	}
	int nDevName1 = m_ctrlDevName1.GetCurSel();
	int nDevName2 = m_ctrlDevName2.GetCurSel();
	int nDevUnit1 = m_ctrlDevUnit1.GetCurSel();
	int nDevUnit2 = m_ctrlDevUnit2.GetCurSel();
	if((CB_ERR==nDevName1) ||  (CB_ERR==nDevUnit1))
	{
		AfxMessageBox("������ѡ��һ��¼���豸����ָ��¼����Ԫ");
		return ;
	}
	bool bR1,bR2;
	bR1 = false;
	bR2 = false;
	int nSize = m_vectMixerCaps.size();
	do 
	{
		if((CB_ERR!=nDevName1) && (CB_ERR!=nDevUnit1))
		{
			CString strDevName1;
			m_ctrlDevName1.GetLBText(nDevName1,strDevName1);
			DWORD nRecordFrom1;
			analysisDevUnit(nRecordFrom1,nDevUnit1/*strDevUnit1*/);
			if ((DWORD)-1 == nRecordFrom1)
			{
				AfxMessageBox("Device Unit1 error");
				break ;
			}
			MAPSTRDEVID::iterator i;
			std::string findDevName = strDevName1.GetBuffer();
			i = m_mapMixerCaps.find(findDevName);
			if (i!=m_mapMixerCaps.end())//�򿪸��豸
			{
				if (!m_wavRecoder1.RecordOpen(m_hWnd,nRecordFrom1,i->second,strDevName1))
				{
					AfxMessageBox("record 1 open error");
					return ;
				}
				bR1 = true;
				Ini_SetString(m_szIniFile,"MIXERSELECT","MIXERDEV",strDevName1);
				Ini_SetInt(m_szIniFile,"MIXERSELECT","MIXERDEVUNIT",nRecordFrom1);
			}

// 			for (UINT i=0;i<nSize;i++)
// 			{
// 				if (strcmp(strDevName1,m_vectMixerCaps[i].szPname) == 0)//�򿪸��豸
// 				{
// 					if(!m_wavRecoder1.RecordOpen(m_hWnd,nRecordFrom1,i,strDevName1))
// 					{
// 						AfxMessageBox("record 1 open error");
// 						return ;
// 					}
// 				}
// 			}
		}
	} while (false);
	
	do 
	{
		int nDevUnit3 = m_ctrlDevUnit3.GetCurSel();
		if ((CB_ERR!=nDevName2) && (CB_ERR!=nDevUnit2))
		{
			CString strDevName2;
			m_ctrlDevName2.GetLBText(nDevName2,strDevName2);
			DWORD nRecordFrom2;
			analysisDevUnit(nRecordFrom2,nDevUnit2);
			if ((DWORD)-1 == nRecordFrom2)
			{
				AfxMessageBox("Device Unit2 error");
				break ;
			}


			for (UINT i=0;i<nSize;i++)
			{
				if (strcmp(strDevName2,m_vectMixerCaps[i].szPname) == 0)//�򿪸��豸
				{
					if (m_bmultiWaveIn && (nDevUnit3 != nDevUnit2) && (nDevUnit3 != CB_ERR))//�����·��
					{
						DWORD nRecordFrom3;
						//CString strDevUnit3;
						//m_ctrlDevUnit3.GetLBText(nDevUnit3,strDevUnit3);
						analysisDevUnit(nRecordFrom3,nDevUnit3);
						if(!m_wavRecoder2.RecordOpen(m_hWnd,nRecordFrom2,nRecordFrom3,i,strDevName2))//RECORDFROMMICPHONE
						{
							AfxMessageBox("record 2 open error");
							break; ;
						}
					}
					else
					{
						if(!m_wavRecoder2.RecordOpen(m_hWnd,nRecordFrom2,i,strDevName2))//RECORDFROMMICPHONE
						{
							AfxMessageBox("record 2 open error");
							break; ;
						}
					}

				}
			}
			bR2 = true;
		}
	} while (false);

	//��ʼ¼��
	if (bR1)
	{
		m_wavRecoder1.RecordStart();
		m_brecodeRunning1 = true;
	}
	if (bR2)
	{
		m_wavRecoder2.RecordStart();
		m_brecodeRunning2 = true;
	}
	
	if (m_brecodeRunning1 || m_brecodeRunning2)
	{
		SetDlgItemText(IDC_STATUS,"����¼��");
	}
}

void CRecoder2Dlg::OnBnClickedRecodestop()
{
	// TODO: Add your control notification handler code here
	if (!m_brecodeRunning1 && !m_brecodeRunning2)
	{
		return ;
	}
	if (m_brecodeRunning1)
	{
		m_wavRecoder1.RecordStop();
	}
	if (m_brecodeRunning2)
	{
		m_wavRecoder2.RecordStop();
	}
	
	char dir[260] = {0};
	::GetCurrentDirectory(259,dir);
	char recoderFile1[256] ={0};
	char recoderFile2[256] = {0};
	sprintf(recoderFile1,"%s\\Left.wav",dir);
	sprintf(recoderFile2,"%s\\Right.wav",dir);
	if(DeleteFile(recoderFile1)==0||DeleteFile(recoderFile2)==0)
	{
		OutputDebugStringA("delete recoder file failed\n");
	}
	try
	{
		if (m_brecodeRunning1)
		{
			m_wavRecoder1.SaveWavFileFromWaveIn(recoderFile1);
			//m_wavRecoder1.SaveLeftRigthWavFileFormWaveIn(recoderFile1,recoderFile2);
		}
		
		if (m_brecodeRunning2)
		{
			m_wavRecoder2.SaveWavFileFromWaveIn(recoderFile2);
		}
		
	}
	catch (CFileException* e)
	{
		AfxMessageBox("record file write error");
		return;
	}
	if (m_brecodeRunning1)
	{
		m_wavRecoder1.RecordClose();
	}
	if (m_brecodeRunning2)
	{
		m_wavRecoder2.RecordClose();
	}
	
	
	m_brecodeRunning1 = false;
	m_brecodeRunning2 = false;
	SetDlgItemText(IDC_STATUS,"ֹͣ¼��");
	//MSE_RealTimeEvalSave();
}

bool CRecoder2Dlg::ctrlSelectChangeDevName(CComboBox & comboBoxSel,CComboBox& comboBoxUnit,
										   CComboBox & comboBoxCha,int nIndex)
{
	CString devName;
	comboBoxSel.GetLBText(nIndex,devName);
	CString srcName;
	int chaIndex = comboBoxCha.GetCurSel();
	if (CB_ERR != chaIndex)
	{
		comboBoxCha.GetLBText(chaIndex,srcName);
	}
	
	comboBoxCha.ResetContent();
	comboBoxUnit.ResetContent();

	int nSize = m_vectMixerCaps.size();
	MAPSTRDEVID::iterator i;
	//std::string strDevName = devName.GetBuffer();
	i = m_mapMixerCaps.find(devName.GetBuffer());
	if (i!=m_mapMixerCaps.end())
	{	
		if (!CMixerDev::EnumMixerDevUnit(m_vectMixerLine,i->second))
		{
			return false;
		}
		int nUnitSize = m_vectMixerLine.size();
		for(int j=0;j<nUnitSize;j++)
		{
			comboBoxUnit.AddString(m_vectMixerLine[j].szName);
		}
	}
	else
	{
		return false;
	}

// 	for(int i=0;i<nSize;i++)
// 	{
// 		if (strcmp(devName,m_vectMixerCaps[i].szPname) == 0)
// 		{
// 			if (!CMixerDev::EnumMixerDevUnit(m_vectMixerLine,i))
// 			{
// 				return false;
// 			}
// 			int nUnitSize = m_vectMixerLine.size();
// 			for(int j=0;j<nUnitSize;j++)
// 			{
// 				comboBoxUnit.AddString(m_vectMixerLine[j].szName);
// 			}
// 			continue;
// 		}
// 		comboBoxCha.AddString(m_vectMixerCaps[i].szPname);
// 	}
	if (CB_ERR != chaIndex)
	{
		comboBoxCha.SelectString(0,srcName);
	}
	
	return true;
}

void CRecoder2Dlg::OnCbnSelchangeDevname1()
{
	// TODO: Add your control notification handler code here
	ctrlSelectChangeDevName(m_ctrlDevName1,m_ctrlDevUnit1,m_ctrlDevName2,m_ctrlDevName1.GetCurSel());
}

void CRecoder2Dlg::OnCbnSelchangeDevname2()
{
	// TODO: Add your control notification handler code here
	if (!ctrlSelectChangeDevName(m_ctrlDevName2,m_ctrlDevUnit2,m_ctrlDevName1,m_ctrlDevName2.GetCurSel()))
	{
		m_bmultiWaveIn = FALSE;
		UpdateData(FALSE);
		m_ctrlDevUnit3.EnableWindow(FALSE);
		m_ctrlMulWaveIn.EnableWindow(FALSE);
	}
	else
	{
		m_ctrlMulWaveIn.EnableWindow(TRUE);
	}
}

void CRecoder2Dlg::OnBnClickedMultiplewavein()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_bmultiWaveIn)
	{
		m_ctrlDevUnit3.ResetContent();
		int nSize = m_vectMixerLine.size();
		for(int i=0;i<nSize;i++)
		{
			m_ctrlDevUnit3.AddString(m_vectMixerLine[i].szName);
		}
		::EnableWindow(m_ctrlDevUnit3.m_hWnd,TRUE);
		
	}
	else
	{
		::EnableWindow(m_ctrlDevUnit3.m_hWnd,FALSE);

	}
	
}

void CRecoder2Dlg::OnBnClickedBdebug()
{
	UINT nBefore = m_wavRecoder1.GetWaveIn()->getNumDevs();
	UINT nAfter = m_wavRecoder1.GetWaveIn()->getNumDevs();
	char buf[260] = {0};
	sprintf_s(buf,"before %d,after %d.\n",nBefore,nAfter);
	OutputDebugStringA(buf);
// 	for (int i = 0;i<10000;i++)
// 	{
// 		OnBnClickedRecodestart();
// 		Sleep(100000);
// 		OnBnClickedRecodestop();
// 	}
}
