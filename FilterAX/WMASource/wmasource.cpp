// WMASource.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "WMASource.h"
#include "WMReader.h"

#define WMASourceName L"WMA Source"

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_NULL},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Output", FALSE, TRUE, FALSE, TRUE, &CLSID_NULL, NULL, _countof(sudPinTypesOut), sudPinTypesOut}
	// pin的名字 Obsolete, not used.
	// Is it rendered       输入pin有用，输出pin一般为FALSE 
	// Is it an output      TRUE表示是输出pin，不然是输入pin 
	// Can the filter create zero instances? 是否能不实例化 
	// Does the filter create multiple instances?是否能创建多个同这样类型的pin 
	// Obsolete. 
	// Obsolete.
	// Number of media types. 该pin支持的媒体类型数量 
	// Pointer to media types.   该pin的媒体类型的描述 
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CWMASource), WMASourceName, MERIT_DO_NOT_USE, _countof(sudpPins), sudpPins }
	// Filter CLSID   该filter的类标志 
	// Filter name.   该filter的名字 
	// Filter merit   该filter的Merit值 
	// Number of pin types.   该filter的pin的数目 
	// Pointer to pin information.   该filter的pin的描述 
};

CFactoryTemplate g_Templates[] = {
	// This entry is for the filter.
	{sudFilter[0].strName, sudFilter[0].clsID, CWMASource::CreateInstance, NULL, &sudFilter[0]},
	//filter的名字 
	//对象的类标识   
	//创建一个实例用的函数 
	// NULL
	//Pointer to filter information.filter的注册信息

	// This entry is for the property page.
	{ L"WMAS Props",&__uuidof(CGrayProp),CGrayProp::CreateInstance, NULL, NULL},

};
int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );
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


//CCWMAPassThru
CCWMAPassThru::CCWMAPassThru(LPUNKNOWN lpunk, HRESULT *phr,CWMAOutputPin* owner)
:CSourceSeeking(NAME("CWMA MediaSeeking"), lpunk, phr,owner->m_pFilter->pStateLock())
,m_pOwner(owner)
{
	m_rtStart = 0;
	m_rtStop = m_pOwner->m_pWMReader->getFileDuration();
	m_rtDuration = m_rtStop;
}

CCWMAPassThru::~CCWMAPassThru()
{

}

STDMETHODIMP CCWMAPassThru::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CCWMAPassThru::ChangeStart()
{
	assert(m_rtStop >= m_rtStart);
	assert(m_pOwner && (m_pOwner->m_pWMReader));
	CAutoLock lock(m_pOwner->m_pFilter->pStateLock());

	m_pOwner->m_pWMReader->controlStop();
	m_pOwner->m_pWMReader->resetBufferUse();
	m_pOwner->m_pWMReader->controlStart(m_rtStart);

	m_pOwner->updateFromSeek();
	return S_OK;
}
HRESULT CCWMAPassThru::ChangeStop()
{
	return S_OK;
}
HRESULT CCWMAPassThru::ChangeRate()
{//rate
	return S_OK;
}


//CWMAOutputPin
CWMAOutputPin::CWMAOutputPin(HRESULT* phr,CSource* ps,LPCWSTR lpPinName,CWMReader* pWMReader)
: CSourceStream(NAME("CWMA Stream"),phr,ps,lpPinName)
, m_pWMReader(pWMReader)
, m_pPassThru(NULL)
{
	if (m_pWMReader)
	{
		m_pWMReader->AddRef();
	}
	m_pPassThru = new CCWMAPassThru(NULL,phr,this);
	if (!m_pPassThru)
	{
		*phr = E_OUTOFMEMORY;
		return ;
	}
	m_pPassThru->AddRef();
}

CWMAOutputPin::~CWMAOutputPin()
{
	if (m_pWMReader)
	{
		m_pWMReader->releaseReader();
		m_pWMReader->Release();
	}
	if (m_pPassThru)
	{
		m_pPassThru->Release();
	}
}

STDMETHODIMP CWMAOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IMediaSeeking)
	{
		return m_pPassThru->QueryInterface(riid,ppv);
	}
	else if (riid == __uuidof(IGSeekPosition))
	{
		return GetInterface((IGSeekPosition*)this,ppv);
	}
	return CSourceStream::NonDelegatingQueryInterface(riid,ppv);
}

void CWMAOutputPin::setWMAOutputPin(CWMReader* pWMReader)
{
	if (m_pWMReader)
	{
		m_pWMReader->Release();
		m_pWMReader = NULL;
	}
	m_pWMReader = pWMReader;
	if (m_pWMReader)
	{
		m_pWMReader->AddRef();
	}
}

HRESULT CWMAOutputPin::setNewPosition(REFERENCE_TIME rtCur)
{
	if (rtCur == m_pPassThru->getStartRT())
	{
		return S_OK;
	}

	if (m_pWMReader->getReaderState() == RS_Stop)
	{
		m_pWMReader->controlStart(rtCur);
	}
	else
	{
		m_pWMReader->controlPause();
		m_pWMReader->controlStart(rtCur);
		m_pWMReader->resetBufferUse();
	}

	return S_OK;
}

HRESULT CWMAOutputPin::DecideBufferSize(IMemAllocator * pAlloc,ALLOCATOR_PROPERTIES * ppropInputRequest)
{
	CheckPointer(pAlloc,E_POINTER);
	CheckPointer(ppropInputRequest,E_POINTER);
	
	if (ppropInputRequest->cbBuffer < SAMPLEBUFFERSIZE)
	{
		ppropInputRequest->cbBuffer = SAMPLEBUFFERSIZE;
	}

	if (ppropInputRequest->cBuffers <= 0)
	{
		ppropInputRequest->cBuffers = 2;
	}

	ALLOCATOR_PROPERTIES actualpp;
	HRESULT hr = pAlloc->SetProperties(ppropInputRequest,&actualpp);
	if (actualpp.cbBuffer < SAMPLEBUFFERSIZE)
	{
		return E_FAIL;
	}

	assert(actualpp.cBuffers >= 1);

	return S_OK;
}

HRESULT CWMAOutputPin::FillBuffer(IMediaSample *pSamp)
{
	CheckPointer(m_pWMReader,E_UNEXPECTED);
	if(m_pWMReader->isEOF())
	{
		return S_FALSE;
	}

	SampleBuffer sb;
	while(!m_pWMReader->getCurrentSample(sb))//对其中的标志位，暂不处理（一般不会存在，除非文件有错误）
	{
		Sleep(1);
		if (m_pWMReader->getReaderState() == RS_Stop)
		{
			return S_FALSE;
		}
	}

	BYTE* pOut;
	long cSize = pSamp->GetSize();
	pSamp->GetPointer(&pOut);

	if (cSize < sb.cbData)
	{
		return E_UNEXPECTED;
	}
	
	memcpy(pOut,sb.pData,sb.cbData);
	pSamp->SetActualDataLength(sb.cbData);
	assert(m_pPassThru);
	REFERENCE_TIME rtStart = m_pPassThru->getStartRT();
	REFERENCE_TIME rt_start = sb.start_tm-rtStart;
	REFERENCE_TIME rt_end = sb.end_tm-rtStart;
	pSamp->SetTime(&rt_start,&rt_end);
	
	if (m_pWMReader->getReaderState() ==  RS_Pause)
	{
		m_pWMReader->controlResume();
	}

	return S_OK;
}

HRESULT CWMAOutputPin::Active(void)
{
	HRESULT hr;

	assert(m_pWMReader);
	assert(m_pPassThru);

	m_pWMReader->resetBufferUse();
	REFERENCE_TIME rtStart = m_pPassThru->getStartRT();
	if(!m_pWMReader->controlStart(rtStart))
	{
		return E_FAIL;
	}

	return CSourceStream::Active();
}

HRESULT CWMAOutputPin::Inactive()
{
	assert(m_pWMReader);
	m_pWMReader->controlStop();
	return CSourceStream::Inactive();
}

HRESULT CWMAOutputPin::OnThreadStartPlay(void)
{
	assert(m_pPassThru);
	REFERENCE_TIME rtStart = m_pPassThru->getStartRT();
	REFERENCE_TIME rtStop = m_pPassThru->getStopRT();
	double rateSeeking = m_pPassThru->getRateSeeking();
	return DeliverNewSegment(rtStart, rtStop, rateSeeking);
}


STDMETHODIMP CWMAOutputPin::Notify(IBaseFilter * pSender, Quality q)
{
	return E_NOTIMPL;
}

void CWMAOutputPin::updateFromSeek(void)
{
	if (ThreadExists()) 
	{
		DeliverBeginFlush();
		Stop();
		DeliverEndFlush();
		Run();
	}
}


HRESULT CWMAOutputPin::CheckMediaType(const CMediaType *pMediaType)
{
	assert(m_pFilter);
	return static_cast<CWMASource*>(m_pFilter)->CheckMediaType(pMediaType);
}

HRESULT CWMAOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)  // List pos. 0-n
{
	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}

	if (iPosition)
	{
		return VFW_S_NO_MORE_ITEMS;
	}
	CheckPointer(m_pFilter,E_UNEXPECTED);
	CheckPointer(pMediaType,E_POINTER);
	return pMediaType->Set(*(getMT()));
}

AM_MEDIA_TYPE* CWMAOutputPin::getMT()
{
	assert(m_pWMReader);
	return (AM_MEDIA_TYPE*)m_pWMReader->getMT();
}

//CWMASource
CWMASource::CWMASource(LPUNKNOWN lpunk, HRESULT *phr)
: CSource(NAME("CWMASource Filter"),lpunk,__uuidof(this),phr)
{
	
}

CWMASource::~CWMASource(void)
{
	
}

CUnknown * WINAPI CWMASource::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr)
{
	CWMASource* pNew = new CWMASource(pUnk,pHr);
	if (!pNew)
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pNew;
}

STDMETHODIMP CWMASource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IFileSourceFilter)
	{
		return GetInterface((IFileSourceFilter*)this,ppv);
	}
	else if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface((ISpecifyPropertyPages*)this,ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CWMASource::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
	CAutoLock lock(pStateLock());
	CheckPointer(pszFileName, E_POINTER);
	
	m_strFile = pszFileName;
	if(!wcsstr(m_strFile.c_str(),L".wma"))
	{
		return E_FAIL;
	}
	HRESULT hr;
	CWMReader* pWMReader = new CWMReader(m_strFile,&hr,1);
	if (!pWMReader)
	{
		return E_OUTOFMEMORY;
	}
	if (FAILED(hr))
	{
		delete pWMReader;
		return hr;
	}

	if(!pWMReader->controlOpen())
	{
		pWMReader->releaseReader();
		pWMReader->Release();
		return E_FAIL;
	}
	if (pWMReader->isMultiStream())
	{
		CWMReader* pWMReader2 = new CWMReader(m_strFile,&hr,2);
		if (!pWMReader2)
		{
			return E_OUTOFMEMORY;
		}
		if (FAILED(hr))
		{
			delete pWMReader;
			delete pWMReader2;
			return hr;
		}
		pWMReader2->controlOpen();
		CWMAOutputPin* pOutPin1 = new CWMAOutputPin(&hr,this,L"Out 1",pWMReader);
		pWMReader->Release();
		if (!pOutPin1)
		{
			return E_OUTOFMEMORY;
		}
		if (FAILED(hr))
		{
			delete pOutPin1;
			return hr;
		}
		CWMAOutputPin* pOutPin2 = new CWMAOutputPin(&hr,this,L"Out 2",pWMReader2);
		pWMReader2->Release();
		if (!pOutPin2)
		{
			return E_OUTOFMEMORY;
		}
		if (FAILED(hr))
		{
			delete pOutPin1;
			delete pOutPin2;
			return hr;
		}
	}
	else
	{
		CWMAOutputPin* pOutPin = new CWMAOutputPin(&hr,this,L"Out",pWMReader);
		if (!pOutPin)
		{
			return E_OUTOFMEMORY;
		}
		if (FAILED(hr))
		{
			delete pOutPin;
			return hr;
		}
	}

	return hr;
}

STDMETHODIMP CWMASource::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
	CAutoLock lock(pStateLock());
	*ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_strFile.size()+1)*2);
	wcscpy(*ppszFileName,m_strFile.c_str());

	return S_OK;
}

HRESULT CWMASource::CheckMediaType(const CMediaType* pmt)
{
	CAutoLock lock(pStateLock());
	if (pmt->formattype == FORMAT_WaveFormatEx
		&& ((WAVEFORMATEX*)pmt->pbFormat)->nChannels > 2
		&& ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
			return VFW_E_INVALIDMEDIATYPE;    // stupid iviaudio tries to fool us
	}

	return (pmt->majortype == MEDIATYPE_Audio
		&& pmt->formattype == FORMAT_WaveFormatEx
		&& (((WAVEFORMATEX*)pmt->pbFormat)->wBitsPerSample == 8
		|| ((WAVEFORMATEX*)pmt->pbFormat)->wBitsPerSample == 16
		|| ((WAVEFORMATEX*)pmt->pbFormat)->wBitsPerSample == 24
		|| ((WAVEFORMATEX*)pmt->pbFormat)->wBitsPerSample == 32)
		&& (((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_PCM))
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;
}

STDMETHODIMP CWMASource::GetPages(CAUUID *pPages)
{
	CAutoLock lock(pStateLock());
	CheckPointer(pPages,E_POINTER);
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(pPages->cElems*sizeof(GUID));
	if (pPages->pElems == NULL) 
	{
		return E_OUTOFMEMORY;
	}
	pPages->pElems[0] = __uuidof(CGrayProp);
	return S_OK;
}





