#pragma once

#include <streams.h>

// {D176F46D-2065-48cb-8122-6F5CEEA343BE}
// DEFINE_GUID(<<name>>, 
// 			0xd176f46d, 0x2065, 0x48cb, 0x81, 0x22, 0x6f, 0x5c, 0xee, 0xa3, 0x43, 0xbe);

#define MYBUFFERSIZE (1024*1024)
#define MYFILTERNAME L"FILTER WINDOW"

class __declspec(uuid("D176F46D-2065-48cb-8122-6F5CEEA343BE"))
CFilterWindow : public CTransformFilter
{
public:
	CFilterWindow(LPUNKNOWN pUnk, REFCLSID clsid);
	~CFilterWindow();

	static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk,HRESULT* pHr);

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	virtual HRESULT Transform(IMediaSample * pIn, IMediaSample *pOut);
	//virtual HRESULT Receive(IMediaSample *pSample);
	// check if you can support mtIn
	virtual HRESULT CheckInputType(const CMediaType* mtIn);
	// check if you can support the transform from this input to this output
	virtual HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	virtual HRESULT DecideBufferSize(IMemAllocator * pAllocator,ALLOCATOR_PROPERTIES *pprop);
	virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
public:

protected:

};

