// XGSQLiteTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XGSQLite/MusicSQLObject.h"
#include "XGSQLiteTest.h"
#include "XGSQLiteTestDlg.h"


#include <time.h>

using namespace std;
using namespace XGSQLITE;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CXGSQLiteTestDlg dialog




CXGSQLiteTestDlg::CXGSQLiteTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CXGSQLiteTestDlg::IDD, pParent)
	, m_MusicName(_T(""))
	, m_MusicNamePY(_T(""))
	, m_MusicNameCount(0)
	, m_MusicSinger(_T(""))
	, m_MusicSingerPY(_T(""))
	, m_IsSinger(FALSE)
	, m_InitCount(0)
	, m_iMusicId(0)
	, m_MusicNew(FALSE)
	, m_PageIndex(0)
	, m_allpage(0)
	, m_Debugtime(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CXGSQLiteTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NAME, m_MusicName);
	DDX_Text(pDX, IDC_EDIT_NAMEPY, m_MusicNamePY);
	DDX_Text(pDX, IDC_EDIT_NAMECOUNT, m_MusicNameCount);
	DDX_Text(pDX, IDC_EDIT_SINGER, m_MusicSinger);
	DDX_Text(pDX, IDC_EDIT_SINGERPY, m_MusicSingerPY);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_CtrlMusicType);
	DDX_Control(pDX, IDC_COMBO_SINGERTYPE, m_CtrlMusicSingerType);
	DDX_Control(pDX, IDC_COMBO_LANGTYPE, m_CtrlMusicLangType);
	DDX_Radio(pDX, IDC_RADIO_SONG, m_IsSinger);
	DDX_Text(pDX, IDC_EDIT1, m_InitCount);
	DDX_Control(pDX, IDC_LIST_RESULT, m_CtrlResultList);
	DDX_Control(pDX, IDC_EDIT_NAME, m_CtrlName);
	DDX_Control(pDX, IDC_EDIT_NAMEPY, m_CtrlNamePY);
	DDX_Control(pDX, IDC_EDIT_NAMECOUNT, m_CtrlNameCount);
	DDX_Control(pDX, IDC_EDIT_SINGER, m_CtrlSinger);
	DDX_Control(pDX, IDC_EDIT_SINGERPY, m_CtrlSingerPY);
	DDX_Text(pDX, IDC_EDIT_MUSICID, m_iMusicId);
	DDX_Control(pDX, IDC_COMBO_TOPINFO, m_CtrlMusicTopInfo);
	DDX_Check(pDX, IDC_CHECK_NEW, m_MusicNew);
	DDX_Text(pDX, IDC_STATIC_PAGEINDEX, m_PageIndex);
	DDX_Text(pDX, IDC_STATIC_ALLPAGE, m_allpage);
	DDX_Text(pDX, IDC_STATIC_DEBUG, m_Debugtime);
}

BEGIN_MESSAGE_MAP(CXGSQLiteTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CXGSQLiteTestDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CXGSQLiteTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_RADIO_SONG, &CXGSQLiteTestDlg::OnBnClickedRadioSong)
	ON_BN_CLICKED(IDC_RADIO_SINGER, &CXGSQLiteTestDlg::OnBnClickedRadioSinger)
	ON_BN_CLICKED(IDC_BUTTON2, &CXGSQLiteTestDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BTN_PRE, &CXGSQLiteTestDlg::OnBnClickedBtnPre)
	ON_BN_CLICKED(IDC_BTN_NEXT, &CXGSQLiteTestDlg::OnBnClickedBtnNext)
	ON_BN_CLICKED(IDCANCEL, &CXGSQLiteTestDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_SINGERINFO, &CXGSQLiteTestDlg::OnBnClickedBtnSingerinfo)
END_MESSAGE_MAP()


// CXGSQLiteTestDlg message handlers

BOOL CXGSQLiteTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_InitCount=10000;

	m_CtrlMusicLangType.InsertString(0,_T("请选择..."));
	m_CtrlMusicLangType.InsertString(1,_T("国语"));
	m_CtrlMusicLangType.InsertString(2,_T("英语"));
	m_CtrlMusicLangType.InsertString(3,_T("粤语"));
	m_CtrlMusicLangType.SetCurSel(0);

	m_CtrlMusicType.InsertString(0,_T("请选择..."));
	m_CtrlMusicType.InsertString(1,_T("经典歌曲"));
	m_CtrlMusicType.InsertString(2,_T("摇滚"));
	m_CtrlMusicType.InsertString(3,_T("红歌"));
	m_CtrlMusicType.SetCurSel(0);

	m_CtrlMusicSingerType.InsertString(0,_T("请选择..."));
	m_CtrlMusicSingerType.InsertString(1,_T("大陆歌手"));
	m_CtrlMusicSingerType.InsertString(2,_T("港台歌手"));
	m_CtrlMusicSingerType.InsertString(3,_T("欧美歌手"));
	m_CtrlMusicSingerType.InsertString(4,_T("日韩歌手"));
	m_CtrlMusicSingerType.SetCurSel(0);

	m_CtrlMusicTopInfo.InsertString(0,_T("请选择..."));
	m_CtrlMusicTopInfo.InsertString(1,_T("热歌排行榜"));
	m_CtrlMusicTopInfo.InsertString(2,_T("金曲排行榜"));
	m_CtrlMusicTopInfo.InsertString(3,_T("点唱排行榜"));
	m_CtrlMusicTopInfo.InsertString(4,_T("劲歌排行榜"));
	m_CtrlMusicTopInfo.SetCurSel(0);

	SetCtrlStatus();

	m_pMusicObject=new CMusicSQLObject("song.db");
	if(m_pMusicObject==NULL)
		m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),_T("OnInitDialog error"));

	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CXGSQLiteTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CXGSQLiteTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CXGSQLiteTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CXGSQLiteTestDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	m_CtrlResultList.ResetContent();
	//CMusicSQLObject* pMusicObj=new CMusicSQLObject("song.db");
	m_pMusicObject->ClearSearchCondition();

	if(m_CtrlMusicLangType.GetCurSel()>0)
		m_pMusicObject->SetSearchConditionLangType(m_CtrlMusicLangType.GetCurSel());

	if(m_CtrlMusicType.GetCurSel()>0)
		m_pMusicObject->SetSearchConditionSongType(m_CtrlMusicType.GetCurSel());

	if(m_CtrlMusicSingerType.GetCurSel()>0)
		m_pMusicObject->SetSearchConditionSingerType(m_CtrlMusicSingerType.GetCurSel());
	
	if(m_MusicNameCount>0)
		m_pMusicObject->SetSearchConditionNameCount(m_MusicNameCount);

	if(m_CtrlMusicTopInfo.GetCurSel()>0)
		m_pMusicObject->SetSearchConditionTopType(m_CtrlMusicTopInfo.GetCurSel());
	
	m_pMusicObject->SetSearchConditionName((char*)(LPCTSTR)m_MusicName);
	m_pMusicObject->SetSearchConditionNamePY((char*)(LPCTSTR)m_MusicNamePY);
	m_pMusicObject->SetSearchConditionSinger((char*)(LPCTSTR)m_MusicSinger);
	m_pMusicObject->SetSearchConditionSingerPY((char*)(LPCTSTR)m_MusicSingerPY);

	if(m_MusicNew)
		m_pMusicObject->SetSearchConditionNew(1);


	char temp[1024]={0};
	//搜索歌曲
	if(m_IsSinger==FALSE)
	{
		list<CMusicInfo*> _list;

		//int count=pMusicObj->GetMusicCount();

		
		//int count = pMusicObj->GetMusicInfo(1,10,_list);
		int count = m_pMusicObject->GetMusicCount();
		m_allpage=count%10>0?count/10+1:count/10;
		m_PageIndex=1;
		SearchMusic();
		

		/*list<CMusicInfo*>::iterator it=_list.begin();
		for(;it!=_list.end();it++)
		{
			CMusicInfo* pDate=*it;
			sprintf_s(temp,sizeof(temp)-1,"%d,%s,%s,%s,%s ,%d,%d ,%d",
				pDate->m_music_id,
				pDate->m_music_name,
				pDate->m_music_singer,
				pDate->m_music_typeinfo,
				pDate->m_music_topinfo,
				pDate->m_music_new,
				pDate->m_music_recommend,
				pDate->m_music_pop
				);
			CString str1(temp);
			m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str1);
			delete pDate;
		}*/
	}
	//搜索歌手
	else
	{

		list<CSingerInfo*> _list;

		int count = m_pMusicObject->GetSingerCount();
		m_allpage=count%10>0?count/10+1:count/10;
		m_PageIndex=1;
		SearchSinger();

		//memset(temp,0,sizeof(temp));
		//sprintf_s(temp,sizeof(temp)-1,"time:%d,recordcount:%d",end,count);
		////str.Format(_T("time:%d,recordcount:%d"),end,count);
		//CString str(temp);
		//m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str);
		//list<CSingerInfo*>::iterator it=_list.begin();
		//for(;it!=_list.end();it++)
		//{
		//	CSingerInfo* pDate=*it;
		//	memset(temp,0,sizeof(temp));
		//	//str.Format(_T("%s"),pDate->m_singername);
		//	sprintf_s(temp,sizeof(temp)-1,"%s,%d",pDate->m_singername,pDate->m_songnum);
		//	CString str1(temp);
		//	m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str1);
		//	delete pDate;
		//}

	}

	UpdateData(FALSE);
}

//生成模拟数据
void CXGSQLiteTestDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	CMusicSQLObject* pObject=new CMusicSQLObject("song.db");
	if(pObject!=NULL)
	{
		if(pObject->CreateDataBase()==XGSQLITE_FAILED)
			return;
	}
	else
	{
			return;
	}
	//插入数据
	list<CMusicInfo*> _list;
	
		char temp[200]={0};
		for(int i=0;i<m_InitCount;i++)
		{
			CMusicInfo* pDate=new CMusicInfo();
			pDate->ClearInfo();
			pDate->m_music_id=i;
			memset(temp,0,sizeof(temp));
			sprintf_s(temp,sizeof(temp),"aaa%06d",i);
			strcpy_s(pDate->m_music_name,sizeof(pDate->m_music_name),temp);
			strcpy_s(pDate->m_music_filename,sizeof(pDate->m_music_filename),temp);
			strcpy_s(pDate->m_music_namepy,sizeof(pDate->m_music_namepy),temp);

			memset(temp,0,sizeof(temp));
			sprintf_s(temp,sizeof(temp),"bbb%06d",i);
			strcpy_s(pDate->m_music_singer,sizeof(pDate->m_music_singer),temp);
			strcpy_s(pDate->m_music_singerpy,sizeof(pDate->m_music_singerpy),temp);

			memset(temp,0,sizeof(temp));
			sprintf_s(temp,sizeof(temp),"[30,60][80,120]");
			strcpy_s(pDate->m_music_timemark,sizeof(pDate->m_music_timemark),temp);

			pDate->m_music_langtype=i%4;
			//pDate->m_music_type=i%3;
			memset(pDate->m_music_typeinfo,0,sizeof(pDate->m_music_typeinfo));
			for(int j=0;j<16;j++)
			{
				if(j==i%9)
					pDate->m_music_typeinfo[j]='1';
				else
					pDate->m_music_typeinfo[j]='0';
			}
			pDate->m_music_singertype=i%6;
			pDate->m_music_lrbc=i%2;
			pDate->m_music_namecount=i%10;
			ZeroMemory(pDate->m_music_musicpos,20);
			//pDate->m_music_musicpos=0;
			pDate->m_music_lighteffect=0;
			pDate->m_music_voicevalue=10;
			pDate->m_music_new=i%2;
			pDate->m_music_recommend=i%2;
			pDate->m_music_pop=i%2;

			memset(pDate->m_music_topinfo,0,sizeof(pDate->m_music_topinfo));
			for(int j=0;j<16;j++)
			{
				if(j==i%5)
					pDate->m_music_topinfo[j]='1';
				else
					pDate->m_music_topinfo[j]='0';
			}
			//pDate->m_music_subtype=0;
			//pDate->m_music_hot=0;
			//pDate->m_music_new=0;
			
			_list.push_front(pDate);
		}
		
		DWORD startm=GetTickCount();
		if(pObject->PutMusicInfo(_list)==XGSQLITE_FAILED)
		{
				m_CtrlResultList.AddString(_T("Insert Error"));
				delete pObject;
				pObject=NULL;
		}

		DWORD time=GetTickCount()-startm;

		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"time:%d",time);
		CString str(temp);
		//str.Format(_T("time:%d"),time);
		m_CtrlResultList.AddString(str);
		//m_CtrlResult.SetWindowText(str);

		list<CMusicInfo*>::iterator it=_list.begin();
		for(;it!=_list.end();it++)
		{
			CMusicInfo* pDate=(CMusicInfo*)*it;
			if(pDate!=NULL)
			{
				delete pDate;
				pDate=NULL;
			}
		}
		delete pObject;
		pObject=NULL;
}

void CXGSQLiteTestDlg::OnBnClickedRadioSong()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	SetCtrlStatus();

	UpdateData(FALSE);
}

void CXGSQLiteTestDlg::OnBnClickedRadioSinger()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	SetCtrlStatus();

	UpdateData(FALSE);
}

//控件状态设定
void CXGSQLiteTestDlg::SetCtrlStatus()
{
	if(m_IsSinger)		//搜索歌手
	{
		m_CtrlName.SetReadOnly(TRUE);
		m_CtrlNamePY.SetReadOnly(TRUE);
		m_CtrlNameCount.SetReadOnly(TRUE);
		m_CtrlMusicType.EnableWindow(FALSE);
		m_CtrlMusicLangType.EnableWindow(FALSE);
		m_CtrlMusicTopInfo.EnableWindow(FALSE);

		m_CtrlMusicSingerType.EnableWindow(TRUE);
		m_CtrlSinger.SetReadOnly(FALSE);
		m_CtrlSingerPY.SetReadOnly(FALSE);
	}
	else
	{
		m_CtrlName.SetReadOnly(FALSE);
		m_CtrlNamePY.SetReadOnly(FALSE);
		m_CtrlNameCount.SetReadOnly(FALSE);
		m_CtrlMusicType.EnableWindow(TRUE);
		m_CtrlMusicLangType.EnableWindow(TRUE);
		m_CtrlMusicTopInfo.EnableWindow(TRUE);

		m_CtrlMusicSingerType.EnableWindow(FALSE);
		m_CtrlSinger.SetReadOnly(FALSE);
		m_CtrlSingerPY.SetReadOnly(TRUE);
	}
}

void CXGSQLiteTestDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	m_CtrlResultList.ResetContent();
	CMusicSQLObject* pObject=new CMusicSQLObject("song.db");
	if(pObject==NULL)
	{
			return;
	}

	CMusicInfo musicnfo;

	int ret=pObject->GetMusicInfoById(m_iMusicId,musicnfo);

	if(ret==0)
	{
		char temp[1024]={0};
		sprintf_s(temp,sizeof(temp)-1,"%d,%s,%s,%d,%s,%s ,%d,%d,%d ",
				musicnfo.m_music_id,
				musicnfo.m_music_name,
				musicnfo.m_music_singer,
				musicnfo.m_music_langtype,
				musicnfo.m_music_typeinfo,
				musicnfo.m_music_topinfo,
				musicnfo.m_music_new,
				musicnfo.m_music_recommend,
				musicnfo.m_music_pop
				);
			CString str1(temp);
			m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str1);
	}

	UpdateData(FALSE);
}

//上一页
void CXGSQLiteTestDlg::OnBnClickedBtnPre()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_PageIndex<=1)
		return;

	m_PageIndex--;

	if(m_IsSinger==FALSE)
		SearchMusic();
	else
		SearchSinger();

	UpdateData(FALSE);
}

//下一页
void CXGSQLiteTestDlg::OnBnClickedBtnNext()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_PageIndex>=m_allpage)
		return;

	m_PageIndex++;

	if(m_IsSinger==FALSE)
		SearchMusic();
	else
		SearchSinger();

	UpdateData(FALSE);
}

void CXGSQLiteTestDlg::SearchMusic()
{

	m_CtrlResultList.ResetContent();

		char temp[1024]={0};

		list<CMusicInfo*> _list;

		//int count=pMusicObj->GetMusicCount();

		DWORD start=GetTickCount();
		m_pMusicObject->GetMusicInfo(m_PageIndex,10,_list);
		DWORD end=GetTickCount()-start;

		m_Debugtime=end;

		memset(temp,0,sizeof(temp));
		//str.Format(_T("time:%d,recordcount:%d "),end,count);
		//m_CtrlResultList.AddString(str);
		//sprintf_s(temp,sizeof(temp)-1,"time:%d,recordcount:%d ",end,count);
		//CString str(temp);
		////string str(temp);
		//m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str);

		list<CMusicInfo*>::iterator it=_list.begin();
		for(;it!=_list.end();it++)
		{
			CMusicInfo* pDate=*it;
			sprintf_s(temp,sizeof(temp)-1,"%d,%s,%s,%s,%s ,%d,%d ,%d",
				pDate->m_music_id,
				pDate->m_music_name,
				pDate->m_music_singer,
				pDate->m_music_typeinfo,
				pDate->m_music_topinfo,
				pDate->m_music_new,
				pDate->m_music_recommend,
				pDate->m_music_pop
				);
			CString str1(temp);
			m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str1);
			delete pDate;
		}
}


void CXGSQLiteTestDlg::SearchSinger()
{
	m_CtrlResultList.ResetContent();

	char temp[1024]={0};
	//搜索歌手
		list<CSingerInfo*> _list;
		//int count=pMusicObj->GetSingerCount();

		DWORD start=GetTickCount();
		m_pMusicObject->GetSingerList(m_PageIndex,10,_list);
		DWORD end=GetTickCount()-start;

		m_Debugtime=end;
		//memset(temp,0,sizeof(temp));
		//sprintf_s(temp,sizeof(temp)-1,"time:%d,recordcount:%d",end,count);
		////str.Format(_T("time:%d,recordcount:%d"),end,count);
		//CString str(temp);
		//m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str);
		list<CSingerInfo*>::iterator it=_list.begin();
		for(;it!=_list.end();it++)
		{
			CSingerInfo* pDate=*it;
			memset(temp,0,sizeof(temp));
			//str.Format(_T("%s"),pDate->m_singername);
			sprintf_s(temp,sizeof(temp)-1,"%s,%d",pDate->m_singername,pDate->m_songnum);
			CString str1(temp);
			m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str1);
			delete pDate;
		}
	
}
void CXGSQLiteTestDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if(m_pMusicObject!=NULL)
	{
		delete m_pMusicObject;
		m_pMusicObject=NULL;
	}
	OnCancel();
}

void CXGSQLiteTestDlg::OnBnClickedBtnSingerinfo()
{
	UpdateData();
	// TODO: Add your control notification handler code here
	m_CtrlResultList.ResetContent();

	CMusicSQLObject* pObject=new CMusicSQLObject("song.db");
	if(pObject==NULL)
	{
			return;
	}

	CSingerInfo singerinfo;
	if(pObject->GetSingerByMusicID(m_iMusicId,singerinfo)==XGSQLITE_SUCCESSD)
	{
		char temp[1024]={0};

		sprintf_s(temp,sizeof(temp)-1,"%s,%d",singerinfo.m_singername,singerinfo.m_songnum);
		CString str1(temp);
		m_CtrlResultList.InsertString(m_CtrlResultList.GetCount(),str1);
	}
	else
	{
		MessageBox(_T("查询失败"));
	}

	UpdateData(FALSE);
}
