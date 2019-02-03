#pragma once

#include <d3d9.h>
#include <DShow.h>
#include <vmr9.h>
#include <Wxutil.h>
#include "GLListDef.h"

#pragma warning(push, 2)

// C4995'function': name was marked as #pragma deprecated
//
// The version of list which shipped with Visual Studio .NET 2003 
// indirectly uses some deprecated functions.  Warning C4995 is disabled 
// because the file cannot be changed and we do not want to 
// display warnings which the user cannot fix.
#pragma warning(disable : 4995)

#include <list>
#pragma warning(pop)
#include <string>
#include "Video.h"
#include "IRender.h"

class CRenderEngine : public ID3DRender,public  IVMRSurfaceAllocator9,public IVMRImagePresenter9
{
public:
	CRenderEngine(void);
	virtual ~CRenderEngine(void);

	ULONG releaseD3D()//for debug purpose
	{
		int nCount;
		do
		{
			nCount = (int)m_pD3ddev->Release();
		}while(nCount>0);

		return nCount;
	}

	// ID3DRender
	virtual bool initialize(/*in*/const COpenGLWindow* pOwner);
	virtual bool attacthFile(/*in*/const char* file,/*in*/bool notMv,/*in*/const char* name,/*out*/DWORD_PTR* dwId);
	virtual bool detachFile(/*in*/const char* name);
	virtual bool detachFileAll();
	virtual bool getDevice(/*out*/IDirect3DDevice9** ppDev);
	virtual bool beginLostDev();
	virtual bool endLostDev();

	// IVMRSurfaceAllocator9 implementation
	virtual HRESULT STDMETHODCALLTYPE InitializeDevice( 
		/* [in] */ DWORD_PTR dwUserID,
		/* [in] */ VMR9AllocationInfo *lpAllocInfo,
		/* [out][in] */ DWORD *lpNumBuffers);

	virtual HRESULT STDMETHODCALLTYPE TerminateDevice( 
		/* [in] */ DWORD_PTR dwID);

	virtual HRESULT STDMETHODCALLTYPE GetSurface( 
		/* [in] */ DWORD_PTR dwUserID,
		/* [in] */ DWORD SurfaceIndex,
		/* [in] */ DWORD SurfaceFlags,
		/* [out] */ IDirect3DSurface9 **lplpSurface);

	virtual HRESULT STDMETHODCALLTYPE AdviseNotify( 
		/* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);

	// IVMRImagePresenter9 implementation
	virtual HRESULT STDMETHODCALLTYPE StartPresenting( 
		/* [in] */ DWORD_PTR dwUserID);

	virtual HRESULT STDMETHODCALLTYPE StopPresenting( 
		/* [in] */ DWORD_PTR dwUserID);

	virtual HRESULT STDMETHODCALLTYPE PresentImage( 
		/* [in] */ DWORD_PTR dwUserID,
		/* [in] */ VMR9PresentationInfo *lpPresInfo);

	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		REFIID riid,
		void** ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

protected:
	int			format_id(D3DFORMAT fmt);
	HRESULT createDevice(const std::string szIniFile,bool bWindowed);
	HRESULT getVideo(DWORD_PTR dwId,CVideo** ppVideo);
	void			shutdown();
protected:
	IDirect3D9*									m_pD3d;
	IDirect3DDevice9* 						m_pD3ddev;
	IDirect3DSurface9*					m_pRenderTarget;
	IDirect3DSwapChain9*				m_D3DSwapChain;
	IDirect3DSurface9*					m_pSwapChainBackBuffer;
	CCritSec										m_cs;
	long												m_nRef;
	int												m_nDeviceCreate;
	bool												m_bWindowed;
	HWND											m_hWnd;
	HWND											m_hHY;
	D3DPRESENT_PARAMETERS*	m_d3dpp;
	D3DPRESENT_PARAMETERS		m_d3dppHY;//幻影窗口
	D3DPRESENT_PARAMETERS		m_d3dppW;//窗口
	D3DPRESENT_PARAMETERS		m_d3dppFS;//全屏
	D3DPRESENT_PARAMETERS		m_d3dPresent;
	D3DCAPS9									m_d3dCaps;
	int												m_nScreenBPP;
	bool												m_bFPSVSYNC;
	bool												m_bZBuffer;
	CGSize											m_winSize;
	std::string									m_szIniFile;

public:
	MAPSTRINGID							m_mapStrId;
};
