#include "StdAfx.h"
#include "OpenGLWindow.h"

COpenGLWindow* COpenGLWindow::cls_window = NULL;

bool parseLine(char* buf,SongListInfo& sli,std::string& category)
{
	if(!buf)
		return false;

	if(buf[0]=='\n' || buf[0]==',')
		return false;

	char* pOld = buf;
	char* pBuffer = strchr(pOld,',');
	if(pBuffer)
	{
		*pBuffer = '\0';
		sli.songname = pOld;
		pOld = pBuffer+1;
	}
	pBuffer = strchr(pOld,',');
	if (pBuffer)
	{
		*pBuffer = '\0';
		sli.singername = pOld;
		pOld = pBuffer+1;
	}
	pBuffer = strchr(pOld,',');
	if(pBuffer)
	{
		*pBuffer = '\0';
		sli.filename = pOld;
		pOld = pBuffer+1;
	}
	pBuffer = strchr(pOld,',');
	if (pBuffer)
	{
		pBuffer = '\0';
		category = pOld;
		pOld = pBuffer+1;
	}
	else
	{
		category = pOld;
	}

	return true;
}

bool readCSV(const char* csvFile,MAPCATEGORY& mc)
{
	char buf[1024] = {0};

	if(!csvFile)
		goto failed;

	FILE* fp = fopen(csvFile,"r");
	if(!fp)
		goto failed;

	while(fgets(buf,1024,fp))
	{
		SongListInfo sli;
		std::string c;
		if(parseLine(buf,sli,c))
		{
			MAPCATEGORY::iterator it = mc.find(c);
			if(it == mc.end())
			{
				MAPSONGLIST songlist;
				songlist.insert(std::make_pair(sli.songname,sli));
				mc.insert(std::make_pair(c,songlist));
			}
			else
			{
				it->second.insert(std::make_pair(sli.songname,sli));
			}
		}
	}
	return true;

failed: 
	if(fp)
		fclose(fp);
	return false;
}

COpenGLWindow::COpenGLWindow(void)
:m_bWindowed(true)
{
	CGSize win_size(1280,720);
	setWinSize(win_size);

	int nsize = sizeof(m_currentDir);
	ZeroMemory(m_currentDir,nsize);
	GetCurrentDirectory(nsize,m_currentDir);
	sprintf_s(m_szIniFile,259,"%s\\config.ini",m_currentDir);//定义配置文件
	sprintf_s(m_szIniFilter,259,"%s\\filter.ini",m_currentDir);

	readCSV("年会KTV评分歌曲.csv",m_mapCategory);

	m_pDM = new CDrawMonitor;
}

COpenGLWindow::~COpenGLWindow(void)
{
	if(m_pDM)
	{
		m_pDM->detachFileAll();
		m_pDM->Release();
	}
}

COpenGLWindow* COpenGLWindow::getWindowSingleton()
{
	if(!cls_window)
	{
		cls_window = new COpenGLWindow;
	}
	return cls_window;
}

void COpenGLWindow::releaseWindowSingleton()
{
	if (cls_window)
	{
		delete cls_window;
	}
}

void COpenGLWindow::updateVideoPos(CVideo* pVideo,int index,int total)
{
	switch(total)
	{
	case 1:
		{
			pVideo->updatePos(CGPoint(-1.0f,  1.0f, 0.0f),CGPoint(-1.0f, -1.0f, 0.0f),CGPoint(1.0f,  1.0f, 0.0f),CGPoint(1.0f, -1.0f, 0.0f));
			break;
		}
	case 2:
		{
			switch(index)
			{
			case 1:
				{
					pVideo->updatePos(CGPoint(-1.0f,  1.0f, 0.0f),CGPoint(-1.0f, -1.0f, 0.0f),CGPoint(0.0f,  1.0f, 0.0f),CGPoint(0.0f, -1.0f, 0.0f));
					break;
				}
			case 2:
				{
					pVideo->updatePos(CGPoint(0.0f,  1.0f, 0.0f),CGPoint(0.0f, -1.0f, 0.0f),CGPoint(1.0f,  1.0f, 0.0f),CGPoint(1.0f, -1.0f, 0.0f));
					break;
				}
			}
			break;
		}
	case 3:
		{
			switch(index)
			{
			case 1:
				{
					pVideo->updatePos(CGPoint(-1.0f,  1.0f, 0.0f),CGPoint(-1.0f, -1.0f, 0.0f),CGPoint(-0.34f,  1.0f, 0.0f),CGPoint(-0.34f, -1.0f, 0.0f));
					break;
				}
			case 2:
				{
					pVideo->updatePos(CGPoint(-0.34f,  1.0f, 0.0f),CGPoint(-0.34f, -1.0f, 0.0f),CGPoint(0.33f,  1.0f, 0.0f),CGPoint(0.33f, -1.0f, 0.0f));
					break;
				}
			case 3:
				{
					pVideo->updatePos(CGPoint(0.33f,  1.0f, 0.0f),CGPoint(0.33f, -1.0f, 0.0f),CGPoint(1.0f,  1.0f, 0.0f),CGPoint(1.0f, -1.0f, 0.0f));
					break;
				}
			}
			break;
		}
	case 4:
		{
			switch(index)
			{
			case 1:
				{
					pVideo->updatePos(CGPoint(-1.0f,1.0f,0.0f),CGPoint(-1.0f,0.0f,0.0f),CGPoint(0.0f,1.0f,0.0f),CGPoint(0.0f,0.0f,0.0f));
					break;
				}
			case 2:
				{
					pVideo->updatePos(CGPoint(0.0f,1.0f),CGPoint(0.0f,0.0f),CGPoint(1.0f,1.0f),CGPoint(1.0f,0.0f));
					break;
				}
			case 3:
				{
					pVideo->updatePos(CGPoint(-1.0f,0.0f),CGPoint(-1.0f,-1.0f),CGPoint(0.0f,0.0f),CGPoint(0.0f,-1.0f));
					break;
				}
			case 4:
				{
					pVideo->updatePos(CGPoint(0.0f,0.0f),CGPoint(0.0f,-1.0f),CGPoint(1.0f,0.0f),CGPoint(1.0f,-1.0f));
					break;
				}
			default:
				{
					OutputDebugStringA("unexpected error\n");
					break;
				}
			}
			break;
		}
	}
}

void COpenGLWindow::updateVideoPos()
{
	MAPSTRINGID::iterator it = m_pDM->m_mapStrId.begin();
	int j = 1;
	for (it;it!=m_pDM->m_mapStrId.end();it++,j++)
	{
		CVideo* pVideo = (CVideo*) it->second;
		if (pVideo && (pVideo->m_nTag==TAG_VIDEO_MY))
		{
			updateVideoPos(pVideo,j,(int)m_pDM->m_mapStrId.size());
		}
	}
}

bool COpenGLWindow::changeFile(const char* file,bool notmv,std::string& newname,const char* oldname)
{
	if (m_pDM->m_mapStrId.empty() || !m_pDM)
	{
		return false;
	}

	MAPSTRINGID::iterator it;
	if (!oldname)
		it = m_pDM->m_mapStrId.begin();
	else
		it = m_pDM->m_mapStrId.find(std::string(oldname));
	std::string strName;
	if (newname == "")
	{
		strName = it->first;
		newname = strName;
	}
	else
		strName = newname;
	CGPoint lt,lb,rt,rb;
	CVideo* pVieo = (CVideo*)it->second;
	if (!pVieo || (pVieo->m_nTag != TAG_VIDEO_MY))
	{
		return false;
	}
	pVieo->getCurPos(lt,lb,rt,rb);
	m_pDM->detachFile(it->first.c_str());
	DWORD_PTR id;
	if(m_pDM->attacthFile(file,notmv,strName.c_str(),&id))
	{
		pVieo = (CVideo*) id;
		pVieo->updatePos(lt,lb,rt,rb);
		return true;
	}
	return false;
}

bool COpenGLWindow::addFile(const char* file,const char* name,bool notmv)
{
	if (m_pDM)
	{
		DWORD_PTR id;
		if(m_pDM->attacthFile(file,notmv,name,&id))
		{
			updateVideoPos();
			return true;
		}
	}
	return false;
}

void COpenGLWindow::run(const char* name,bool bFirst)
{
	MAPSTRINGID::iterator it = m_pDM->m_mapStrId.find(std::string(name));
	if (it!=m_pDM->m_mapStrId.end())
	{
		CVideo* pVideo = (CVideo*)it->second;
		if (pVideo && (pVideo->m_nTag == TAG_VIDEO_MY))
		{
			if(pVideo->m_pGraph)
				pVideo->m_pGraph->ktvStartPlayer(bFirst);
		}
	}
}

void COpenGLWindow::runAll()
{
	MAPSTRINGID::iterator it = m_pDM->m_mapStrId.begin();
	for(it;it!=m_pDM->m_mapStrId.end();it++)
	{
		CVideo* pVideo = (CVideo*)it->second;
		if (pVideo && (pVideo->m_nTag == TAG_VIDEO_MY))
		{
			if(pVideo->m_pGraph)
				pVideo->m_pGraph->ktvStartPlayer(true);
		}
	}
}

void COpenGLWindow::setWinStyleRect()
{
	// window argument
	int width=m_winSize.width + GetSystemMetrics(SM_CXFIXEDFRAME)*2;//左右边框
	int height=m_winSize.height + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + GetSystemMetrics(SM_CYCAPTION);//上下边框+标题框

	m_rectW.left=(GetSystemMetrics(SM_CXSCREEN)-width)/2;
	m_rectW.top=(GetSystemMetrics(SM_CYSCREEN)-height)/2;
	m_rectW.right=m_rectW.left+width;
	m_rectW.bottom=m_rectW.top+height;
	m_styleW=WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE; //WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX;

	m_rectFS.left=0;
	m_rectFS.top=0;
	m_rectFS.right=m_winSize.width;
	m_rectFS.bottom=m_winSize.height;
	m_styleFS=WS_POPUP|WS_VISIBLE; //WS_POPUP|WS_VISIBLE
	//m_styleFS = WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_POPUP;
	//m_styleFS = WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
}
