// GlpWmaPlayerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GlpWmaPlayer.h"
#include "GlpWmaPlayerDlg.h"

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


// CGlpWmaPlayerDlg �Ի���




CGlpWmaPlayerDlg::CGlpWmaPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGlpWmaPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGlpWmaPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGlpWmaPlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OPENFILE, &CGlpWmaPlayerDlg::OnBnClickedOpenfile)
	ON_BN_CLICKED(IDC_BPLAY, &CGlpWmaPlayerDlg::OnBnClickedBplay)
	ON_BN_CLICKED(IDC_BPAUSE, &CGlpWmaPlayerDlg::OnBnClickedBpause)
	ON_BN_CLICKED(IDC_BSTOP, &CGlpWmaPlayerDlg::OnBnClickedBstop)
	ON_BN_CLICKED(IDC_BSWITCHAUDIO, &CGlpWmaPlayerDlg::OnBnClickedBswitchaudio)
END_MESSAGE_MAP()


// CGlpWmaPlayerDlg ��Ϣ�������

BOOL CGlpWmaPlayerDlg::OnInitDialog()
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

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CGlpWmaPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGlpWmaPlayerDlg::OnPaint()
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
HCURSOR CGlpWmaPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

std::wstring CGlpWmaPlayerDlg::GetMoviePath()
{
	std::wstring ws;
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	TCHAR  szBuffer[MAX_PATH];
	szBuffer[0] = NULL;

	static const TCHAR szFilter[]  
	= TEXT("windows media audio (.wma)\0*.wma\0\0");
// 		TEXT("Video Files (.ASF, .AVI, .MPG, .MPEG, .VOB, .QT, .WMV)\0*.ASF;*.AVI;*.MPG;*.MPEG;*.VOB;*.QT;*.WMV\0") \
// 		TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize         = sizeof(OPENFILENAME);
	ofn.hwndOwner           = m_hWnd;
	ofn.hInstance           = NULL;
	ofn.lpstrFilter         = szFilter;
	ofn.nFilterIndex        = 1;
	ofn.lpstrCustomFilter   = NULL;
	ofn.nMaxCustFilter      = 0;
	ofn.lpstrFile           = szBuffer;
	ofn.nMaxFile            = MAX_PATH;
	ofn.lpstrFileTitle      = NULL;
	ofn.nMaxFileTitle       = 0;
	ofn.lpstrInitialDir     = NULL;
	ofn.lpstrTitle          = TEXT("Select a video file to play...");
	ofn.Flags               = OFN_HIDEREADONLY;
	ofn.nFileOffset         = 0;
	ofn.nFileExtension      = 0;
	ofn.lpstrDefExt         = TEXT("AVI");
	ofn.lCustData           = 0L;
	ofn.lpfnHook            = NULL;
	ofn.lpTemplateName  = NULL; 

	if (GetOpenFileName (&ofn))  // user specified a file
	{
		return  (ws = szBuffer) ;
	}

	return L"";
}

void CGlpWmaPlayerDlg::OnBnClickedOpenfile()
{
	m_file = GetMoviePath();
	if (m_file != L"")
	{
		m_cMG.initGraph(m_file.c_str());
	}
}

void CGlpWmaPlayerDlg::OnBnClickedBplay()
{
	m_cMG.start();
}

void CGlpWmaPlayerDlg::OnBnClickedBpause()
{
	m_cMG.pause();
}

void CGlpWmaPlayerDlg::OnBnClickedBstop()
{
	m_cMG.stop();
}

void CGlpWmaPlayerDlg::OnBnClickedBswitchaudio()
{
	static BOOL b = TRUE;
	b = !b;
	m_cMG.switchAudio(b);
}
