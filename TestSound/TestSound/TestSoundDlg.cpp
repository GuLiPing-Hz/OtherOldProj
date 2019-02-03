// TestSoundDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TestSound.h"
#include "TestSoundDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CTestSoundDlg 对话框




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


// CTestSoundDlg 消息处理程序

BOOL CTestSoundDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	m_sound = new CGLSound(m_hWnd);

	m_ctrlSliderVol.SetRange(0,100);
	m_ctrlSliderVol.SetPos(m_fVol*100);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestSoundDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
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
