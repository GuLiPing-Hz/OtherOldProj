#include "StdAfx.h"
#include "Graph.h"
#include "uguid.h"
#include "OpenGLWindow.h"
#include <comutil.h>

static const char g_strFileType[][50] = {"AVI","MP4","MPEG2"};
static char g_defaultGUID[] = "00000000-0000-0000-0000-000000000000";

#ifdef _GWINXP
int g_bCountToCreateListener = 0;
bool g_bNeedToReleaseListener = false;
#endif

//IBaseFilter*	CGraph::m_pVMR9=NULL;

void CGraph::releaseVMR9()
{
	//SAFE_RELEASE(m_pVMR9);
}

CGraph::CGraph(HWND hwnd)
:m_pBuilder(NULL)
,m_pGraph(NULL)
,m_pSourceFilter(NULL)
,m_pDemultiplexer(NULL)
,m_pVDecoder(NULL)
,m_pADecoder1(NULL)
,m_pADecoder2(NULL)
,m_pAudioSwitcher(NULL)
,m_pAudioRender(NULL)
,m_pFileSourceFilter(NULL)
,m_pBasicAudio(NULL)
,m_pMediaControl(NULL)
,m_pMediaSeeking(NULL)
,m_pMediaEventEx(NULL)
,m_pStreamSelect(NULL)
,m_pOSAudioSwitch(NULL)
,m_pOSChangePitch(NULL)
,m_pVMR9(NULL)
,m_nStream(true)
// ,m_lpIVMRSurfAllocNotify(NULL)
,m_hWnd(hwnd)
,m_binitGraph(false)
,m_bUseSameFilter(false)
,m_bNotMv(false)
{
	
}

CGraph::~CGraph()
{
	SAFE_RELEASE(m_pVMR9);
	errorUninit();
}

void CGraph::errorUninit()
{
	m_bUseSameFilter = false;//彻底释放
	gCloseGraph();
}

void CGraph::uinitGraph()
{
	RemoveAllFilter(m_pGraph);
	SAFE_RELEASE(m_pFileSourceFilter);
	SAFE_RELEASE(m_pSourceFilter);


	if (!m_bUseSameFilter)
	{
		SAFE_RELEASE(m_pBasicAudio);
		SAFE_RELEASE(m_pMediaSeeking);
		SAFE_RELEASE(m_pMediaControl);
		if (m_pMediaEventEx)
		{
			m_pMediaEventEx->SetNotifyWindow(NULL,0,0);
		}
		SAFE_RELEASE(m_pMediaEventEx);

		SAFE_RELEASE(m_pGraph);
		SAFE_RELEASE(m_pBuilder);

		SAFE_RELEASE(m_pStreamSelect);
		SAFE_RELEASE(m_pOSAudioSwitch);
		SAFE_RELEASE(m_pOSChangePitch);

		//SAFE_RELEASE(m_pVMR9);
		SAFE_RELEASE(m_pDemultiplexer);
		SAFE_RELEASE(m_pVDecoder);
		SAFE_RELEASE(m_pADecoder1);
		SAFE_RELEASE(m_pADecoder2);
		SAFE_RELEASE(m_pAudioSwitcher);
		SAFE_RELEASE(m_pAudioRender);
	}
}

HRESULT CGraph::initGraph(const char* filename)
{
	HRESULT hr;
	_bstr_t path = filename;

	if( ! path.length() )
	{
		OutputDebugStringA("path error\n");
		hr = E_INVALIDARG;
		goto failed;
	}

	COpenGLWindow* m_pOwner = COpenGLWindow::getWindowSingleton();
	eFileType eft;
	if (strstr(filename,".avi"))
	{
		eft = avi;
	}
	else if(strstr(filename,".mp4") || strstr(filename,".mov") || strstr(filename,".st4"))
	{
		eft = mp4;
	}
	else
	{
		eft = mpeg2;
	}

	if (!m_binitGraph)
	{
		//hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,NULL,CLSCTX_INPROC_SERVER,IID_ICaptureGraphBuilder2,(void**)&m_pBuilder);
		hr = CoCreateInstance(CLSID_FilterGraph,NULL,/*CLSCTX_ALL*/CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void**)&m_pGraph);
		//The Filter Graph Manager is provided by an in-process DLL, so the execution context is CLSCTX_INPROC_SERVER.
		if (FAILED(hr))
		{
			//LOG_ERS2("create CLSID_FilterGraph HRESULT",hr);
			goto failed;
		}
		//设置图表
		//hr = m_pBuilder->SetFiltergraph(m_pgraph);
		hr = m_pGraph->QueryInterface(IID_IBasicAudio,(void**)&m_pBasicAudio);
		if (FAILED(hr))
		{
			//LOG_ERS2("IID_IBasicAudio HRESULT",hr);
			goto failed;
		}
		hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
		if (FAILED(hr))
		{
			//LOG_ERS2("IID_IMediaControl HRESULT",hr);
			goto failed;
		}
		hr = m_pGraph->QueryInterface(IID_IMediaEventEx,(void**)&m_pMediaEventEx);
		if (FAILED(hr))
		{
			//LOG_ERS2("IID_IMediaEventEx HRESULT",hr);
			goto failed;
		}
		hr = m_pGraph->QueryInterface(IID_IMediaSeeking,(void**)&m_pMediaSeeking);
		if (FAILED(hr))
		{
			//LOG_ERS2("IID_IMediaSeeking HRESULT",hr);
			goto failed;
		}

		if (m_pMediaSeeking)
		{
			FAIL_GOTOFAILED(m_pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME));
		}
		if (m_pMediaEventEx)
		{
			FAIL_GOTOFAILED(m_pMediaEventEx->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0));
		}
// 		hr = CoCreateInstance(CLSID_FILESourceAsync, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (void**)&m_pSourceFilter);
// 		hr = m_pSourceFilter->QueryInterface(IID_IFileSourceFilter,(void**)&m_pIfileSourceFilter);
		GUID clsid_demultiplexer;
		hr = CLSIDFromString(Ini_GetStringW(m_pOwner->getFilterIniFile(),g_strFileType[eft],"DEMULTIPLEXER",g_defaultGUID),&clsid_demultiplexer);
		if (FAILED(hr))
		{
			//LOG_ERS2("read demultiplexer clsid failed,HRESULT",hr);
			goto failed;
		}
		hr = CoCreateInstance(clsid_demultiplexer, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (void**)&m_pDemultiplexer);
		if (FAILED(hr))
		{
			//LOG_ERS2("create CLSID_Demultiplexer HRESULT",hr);
			goto failed;
		}
		GUID clsid_vdecoder;
		hr = CLSIDFromString(Ini_GetStringW(m_pOwner->getFilterIniFile(),g_strFileType[eft],"VIDEODECODER",g_defaultGUID),&clsid_vdecoder);
		if (FAILED(hr))
		{
			//LOG_ERS2("read vdecoder clsid failed,HRESULT",hr);
			goto failed;
		}
		hr = CoCreateInstance(clsid_vdecoder, NULL, CLSCTX_INPROC_SERVER, IID_IGlp_Debug, (void**)&m_pVDecoder);
		if (FAILED(hr))
		{
			//LOG_ERS2("create CLSID_VideoDecoder HRESULT",hr);
			goto failed;
		}
		GUID clsid_adecoder1;
		hr = CLSIDFromString(Ini_GetStringW(m_pOwner->getFilterIniFile(),g_strFileType[eft],"AUDIODECODER1",g_defaultGUID),&clsid_adecoder1);
		if (FAILED(hr))
		{
			//LOG_ERS2("read adecoder1 clsid failed,HRESULT",hr);
			goto failed;
		}
		hr = CoCreateInstance(clsid_adecoder1, NULL, CLSCTX_INPROC_SERVER, IID_IGlp_Debug, (void**)&m_pADecoder1);
		if (FAILED(hr))
		{
			//LOG_ERS2("create CLSID_AudioDecoder1 HRESULT",hr);
			goto failed;
		}
		GUID clsid_adecoder2;
		hr = CLSIDFromString(Ini_GetStringW(m_pOwner->getFilterIniFile(),g_strFileType[eft],"AUDIODECODER2",g_defaultGUID),&clsid_adecoder2);
		if (FAILED(hr))
		{
			//LOG_ERS2("read adecoder2 clsid failed,HRESULT",hr);
			goto failed;
		}
		hr = CoCreateInstance(clsid_adecoder2, NULL, CLSCTX_INPROC_SERVER, IID_IGlp_Debug, (void**)&m_pADecoder2);
		if (FAILED(hr))
		{
			//LOG_ERS2("create CLSID_AudioDecoder2 HRESULT",hr);
			goto failed;
		}
		GUID clsid_audioswitch;
		hr = CLSIDFromString(Ini_GetStringW(m_pOwner->getFilterIniFile(),g_strFileType[eft],"AUDIOSWITCH",g_defaultGUID),&clsid_audioswitch);
		if (FAILED(hr))
		{
			//LOG_ERS2("read audioswitch clsid failed,HRESULT",hr);
			goto failed;
		}
		hr = CoCreateInstance(clsid_audioswitch, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (void**)&m_pAudioSwitcher);
		if (FAILED(hr))
		{
			//LOG_ERS2("create CLSID_AudioSwitcher HRESULT",hr);
			goto failed;
		}
		hr = m_pAudioSwitcher->QueryInterface(IID_IAMStreamSelect,(void**)&m_pStreamSelect);
		if (FAILED(hr))
		{
			//LOG_ERS2("IID_IAMStreamSelect HRESULT",hr);
			goto failed;
		}
		hr = m_pAudioSwitcher->QueryInterface(IID_IOS_AudioSwitch,(void**)&m_pOSAudioSwitch);
		if (FAILED(hr))
		{
			//LOG_ERS2("IID_IOS_AudioSwitch HRESULT",hr);
			goto failed;
		}
		hr = m_pAudioSwitcher->QueryInterface(IID_IOS_ChangePitch,(void**)&m_pOSChangePitch);
		if (FAILED(hr))
		{
			//LOG_ERS2("IID_IOS_ChangePitch HRESULT",hr);
			goto failed;
		}
		hr = CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (void**)&m_pAudioRender);
		if (FAILED(hr))
		{
			//LOG_ERS2("CLSID_DSoundRender HRESULT",hr);
			goto failed;
		}
		if (m_bUseSameFilter)//只创建一次filter
		{
			m_binitGraph = true;
		}
	}

	hr = m_pGraph->AddSourceFilter(path,L"File Source (Async.)",&m_pSourceFilter);
	// 	static int n = 0;//debug
	// 	n++;
	// 	if(n ==2)
	// 	{
	// 		hr = E_FAIL;
	// 		goto failed;
	// 	}
	if (FAILED(hr))
	{
		//LOG_ERS2("SourceFileter Add HRESULT",hr);
		goto failed;
	}

	FAIL_GOTOFAILED(m_pGraph->AddFilter(m_pDemultiplexer,L"Demultiplexer"));

	FAIL_GOTOFAILED(m_pGraph->AddFilter(m_pVDecoder,L"Video Decoder"));

	if(!m_bNotMv)
	{
		FAIL_GOTOFAILED(m_pGraph->AddFilter(m_pADecoder1,L"Audio Decoder1"));
		FAIL_GOTOFAILED(m_pGraph->AddFilter(m_pADecoder2,L"Audio Decoder2"));//先提供单音轨的demo
		FAIL_GOTOFAILED(m_pGraph->AddFilter(m_pAudioSwitcher,L"Audio Switch"));
		FAIL_GOTOFAILED(m_pGraph->AddFilter(m_pAudioRender,L"DSound Render"));
	}

	if (!m_pVMR9)
	{
		hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (void**)&m_pVMR9);
		if (FAILED(hr))
		{
			//LOG_ERS2("CLSID_VideoMixingRenderer9 HRESULT",hr);
			goto failed;
		}

		IVMRFilterConfig9 *filterConfig = NULL;
		FAIL_GOTOFAILED( m_pVMR9->QueryInterface(IID_IVMRFilterConfig9, (void**)&filterConfig) );

		hr = filterConfig->SetRenderingMode( VMR9Mode_Renderless  );//VMR9Mode_Windowless
		//FAIL_RET( filterConfig->SetNumberOfStreams(1) );
		filterConfig->Release();
		FAIL_GOTOFAILED(hr);
	}
	///m_pIfileSourceFilter->Load(path,NULL);

	//hr = m_pgraph->AddFilter(m_pSourceFilter,L"File Source (Async.)");
// 	if (FAILED(hr = setAllocatorPresenter( m_pVMR9,pallocator )))
// 	{
// 		//LOG_ERS2("setAllocatorPresenter HRESULT",hr);
// 		goto failed;
// 	}
	if( FAILED(hr = m_pGraph->AddFilter(m_pVMR9, L"Video Mixing Renderer 9")) )
	{
		//LOG_ERS2("add vmr9 failsed,HRESULT",hr);
		goto failed;
	}

// 	FAIL_RET( SetAllocatorPresenter( m_pVMR9, winsize,pallocator ) );//+1
// 	{
// 		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pSourceFilter,m_pDemultiplexer)))
// 		{
// 			//LOG_ERS2("connect filter SourceFilter Demultiplexer HRESULT",hr);
// 			goto failed;
// 		}
// 		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pDemultiplexer,m_pVDecoder)))
// 		{
// 			//LOG_ERS2("connect filter Demultiplexer VDecoder HRESULT",hr);
// 			goto failed;
// 		}
// 
// 		if(!m_bNotMv)
// 		{
// 			if( FAILED(hr = TryConnectFilter(m_pGraph,m_pDemultiplexer,m_pADecoder1)))
// 			{
// 				//LOG_ERS2("connect filter Demultiplexer ADecoder1 HRESULT",hr);
// 				goto failed;
// 			}
// 			if( FAILED(hr = TryConnectFilter(m_pGraph,m_pDemultiplexer,m_pADecoder2)))
// 			{
// 				//LOG_ERS2("connect filter Demultiplexer ADecoder2! The song may have only one audio line,HRESULT",hr);
// 				goto failed;
// 			}
// 		}
// 	}
// 
// 	if (FAILED(hr = TryConnectFilter(m_pGraph,m_pVDecoder,m_pVMR9)))
// 	{
// 		//LOG_ERS2("connect filter VDecoder VMR9 HRESULT",hr);
// 		goto failed;
// 	}
// 
// 	if(!m_bNotMv)
// 	{
// 		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pADecoder1,m_pAudioSwitcher)))
// 		{
// 			//LOG_ERS2("connect filter ADecoder1 AudioSwitch HRESULT",hr);
// 			goto failed;
// 		}
// 		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pADecoder2,m_pAudioSwitcher)))
// 		{
// 			//LOG_ERS2("connect filter ADecoder2 AudioSwitch! HRESULT",hr);
// 			//m_pAllocator = NULL;
// 			//return hr;
// 			hr = S_OK;
// 		}
// 		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pAudioSwitcher,m_pAudioRender)))
// 		{
// 			//LOG_ERS2("connect filter AudioSwitch AudioRender HRESULT",hr);
// 			goto failed;
// 		}
// 	}
	return hr;

failed:
// 	if(m_pVMR9)
// 		setAllocatorPresenter( m_pVMR9,NULL );
	return hr;
}


HRESULT CGraph::gStartGraph(const char * fileName,bool notMV)
{
	HRESULT hr = E_FAIL;
	m_bNotMv = notMV;
	hr = initGraph(fileName);
	//////////////////////////////////////////////////////////////////////////
	return hr;
}

HRESULT CGraph::gConnectGraph()
{
	HRESULT hr;
	//FAIL_RET( SetAllocatorPresenter( m_pVMR9, winsize,pallocator ) );//+1
	{
		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pSourceFilter,m_pDemultiplexer)))
		{
			//LOG_ERS2("connect filter SourceFilter Demultiplexer HRESULT",hr);
			goto failed;
		}
		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pDemultiplexer,m_pVDecoder)))
		{
			//LOG_ERS2("connect filter Demultiplexer VDecoder HRESULT",hr);
			goto failed;
		}

		if(!m_bNotMv)
		{
			if( FAILED(hr = TryConnectFilter(m_pGraph,m_pDemultiplexer,m_pADecoder1)))
			{
				//LOG_ERS2("connect filter Demultiplexer ADecoder1 HRESULT",hr);
				goto failed;
			}
			if( FAILED(hr = TryConnectFilter(m_pGraph,m_pDemultiplexer,m_pADecoder2)))
			{
				//LOG_ERS2("connect filter Demultiplexer ADecoder2! The song may have only one audio line,HRESULT",hr);
				goto failed;
			}
		}
	}

	if (FAILED(hr = TryConnectFilter(m_pGraph,m_pVDecoder,m_pVMR9)))
	{
		//LOG_ERS2("connect filter VDecoder VMR9 HRESULT",hr);
		goto failed;
	}

	if(!m_bNotMv)
	{
		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pADecoder1,m_pAudioSwitcher)))
		{
			//LOG_ERS2("connect filter ADecoder1 AudioSwitch HRESULT",hr);
			goto failed;
		}
		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pADecoder2,m_pAudioSwitcher)))
		{
			//LOG_ERS2("connect filter ADecoder2 AudioSwitch! HRESULT",hr);
			//m_pAllocator = NULL;
			//return hr;
			hr = S_OK;
		}
		if( FAILED(hr = TryConnectFilter(m_pGraph,m_pAudioSwitcher,m_pAudioRender)))
		{
			//LOG_ERS2("connect filter AudioSwitch AudioRender HRESULT",hr);
			goto failed;
		}
	}
	return hr;

failed:
	return hr;
}

HRESULT CGraph::gCloseGraph()
{
	if( m_pMediaControl != NULL ) 
	{
		OAFilterState state;
		do {
			m_pMediaControl->Stop();
			m_pMediaControl->GetState(0, & state );
		} while( state != State_Stopped ) ;
	}

	uinitGraph();

	return S_OK;
}

HRESULT CGraph::getDefaultNotify(IVMRSurfaceAllocatorNotify9** ppIVMRSurfAllocNotify)
{
	if (m_pVMR9)
	{
		return m_pVMR9->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, (void**)ppIVMRSurfAllocNotify);
	}
	return E_FAIL;
}

// HRESULT CGraph::setAllocatorPresenter( IBaseFilter* filter,CRenderEngine* pallocator )
// {
// 	if( filter == NULL )
// 	{
// 		return E_FAIL;
// 	}
// 
// 	HRESULT hr;
// 
// 	//CComPtr<IVMRSurfaceAllocatorNotify9> lpIVMRSurfAllocNotify;
// 	IVMRSurfaceAllocatorNotify9*	 lpIVMRSurfAllocNotify = NULL;
// 	FAIL_RET( filter->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, reinterpret_cast<void**>(&lpIVMRSurfAllocNotify)) );//9
// 
// 	// create our surface allocator
// 	m_pAllocator = pallocator;
// 
// 	// let the allocator and the notify know about each other206
// 	if(m_pAllocator)
// 	{
// 		hr = m_pAllocator->AdviseNotify(lpIVMRSurfAllocNotify) ;
// 		if (FAILED(hr))
// 		{
// 			LOG_ERN0("AdviseNotify failed");
// 			goto failed;
// 		}
// 	}
// 	hr = lpIVMRSurfAllocNotify->AdviseSurfaceAllocator( (DWORD_PTR)this, m_pAllocator );
// 	if (FAILED(hr))
// 	{
// 		LOG_ERN0("AdviseSurfaceAllocator failed");
// 	}
// 
// failed:
// 	//lpIVMRSurfAllocNotify->Release();
// 	return hr;
// }

void CGraph::switchAudioEx(bool bFirstAudio)
{
	if (m_pOSAudioSwitch)
	{
		m_pOSAudioSwitch->SwitchATrack(bFirstAudio);
	}
}

void CGraph::changeCurPitch(const int nPitch)
{
	int tmp_nPitch = nPitch;
	tmp_nPitch = tmp_nPitch<-5?-5:(tmp_nPitch>5?5:tmp_nPitch);
	if (m_pOSChangePitch)
	{
		m_pOSChangePitch->ChangeCurPitch(tmp_nPitch);
	}
}

void CGraph::switchAudio()
{
	if (m_pStreamSelect)
	{
		DWORD nStreams=1;
		m_pStreamSelect->Count(&nStreams);
		if (nStreams>1)
		{
			if (m_nStream)
			{
				m_pStreamSelect->Enable(1,AMSTREAMSELECTENABLE_ENABLE);//
				m_nStream = false;
			}
			else
			{
				m_pStreamSelect->Enable(0,AMSTREAMSELECTENABLE_ENABLE);//0
				m_nStream = true;
			}

		}
	}
}

int CGraph::restartPlayer()
{
	if(0 != setCurPosition(0))
	{
		return -1;
	}
	return startPlayer();
}

int CGraph::ktvStartPlayer(bool bFirstAudio)
{
	if(m_pMediaControl)
	{
		switchAudioEx(bFirstAudio);//手动连接的时候，SwitchInit已经初始化完成，所以可以放在前面。
		return startPlayer();
	}
	return 0;
}

int CGraph::startPlayer()
{
	if (m_pMediaControl)
	{
		FAILDE_RETURNNEGATIVE(m_pMediaControl->Run());
	}
	return 0;
}

int CGraph::stopPlayer()
{
	if( m_pMediaControl ) 
	{
		OAFilterState state;
		do {
			m_pMediaControl->Stop();
			m_pMediaControl->GetState(0, & state );
		} while( state != State_Stopped ) ;
	}
	return 0;
}

int CGraph::pausePlayer()
{
	if (m_pMediaControl)
	{
		OAFilterState state;
		m_pMediaControl->GetState(0,&state);
		if(state == State_Running)
		{
			FAILDE_RETURNNEGATIVE(m_pMediaControl->Pause());
		}
	}
	return 0;
}

int CGraph::resumePlayer()
{
	if (m_pMediaControl)
	{
		OAFilterState state;
		m_pMediaControl->GetState(0,&state);
		if (state == State_Paused)
		{
			FAILDE_RETURNNEGATIVE(m_pMediaControl->Run());
		}
	}
	return 0;
}

//设置当前位置(毫秒)
int CGraph::setCurPosition(ulong nposition_ms,bool bAbsolute)
{
	if (m_pMediaSeeking)
	{
		LONGLONG nanosecond_unit = nposition_ms*10000;
		DWORD flagCur = bAbsolute?(AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame):(AM_SEEKING_RelativePositioning|AM_SEEKING_SeekToKeyFrame);
		FAILDE_RETURNNEGATIVE(m_pMediaSeeking->SetPositions(&nanosecond_unit, flagCur, 0, AM_SEEKING_NoPositioning));
	}
	return 0;
}

//设置起始结束位置(毫秒)
int CGraph::setStartStopPosition(ulong nstart_ms,ulong nstop_ms)
{
	if (m_pMediaSeeking)
	{
		LONGLONG nanosecond_start_unit = nstart_ms*10000;
		LONGLONG nanosecond_stop_unit = nstop_ms*10000;
		FAILDE_RETURNNEGATIVE(m_pMediaSeeking->SetPositions(&nanosecond_start_unit, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 
			&nanosecond_stop_unit, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame));
	}
	return 0;
}
//获取当前位置(毫秒)
int CGraph::getPosition(ulong &nposition_ms)
{
	if (m_pMediaSeeking)
	{
		LONGLONG nanosecond_unit;
		FAILDE_RETURNNEGATIVE(m_pMediaSeeking->GetCurrentPosition(&nanosecond_unit));
		nposition_ms = ulong(nanosecond_unit/10000);//毫秒
	}
	return 0;
}

int CGraph::getDuration(ulong &nduration_ms)
{
	if (m_pMediaSeeking)
	{
		LONGLONG nanosecond_unit;
		FAILDE_RETURNNEGATIVE(m_pMediaSeeking->GetDuration(&nanosecond_unit));
		nduration_ms = ulong(nanosecond_unit/10000);//毫秒
	}
	return 0;
}


int CGraph::getVolume(long &lVolume)
{
	if (m_pBasicAudio)
	{
		FAILDE_RETURNNEGATIVE(m_pBasicAudio->get_Volume(&lVolume));
	}
	return 0;
}

int CGraph::setVolume(const long lVolume)
{
	if (m_pBasicAudio)
	{
		FAILDE_RETURNNEGATIVE(m_pBasicAudio->put_Volume(lVolume));
	}
	return 0;
}

HRESULT CGraph::getGraphEvent(LONG& evCode,LONG& evParam1,LONG evParam2)
{
	HRESULT hr = E_FAIL;
	if (m_pMediaEventEx)
	{
		hr = m_pMediaEventEx->GetEvent(&evCode,(LONG_PTR*)&evParam1,(LONG_PTR*)&evParam2,0);
	}
	return hr;
}

HRESULT CGraph::freeGraphEvent(LONG evCode,LONG evParam1,LONG evParam2)
{
	HRESULT hr = E_FAIL;
	if (m_pMediaEventEx)
	{
		hr = m_pMediaEventEx->FreeEventParams(evCode,evParam1,evParam2);
	}
	return hr;
}

void CGraph::initAudioTrack(long nIndex)
{
	if (m_pStreamSelect)
	{
		DWORD nStreams=1;
		m_pStreamSelect->Count(&nStreams);
		if (nStreams>1)
		{
			m_pStreamSelect->Enable(nIndex,AMSTREAMSELECTENABLE_ENABLE);
			if (nIndex == 0)
			{
				m_nStream = true;
			}
			else
			{
				m_nStream = false;
			}
		}
	}
}
