// FilterWindow.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "FilterWindow.h"

const AMOVIESETUP_MEDIATYPE sudPinTypes[] = 
{
	{&MEDIATYPE_Video,&MEDIASUBTYPE_NULL},
	{&MEDIATYPE_Audio,&MEDIASUBTYPE_NULL},
	{&MEDIATYPE_Stream,&MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN sudPins[] =
{
	{L"Input",FALSE,FALSE,FALSE,FALSE,&CLSID_NULL,NULL,_countof(sudPinTypes),sudPinTypes},
	{L"Output",FALSE,TRUE,FALSE,FALSE,&CLSID_NULL,NULL,0,NULL}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CFilterWindow),MYFILTERNAME,MERIT_DO_NOT_USE,_countof(sudPins),sudPins},
};
//////////////////////////////////////////////////////////////////////////
CFactoryTemplate g_Templates[] = 
{
	{sudFilter[0].strName,sudFilter[0].clsID,CFilterWindow::CreateInstance,NULL,&sudFilter[0]}
};
int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}
STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

//
// Win32 DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  dwReason, 
					  LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
//////////////////////////////////////////////////////////////////////////
CUnknown* WINAPI CFilterWindow::CreateInstance(LPUNKNOWN pUnk,HRESULT* pHr)
{
	CUnknown* pCunk = new CFilterWindow(pUnk,__uuidof(CFilterWindow));
	if (!pCunk)
	{
		*pHr = E_OUTOFMEMORY;
		return NULL;
	}

	*pHr = S_OK;
	return pCunk;
}

CFilterWindow::CFilterWindow(LPUNKNOWN pUnk, REFCLSID clsid)
: CTransformFilter(NAME("CFilterWindow Filter"),pUnk,clsid)
{

}
CFilterWindow::~CFilterWindow()
{

}
STDMETHODIMP CFilterWindow::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return __super::NonDelegatingQueryInterface(riid,ppv);
}

HRESULT CFilterWindow::Transform(IMediaSample * pIn, IMediaSample *pOut)
{
	CheckPointer(pIn,E_POINTER);
	CheckPointer(pOut,E_POINTER);

	REFERENCE_TIME rtStart,rtStop;
	HRESULT hr = pIn->GetTime(&rtStart,&rtStop);
	long nLen = pIn->GetActualDataLength();
	BYTE* pInBuffer = NULL;
	BYTE* pOutBuffer = NULL;
	long nOutSize =pOut->GetSize();
	hr = pIn->GetPointer(&pInBuffer);
	hr != pOut->GetPointer(&pOutBuffer);
	if (FAILED(hr))
	{
		return hr;
	}
	char buf[260] = {0};
	if (/*rtStart < 0 &&*/ rtStart != -3689348814741910324)
	{
// 		sprintf_s(buf,259,"FilterWindow rtStart: %lld,rtStop: %lld,nLen %d\n",rtStart,rtStop,nLen);
// 		OutputDebugStringA(buf);
	}

	long minSize = min(nLen,nOutSize);
	memcpy(pOutBuffer,pInBuffer,minSize);
	hr = pOut->SetActualDataLength(minSize);

	return hr;
}
 
// check if you can support mtIn
HRESULT CFilterWindow::CheckInputType(const CMediaType* mtIn)
{
	return S_OK;
}
// check if you can support the transform from this input to this output
HRESULT CFilterWindow::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	if (*mtIn == *mtOut)
	{
		return S_OK;
	}
	return E_FAIL;
}
HRESULT CFilterWindow::DecideBufferSize(IMemAllocator * pAllocator,ALLOCATOR_PROPERTIES *pprop)
{
	CheckPointer(pAllocator,E_POINTER);
	CheckPointer(pprop,E_POINTER);

	ALLOCATOR_PROPERTIES actualPP;
	memset(&actualPP,0,sizeof(actualPP));

	pprop->cBuffers = pprop->cBuffers < 1 ? 1 : pprop->cBuffers;
	pprop->cbBuffer = MYBUFFERSIZE;

	HRESULT hr = pAllocator->SetProperties(pprop,&actualPP);
	if (FAILED(hr))
	{
		return hr;
	}

	ASSERT(actualPP.cBuffers >= 1);
	ASSERT(actualPP.cbBuffer >= MYBUFFERSIZE);

	return S_OK;
}

HRESULT CFilterWindow::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	if (!m_pInput || !m_pInput->IsConnected())
	{
		return E_UNEXPECTED;
	}

	CComPtr<IEnumMediaTypes> pEm;
	if (FAILED(m_pInput->GetConnected()->EnumMediaTypes(&pEm)))
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	if (iPosition>0 && FAILED(pEm->Skip(iPosition)))
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	AM_MEDIA_TYPE* tmp = NULL;
	if (S_OK != pEm->Next(1,&tmp,NULL) || !tmp)
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	CopyMediaType(pMediaType,tmp);
	DeleteMediaType(tmp);

	return S_OK;
}

