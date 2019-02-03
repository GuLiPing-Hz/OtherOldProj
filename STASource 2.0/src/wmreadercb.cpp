#include "stdafx.h"
#include "wmreadercb.h"

CWMReaderCallBack::CWMReaderCallBack():m_cRef(1),m_asyncevnet(NULL)
,m_pReader(NULL)
{
	
}

CWMReaderCallBack::~CWMReaderCallBack()
{
	SafeRelease(m_pReader);
}

bool CWMReaderCallBack::Initialize(WMCBFUNC eofunc,HANDLE asyncevent,CRingEx* ring,IWMReader* pReader)
{
	if(ring == NULL || asyncevent == NULL) 
		return false;
	m_asyncevnet = asyncevent;
	m_lpRingex = ring;
	m_eof = eofunc;
	
	ASSERT( pReader != NULL);

	m_pReader = pReader;
	m_pReader->AddRef();
	return true;
}

HRESULT STDMETHODCALLTYPE CWMReaderCallBack::QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if ((IID_IWMReaderCallback == riid) || (IID_IUnknown == riid))
	{
		*ppvObject = static_cast<IWMReaderCallback*> (this);
		AddRef();
		return S_OK;
	}
	else if (IID_IWMReaderCallbackAdvanced == riid)
	{
		*ppvObject = static_cast<IWMReaderCallbackAdvanced*> (this);
		AddRef();
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CWMReaderCallBack::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CWMReaderCallBack::Release()
{
	if (0 == InterlockedDecrement(&m_cRef))
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT CWMReaderCallBack::OnStatus(WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext)
{
	switch (Status)
	{
	case WMT_OPENED:
		SetEvent(m_asyncevnet);
		break;
	case WMT_CLOSED:
		SetEvent(m_asyncevnet);
		break;
	case WMT_STARTED:
		m_bEof = false;
		break;
	case WMT_STOPPED:
		SetEvent(m_asyncevnet);
		break;
	case WMT_EOF:
		m_bEof = true;
		if(!m_eof.empty()) 
			m_eof(hr);
	case WMT_ERROR:
		break;
	case WMT_END_OF_STREAMING:
		break;
	case WMT_MISSING_CODEC:
		OutputDebugString(TEXT("WMReader Can't decode data! \n"));
		m_bEof = TRUE;
		break;
	case WMT_BUFFERING_START:
		break;
	case WMT_BUFFERING_STOP:
		break;
	case WMT_LOCATING:
		break;
	case WMT_SAVEAS_START:
		break;
	case WMT_SAVEAS_STOP:
		break;
	default:
		break;
	}

	return S_OK;
}

HRESULT CWMReaderCallBack::OnSample(DWORD dwOutputNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR *pSample,void __RPC_FAR *pvContext)
{
	CheckPointer(pSample,E_POINTER);

	if(m_dwOoutputNum != dwOutputNum) 
		return S_OK;

	BYTE	*pData = NULL;
	DWORD	cbData = 0;
	HRESULT hr = pSample->GetBufferAndLength(&pData, &cbData);
	if(FAILED(hr))
	{
		return hr;
	}

	if(m_lpRingex->SetData(pData,cbData,m_pReader) == cbData) 
	{
		return S_OK;
	}

#if(OUTPUTPLAYERDEBUG)
	_stprintf(g_buffer,TEXT("CWMReaderCallBack::TimeStamp:%u Duration:%u \n"),cnsSampleTime,cnsSampleDuration);
	OutputDebugStr(g_buffer);
#endif
	return S_FALSE;
}

HRESULT CWMReaderCallBack::OnTime(QWORD qwCurrentTime,void __RPC_FAR * pvContext)
{
	return S_OK;
}

HRESULT CWMReaderCallBack::OnOutputPropsChanged(DWORD dwOutputNum,WM_MEDIA_TYPE __RPC_FAR * pMediaType,void __RPC_FAR * pvContext )
{
	return S_OK;
}

HRESULT CWMReaderCallBack::OnStreamSelection(WORD wStreamCount,WORD __RPC_FAR * pStreamNumbers,WMT_STREAM_SELECTION __RPC_FAR * pSelections,void __RPC_FAR * pvContext)
{
	return S_OK;
}

HRESULT CWMReaderCallBack::OnStreamSample(WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext)
{
	return S_OK;
}

HRESULT CWMReaderCallBack::AllocateForOutput(DWORD dwOutputNum,DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext)
{
#ifdef _TRY_SAMPLE_OPTIMIZATION_
	mSampleAdmin.GetEmptySample(cbBuffer, ppBuffer);
	return S_OK;
#else
	return E_NOTIMPL;
#endif
}

HRESULT CWMReaderCallBack::AllocateForStream(WORD wStreamNum,DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext)
{
	return E_NOTIMPL;
}
HRESULT	CWMReaderCallBack::InitSampleAdmin(int inCount, DWORD inBufferSize)
{
#ifdef _TRY_SAMPLE_OPTIMIZATION_
	return mSampleAdmin.Init( 5, inBufferSize );	
#endif 
	return S_FALSE;
}
HRESULT CWMReaderCallBack::UninitSampleAdmin(void)
{
#ifdef _TRY_SAMPLE_OPTIMIZATION_
	return mSampleAdmin.Uninit();
#endif 
	return S_FALSE;
}