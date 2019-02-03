#include "StdAfx.h"
#include "WMReader.h"

CWMReader::CWMReader(std::wstring file,HRESULT* phr,int nStreamNum)
:m_pReader(NULL)
,m_pReaderAdvanced(NULL)
,m_pHeaderInfo(NULL)
,m_bSeekable(FALSE)
,m_bStridable(FALSE) 
,m_bEOF(FALSE)
,m_bMultStream(FALSE)
,m_nLanguageLen(0)
,m_hrSync(S_OK)
,m_hSyncEv(NULL)
,m_strFile(file)
,m_nDuration(0)
,m_uBitRate(0)
,m_bBroadcast(FALSE)
,m_pMT(NULL)
,m_uAVNumOut(0)
,m_uTotalStream(0)
,m_nTimeElapsed(0)
,m_uInCurrentIndex(0)
,m_uOutCurrentIndex(0)
,m_uUsedBuffer(0)
,m_lRef(1)
,m_eState(RS_Stop)
,m_StreamNum(nStreamNum)
{
	initSampleBuffer();
	m_hSyncEv = CreateEvent(NULL, FALSE, FALSE, NULL);
	// Create a reader object, requesting only playback rights.
	*phr = WMCreateReader(NULL, WMT_RIGHT_PLAYBACK, &m_pReader);
}

CWMReader::~CWMReader(void)
{
	SAFE_CLOSEHANDLE(m_hSyncEv);
	uninitSampleBuffer();
	SAFE_ARRAY_DELETE(m_pMT);
}

void CWMReader::releaseReader()
{
	controlClose();
	SAFE_RELEASE(m_pReaderAdvanced);
	SAFE_RELEASE(m_pHeaderInfo);
	SAFE_RELEASE(m_pReader);
}

void	CWMReader::initSampleBuffer()
{
	memset(m_sSampleBuffer,0,sizeof(m_sSampleBuffer));
	for (int i=0;i<SAMPLEBUFFERLEN;i++)
	{
		m_sSampleBuffer[i].pData = (BYTE*)malloc(SAMPLEBUFFERSIZE);
		assert(m_sSampleBuffer[i].pData);
		m_sSampleBuffer[i].cbData = SAMPLEBUFFERSIZE;
	}
}
void	CWMReader::uninitSampleBuffer()
{
	for (int i=0;i<SAMPLEBUFFERLEN;i++)
	{
		assert(m_sSampleBuffer[i].pData);
		free(m_sSampleBuffer[i].pData);
		m_sSampleBuffer[i].cbData = 0;
	}
	memset(m_sSampleBuffer,0,sizeof(m_sSampleBuffer));
}

HRESULT STDMETHODCALLTYPE CWMReader::QueryInterface(REFIID riid , void __RPC_FAR *__RPC_FAR *ppvObject) 
{
	if (riid == IID_IWMReaderCallback)
	{
		return GetInterface(static_cast<IWMReaderCallback*>(this),ppvObject);
	}
	else if (riid == IID_IUnknown)
	{
		return GetInterface(static_cast<IUnknown*>(this),ppvObject);
	}
	*ppvObject = NULL;
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CWMReader::AddRef()
{
	assert(m_lRef > 0);
	return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CWMReader::Release()
{
	assert(m_lRef > 0);
	if (0 == InterlockedDecrement(&m_lRef))
	{
		delete this;
		return 0;
	}
	return m_lRef;
}

HRESULT CWMReader::selectStream(int nStreamNumber)
{
	WMT_STREAM_SELECTION wmtSS=WMT_OFF/*WMT_ON*/;
	m_pReaderAdvanced->SetManualStreamSelection(TRUE);
	WORD nAudioStreamNum=nStreamNumber;

	HRESULT hr = m_pReaderAdvanced->SetStreamsSelected(1,&nAudioStreamNum,&wmtSS);
	return hr;
}

bool CWMReader::getCurrentSample(SampleBuffer& sb)
{
	if (m_uUsedBuffer <= 0)
	{
		return false;
	}
	if(m_uOutCurrentIndex == SAMPLEBUFFERLEN)
	{
		m_uOutCurrentIndex = 0;
	}
	memcpy(&sb,&m_sSampleBuffer[m_uOutCurrentIndex++],sizeof(SampleBuffer));
	InterlockedDecrement(&m_uUsedBuffer);
	return true;
}

STDMETHODIMP CWMReader::OnSample( DWORD dwOutputNum,QWORD cnsSampleTime,QWORD cnsSampleDuration
							  ,DWORD dwFlags,INSSBuffer *pSample,void *pvContext)
{
	// Check the output number of the sample against the stored output number.
	// Because only the first audio output is stored, all other outputs,
	// regardless of type, will be ignored.
	if (dwOutputNum != m_uAVNumOut)
	{
		return S_OK;
	}

	BYTE*			pData = NULL;
	DWORD       cbData = 0;
	// Get the sample from the buffer object.
	HRESULT hr = pSample->GetBufferAndLength(&pData, &cbData);
	if (FAILED(hr))
	{
		return hr;
	}

	m_nTimeElapsed = cnsSampleTime;

	while(m_uUsedBuffer>=SAMPLEBUFFERLEN)
	{
		Sleep(1);
		if (m_eState == RS_Stop)
		{
			return S_OK;
		}
	}
	
	assert (cbData <= SAMPLEBUFFERSIZE);
	memcpy(m_sSampleBuffer[m_uInCurrentIndex].pData, pData, cbData);

	m_sSampleBuffer[m_uInCurrentIndex].cbData = cbData;
	m_sSampleBuffer[m_uInCurrentIndex].start_tm = cnsSampleTime;
	m_sSampleBuffer[m_uInCurrentIndex].end_tm = cnsSampleTime+cnsSampleDuration;
	m_uInCurrentIndex++;
	if (m_uInCurrentIndex == SAMPLEBUFFERLEN)
	{
		m_uInCurrentIndex = 0;
	}
	InterlockedIncrement(&m_uUsedBuffer);

	if (m_uUsedBuffer>=SAMPLEBUFFERLEN)
	{
		controlPause();
	}

	return S_OK;
}

STDMETHODIMP CWMReader::OnStatus( WMT_STATUS Status, HRESULT hr, WMT_ATTR_DATATYPE dwType,BYTE *pValue,void *pvContext)
{
	// This switch checks for the important messages sent by the reader object.
	switch (Status)
	{
		// The reader is finished opening a file.
	case WMT_OPENED:
		{
			//TRACE("Source opened.\n");
			setSyncEvent(hr);	
			break;
		}
		// The reader is finished closing a file.
	case WMT_CLOSED:
		{
			//TRACE("Source closed.\n");
			setSyncEvent(hr);
			break;
		}
		// Playback of the opened file has begun.
	case WMT_STARTED:
		{
			//TRACE("Source started.\n");
			m_bEOF = FALSE;
			break;	
		}
		// The previously playing reader has stopped.
	case WMT_STOPPED:
		{
			//TRACE("Source stopped.\n");
			setSyncEvent(hr);
			break;
		}
	case WMT_EOF:
		{
			//TRACE("EOF\n");
			m_bEOF = TRUE;
			controlStop();
		}
	case WMT_ERROR:
	case WMT_END_OF_STREAMING:
	case WMT_MISSING_CODEC:
		{
			//TRACE("Runtime error or End of File.\n");
			break;
		}

		// The reader has begun buffering.
	case WMT_BUFFERING_START:
		{
			//TRACE("Buffering started.\n");
			break;
		}

		// the reader has completed buffering.
	case WMT_BUFFERING_STOP:
		{
			//TRACE("Buffering stopped.\n");
			break;
		}

	case WMT_LOCATING:
		{
			//TRACE("Locating the source...\n");
			break;
		}

		// Save as process
	case WMT_SAVEAS_START:
		{
			//TRACE("Start to save file as...\n");
			break;
		}

	case WMT_SAVEAS_STOP:
		{
			//TRACE("Finished to save file as.\n");
			break;
		}

	default:
		break;
	}

	return S_OK;
}

HRESULT  CWMReader::setAttribute(LPCWSTR pwszName,WMT_ATTR_DATATYPE eType, BYTE ** ppbValue,WORD cbLength)
{
	CheckPointer(ppbValue,E_POINTER);
	ValidateReadPtr(*ppbValue,cbLength);
	if( !m_pHeaderInfo )
	{
		return E_UNEXPECTED;
	}
	HRESULT hr = m_pHeaderInfo->SetAttribute( m_uAVNumOut,pwszName,eType,*ppbValue,cbLength);
	return hr;
}

HRESULT CWMReader::getHeaderAttribute(LPCWSTR pwszName, BYTE** ppbValue,WORD	wStreamNum/* = 0*/)
{
	CheckPointer(ppbValue,E_POINTER);

	BYTE*								pbValue = NULL;
	HRESULT							hr = S_OK;
	WMT_ATTR_DATATYPE	wmtType;
	WORD								cbLength = 0;

	*ppbValue = NULL;
	// Sanity check
	if(!m_pHeaderInfo)
	{
		return E_UNEXPECTED;
	}

	// Get the count of bytes to be allocated for pbValue
	hr = m_pHeaderInfo->GetAttributeByName(&wStreamNum,pwszName,&wmtType,NULL,&cbLength);
	if (FAILED(hr))
	{
		return hr;
	}
	// No such an attribute, so return
	if (ASF_E_NOTFOUND == hr)
	{
		return S_OK;
	}

	pbValue = new BYTE[cbLength];
	if (NULL == pbValue)
	{
		return E_OUTOFMEMORY;
	}
	// Get the actual value
	m_pHeaderInfo->GetAttributeByName(&wStreamNum,pwszName,&wmtType,pbValue,&cbLength);
	*ppbValue = pbValue;

	return S_OK;
}

HRESULT CWMReader::retrieveAttributes(void)
{
	BYTE*   pbValue = NULL;
	HRESULT hr = S_OK;
	
	do
	{
		// Get attribute "Title"
// 		hr = getHeaderAttribute(g_wszWMTitle, &pbValue);
// 		BREAK_IF_FAILED(hr);
// 		// free the mem
// 		SAFE_ARRAY_DELETE(pbValue);

		// Get attribute "Author"
// 		hr = getHeaderAttribute(g_wszWMAuthor, &pbValue);
// 		BREAK_IF_FAILED(hr);
// 		// free the mem
// 		SAFE_ARRAY_DELETE(pbValue);

		// Get attribute "Copyright"
// 		hr = getHeaderAttribute(g_wszWMCopyright, &pbValue);
// 		BREAK_IF_FAILED(hr);
// 		// free the mem
// 		SAFE_ARRAY_DELETE(pbValue);

		// Get attribute "Duration"
		hr = getHeaderAttribute(g_wszWMDuration, &pbValue);
		BREAK_IF_FAILED(hr);
		if (pbValue)
		{
			m_nDuration = *((QWORD*) pbValue);
			SAFE_ARRAY_DELETE(pbValue);
		}
		else
		{
			m_nDuration = 0;
		}

		// Retrieve Seekable attribute
		hr = getHeaderAttribute(g_wszWMSeekable, &pbValue);
		BREAK_IF_FAILED(hr);
		if (pbValue)
		{
			m_bSeekable = *((BOOL*) pbValue);
			// free the mem
			SAFE_ARRAY_DELETE(pbValue);
		}
		else
		{
			m_bSeekable = FALSE;
		}

		// Retrieve stridable attribute
		hr = getHeaderAttribute(g_wszWMStridable, &pbValue);
		BREAK_IF_FAILED(hr);
		if (pbValue)
		{
			m_bStridable = *((BOOL*) pbValue);
			SAFE_ARRAY_DELETE(pbValue);
		}
		else
		{
			m_bStridable = FALSE;
		}
		/*if (!mIsstridable)
		{
			BOOL tmpbStridable = TRUE;
			BOOL * ptmp = &tmpbStridable;
			hr = SetAttribute(g_wszWMStridable,(BYTE**)&ptmp);
			if (hr == S_OK)
			{
				mIsstridable = TRUE;
			}
		}*/
		hr = getHeaderAttribute(g_wszWMBitrate,&pbValue);
		BREAK_IF_FAILED(hr);
		if (NULL != pbValue)
		{
			m_uBitRate = *((DWORD*) pbValue);
			SAFE_ARRAY_DELETE(pbValue);
		}
		else
		{
			m_uBitRate = 0;
		}

		// Retrieve Broadcast attribute
		hr = getHeaderAttribute(g_wszWMBroadcast, &pbValue);
		BREAK_IF_FAILED(hr);
		if (NULL != pbValue)
		{
			m_bBroadcast = *((BOOL*) pbValue);
			SAFE_ARRAY_DELETE(pbValue);
		}
		else
		{
			m_bBroadcast = FALSE;
		}

	} while (FALSE);
		
	return hr;
}

HRESULT CWMReader::getAVOutput(BOOL bAudio)
{
	// Sanity check
	assert(m_pReader);

	HRESULT								hr = S_OK;
	IWMOutputMediaProps*		pProps = NULL;
	ULONG								cbType = 0;
	GUID  requiredGuid = bAudio? WMMEDIATYPE_Audio:WMMEDIATYPE_Video;
	do
	{
		// Find out the output count
		DWORD   outputs = 0;
		hr = m_pReader->GetOutputCount(&outputs);
		BREAK_IF_FAILED(hr);

		// Find out one audio output.
		// Note: This sample only shows how to handle one audio output.
		//  If there is more than one audio output, the first one will be picked.
		DWORD i;
		for ( i = 0; i < outputs; i++)
		{
			SAFE_RELEASE(pProps);
			hr = m_pReader->GetOutputProps(i, &pProps);
			BREAK_IF_FAILED(hr);

			// Find out the space needed for pMediaType
			hr = pProps->GetMediaType(NULL, &cbType);
			BREAK_IF_FAILED(hr);

			SAFE_ARRAY_DELETE(m_pMT);
			m_pMT = ( WM_MEDIA_TYPE* ) new BYTE[cbType];
			//		m_pAMMediaType = (WM_MEDIA_TYPE*) new BYTE[cbType];
			if (NULL == m_pMT)
			{
				hr = HRESULT_FROM_WIN32(GetLastError()) ;
				break;
			}
			// Get the value for MediaType
			hr = pProps->GetMediaType(m_pMT, &cbType);
			BREAK_IF_FAILED(hr);

			// Find the first audio/video output!
			if (requiredGuid == m_pMT->majortype)
			{
				break;
			}
		}

		BREAK_IF_FAILED(hr);
		if (i == outputs)
		{
			// Couldn't find any audio/video output number in the file
			hr = E_UNEXPECTED;
			break;
		}

		// Store the wave format for this output
		m_uAVNumOut = i;
		//mPlayer->SetInputFormat(pMediaType->pbFormat, pMediaType->cbFormat);

	} while (FALSE);

	SAFE_RELEASE(pProps);
	return hr;
}

BOOL CWMReader::controlOpen(std::wstring file /*= L""*/,BOOL bAudio/* = TRUE*/)
{
	if (file == L"")
	{
		file = m_strFile;
	}
	else if (file != m_strFile)
	{
		m_strFile = file;
	}

	assert(m_hSyncEv);
	assert(m_pReader);

	IWMProfile *					pProfile = NULL;
	IWMStreamConfig *		pConfig = NULL ;
	IWMStreamConfig3 *		pConfig3 = NULL ;
	HRESULT hr = S_OK;
	do 
	{
		ResetEvent(m_hSyncEv);
		// Close previously opened file, if any.
		controlClose();

		// Open the file with the reader object. This method call also sets
		// the status callback that the reader will use.
		hr = m_pReader->Open(file.c_str(), static_cast<IWMReaderCallback*>(this), NULL);
		BREAK_IF_FAILED(hr);

		// Wait for the Open call to complete. The event is set in the OnStatus 
		// callback when the reader reports completion.
		WaitForSingleObject(m_hSyncEv,INFINITE);
		// Check the HRESULT reported by the reader object to the OnStatus 
		// callback. Most errors in opening files will be reported this way.
		hr = m_hrSync;
		BREAK_IF_FAILED(hr);

		SAFE_RELEASE(m_pHeaderInfo);
		hr = m_pReader->QueryInterface(IID_IWMHeaderInfo, (void **) &m_pHeaderInfo);
		BREAK_IF_FAILED(hr);
		//-----------------------------------------------------------------

		hr = retrieveAttributes();

		// Set the audio/video ouput number for the current file. 
		// Only the first audio/video output is retrieved, regardless of 
		// the number of outputs in the file.
		hr = getAVOutput(bAudio);
		BREAK_IF_FAILED(hr);

		SAFE_RELEASE(m_pReaderAdvanced);
		hr = m_pReader->QueryInterface( IID_IWMReaderAdvanced, ( void ** )&m_pReaderAdvanced );
		BREAK_IF_FAILED(hr);

		hr = m_pReader->QueryInterface(IID_IWMProfile, (void **) &pProfile);
		BREAK_IF_FAILED(hr);

		DWORD dwStreamCount = 0;
		hr = pProfile->GetStreamCount(&dwStreamCount);//得到流个数
		BREAK_IF_FAILED(hr);
		m_strLanguage = "";
		for (DWORD dwIndex = 0; dwIndex < dwStreamCount; dwIndex++)
		{
			SAFE_RELEASE(pConfig);
			hr = pProfile->GetStream(dwIndex, &pConfig);
			BREAK_IF_FAILED(hr);

			SAFE_RELEASE(pConfig3);
			hr = pConfig->QueryInterface(IID_IWMStreamConfig3, (void **)&pConfig3);
			BREAK_IF_FAILED(hr);

			// Determine the stream type
			GUID guid = GUID_NULL;
			hr = pConfig->GetStreamType(&guid);
			BREAK_IF_FAILED(hr);

			if(WMMEDIATYPE_Audio == guid)
			{
				WORD wStreamNum = 0;
				hr = pConfig->GetStreamNumber(&wStreamNum);
				BREAK_IF_FAILED(hr);

				WCHAR pwszStreamName[20] ;
				WORD pcchStreamName = 20;
				char * pszStreamName;
				hr = pConfig3->GetLanguage(pwszStreamName , &pcchStreamName);
				BREAK_IF_FAILED(hr);
				pszStreamName = unicodeToAnsi(pwszStreamName);
				m_strLanguage = m_strLanguage + pszStreamName + ";";
				//free the memory
				SAFE_DELETE(pszStreamName);
			}
		}
		m_nLanguageLen = strlen(m_strLanguage.c_str());
		m_uTotalStream = dwStreamCount;
		if (m_uTotalStream>1)
		{
			m_bMultStream = TRUE;
		}
		else
		{
			m_bMultStream = FALSE;
		}
	} while (FALSE);

	SAFE_RELEASE(pConfig);
	SAFE_RELEASE(pConfig3);
	SAFE_RELEASE(pProfile);

	if (FAILED(hr))
	{
		return FALSE;
	}
	return TRUE;
}

HRESULT CWMReader::detectBandwidth(DWORD& bandwidth)
{
	HRESULT hr = NOERROR;
	// Create a reader object
	if (m_pReader)
	{
		IWMReaderNetworkConfig * pNetConfig = NULL;
		hr = m_pReader->QueryInterface(IID_IWMReaderNetworkConfig, (void**)&pNetConfig);
		if (SUCCEEDED(hr))
		{
			hr = pNetConfig->GetConnectionBandwidth(&bandwidth);
			pNetConfig->Release();
		}
	}
	return hr;
}

BOOL CWMReader::controlStart(QWORD nStartTime,float fRate)
{
	assert(m_pReader);
	HRESULT hr = m_pReader->Start(nStartTime,0,fRate,NULL);
	if (FAILED(hr))
	{
		hr = m_pReader->Start(nStartTime,0,1.0f,NULL);
	}

	if (SUCCEEDED(hr))
	{
		m_eState = RS_Running;
		if (m_bMultStream)
		{
			hr = selectStream(m_StreamNum);
		}
		return TRUE;
	}
	return FALSE;
}
BOOL CWMReader::controlPause()
{
	assert(m_pReader);
	HRESULT hr = m_pReader->Pause();
	if (SUCCEEDED(hr))
	{
		m_eState = RS_Pause;
		return TRUE;
	}
	return FALSE;

}
BOOL CWMReader::controlResume()
{
	assert(m_pReader);
	HRESULT hr = m_pReader->Resume();
	if (SUCCEEDED(hr))
	{
		m_eState = RS_Running;
		return TRUE;
	}
	return FALSE;
}
BOOL CWMReader::controlStop()
{
	assert(m_pReader);
	HRESULT hr = m_pReader->Stop();
	if (SUCCEEDED(hr))
	{
		m_eState = RS_Stop;
		return TRUE;
	}
	return FALSE;
}
BOOL CWMReader::controlClose()
{
	assert(m_pReader);
	HRESULT hr = m_pReader->Close();
	if (SUCCEEDED(hr))
	{
		WaitForSingleObject(m_hSyncEv,INFINITE);
		m_eState = RS_Stop;
		return SUCCEEDED(m_hrSync);
	}
	else
	{
		return FALSE;
	}
}

void CWMReader::setSyncEvent(HRESULT hr)
{
	m_hrSync = hr;
	SetEvent(m_hSyncEv);
}

char* CWMReader::unicodeToAnsi(wchar_t* wszString)
{
	int ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
	char* szAnsi = new char[ansiLen + 1];
	::WideCharToMultiByte(CP_ACP, NULL, wszString, wcslen(wszString), szAnsi, ansiLen, NULL, NULL);
	szAnsi[ansiLen] = '\0';
	return szAnsi;
}

