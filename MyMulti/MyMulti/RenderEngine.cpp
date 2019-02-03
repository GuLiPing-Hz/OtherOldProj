#include "StdAfx.h"
#include <d3dx9math.h>
#include "RenderEngine.h"
#include "OpenGLWindow.h"
CRenderEngine::CRenderEngine(void)
:m_pD3ddev(NULL)
,m_nRef(1)
,m_pD3d(NULL)
,m_nScreenBPP(32)
,m_bFPSVSYNC(true)
,m_d3dpp(NULL)
,m_bZBuffer(true)
,m_pRenderTarget(NULL)
,m_D3DSwapChain(NULL)
,m_pSwapChainBackBuffer(NULL)
{
	m_pD3d = Direct3DCreate9(D3D_SDK_VERSION);
}

CRenderEngine::~CRenderEngine(void)
{
	shutdown();
}

void CRenderEngine::shutdown()
{
	int n=0;
	SAFE_RELEASE(m_pSwapChainBackBuffer);
	SAFE_RELEASE(m_D3DSwapChain);
	SAFE_RELEASE(m_pRenderTarget);

	if(m_pD3ddev != NULL) 
	{
		n = (int)m_pD3ddev->Release();
	}
	if (n)
	{
		assert(n != 0);
		releaseD3D();
	}
	if(m_pD3d != NULL) 
	{
		m_pD3d->Release();
	}

	m_pD3ddev = NULL;
	m_pD3d = NULL;
}

int CRenderEngine::format_id(D3DFORMAT fmt)
{
	switch(fmt) {
		case D3DFMT_R5G6B5:		return 1;
		case D3DFMT_X1R5G5B5:	return 2;
		case D3DFMT_A1R5G5B5:	return 3;
		case D3DFMT_X8R8G8B8:	return 4;
		case D3DFMT_A8R8G8B8:	return 5;
		default:				return 0;
	}
}

HRESULT CRenderEngine::createDevice(const std::string szIniFile,bool bWindowed)
{
	// Get adapter info
	//D3DENUM_WHQL_LEVEL

	CheckPointer(m_pD3d,E_POINTER);

	D3DADAPTER_IDENTIFIER9 AdID;
	UINT nModes, i;
	D3DFORMAT Format=D3DFMT_UNKNOWN;
	static bool s_b = true;
	if (s_b)
	{
		m_pD3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0 , &AdID);//D3DENUM_NO_WHQL_LEVEL
		//w riteLog("[DXLOG CAllocator::CreateDevice]:D3D Driver: %s",AdID.Driver);
		//LOG_IFS2("D3D Driver",AdID.Driver);
		//LOG_IFS2("Description",AdID.Description);
		//LOG_IF0("Version:")<<HIWORD(AdID.DriverVersion.HighPart) << "." <<LOWORD(AdID.DriverVersion.HighPart) << "."  \
			<<HIWORD(AdID.DriverVersion.LowPart) << "." <<LOWORD(AdID.DriverVersion.LowPart)<<"\n";
		s_b = false;
	}
	//////////////////////////////////////////////////////////////////////////
	D3DDISPLAYMODE dm;
	// Set up Window presentation parameters
	HRESULT hr = m_pD3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &dm);
	if(FAILED(hr))
	{
		return hr;
	}

	//窗口参数
	ZeroMemory(&m_d3dppW, sizeof(m_d3dppW));

	m_d3dppW.BackBufferWidth  = m_winSize.width;
	m_d3dppW.BackBufferHeight = m_winSize.height;
	m_d3dppW.BackBufferFormat = dm.Format;
	m_d3dppW.BackBufferCount  = 1;
	m_d3dppW.MultiSampleType  = D3DMULTISAMPLE_NONE;
	m_d3dppW.hDeviceWindow    = m_hWnd;
	m_d3dppW.Windowed         = TRUE;
	//垂直同步
	if(m_bFPSVSYNC)
	{
		m_d3dppW.PresentationInterval=/*D3DPRESENT_INTERVAL_IMMEDIATE*/D3DPRESENT_INTERVAL_ONE;
	}
	else
	{
		m_d3dppW.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	//else					  
	m_d3dppW.SwapEffect = /*D3DSWAPEFFECT_DISCARD*//*D3DSWAPEFFECT_COPY*/D3DSWAPEFFECT_FLIP;

	if(m_bZBuffer)
	{
		m_d3dppW.EnableAutoDepthStencil = TRUE;
		m_d3dppW.AutoDepthStencilFormat = D3DFMT_D16;
	}
	//////////////////////////////////////////////////////////////////////////
	// Set up Full Screen presentation parameters
	int nAdapterCount = m_pD3d->GetAdapterCount();
	//w riteLog("[DXLOG CAllocator::CreateDevice] : nAdapterCount:%d",nAdapterCount);
	int nDeviceCreate = 0;
	if ( nAdapterCount>=2 )//如果有2个显示器，则在副显示器创建device
	{
		uint nDeviceId = Ini_GetInt(szIniFile.c_str(),"ADAPTERSELECT","ADAPTERID",1);
		nModes=m_pD3d->GetAdapterModeCount(nDeviceId,dm.Format);
		for(i=0; i<nModes; i++)
		{
			m_pD3d->EnumAdapterModes(nDeviceId,dm.Format, i, &dm);
			if(dm.Width != (UINT)m_winSize.width || dm.Height != (UINT)m_winSize.height) continue;
			if(m_nScreenBPP==16 && (format_id(dm.Format) > format_id(D3DFMT_A1R5G5B5))) continue;
			if(format_id(dm.Format) > format_id(Format)) Format=dm.Format;
		}
		nDeviceCreate = nDeviceId;
	}
	else
	{
		nModes=m_pD3d->GetAdapterModeCount(D3DADAPTER_DEFAULT,dm.Format);
		for(i=0; i<nModes; i++)
		{
			m_pD3d->EnumAdapterModes(D3DADAPTER_DEFAULT,dm.Format, i, &dm);
			if(dm.Width != (UINT)m_winSize.width || dm.Height != (UINT)m_winSize.height) continue;
			if(m_nScreenBPP==16 && (format_id(dm.Format) > format_id(D3DFMT_A1R5G5B5))) continue;
			if(format_id(dm.Format) > format_id(Format)) Format=dm.Format;
		}
	}

	m_pD3d->GetDeviceCaps(nDeviceCreate,D3DDEVTYPE_HAL,&m_d3dCaps);

	if(Format == D3DFMT_UNKNOWN)
	{
		//LOG_ERN0("Can't find appropriate full screen video mode");
		if(!bWindowed) return false;
	}
	//全屏参数
	ZeroMemory(&m_d3dppFS, sizeof(m_d3dppFS));

	m_d3dppFS.BackBufferWidth  = m_winSize.width;
	m_d3dppFS.BackBufferHeight = m_winSize.height;
	m_d3dppFS.BackBufferFormat = Format;
	m_d3dppFS.BackBufferCount  = 1;
	m_d3dppFS.MultiSampleType  = D3DMULTISAMPLE_NONE;
	m_d3dppFS.hDeviceWindow    = m_hWnd;
	m_d3dppFS.Windowed         = FALSE;

	m_d3dppFS.SwapEffect       = D3DSWAPEFFECT_FLIP;
	m_d3dppFS.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	//垂直同步
	if(m_bFPSVSYNC)
	{
		m_d3dppFS.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
	else					  
	{
		m_d3dppFS.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	if(m_bZBuffer)
	{
		m_d3dppFS.EnableAutoDepthStencil = TRUE;
		m_d3dppFS.AutoDepthStencilFormat = D3DFMT_D16;
	}

	m_d3dpp = bWindowed ? &m_d3dppW : &m_d3dppFS;

	if(format_id(m_d3dpp->BackBufferFormat) < 4) m_nScreenBPP=16;
	else m_nScreenBPP=32;

	int vp = 0;
	if( m_d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) 
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED;
	//////////////////////////////////////////////////////////////////////////
	m_nDeviceCreate = nDeviceCreate;
	FAIL_RET( m_pD3d->CreateDevice(  nDeviceCreate,/*D3DADAPTER_DEFAULT*/
		D3DDEVTYPE_HAL,
		m_hWnd,vp,m_d3dpp,&m_pD3ddev) );
	hr = m_pD3ddev->ShowCursor(FALSE);

// 	if (m_hHY)
// 	{
// 		memcpy(&m_d3dppHY,m_d3dpp,sizeof(m_d3dppHY));
// 		m_d3dppHY.hDeviceWindow = m_hHY;
// 		assert(m_D3DSwapChain == NULL);
// 		hr = m_D3DDev->CreateAdditionalSwapChain(&m_d3dppHY,&m_D3DSwapChain);
// 		if (FAILED(hr))
// 		{
// 			w riteLog("[DXLOG CAllocator::CreateDevice] : CreateAdditionalSwapChain error");
// 		}
// 	}

	assert(m_pRenderTarget == NULL);
	hr = m_pD3ddev->GetRenderTarget( 0, & m_pRenderTarget );
	if(FAILED(hr))
	{
		return hr;
	}
// 	D3DRS_CULLMODE//背面剔除模式
// 	D3DCULL_NONE //完全禁用背面消隐
// 	D3DCULL_CW //只对顺时针绕序的三角形进行消隐
// 	D3DCULL_CCW //只对逆时针绕序的三角形进行消隐（在消隐中为默认值）
	FAIL_RET(m_pD3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));//
	FAIL_RET(m_pD3ddev->SetRenderState(D3DRS_LIGHTING, FALSE));//不启用光照
//启用alpha混合模式
	FAIL_RET(m_pD3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	FAIL_RET(m_pD3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	FAIL_RET(m_pD3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

	m_pD3ddev->SetRenderState(D3DRS_ZENABLE,TRUE);
	m_pD3ddev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);   //将深度测试函数设为D3DCMP_LESS
	m_pD3ddev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	//FAIL_RET(hr = m_D3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));alpha测试，类似于深度缓冲区的测试，
	//FAIL_RET(hr = m_D3DDev->SetRenderState(D3DRS_ALPHAREF, 0x10));//对小于这个值的纹理不会描绘，当透明处理
	//FAIL_RET(hr = m_D3DDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER));

	FAIL_RET(m_pD3ddev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));//设置u坐标， 纹理包装寻址方式
	FAIL_RET(m_pD3ddev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));//设置v坐标，纹理包装寻址方式
	FAIL_RET(m_pD3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));//放大采用的方法
	FAIL_RET(m_pD3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));//缩小采用的方法,
	FAIL_RET(m_pD3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));//缩小的时候采用的方法

	FAIL_RET(m_pD3ddev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE)); //设定纹理的alpha运算

	D3DXMATRIX matOrtho;
	D3DXMATRIX matIdentity;

	//Setup an orthographic perspective
	//D3DXMatrixOrthoLH (&matOrtho, (float) m_winSize.width, (float) m_winSize.height, 1.0f, 100.0f);
	//FAIL_RET(m_pD3ddev->SetTransform (D3DTS_TEXTURE0, &matOrtho));//2D

	D3DXMATRIX matPerspect;
	D3DXMatrixPerspectiveFovLH(&matPerspect,D3DXToRadian(45),m_winSize.width/m_winSize.height,1.0f,1000.0f);
	m_pD3ddev->SetTransform(D3DTS_PROJECTION,&matPerspect);
	D3DXMatrixIdentity (&matIdentity);
	FAIL_RET(m_pD3ddev->SetTransform (D3DTS_VIEW, &matIdentity));
	FAIL_RET(m_pD3ddev->SetTransform (D3DTS_WORLD, &matIdentity));

	return hr;
}

HRESULT CRenderEngine::getVideo(DWORD_PTR dwId,CVideo** ppVideo)
{
	if(!dwId)
	{
		return VFW_E_NOT_FOUND;
	}

	CVideo* pVideo = reinterpret_cast<CVideo*>(dwId);
	if(!pVideo || pVideo->m_nTag!=TAG_VIDEO_MY)
	{
		return VFW_E_NOT_FOUND;
	}

	*ppVideo = pVideo;
	return S_OK;
}

// ID3DRender
bool CRenderEngine::initialize(/*in*/const COpenGLWindow* pOwner)
{
	assert (pOwner != NULL);
	m_winSize = pOwner->getWinSize();
	m_szIniFile = pOwner->getszIniFile();
	m_hWnd = pOwner->getWnd();
	m_bWindowed = pOwner->m_bWindowed;

	return SUCCEEDED(createDevice(m_szIniFile,m_bWindowed));
}
bool CRenderEngine::attacthFile(/*in*/const char* file,/*in*/bool notMv,/*in*/const char* name,/*out*/DWORD_PTR* dwId)
{
	if (dwId)
		*dwId = 0;
	CGraph* pGraph = new CGraph(m_hWnd);
	if(!pGraph)
		return false;
	HRESULT hr = pGraph->gStartGraph(file,notMv);
	if (FAILED(hr))
	{
		SAFE_DELETE(pGraph);
		return false;
	}
	//strcspn()
	std::string objName;
	if(!name)
	{
		const char* p1 = strrchr(file,'/');
		const char* p2 = strrchr(file,'\\');
		if (p1&&p2)
			objName = p1>p2?p1+1:p2+1;
		else if(p1)
			objName = p1+1;
		else if(p2)
			objName = p2+1;
		else
			objName = file;
	}
	else
		objName = name;
	CVideo* pVideo = new CVideo(objName.c_str());
	if (!pVideo)
	{
		SAFE_DELETE(pGraph);
		return false;
	}
	pVideo->m_dwID = (DWORD_PTR) pVideo;
	if(!pVideo->setGraph(pGraph))
	{
		return false;
	}

	{
		CAutoLock lock(&m_cs);

		HMONITOR hMonitor = m_pD3d->GetAdapterMonitor(m_nDeviceCreate);
		hr = pVideo->m_pDefaultNotify->SetD3DDevice(m_pD3ddev,hMonitor);
		if(FAILED(hr))
		{
			OutputDebugStringA("IVMRSurfaceAllocatorNotify9  SetD3DDevice failed\n");
			return false;
		}

		hr = pVideo->m_pDefaultNotify->AdviseSurfaceAllocator(pVideo->m_dwID,(IVMRSurfaceAllocator9*)this);
		if(FAILED(hr))
		{
			OutputDebugStringA("IVMRSurfaceAllocatorNotify9 AdviseSurfaceAllocator failed\n");
			return false;
		}

		hr = StartPresenting(pVideo->m_dwID);
		if (FAILED(hr))
		{
			OutputDebugStringA("StartPresenting failed\n");
			return false;
		}

		if (dwId)
			*dwId = pVideo->m_dwID;
		m_mapStrId.insert(std::make_pair(std::string(objName),pVideo->m_dwID));
	}

	hr = pGraph->gConnectGraph();
	if(FAILED(hr))
	{
		OutputDebugStringA("connect graph failed\n");
		return false;
	}

	return true;
}
bool CRenderEngine::detachFileAll()
{
	MAPSTRINGID::iterator it = m_mapStrId.begin();
	for (it;it!=m_mapStrId.end();it++)
	{
		CVideo* pVideo;
		if (SUCCEEDED(getVideo(it->second,&pVideo)))
		{
			if (pVideo->m_pGraph)
				pVideo->m_pGraph->stopPlayer();
			{
				CAutoLock lock(&m_cs);
				StopPresenting(it->second);
				delete pVideo;
			}
		}
	}

	{
		CAutoLock lock(&m_cs);
		m_mapStrId.clear();
	}

	//CGraph::releaseVMR9();

	return true;
}
bool CRenderEngine::detachFile(/*in*/const char* name)
{
	MAPSTRINGID::iterator it = m_mapStrId.find(name);
	if (it != m_mapStrId.end())
	{
		CVideo* pVideo;
		if(SUCCEEDED(getVideo(it->second,&pVideo)))
		{
			if (pVideo->m_pGraph)
				pVideo->m_pGraph->stopPlayer();
			{
				CAutoLock lock(&m_cs);
				StopPresenting(it->second);
				delete pVideo;
			}
		}
		{
			CAutoLock lock(&m_cs);
			m_mapStrId.erase(it);
		}
	}
	return true;
}

bool CRenderEngine::getDevice(/*out*/IDirect3DDevice9** ppDev)
{
	if(!ppDev)
		return false;
	*ppDev = m_pD3ddev;
	return true;
}

bool CRenderEngine::beginLostDev()
{
	detachFileAll();
	SAFE_RELEASE(m_pSwapChainBackBuffer);
	SAFE_RELEASE(m_D3DSwapChain);
	SAFE_RELEASE(m_pRenderTarget);
	int n = 0;
	if(m_pD3ddev != NULL) 
	{
		n = (int)m_pD3ddev->Release();
	}
	assert( n == 0);
	return true;
}
bool CRenderEngine::endLostDev()
{
	return SUCCEEDED(createDevice(m_szIniFile,m_bWindowed));
}

HRESULT CRenderEngine::InitializeDevice( 
	/* [in] */ DWORD_PTR dwUserID,
	/* [in] */ VMR9AllocationInfo *lpAllocInfo,
	/* [out][in] */ DWORD *lpNumBuffers)
{
	D3DCAPS9 d3dcaps;
	DWORD dwWidth = 1;
	DWORD dwHeight = 1;

	CVideo* pVideo = NULL;
	HRESULT hr = getVideo(dwUserID,&pVideo);
	if(FAILED(hr))
	{
		OutputDebugStringA("get video failed\n");
		return E_FAIL;
	}

	// check we are provided valid parameters
	CheckPointer(lpAllocInfo,E_POINTER);
	CheckPointer(lpNumBuffers,E_POINTER);

	if( *lpNumBuffers <1 )
	{
		*lpNumBuffers = 1;
	}
	// check we know about the default IVMRSurfaceAllocatorNotify9
	if(!(pVideo->m_pDefaultNotify) )
	{
		OutputDebugStringA("CMultiVMR9Wizard::InitializeDevice: FATAL: video source contains NULL pointer to IVMRSurfaceAllocatorNotify9\n");
		return E_FAIL;
	}

	{
		CAutoLock lock(&m_cs);
		SAFE_RELEASE(pVideo->m_pTex);
		hr = pVideo->allocateSurfaceBuffer(*lpNumBuffers);
		if(FAILED(hr))
		{
			return hr;
		}
		lpAllocInfo->dwFlags = VMR9AllocFlag_OffscreenSurface;//VMR9AllocFlag_3DRenderTarget | VMR9AllocFlag_TextureSurface;
		hr = pVideo->m_pDefaultNotify->AllocateSurfaceHelper(lpAllocInfo,lpNumBuffers,pVideo->m_ppSur);
		if(FAILED(hr))
		{
			return hr;
		}
		pVideo->m_dwNumBufActuallyAllocated = *lpNumBuffers;

		// here we are creating private texture to be used by the render engine
		D3DFORMAT format;
		if (lpAllocInfo->Format > '0000') // surface is YUV.
		{
			format = D3DFMT_X8R8G8B8;// TODO: get current display format
		}
		else // RGB: use the same as original
		{
			format =  lpAllocInfo->Format;
		}

		// first, check if we have to comply with pow2 requirement
		ZeroMemory( &d3dcaps, sizeof(D3DCAPS9));

		hr = m_pD3ddev->GetDeviceCaps(&d3dcaps);

		if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_POW2 ) // have to make it pow2 :-(
		{
			while( dwWidth < lpAllocInfo->dwWidth )
				dwWidth = dwWidth << 1;

			while( dwHeight < lpAllocInfo->dwHeight )
				dwHeight = dwHeight << 1;
		}
		else
		{
			dwWidth = lpAllocInfo->dwWidth;
			dwHeight = lpAllocInfo->dwHeight;
		}

		SAFE_RELEASE( pVideo->m_pTex );
		hr = m_pD3ddev->CreateTexture(dwWidth,dwHeight,1,D3DUSAGE_RENDERTARGET,format
			,D3DPOOL_DEFAULT// we are not going to get into surface bits, so we do not need managed
			,&(pVideo->m_pTex),NULL);
		if(FAILED(hr))
		{
			OutputDebugStringA("Create texutre failed\n");
			return hr;
		}

		CComPtr<IDirect3DSurface9> pS;
		hr = pVideo->m_pTex->GetSurfaceLevel(0, &pS.p);
		if(FAILED(hr))
		{
			OutputDebugStringA("GetSurfaceLevel failed\n");
			return hr;
		}
		hr = m_pD3ddev->ColorFill( pS, NULL, D3DCOLOR_XRGB(0x00,0x00,0x00));

		D3DSURFACE_DESC d3dsd;
		hr = pVideo->m_pTex->GetLevelDesc(0, &d3dsd);
		if(FAILED(hr))
		{
			OutputDebugStringA("GetLevelDesc failed\n");
			return hr;
		}

		pVideo->setSrcRect((float)lpAllocInfo->dwWidth/(float)d3dsd.Width,(float)lpAllocInfo->dwHeight/(float)d3dsd.Height);
	}

	return S_OK;
}

HRESULT CRenderEngine::TerminateDevice( /* [in] */ DWORD_PTR dwID)
{
	CVideo* pVideo = NULL;
	HRESULT hr = getVideo(dwID,&pVideo);
	if(FAILED(hr))
	{
		OutputDebugStringA("get video failed\n");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CRenderEngine::GetSurface( 
	/* [in] */ DWORD_PTR dwUserID,
	/* [in] */ DWORD SurfaceIndex,
	/* [in] */ DWORD SurfaceFlags,
	/* [out] */ IDirect3DSurface9 **lplpSurface)
{
	CheckPointer(lplpSurface,E_POINTER);

	CVideo* pVideo = NULL;
	HRESULT hr = getVideo(dwUserID,&pVideo);
	if(FAILED(hr))
	{
		OutputDebugStringA("get video failed\n");
		return E_FAIL;
	}

	if (SurfaceIndex >= pVideo->m_dwNumBufActuallyAllocated)
	{
		OutputDebugStringA("SurfaceIndex is invalid argument\n");
		return E_INVALIDARG;
	}

	CheckPointer(pVideo->m_ppSur[SurfaceIndex],E_UNEXPECTED);

	CAutoLock lock(&m_cs);
	*lplpSurface = pVideo->m_ppSur[SurfaceIndex];
	(*lplpSurface)->AddRef();

	return S_OK;
}

HRESULT CRenderEngine::AdviseNotify( /* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify)
{
	return S_OK;
}

// IVMRImagePresenter9 implementation
HRESULT CRenderEngine::StartPresenting( /* [in] */ DWORD_PTR dwUserID)
{
	CAutoLock lock(&m_cs);

	CVideo* pVideo = NULL;
	HRESULT hr = getVideo(dwUserID,&pVideo);
	if(FAILED(hr))
	{
		OutputDebugStringA("get video failed\n");
		return E_FAIL;
	}

	CheckPointer(m_pD3ddev,E_UNEXPECTED);

	return S_OK;
}

HRESULT CRenderEngine::StopPresenting( /* [in] */ DWORD_PTR dwUserID)
{
	return S_OK;
}

HRESULT CRenderEngine::PresentImage( 
	/* [in] */ DWORD_PTR dwUserID,
	/* [in] */ VMR9PresentationInfo *lpPresInfo)
{

	CheckPointer(lpPresInfo,E_POINTER);
	CheckPointer(lpPresInfo->lpSurf,E_POINTER);

	CVideo* pVideo = NULL;
	HRESULT hr = getVideo(dwUserID,&pVideo);
	if(FAILED(hr))
	{
		OutputDebugStringA("get video failed\n");
		return E_FAIL;
	}

	CheckPointer(pVideo->m_pTex,E_UNEXPECTED);

	CAutoLock lock(&m_cs);

	CComPtr<IDirect3DDevice9> pSampleDev;
	hr = lpPresInfo->lpSurf->GetDevice(&pSampleDev.p);
	if(FAILED(hr))
	{
		OutputDebugStringA("get device from surface failed\n");
		return hr;
	}

	CComPtr<IDirect3DSurface9> pSur;
	hr = pVideo->m_pTex->GetSurfaceLevel(0,&pSur.p);
	if(FAILED(hr))
	{
		OutputDebugStringA("get surface from video texture failed\n");
		return hr;
	}

	hr = pSampleDev->StretchRect(lpPresInfo->lpSurf,NULL,pSur,NULL
		,D3DTEXF_NONE);//大小变化时候指定的过滤方式
	if(FAILED(hr))
	{
		OutputDebugStringA("stretch surface from the dest failed\n");
		return hr;
	}
	return S_OK;
}

// IUnknown
HRESULT CRenderEngine::QueryInterface( REFIID riid,void** ppvObject)
{
	CheckPointer(ppvObject,E_POINTER);

	if( riid == __uuidof(IVMRSurfaceAllocator9) ) {
		return GetInterface((IVMRSurfaceAllocator9*)this,ppvObject);
// 		*ppvObject = static_cast<IVMRSurfaceAllocator9*>( this );
// 		AddRef();
// 		hr = S_OK;
	}
	else if( riid == __uuidof(IVMRImagePresenter9) ) {
		return GetInterface((IVMRImagePresenter9*)this,ppvObject);
// 		*ppvObject = static_cast<IVMRImagePresenter9*>( this );
// 		AddRef();
// 		hr = S_OK;
	}
	else if( riid == IID_IUnknown ) {
		return GetInterface((IUnknown*)(IVMRSurfaceAllocator9*)this,ppvObject);
// 		*ppvObject = 
// 			static_cast<IUnknown*>( 
// 			static_cast<IVMRSurfaceAllocator9*>( this ) );
// 		AddRef();
// 		hr = S_OK;    
	}

	return E_NOINTERFACE;
}

ULONG CRenderEngine::AddRef()
{
	assert(m_nRef != 0);
	InterlockedIncrement(&m_nRef);
	return m_nRef;
}

ULONG CRenderEngine::Release()
{
	InterlockedDecrement(&m_nRef);
	if(m_nRef == 0)
	{
		delete this;
	}
	return m_nRef;
}

