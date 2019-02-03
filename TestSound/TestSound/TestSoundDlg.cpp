// TestSoundDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TestSound.h"
#include "TestSoundDlg.h"

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


// CTestSoundDlg �Ի���




CTestSoundDlg::CTestSoundDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestSoundDlg::IDD, pParent)
	, m_fVol(0.5f)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CTestSoundDlg::~CTestSoundDlg()
{
	if (m_sound)
	{
		delete m_sound;
	}
}

void CTestSoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SVOLUME, m_ctrlSliderVol);
}

BEGIN_MESSAGE_MAP(CTestSoundDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PLAY, &CTestSoundDlg::OnBnClickedPlay)
	ON_BN_CLICKED(IDC_STOP, &CTestSoundDlg::OnBnClickedStop)
//	ON_NOTIFY(NM_THEMECHANGED, IDC_SVOLUME, &CTestSoundDlg::OnNMThemeChangedSvolume)
ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CTestSoundDlg ��Ϣ�������

BOOL CTestSoundDlg::OnInitDialog()
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

	m_sound = new CGLSound(m_hWnd);

	m_ctrlSliderVol.SetRange(0,100);
	m_ctrlSliderVol.SetPos(m_fVol*100);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CTestSoundDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestSoundDlg::OnPaint()
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
HCURSOR CTestSoundDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTestSoundDlg::OnBnClickedPlay()
{

	m_sound->SoundLoad("C:\\Users\\Administrator\\Desktop\\bgm.mp3",false);
	m_sound->SoundSetGStreamVol(0.3f);
	m_sound->SoundPlay();
}

void CTestSoundDlg::OnBnClickedStop()
{
	m_sound->SoundStop();
}

void CTestSoundDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);

	if (pScrollBar->m_hWnd == m_ctrlSliderVol.GetSafeHwnd())
	{
		int p=m_ctrlSliderVol.GetPos();
		float f = p/100.0f;
		if (f != m_fVol)
		{
			m_fVol = f;
			m_sound->SoundSetGStreamVol(m_fVol);
		}
	}
}
