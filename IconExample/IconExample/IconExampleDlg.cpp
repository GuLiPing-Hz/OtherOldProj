// IconExampleDlg.cpp : 实现文件
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


// CIconExampleDlg 对话框

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


// CIconExampleDlg 消息处理程序

BOOL CIconExampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	m_ctrlProgress.SetRange(0,100);
	m_ctrlProgress.SetPos(0);
	// TODO: 在此添加额外的初始化代码
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIconExampleDlg::OnPaint()
{
	if (IsIconic())
	{
		return ;
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

void CIconExampleDlg::exitNotifyIcon()
{
	NOTIFYICONDATA tnid; 
	tnid.cbSize=sizeof(NOTIFYICONDATA); 
	tnid.hWnd=m_hWnd; 
	tnid.uID=ID_ICON_GLP;//保证删除的是我们的图标 
	Shell_NotifyIcon(NIM_DELETE,&tnid);   //卸载图标 
}

LRESULT CIconExampleDlg::onMyIcon(WPARAM wParam,LPARAM lParam)
{
	UINT uID;//发出该消息的图标的ID 
	UINT uMouseMsg;//鼠标动作 
	POINT pt; 
	uID=(UINT) wParam; 
	uMouseMsg=(UINT) lParam; 
	if(uMouseMsg==WM_LBUTTONDOWN)//如果是单击左键 
	{ 
		switch(uID) 
		{ 
		case ID_ICON_GLP: 
			SetForegroundWindow();//放置在前面 
			ShowWindow(SW_SHOWNORMAL);//简单的显示主窗口完事儿 
			break; 
		} 
	} 
	if(uMouseMsg==WM_RBUTTONDOWN)//如果是单击右键 
	{ 
		switch(uID) 
		{ 
		case ID_ICON_GLP: 
			CMenu menu;   //定义右键菜单对象 
			GetCursorPos(&pt);   //获取当前鼠标位置 
			menu.LoadMenu(IDR_MENU1);   //载入右键快捷菜单 
			SetForegroundWindow();//放置在前面 
			CMenu* pmenu;    //定义右键菜单指针 
			pmenu=menu.GetSubMenu(0);      //该函数取得被指定菜单激活的下拉式菜单或子菜单的句柄 
			ASSERT(pmenu!=NULL);       
			pmenu-> TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN,pt.x,pt.y,this);   //在指定位置显示右键快捷菜单 
			HMENU hmenu=pmenu -> Detach();   
			pmenu ->DestroyMenu();   
		} 
	} 

	return 0;
}
//当用户拖动最小化窗口时系统调用此函数取得光标显示。
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
		NOTIFYICONDATA tnd;   //NOTIFYICONDATA 结构声明 
		tnd.cbSize=sizeof(NOTIFYICONDATA);    //结构体的大小，以字节为单位。 
		tnd.hWnd=this->m_hWnd;    //窗口句柄，标示的窗口用来接收与托盘图标相关的消息。 
		tnd.uID=ID_ICON_GLP;  //应用程序定义的任务栏图标的标识符。Shell_NotifyIcon函数调用时，hWnd和uID成员用来标示具体要操作的图标 
		tnd.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;   //指示具体哪些其他成员为合法数据 
		tnd.uCallbackMessage=WM_MYICON;     //应用程序定义的消息标示，当托盘图标区域发生鼠标事件或者使用键盘选择或激活图标时，系统将使用此标示向由hWnd成员标示的窗口发送消息。 
		tnd.hIcon=AfxGetApp()->LoadIcon(ID_ICON_GLP_);   //图标ID 
		wcscpy(tnd.szTip,L"提示文本");                  //托盘图标提示文本 
		Shell_NotifyIcon(NIM_ADD,&tnd);      //安装托盘图标 
		ShowWindow(SW_HIDE); //隐藏主窗口
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
