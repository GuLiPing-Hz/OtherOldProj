#pragma once
#include "util.h"
#include "uguid.h"
#include "iaudioswitch.h"
//	define the prototype of the class factory entry point in a COM dll
typedef HRESULT (STDAPICALLTYPE *DllGetClassObjectT)(REFCLSID clsid, REFIID iid, LPVOID *ppv);

class CManualGraph
{
public:
	
	static HRESULT CreateObjectFromPath(REFCLSID clsid, IBaseFilter **ppUnk, const TCHAR* lpszPath);
	
	CManualGraph(void);
	virtual ~CManualGraph(void);

	HRESULT initGraph(const TCHAR* fileName,HWND hWnd=NULL);
	void uninitGraph();

	void start();
	void pause();
	void stop();
	void switchAudio(BOOL bFirst);
private:
	static HRESULT CreateObjectFromPath(HMODULE hModule, REFCLSID clsid, IBaseFilter **ppUnk);

private:
	IGraphBuilder*		m_pGraph;

	IMediaControl*		m_pMC;
	IOS_AudioSwitch*	m_pAS;

	IBaseFilter*				m_pWmaSource;
	IBaseFilter*				m_pAudioSwitch;
	IBaseFilter*				m_pAudioRender;
};


