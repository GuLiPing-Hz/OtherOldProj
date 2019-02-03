#pragma once

#include "Opengl.h"
#include "DrawMonitor.h"

#include <string>
#include <map>

typedef struct _SongListInfo{
	std::string songname;
	std::string singername;
	std::string filename;
}SongListInfo;
typedef std::map<std::string,SongListInfo> MAPSONGLIST;
typedef std::map<std::string,MAPSONGLIST> MAPCATEGORY;

class COpenGLWindow
{
public:
	COpenGLWindow(void);
	virtual ~COpenGLWindow(void);

	static COpenGLWindow* getWindowSingleton();
	static void						   releaseWindowSingleton();
	bool addFile(const char* file,const char* name,bool notmv);
	//需要之前判断是否为空，如果为空，则使用addFile。
	bool changeFile(const char* file,bool notmv,std::string& newname,const char* oldname=NULL);
	void run(const char* name,bool bFirst=false);
	void runAll();

	const char*			getszIniFile()const  {return m_szIniFile;}
	const char*			getFilterIniFile()const{return m_szIniFilter;}
	void						setWinSize(const CGSize& winsize){m_winSize = winsize;setWinStyleRect();}
	CGSize					getWinSize()const{return m_winSize;}
	void						setWinStyleRect();
	void						setWnd(HWND hWnd){m_hWnd=hWnd;}
	HWND					getWnd()const{return m_hWnd;}
private:
	void updateVideoPos(CVideo* pVideo,int index,int total);
	void updateVideoPos();
public:
	MAPCATEGORY					m_mapCategory;
	CDrawMonitor*					m_pDM;
public:
	static COpenGLWindow*	cls_window;
	char										m_currentDir[260];
	char										m_szIniFile[260];
	char										m_szIniFilter[260];
	CGSize									m_winSize;
	HWND									m_hWnd;
	HWND									m_hHY;
	bool										m_bWindowed;
	RECT									m_rectFS;//全屏参数
	RECT									m_rectHY;//幻影窗口
	LONG									m_styleFS;
	RECT									m_rectW;//窗口参数
	LONG									m_styleW;
};
