#include "stdafx.h"
#include "wmreader.h"

CWMReader::CWMReader():m_lpReader(NULL),m_syncevent(NULL),m_lpReaderAdvanced(NULL){

}

CWMReader::~CWMReader(){
	UnInitialize();
}

HRESULT STDMETHODCALLTYPE CWMReader::QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject){
	if ((IID_IWMReaderCallback == riid) || (IID_IUnknown == riid)){
		*ppvObject = static_cast<IWMReaderCallback*> (this);
		AddRef();
		return S_OK;
	}else if (IID_IWMReaderCallbackAdvanced == riid){
		*ppvObject = static_cast<IWMReaderCallbackAdvanced*> (this);
		AddRef();
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CWMReader::AddRef(){
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CWMReader::Release(){
	if (0 == InterlockedDecrement(&m_cRef)){
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT CWMReader::OnStatus(WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext){
	switch (Status){
	case WMT_OPENED:
		SetEvent(m_syncevent);
		break;
	case WMT_CLOSED:
		SetEvent(m_syncevent);
		break;
	case WMT_STARTED:
		m_fileattribute._iseof = false;
		break;
	case WMT_STOPPED:
		SetEvent(m_syncevent);
		break;
	case WMT_EOF:
		m_fileattribute._iseof = true;
		Stop();
	case WMT_ERROR:
		break;
	case WMT_END_OF_STREAMING:
		break;
	case WMT_MISSING_CODEC:
		OutputDebugString(TEXT("WMReader Can't decode data! \n"));
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

HRESULT CWMReader::OnSample(DWORD dwOutputNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR *pSample,void __RPC_FAR *pvContext){
	CheckPointer(pSample,E_POINTER);
	if(m_dwOoutputNum != dwOutputNum) return S_OK;

	BYTE	*pData = NULL;
	DWORD	cbData = 0;
	HRESULT hr = pSample->GetBufferAndLength(&pData, &cbData);
	if(FAILED(hr)){
		return hr;
	}
	if(m_ringex(pData,cbData) == cbData) return S_OK;
	return S_FALSE;
}

HRESULT CWMReader::OnTime(QWORD qwCurrentTime,void __RPC_FAR * pvContext){
	return S_OK;
}

HRESULT CWMReader::OnOutputPropsChanged(DWORD dwOutputNum,WM_MEDIA_TYPE __RPC_FAR * pMediaType,void __RPC_FAR * pvContext ){
	return S_OK;
}

HRESULT CWMReader::OnStreamSelection(WORD wStreamCount,WORD __RPC_FAR * pStreamNumbers,WMT_STREAM_SELECTION __RPC_FAR * pSelections,void __RPC_FAR * pvContext){
	return S_OK;
}

HRESULT CWMReader::OnStreamSample(WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext){
	return S_OK;
}

HRESULT CWMReader::AllocateForOutput(DWORD dwOutputNum,DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext){
	return E_NOTIMPL;
}

HRESULT CWMReader::AllocateForStream(WORD wStreamNum,DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext){
	return E_NOTIMPL;
}

HRESULT CWMReader::Initialize(){
	if(m_lpReader != NULL) return S_OK;
	if(m_syncevent == NULL){
		m_syncevent = CreateEvent(NULL,FALSE,FALSE,NULL);
		if(m_syncevent == NULL) return E_OUTOFMEMORY;
	}
	HRESULT hr = WMCreateReader(NULL,WMT_RIGHT_PLAYBACK,&m_lpReader);
	if(hr != S_OK){
		OutputDebugStr(TEXT("Create WMReader failed! \n"));
		return E_INVALIDARG;
	}
	return S_OK;
}

void CWMReader::UnInitialize(){
	SafeRelease(m_lpReaderAdvanced);
	SafeRelease(m_lpReader);
}

HRESULT CWMReader::Open(LPCTSTR pwszUrl){
	if(!IsOk()) return E_INVALIDARG;
	if(IsNull(pwszUrl)) return S_FALSE;
	ResetEvent(m_syncevent);
	Close();

	hr = m_lpReader->Open(pwszUrl, this, NULL);
	if(hr != S_OK){
		OutputDebugStr(TEXT("WMReader:Open file failed!\n"));
		return hr;
	}
	return FillHeader();
}

HRESULT CWMReader::Open(IStream* stream){
	if(!IsOk()) return E_INVALIDARG;
	ResetEvent(m_syncevent);
	Close();

	m_stream = (CStream*)stream;
	HRESULT hr = S_OK;
	IWMReaderAdvanced2* lpAdvance = NULL;
	hr = m_lpReader->QueryInterface(IID_IWMReaderAdvanced2,(void**)&lpAdvance);
	if(hr != S_OK){
		OutputDebugStr(TEXT("WMReader:QueryInterface IID_IWMReaderAdvanced2 failed! \n"));
		return hr;
	}
	hr = lpAdvance->OpenStream(stream,this,NULL);
	if(hr != S_OK){
		OutputDebugStr(TEXT("IWMReaderAdvanced2:Openstream failed! \n"));
		return hr;
	}
	return FillHeader();
}

HRESULT CWMReader::FillHeader(){
	m_fileattribute.clear();

	HRESULT hr = S_OK;
	do 
	{
		WaitForSingleObject(mAsyncEvent,INFINITE);

		{
			IWMHeaderInfo*	lpInfo = NULL;
			hr = mIReader->QueryInterface(IID_IWMHeaderInfo3, (void **)&lpInfo);
			BREAK_IF_ISNOTSOK(hr);

			WORD  streamnum = 0;
			WMT_ATTR_DATATYPE pType;
			BYTE pbAttribValue[MAX_PATH];
			WORD dwAttribValueLen = MAX_PATH;
			dwAttribValueLen = MAX_PATH;
			pbAttribValue[0] = 0;

			hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMDuration,&pType,pbAttribValue,&dwAttribValueLen);
			BREAK_IF_ISNOTSOK(hr);
			m_fileattribute._duration = *((QWORD*)pbAttribValue);

			hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMSeekable,&pType,pbAttribValue,&dwAttribValueLen);
			BREAK_IF_ISNOTSOK(hr);
			m_fileattribute._canseek = *((BOOL*)pbAttribValue) == TRUE;

			hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMBroadcast,&pType,pbAttribValue,&dwAttribValueLen);
			BREAK_IF_ISNOTSOK(hr);
			m_fileattribute._canbroadcast = *((BOOL*)pbAttribValue) == TRUE;

			hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMHasAudio,&pType,pbAttribValue,&dwAttribValueLen);
			BREAK_IF_ISNOTSOK(hr);
			m_fileattribute._asftype._audio = (*((BOOL*) pbAttribValue))==TRUE?true:false;

			hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMHasImage,&pType,pbAttribValue,&dwAttribValueLen);
			BREAK_IF_ISNOTSOK(hr);
			m_fileattribute._asftype._image = (*((BOOL*) pbAttribValue))==TRUE?true:false;

			hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMHasScript,&pType,pbAttribValue,&dwAttribValueLen);
			BREAK_IF_ISNOTSOK(hr);
			m_fileattribute._asftype._script = (*((BOOL*) pbAttribValue))==TRUE?true:false;

			hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMHasVideo,&pType,pbAttribValue,&dwAttribValueLen);
			BREAK_IF_ISNOTSOK(hr);
			m_fileattribute._asftype._video = (*((BOOL*) pbAttribValue))==TRUE?true:false;
		}

		{			
			ULONG cbType		= 0;
			GUID  requiredGuid	= m_fileattribute._asftype._video?WMMEDIATYPE_Video:WMMEDIATYPE_Audio;
			DWORD outputs		= 0;
			hr = m_lpReader->GetOutputCount(&outputs);  ///< 媒体流的数量
			BREAK_IF_ISNOTSOK(hr);

			DWORD i = 0;
			for(i = 0; i < outputs; i++){
				IWMOutputMediaProps* pProps = NULL;
				hr = m_lpReader->GetOutputProps(i, &pProps);
				BREAK_IF_ISNOTSOK(hr);

				hr = pProps->GetMediaType(NULL, &cbType);
				BREAK_IF_ISNOTSOK(hr);

				SafeComMemFree(m_fileattribute._pAMMediaType);
				m_fileattribute._pAMMediaType = (AM_MEDIA_TYPE*)CoTaskMemAlloc(cbType);
				if(NULL == m_fileattribute._pAMMediaType){
					OutputDebugStr(TEXT("WMReader:CoTaskMemAlloc failed!\n"));
					hr = HRESULT_FROM_WIN32(GetLastError()) ;
					break;
				}
				
				hr = pProps->GetMediaType((WM_MEDIA_TYPE*)m_fileattribute._pAMMediaType, &cbType);
				SafeRelease(pProps);
				BREAK_IF_ISNOTSOK(hr);
				
				if (requiredGuid == m_fileattribute._pAMMediaType->majortype)
				{
					break;
				}
			}

			BREAK_IF_ISNOTSOK(hr);
			if(i == outputs){
				hr = E_UNEXPECTED;
				break;
			}

			m_fileattribute._number = i;
		}

		SafeRelease(m_lpReaderAdvanced);
		hr = mIReader->QueryInterface( IID_IWMReaderAdvanced, (void**)&m_lpReaderAdvanced );
		BREAK_IF_ISNOTSOK(hr);

		IWMProfile* lpWMProfile = NULL;
		IWMStreamConfig* lpConfig = NULL;
		IWMStreamConfig3* lpConfigex = NULL;

		hr = m_lpReader->QueryInterface(IID_IWMStreamConfig3,(void**)&lpWMProfile);
		BREAK_IF_ISNOTSOK(hr);

		DWORD dwStreamCount = 0;
		hr = lpWMProfile->GetStreamCount(&dwStreamCount);//得到流个数
		BREAK_IF_ISNOTSOK(hr);
		TCHAR buffer[MAX_PATH];
		WORD  size = MAX_PATH;

		for(DWORD dwIndex = 0; dwIndex < dwStreamCount; dwIndex++){
			hr = lpWMProfile->GetStream(dwIndex, &lpConfig);
			BREAK_IF_ISNOTSOK(hr);

			hr = lpConfig->QueryInterface(IID_IWMStreamConfig3, (void **)&lpConfigex);
			BREAK_IF_ISNOTSOK(hr);

			// Determine the stream type
			GUID guid = GUID_NULL;
			hr = lpConfig->GetStreamType(&guid);
			BREAK_IF_ISNOTSOK(hr);

			if(WMMEDIATYPE_Audio == guid)
			{
				ZeroMemory(buffer,sizeof(buffer));
				lpConfigex->GetLanguage(buffer,&size);
				m_fileattribute._language += buffer;
				m_fileattribute._language += TEXT(";");
			}

			SafeRelease(lpConfig);
			SafeRelease(lpConfigex);
		}

		SafeRelease(lpWMProfile);

		if(dwStreamCount>1){
			m_fileattribute._isMultiStream = true;
		}else{
			break;
		}
	}while (false);

	if(hr != S_OK){
		Close();
	}
	return hr;
}

HRESULT CWMReader::Close(void){
	if(IsOk()){
		HRESULT hr = m_lpReader->Close();
		if(hr != S_OK){
			return hr;
		}
		WaitForSingleObject(mAsyncEvent,INFINITE);
		return hr;
	}
	return E_FAIL;
}

HRESULT CWMReader::Start(QWORD inStartTime = 0){
	m_fileattribute._iseof = (inStartTime >= m_fileattribute._duration);
	if(m_fileattribute._iseof){
		return S_OK;
	}

	HRESULT hr = E_INVALIDARG;
	if(IsOk()){
		m_fileattribute._qwTimeElapsed = 0x0123456789;  // meaningless value
		hr = mIReader->Start(inStartTime,0,1.0,NULL);
		if(hr != S_OK){
			OutputDebugStr(TEXT("WMReader Start failed! \n"));
		}
	}
	return hr;
}

HRESULT CWMReader::Stop(void){
	if(IsOk()) return m_lpReader->Stop();
	return E_FAIL;
}

HRESULT CWMReader::Pause(void){
	if(IsOk()) return m_lpReader->Pause();
	return E_FAIL;
}

HRESULT CWMReader::Resume(void){
	if(IsOk()) return m_lpReader->Resume();
	return E_FAIL;
}