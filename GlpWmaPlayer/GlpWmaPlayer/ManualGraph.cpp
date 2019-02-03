#include "StdAfx.h"
#include <assert.h>
#include "ManualGraph.h"

HRESULT CManualGraph::CreateObjectFromPath(REFCLSID clsid, IBaseFilter **ppUnk, const TCHAR* lpszPath)
{
	//	load the target DLL directly
	HMODULE hModule = LoadLibrary(lpszPath);
	if (hModule == NULL) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	return CreateObjectFromPath(hModule, clsid, ppUnk);
}

HRESULT CManualGraph::CreateObjectFromPath(HMODULE hModule, REFCLSID clsid, IBaseFilter **ppUnk)
{
	assert(hModule);

	//	the entry point is an exported function
	DllGetClassObjectT fn = (DllGetClassObjectT) GetProcAddress(hModule, "DllGetClassObject");
	if (fn == NULL) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	//	create a class factory
	IClassFactory* pClassFactory = NULL;
	HRESULT hr = fn(clsid, IID_IClassFactory, (LPVOID *) &pClassFactory);
	if (SUCCEEDED(hr)) {
		if (pClassFactory == NULL) 
		{
			hr = E_NOINTERFACE;
		}
		else 
		{
			//	ask the class factory to create the object
			hr = pClassFactory->CreateInstance(NULL, IID_IBaseFilter, (LPVOID *) ppUnk);
		}
	}
	return hr;
}

CManualGraph::CManualGraph(void)
:m_pGraph(NULL)
,m_pMC(NULL)
,m_pAS(NULL)
,m_pWmaSource(NULL)
,m_pAudioSwitch(NULL)
,m_pAudioRender(NULL)
{
}

CManualGraph::~CManualGraph(void)
{
	stop();
	uninitGraph();
}

HRESULT CManualGraph::initGraph(const TCHAR* fileName,HWND hWnd)
{
	CoInitialize(NULL);
	HRESULT hr;
	hr = CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void**)&m_pGraph);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pGraph->QueryInterface(IID_IMediaControl,(void**)&m_pMC);

	hr = CreateObjectFromPath(CLSID_WMASource,&m_pWmaSource
		,L"D:\\mywork\\Visual Studio 2005\\Projects\\GlpWmaPlayer\\GlpWmaPlayer\\codec\\WMASource.ax");
	if (FAILED(hr))
	{
		return hr;
	}
	IFileSourceFilter* pFSF;
	hr = m_pWmaSource->QueryInterface(IID_IFileSourceFilter,(void**)&pFSF);
	hr = pFSF->Load(fileName,NULL);
	pFSF->Release();
	
	hr = CreateObjectFromPath(CLSID_AudioSwitch,&m_pAudioSwitch
		,L"D:\\mywork\\Visual Studio 2005\\Projects\\GlpWmaPlayer\\GlpWmaPlayer\\codec\\AudioSwitch.ax");
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pAudioSwitch->QueryInterface(__uuidof(IOS_AudioSwitch),(void**)&m_pAS);

	hr = CoCreateInstance(CLSID_DSoundRender,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**)&m_pAudioRender);

	hr = m_pGraph->AddFilter(m_pWmaSource,L"Wma Source");
	hr = m_pGraph->AddFilter(m_pAudioSwitch,L"Audio Switch");
	hr = m_pGraph->AddFilter(m_pAudioRender,L"Audio Render");

	hr = TryConnectFilter(m_pGraph,m_pWmaSource,m_pAudioSwitch);
	hr = TryConnectFilter(m_pGraph,m_pWmaSource,m_pAudioSwitch);
	hr = TryConnectFilter(m_pGraph,m_pAudioSwitch,m_pAudioRender);

	return hr;
}

void CManualGraph::uninitGraph()
{
	RemoveAllFilter(m_pGraph);

	SAFE_RELEASE(m_pMC);
	SAFE_RELEASE(m_pGraph);
	SAFE_RELEASE(m_pAudioRender);
	SAFE_RELEASE(m_pAS);
	SAFE_RELEASE(m_pAudioSwitch);
	SAFE_RELEASE(m_pWmaSource);

	CoUninitialize();
}

void CManualGraph::start()
{
	if (m_pMC)
	{
		m_pMC->Run();
	}
}

void CManualGraph::pause()
{
	if (m_pMC)
	{
		m_pMC->Pause();
	}
}
void CManualGraph::stop()
{
	if( m_pMC != NULL ) 
	{
		OAFilterState state;
		do 
		{
			m_pMC->Stop();
			m_pMC->GetState(0, & state );
		} while( state != State_Stopped ) ;
	}
}

void CManualGraph::switchAudio(BOOL bFirst)
{
	if (m_pAS)
	{
		m_pAS->SwitchATrack(bFirst);
	}
}
