// IconExampleDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include <iostream>
#include <boost/array.hpp>
#include <cmath>
#include "avhttp.hpp"
#include "IconExample.h"
#include "IconExampleDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIconExampleDlg �Ի���

#include <string.h>
#include <stdio.h>
#define ID_ICON_GLP 1000

CIconExampleDlg::CIconExampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIconExampleDlg::IDD, pParent)
	,m_pMDHelp(NULL)
	, m_strUrl(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CIconExampleDlg::~CIconExampleDlg()
{
	exitNotifyIcon();
}

void CIconExampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_ctrlUrl);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
	DDX_Text(pDX, IDC_EDIT1, m_strUrl);
}

BEGIN_MESSAGE_MAP(CIconExampleDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_MYICON,&CIconExampleDlg::onMyIcon)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_ICON_EXIT, &CIconExampleDlg::OnIconExit)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON1, &CIconExampleDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CIconExampleDlg ��Ϣ�������

BOOL CIconExampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	m_ctrlProgress.SetRange(0,100);
	m_ctrlProgress.SetPos(0);
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CIconExampleDlg::OnPaint()
{
	if (IsIconic())
	{
		return ;
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

void CIconExampleDlg::exitNotifyIcon()
{
	NOTIFYICONDATA tnid; 
	tnid.cbSize=sizeof(NOTIFYICONDATA); 
	tnid.hWnd=m_hWnd; 
	tnid.uID=ID_ICON_GLP;//��֤ɾ���������ǵ�ͼ�� 
	Shell_NotifyIcon(NIM_DELETE,&tnid);   //ж��ͼ�� 
}

LRESULT CIconExampleDlg::onMyIcon(WPARAM wParam,LPARAM lParam)
{
	UINT uID;//��������Ϣ��ͼ���ID 
	UINT uMouseMsg;//��궯�� 
	POINT pt; 
	uID=(UINT) wParam; 
	uMouseMsg=(UINT) lParam; 
	if(uMouseMsg==WM_LBUTTONDOWN)//����ǵ������ 
	{ 
		switch(uID) 
		{ 
		case ID_ICON_GLP: 
			SetForegroundWindow();//������ǰ�� 
			ShowWindow(SW_SHOWNORMAL);//�򵥵���ʾ���������¶� 
			break; 
		} 
	} 
	if(uMouseMsg==WM_RBUTTONDOWN)//����ǵ����Ҽ� 
	{ 
		switch(uID) 
		{ 
		case ID_ICON_GLP: 
			CMenu menu;   //�����Ҽ��˵����� 
			GetCursorPos(&pt);   //��ȡ��ǰ���λ�� 
			menu.LoadMenu(IDR_MENU1);   //�����Ҽ���ݲ˵� 
			SetForegroundWindow();//������ǰ�� 
			CMenu* pmenu;    //�����Ҽ��˵�ָ�� 
			pmenu=menu.GetSubMenu(0);      //�ú���ȡ�ñ�ָ���˵����������ʽ�˵����Ӳ˵��ľ�� 
			ASSERT(pmenu!=NULL);       
			pmenu-> TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN,pt.x,pt.y,this);   //��ָ��λ����ʾ�Ҽ���ݲ˵� 
			HMENU hmenu=pmenu -> Detach();   
			pmenu ->DestroyMenu();   
		} 
	} 

	return 0;
}
//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CIconExampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CIconExampleDlg::OnIconExit()
{
	AfxPostQuitMessage(0);
}

void CIconExampleDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(nType == SIZE_MINIMIZED)
	{
		NOTIFYICONDATA tnd;   //NOTIFYICONDATA �ṹ���� 
		tnd.cbSize=sizeof(NOTIFYICONDATA);    //�ṹ��Ĵ�С�����ֽ�Ϊ��λ�� 
		tnd.hWnd=this->m_hWnd;    //���ھ������ʾ�Ĵ�����������������ͼ����ص���Ϣ�� 
		tnd.uID=ID_ICON_GLP;  //Ӧ�ó������������ͼ��ı�ʶ����Shell_NotifyIcon��������ʱ��hWnd��uID��Ա������ʾ����Ҫ������ͼ�� 
		tnd.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;   //ָʾ������Щ������ԱΪ�Ϸ����� 
		tnd.uCallbackMessage=WM_MYICON;     //Ӧ�ó��������Ϣ��ʾ��������ͼ������������¼�����ʹ�ü���ѡ��򼤻�ͼ��ʱ��ϵͳ��ʹ�ô˱�ʾ����hWnd��Ա��ʾ�Ĵ��ڷ�����Ϣ�� 
		tnd.hIcon=AfxGetApp()->LoadIcon(ID_ICON_GLP_);   //ͼ��ID 
		wcscpy(tnd.szTip,L"��ʾ�ı�");                  //����ͼ����ʾ�ı� 
		Shell_NotifyIcon(NIM_ADD,&tnd);      //��װ����ͼ�� 
		ShowWindow(SW_HIDE); //����������
	}
	// TODO: Add your message handler code here
}

DWORD CALLBACK CIconExampleDlg::DownloadThread(LPVOID pParam)
{
	CIconExampleDlg* p = (CIconExampleDlg*)pParam;
	try
	{
		boost::asio::io_service io;
		avhttp::multi_download d(io);

		avhttp::settings s;
		boost::system::error_code ec;
		s.check_certificate = false;
		// s.m_download_rate_limit = 102400;

		char buf[400] = {0};
		WideCharToMultiByte(CP_ACP,0,p->m_strUrl.GetBuffer(),p->m_strUrl.GetLength(),buf,400,"",NULL);
		d.start(buf, s,ec);

		boost::thread t(boost::bind(&boost::asio::io_service::run,& io));
		boost::int64_t bytes_download = 0;
		int percent = 0;
		p->m_ctrlProgress.SetPos(percent);

		while(percent != 100 && !d.stopped())
		{
			boost::int64_t file_size = d.file_size();
			bytes_download = d.bytes_download();
			percent = (int)(((double)bytes_download / (double)file_size) * 100.0f);
			p->m_ctrlProgress.SetPos(percent);
			Sleep(500);
		}

		t.join();

		MessageBoxA(p->GetSafeHwnd(),"Download completed","Info",MB_OK);
	}
	catch(std::exception& e)
	{
		MessageBoxA(p->GetSafeHwnd(),e.what(),"Info",MB_OK);
	}

	return 0;
}

void CIconExampleDlg::OnBnClickedButton1()
{
	UpdateData();
	CString url;
	m_ctrlUrl.GetWindowText(url);
	if (url == "")
	{
		return ;
	}
	
	DWORD threadID;
	HANDLE hThread = CreateThread(NULL,0,&CIconExampleDlg::DownloadThread,(void*)this,0,&threadID);
	CloseHandle(hThread);
}
