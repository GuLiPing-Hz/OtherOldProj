#include "stdafx.h"
#include "staopin.h"
#include "Stream.h"
#include <process.h>

CSTAOutputpin::CSTAOutputpin(HRESULT *phr, CSource* pFilter, LPCWSTR pPinName,WORD nAudioStreamIndex):
CSourceStream(NAME("STASorce output pin"),phr,pFilter,pPinName),m_lpMediaType(NULL),
CSourceSeeking(NAME("MediaSeeking"), NULL, phr, pFilter->pStateLock()){
	if(!m_wmreader.Initialize(fastdelegate::MakeDelegate(this,&CSTAOutputpin::MediaEof))){
		*phr = E_INVALIDARG;
	}

	CSourceSeeking::m_rtStart    = 0;
	CSourceSeeking::m_rtStop     = 1 * UNITS;
	CSourceSeeking::m_rtDuration = 1 * UNITS;
	m_nAudioStreamIndex	=nAudioStreamIndex;
	m_hEvent					 = NULL;
	m_bQuit						 = false;
//	m_hEvent = CreateEvent(NULL,FALSE, FALSE, NULL);
//	HANDLE hTheard = ( HANDLE )_beginthreadex(NULL, 0, ( unsigned int ( __stdcall * )( void * ) )WmCloseThreadFunc, ( LPVOID )this, 0, 0 );
//	if ( hTheard != NULL )
//	 {
//	 	CloseHandle( hTheard );
//	 	hTheard = NULL;
//	 }
}

CSTAOutputpin::~CSTAOutputpin()
{
	m_global.GlobalSetEvent();
	Stop();
	WMClose();

	//为了关闭WmClose,迫不得已，出此下策，线程控制释放，如果没有返回，直接杀掉线程 add by chy
	//////////////////////////////////////////////////////////////////////////
	// 	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	// 	 HANDLE hTheard = ( HANDLE )_beginthreadex(NULL, 0, ( unsigned int ( __stdcall * )( void * ) )WmCloseThreadFunc, ( LPVOID )this, 0, 0 );
	// 	if ( WAIT_TIMEOUT == WaitForSingleObject(m_hEvent, 2000) )
	// 	{
	// 		OutputDebugString(TEXT(" --------------------------- CSTAOutputpin wmclose timeout,theard exit! \n"));
	// 		// 如果线程超时，强制关闭线程
	// 		DWORD dwExitCode = 0;
	// 		::GetExitCodeThread( hTheard, &dwExitCode ); 	
	// 		TerminateThread( hTheard, dwExitCode );
	// 	}
	m_bQuit= true;
//	SetEvent( m_hEvent );
//	WaitForSingleObject(m_hEvent, 20000);
	if ( m_hEvent != NULL )
	{
		if(m_hEvent != NULL)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
	}
	// 	if ( hTheard != NULL )
	// 	{
	// 		CloseHandle( hTheard );
	// 		hTheard = NULL;
	// 	}
	//////////////////////////////////////////////////////////////////////////

	SafeComMemFree(m_lpMediaType);
	m_thread.Clear();
}

DWORD WINAPI CSTAOutputpin::WmCloseThreadFunc( LPVOID pParam )
{
	CSTAOutputpin* pThis = (CSTAOutputpin*)pParam;
	if ( pThis == NULL )
		return 0;
	while( !pThis->m_bQuit )
	{
		Command cmd;
		pThis->CheckRequest(&cmd);
		if ( cmd == CMD_PAUSE )
		{
			Sleep(100);
		}
		Sleep(50);
	}
	ResetEvent( pThis->m_hEvent );
	return 0;
}

HRESULT STDMETHODCALLTYPE CSTAOutputpin::QueryInterface(REFIID riid, void **ppv){
	return CSourceStream::QueryInterface(riid,ppv);
}                           

ULONG STDMETHODCALLTYPE CSTAOutputpin::AddRef(){
	return CSourceStream::AddRef();
}

ULONG STDMETHODCALLTYPE CSTAOutputpin::Release(){
	return CSourceStream::Release();
}

STDMETHODIMP CSTAOutputpin::NonDelegatingQueryInterface(REFIID riid,void** ppv){
	CheckPointer(ppv,E_POINTER);
	if(riid == IID_IMediaSeeking){
		//return CSourceSeeking::NonDelegatingQueryInterface(riid,ppv);
		return GetInterface((IMediaSeeking*)this,ppv);
	}else{
		return CSourceStream::NonDelegatingQueryInterface(riid,ppv);
	}
}

HRESULT CSTAOutputpin::Parsethefile(LPCTSTR pszFileName){
	if(IsNull(pszFileName)) return S_FALSE;
	m_file._cfile = new Burning::CFile();
	if(m_file._cfile->Open(pszFileName,FOP_READ) == FILE_CANTOPEN) return S_FALSE;
	if(m_file._cfile->GetSize() < (g_head + g_last)) return S_FALSE;
	byte buffer[66];

	/** 
	以下数据注意:wchar_t 与 char 之间的位数差异必须考虑.编译为 char 版本的 stasource 会造成数据读取或者判断错误.必须严格检查.
	by lwj 2011-06-14 11:34
	*/
	ZeroMemory(buffer,66);
	m_file._cfile->Read(buffer,8);		///< 获取加密标识
	if(lstrcmpi((TCHAR*)buffer,TEXT("9158")) != 0){
		return S_FALSE;
	}


	m_file._cfile->Skin(m_file._cfile->GetSizeEx() - 16);
	ZeroMemory(buffer,66);
	m_file._cfile->Read(buffer,16);
	if(lstrlen((TCHAR*)buffer) > 0){
		m_file._cfile->Skin(16);
		m_file._version = TEXT("1.0.0");	///< 该文件为 1.0 版本编码
		m_file._suffix = (TCHAR*)buffer;
		m_file._complete._size = m_file._cfile->GetSizeEx() - 32;
		m_file._complete._complete = true;	///< 1.0 版本编码文件不允许边下载边播放

#if(OUTPUTPLAYERDEBUG)
		CFile temp;
		temp.Open(TEXT("c:\\temp.wma"),FOP_WRITE);
		byte* buffer2 = NULL;
		unsigned int size = m_file._cfile->GetSizeEx() - 32 - 32;
		buffer2 = new byte[size];
		m_file._cfile->Read(buffer2,size,32);
		m_file._cfile->Skin(32);
		temp.Write(buffer2,size);	///< 读取指针被延后,需要回调
		SafeDeleteArray(buffer2);
#endif
	}else{
		m_file._cfile->Skin(8);
		ZeroMemory(buffer,66);
		m_file._cfile->Read(buffer,16);		///< 获取加密版本号
		m_file._version = (TCHAR*)buffer;

		ZeroMemory(buffer,66);
		m_file._cfile->Read(buffer,16);		///< 读取文件后缀 
		m_file._suffix = (TCHAR*)buffer;

		ZeroMemory(buffer,66);
		m_file._cfile->Read(buffer,64);		///< 获取文件长度
		m_file._complete._size = ::_ttoi64((TCHAR*)buffer);

		LONGLONG size = m_file._cfile->GetSize();
		m_file._complete._complete = ((size - g_head - g_last) >= m_file._complete._size);

		//#if(OUTPUTPLAYERDEBUG)
		//		CFile temp;
		//		temp.Open(TEXT("c:\\temp.wma"),FOP_WRITE);
		//		byte* buffer2 = NULL;
		//		unsigned int size2 = m_file._cfile->GetSizeEx() - g_head - g_last;
		//		buffer2 = new byte[size2];
		//		m_file._cfile->Read(buffer2,size2,g_head);
		//		m_file._cfile->Skin(g_head);
		//		temp.Write(buffer2,size2);
		//		SafeDeleteArray(buffer2);
		//#endif
	}

	return S_OK;
}

HRESULT CSTAOutputpin::Load(LPCOLESTR pszFileName)
{
	if(IsNull(pszFileName) || !m_wmreader.IsOk()) 
		return E_INVALIDARG;
	CAutoLock lock(&m_SharedState);
	m_file.clear();
	HRESULT hr = S_OK;
	m_file._file = pszFileName;
	hr = Parsethefile(pszFileName);
	m_file._ista = (hr == S_OK);		///< 检测是否是 STA文件
	WMClose();							///< 关闭之前的文件,未完成....

	if(hr == S_OK)
	{
		OutputDebugStr(TEXT("CSTAOutputpin::Load mine data stream.\n"));

		m_file._stream = new CStream(m_file._complete._size);
		m_file._stream->AddRef();

		byte* buffer = (byte*)CoTaskMemAlloc(MAX_PATH);
		if(buffer == NULL)
		{
			OutputDebugStr(TEXT("Load:Buffer create failed!\n"));
			return E_OUTOFMEMORY;
		}

		ULONGLONG nRead = 0,nNow = 0,nLoadsize = 0,canread = 0;

		if(!m_file._complete._complete)
		{
			m_file._complete._readed = m_file._cfile->GetSize() - g_head;
			nLoadsize = m_file._complete._readed;

			m_thread.Clear();
			unsigned int dwID;
			m_thread._thread = (HANDLE)_beginthreadex(NULL,0,ThreadProc1,this,0,&dwID);

			if(m_thread._thread == NULL)
			{
				return S_FALSE;
			}
		}
		else
		{
			nLoadsize = m_file._complete._size;
		}	

		while(nRead < nLoadsize)
		{
			canread = nLoadsize - nRead;
			canread = min(canread,MAX_PATH);
			nNow = m_file._cfile->Read(buffer,canread);
			if(nNow < 0)
			{
				OutputDebugStr(TEXT("Load:Read file failed!\n"));
				return S_FALSE;
			}
			m_file._stream->WriteEx(buffer,nNow,NULL);
			nRead += nNow;
		}

		m_file._stream->SeekToBegin();
		SafeComMemFree(buffer);

		if(hr != S_OK)
		{
			OutputDebugStr(TEXT("CreateStreamOnHGlobal failed!\n"));
			return hr;
		}

		hr = m_wmreader.Open(m_file._stream);
	}
	else
	{
		OutputDebugStr(TEXT("CSTAOutputpin::Load IWMReader.\n"));
		hr = m_wmreader.Open(pszFileName);
		if (hr!=S_OK)
		{
			OutputDebugStr(TEXT("m_wmreader.Open error.\n"));
		}
	}

	if(hr == S_OK)
	{
		hr = FillHeader();
		if (hr!=S_OK)
		{
			OutputDebugStr(TEXT("m_wmreader.FillHeader error.\n"));
		}
	}
#ifdef _TRY_SAMPLE_OPTIMIZATION_
	IWMReaderAdvanced * pAdvanced = NULL;
	m_wmreader._lpReader->QueryInterface(IID_IWMReaderAdvanced, (void **) &pAdvanced);
	if ( pAdvanced != NULL && m_wmreader._lpReaderCB != NULL )
	{
		DWORD maxSize = 0;
		pAdvanced->GetMaxOutputSampleSize(m_wmreader._lpReaderCB->GetOutputNumber(), &maxSize);
		pAdvanced->SetAllocateForOutput(m_wmreader._lpReaderCB->GetOutputNumber(), TRUE);
		pAdvanced->Release();

		// Init the sample list
		// Attention: 
		// When playing video, more samples should provided (suggest: 12).
		// But while playing audio, you can use less samples (suggest: 5).
		m_wmreader._lpReaderCB->InitSampleAdmin( 5, maxSize );	
	}

#endif 
	return hr;
}

void CSTAOutputpin::Execute(void)
{
	ULONGLONG sizenow = 0,nRead = 0,nNow = 0,nLoadsize = 0;
	byte buffer[MAX_PATH];
	while(!m_thread._quit && !m_file._complete._complete)
	{
		sizenow = m_file._cfile->GetSizeEx();
		if(sizenow == (m_file._complete._size + g_head + g_last))
		{
			sizenow -= g_last;
		}
		sizenow -= g_head;
		if(sizenow > m_file._complete._readed)
		{
			ZeroMemory(buffer,sizeof(buffer));
			nRead = 0;
			nLoadsize = sizenow - m_file._complete._readed;

#if(OUTPUTPLAYERDEBUG)
			_stprintf(g_buffer,TEXT("有数据进入,增添: %d 字节,当前已有数据: %d \n"),nLoadsize,m_file._complete._readed);
			OutputDebugStr(g_buffer);
#endif

			while(nRead < nLoadsize)
			{
				nNow = m_file._cfile->Read(buffer,MAX_PATH);
				if(nNow < 0)
				{
					OutputDebugStr(TEXT("Load:Read file failed!\n"));
					break;
				}
				m_file._stream->WriteEx(buffer,nNow,NULL);
				nRead += nNow;
			}

			m_file._complete._readed += nLoadsize;
			m_file._complete._complete = (m_file._complete._size <= m_file._complete._readed);
		}
		else
		{
			SleepEx(100,TRUE);
		}
	}

#if(OUTPUTPLAYERDEBUG)
	_stprintf(g_buffer,TEXT("歌曲加载完成. \n"));
	OutputDebugStr(g_buffer);
#endif
}

HRESULT CSTAOutputpin::RetrieveAttributes()
{
	HRESULT hr = S_OK;
	IWMHeaderInfo3* lpInfo = NULL;		
	hr = m_wmreader._lpReader->QueryInterface(IID_IWMHeaderInfo3,(void**)&lpInfo);
	if(hr != S_OK){
		OutputDebugStr(TEXT("QueryInterface wmheaderinfo3 failed!\n"));
		return hr;
	}

	do
	{
		WORD  streamnum = 0;
		WMT_ATTR_DATATYPE pType;
		BYTE pbAttribValue[MAX_PATH];
		WORD dwAttribValueLen = MAX_PATH;
		dwAttribValueLen = MAX_PATH;
		pbAttribValue[0] = 0;

		hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMDuration,&pType,pbAttribValue,&dwAttribValueLen);
		BREAK_IF_ISNOTSOK(hr);
		m_file._duration = *((QWORD*)pbAttribValue);

		hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMSeekable,&pType,pbAttribValue,&dwAttribValueLen);
		BREAK_IF_ISNOTSOK(hr);
		m_file._canseek = (*((BOOL*)pbAttribValue) == TRUE);

		hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMBroadcast,&pType,pbAttribValue,&dwAttribValueLen);
		BREAK_IF_ISNOTSOK(hr);
		m_file._canbroadcast = (*((BOOL*)pbAttribValue) == TRUE);

		hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMHasAudio,&pType,pbAttribValue,&dwAttribValueLen);
		BREAK_IF_ISNOTSOK(hr);
		m_file._asftype._audio = (*((BOOL*) pbAttribValue))==TRUE?true:false;

		hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMHasImage,&pType,pbAttribValue,&dwAttribValueLen);
		BREAK_IF_ISNOTSOK(hr);
		m_file._asftype._image = (*((BOOL*) pbAttribValue))==TRUE?true:false;

		hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMHasScript,&pType,pbAttribValue,&dwAttribValueLen);
		BREAK_IF_ISNOTSOK(hr);
		m_file._asftype._script = (*((BOOL*) pbAttribValue))==TRUE?true:false;

		hr = lpInfo->GetAttributeByName(&streamnum,g_wszWMHasVideo,&pType,pbAttribValue,&dwAttribValueLen);
		BREAK_IF_ISNOTSOK(hr);
		m_file._asftype._video = (*((BOOL*) pbAttribValue))==TRUE?true:false;

		SafeRelease(lpInfo);
	}while(false);

	return hr;
}

HRESULT CSTAOutputpin::GetAVOutput()
{
	HRESULT	hr				= S_OK;
	ULONG	cbType			= 0;
	GUID	requiredGuid	= m_file._asftype._video?WMMEDIATYPE_Video:WMMEDIATYPE_Audio;
	do
	{
		DWORD   outputs = 0;
		hr = m_wmreader._lpReader->GetOutputCount(&outputs);  ///< 媒体流的数量
		BREAK_IF_ISNOTSOK(hr);

		DWORD i = 0;
		for (i = 0; i < outputs; i++){
			IWMOutputMediaProps* pProps = NULL;
			hr = m_wmreader._lpReader->GetOutputProps(i,&pProps);
			BREAK_IF_ISNOTSOK(hr);

			hr = pProps->GetMediaType(NULL, &cbType);
			BREAK_IF_ISNOTSOK(hr);

			SafeComMemFree(m_lpMediaType);
			m_lpMediaType = (AM_MEDIA_TYPE*)CoTaskMemAlloc(cbType);
			if(NULL == m_lpMediaType){
				OutputDebugStr(TEXT("WMReader:CoTaskMemAlloc failed!\n"));
				hr = HRESULT_FROM_WIN32(GetLastError()) ;
				break;
			}

			hr = pProps->GetMediaType((WM_MEDIA_TYPE*)m_lpMediaType, &cbType);
			SafeRelease(pProps);
			BREAK_IF_ISNOTSOK(hr);

			if (requiredGuid == m_lpMediaType->majortype)
			{
				break;
			}
		}

		BREAK_IF_ISNOTSOK(hr);
		if(i == outputs){
			hr = E_UNEXPECTED;
			break;
		}

		m_wmreader._lpReaderCB->SetOutputNumber(i);
		BREAK_IF_ISNOTSOK(InitBuffer());
	}while(false);
	return hr;
}

HRESULT CSTAOutputpin::InitBuffer()
{
	CheckPointer(m_lpMediaType,E_INVALIDARG);
	HRESULT hr = S_OK;

	if(m_lpMediaType->majortype == WMMEDIATYPE_Video){				///< 视屏数据
		hr = E_NOTIMPL;
	}else if(m_lpMediaType->majortype == WMMEDIATYPE_Audio){		///< 银屏数据
		if(m_lpMediaType->formattype == WMFORMAT_WaveFormatEx){
			WAVEFORMATEX* lpWfx = (WAVEFORMATEX*)m_lpMediaType->pbFormat;
			DWORD size = (lpWfx->nSamplesPerSec * (lpWfx->wBitsPerSample / 8) * lpWfx->nChannels * SAMPLEDATABLOCKSUM);	///< 设置缓冲区为 2秒的数据缓冲
			if(!m_wmreader._ringex.Initialize(size)){
				hr = E_FAIL;
			}
		}else{
			hr = E_NOTIMPL;
		}
	}

	return hr;
}

HRESULT CSTAOutputpin::FillHeader()
{
	m_wmreader.WMReaderWaitEvent();

	HRESULT hr = S_OK;
	do 
	{
		hr = RetrieveAttributes();
		BREAK_IF_ISNOTSOK(hr);
		hr = GetAVOutput();
		BREAK_IF_ISNOTSOK(hr);
		IWMProfile* lpProfile = NULL;
		hr = m_wmreader._lpReader->QueryInterface(IID_IWMProfile, (void **) &lpProfile);
		BREAK_IF_ISNOTSOK(hr);

		DWORD dwStreamCount = 0;
		hr = lpProfile->GetStreamCount(&dwStreamCount);//得到流个数
		BREAK_IF_ISNOTSOK(hr);
		TCHAR buffer[MAX_PATH];
		WORD  size = MAX_PATH;

		for(DWORD dwIndex = 0; dwIndex < dwStreamCount; dwIndex++)
		{
			IWMStreamConfig*  lpConfig  = NULL;
			IWMStreamConfig3* lpConfig3 = NULL;
			hr = lpProfile->GetStream(dwIndex, &lpConfig);
			BREAK_IF_ISNOTSOK(hr);
			hr = lpConfig->QueryInterface(IID_IWMStreamConfig3, (void **)&lpConfig3);
			BREAK_IF_ISNOTSOK(hr);
			// Determine the stream type
			GUID guid = GUID_NULL;
			hr = lpConfig->GetStreamType(&guid);
			BREAK_IF_ISNOTSOK(hr);
			if( WMMEDIATYPE_Audio == guid )
			{
				ZeroMemory(buffer,sizeof(buffer));
				lpConfig3->GetLanguage(buffer,&size);
				m_file._language += buffer;
				m_file._language += TEXT(";");
			}
			SafeRelease(lpConfig3);
			SafeRelease(lpConfig);
		}

		SafeRelease(lpProfile);

		if( dwStreamCount > 1 )
		{
			m_file._isMultiStream = true;
			///< 音轨切换
			hr = SetStream( m_nAudioStreamIndex );
			if(hr != S_OK)
			{
				OutputDebugStr(TEXT("FillHeader infomation failed!\n"));
				WMClose();
			}
		}
		else if ( m_nAudioStreamIndex > 1 )
		{
			OutputDebugStr(TEXT("pin2:dwStreamCount <=1 \n"));
			m_nAudioStreamIndex = 1;
			///< 音轨切换
			hr= SetStream(m_nAudioStreamIndex);
			if(hr != S_OK)
			{
				OutputDebugStr(TEXT("FillHeader infomation failed!\n"));
				WMClose();
			}
		}		

	}while (false);

	if(hr != S_OK)
	{
		OutputDebugStr(TEXT("FillHeader infomation failed!\n"));
		WMClose();
	}

	QWORD qFileDuration = m_file._duration;
	CRefTime FileDuration;
	FileDuration.m_time = qFileDuration;
	m_rtStop = m_rtDuration = FileDuration;

	if(!m_file._canseek){
		m_dwSeekingCaps = m_dwSeekingCaps&(~AM_SEEKING_CanSeekAbsolute);
	}

	if(m_lpMediaType != NULL){
		WAVEFORMATEX* lpWfx = (WAVEFORMATEX*)m_lpMediaType->pbFormat;
		m_file._datalensd = lpWfx->nSamplesPerSec * (lpWfx->wBitsPerSample / 8) * lpWfx->nChannels;
		m_file._dataAlign = lpWfx->nBlockAlign;
		m_file._init = true;
	}
	//WMStart();

	return hr;
}

void CSTAOutputpin::MediaEof(HRESULT hr){
	m_file._iseof = true;
}

HRESULT	CSTAOutputpin::WMStart()
{
	if(!IsOk()) return E_UNEXPECTED;
	m_file._iseof = (m_rtStart >= m_rtStop);
	if(m_file._iseof) return S_OK;

	m_file.StreamClear();
	m_wmreader.Clear();

	m_file._qwTimeElapsed = 0x0123456789;
	return m_wmreader._lpReader->Start(m_rtStart,0,1.0,NULL);
}

HRESULT	CSTAOutputpin::WMPause()
{
	if(!IsOk()) 
		return E_UNEXPECTED;
	Pause();
	return m_wmreader._lpReader->Pause();
}

HRESULT	CSTAOutputpin::WMResume(){
	if(!IsOk()) return E_UNEXPECTED;
	return m_wmreader._lpReader->Resume();
}
HRESULT	CSTAOutputpin::WMStop(){
	if(!IsOk()) return E_UNEXPECTED;
	m_file.StreamClose();
	m_wmreader.AllStop();
	HRESULT hr = m_wmreader._lpReader->Stop();
	if(hr == S_OK){
		m_wmreader.WMReaderWaitEvent();
	}
	return hr;
}

HRESULT	CSTAOutputpin::WMClose()
{
	if(!IsOk()) 
		return E_UNEXPECTED;

	m_file.StreamClose();

	m_wmreader.AllStop();

	OutputDebugStr(TEXT("CSTAOutputpin::WMClose begin!\n"));
	HRESULT hr = m_wmreader._lpReader->Close();
	OutputDebugStr(TEXT("CSTAOutputpin::WMClose End!\n"));

	if( hr == S_OK )
	{
		m_wmreader.WMReaderWaitEvent();
	}
#ifdef _TRY_SAMPLE_OPTIMIZATION_
	if ( m_wmreader._lpReaderCB != NULL )
	{
		m_wmreader._lpReaderCB->UninitSampleAdmin();
	}
#endif 
	return hr;
}

HRESULT CSTAOutputpin::Active(void)
{
	m_wmreader._ringex.Reset();
	HRESULT hr = WMStart();
	if (FAILED(hr))
	{
		return hr;
	}
	m_wmreader._ringex.RingWaitFillEvent(m_file._datalensd * 2,500);
	m_async.Passage();
	return CSourceStream::Active();
}

HRESULT CSTAOutputpin::Inactive()
{
	m_wmreader._ringex.Close();
	HRESULT hr = WMStop();
	if (FAILED(hr))
	{
		return hr;
	}
	m_async.clear();
	m_async._start = 0;
	return CSourceStream::Inactive();
}

HRESULT CSTAOutputpin::GetCurFile(LPOLESTR *ppszFileName,AM_MEDIA_TYPE *pmt){
	DWORD temp = sizeof(TCHAR) * (1 + m_file._file.size());
	*ppszFileName = (LPOLESTR) CoTaskMemAlloc(temp);
	if(ppszFileName == NULL ) return E_OUTOFMEMORY;
	CopyMemory(*ppszFileName,m_file._file.c_str(),temp);
	return S_OK;
}

HRESULT CSTAOutputpin::GetMediaType(int iPosition, CMediaType *pmt){
	CheckPointer(pmt,E_POINTER);
	CheckPointer(m_lpMediaType,E_INVALIDARG);
	if(iPosition < 0 || !IsOk()) return E_INVALIDARG;

	CAutoLock lock(m_pFilter->pStateLock());
	HRESULT hr = pmt->Set(*m_lpMediaType);
	if(hr != S_OK){
		OutputDebugStr(TEXT("CMineOutPutPin:CMediatype set mediatype failed! \n"));
		return hr;
	}

	if(iPosition > 4){
		return VFW_S_NO_MORE_ITEMS;
	}

	return NOERROR;
}

HRESULT CSTAOutputpin::SetMediaType(const CMediaType *pMediaType){
	CheckPointer(pMediaType,E_POINTER);
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	return CSourceStream::SetMediaType(pMediaType);
}

HRESULT CSTAOutputpin::GetLanguageLen(DWORD& dwlen){
	dwlen = m_file._language.size();
	return S_OK;
}

HRESULT CSTAOutputpin::GetLanguage(LPWSTR language)
{
	if(language == NULL) return E_FAIL;
	lstrcpy(language,m_file._language.c_str());
	return S_OK;
}

HRESULT CSTAOutputpin::IsMultiStream(bool& multistream)
{
	multistream = m_file._isMultiStream;
	return S_OK;
}

HRESULT CSTAOutputpin::SetStream(DWORD number)
{
	WMT_STREAM_SELECTION wmtSS = WMT_OFF;
	IWMReaderAdvanced * lpReaderAdvanced = NULL;
	HRESULT hr = m_wmreader._lpReader->QueryInterface( IID_IWMReaderAdvanced,(void**)&lpReaderAdvanced );
	if(hr != S_OK)
	{
		OutputDebugStr(TEXT("QueryInterface IID_IWMReaderAdvanced failed!\n"));
		return hr;
	}
	lpReaderAdvanced->SetManualStreamSelection(TRUE);
	WORD nAudioStreamNum = number;
	hr = lpReaderAdvanced->SetStreamsSelected(1,&nAudioStreamNum,&wmtSS);
	if(hr == E_INVALIDARG)
	{
		OutputDebugStr(TEXT("nAudioStreamNum is null!\n"));
	}
	if(hr == NS_E_INVALID_REQUEST)
	{
		OutputDebugStr(TEXT("No file is loaded in the synchronous reader\n"));
	}
	if(hr == E_OUTOFMEMORY)
	{
		OutputDebugStr(TEXT("The method is unable to allocate memory for an internal object\n"));
	}
	SafeRelease(lpReaderAdvanced);
	return hr;
}

HRESULT CSTAOutputpin::SetDelayTime(DWORD time)
{
	m_global._delay = time;
	return S_OK;
}

HRESULT CSTAOutputpin::FillBuffer(IMediaSample * pSample)
{
	CheckPointer(pSample,E_POINTER);
	if(!IsOk() || m_file._iseof || m_async._start + m_rtStart >= m_rtStop) 
		return S_FALSE;

	DWORD passage = m_async.Passage();	///< 两次调用的时间差.

#if(OUTPUTPLAYERDEBUG)
	_stprintf(g_buffer,TEXT("FillBuffer::TimeStamp:%u \n"),passage);
	OutputDebugStr(g_buffer);
#endif

	if(passage + 10 < m_global._delay)
	{
		passage = m_global._delay - passage - 10;
		m_global.GlobalWaitEvent(passage);
	}

	CAutoLock   lock(&m_SharedState);
	BYTE *pData;
	long lDataLen;
	pSample->GetPointer(&pData);	///< 得到Sample的地址指针
	lDataLen = pSample->GetSize();	///< 得到Sample的大小
	ZeroMemory(pData,lDataLen);

	if(m_wmreader._ringex.Length() > 0)
	{
		float datalen = m_wmreader._ringex.Read(pData,lDataLen);
		m_async._lastDuration = datalen / m_file._datalensd * 10000000;
		CRefTime rtStart,rtSampleTime;
		rtStart = m_async._start;
		rtSampleTime = m_async._start + m_async._lastDuration;
		m_async._start = rtSampleTime;
		pSample->SetTime((REFERENCE_TIME *)&rtStart,(REFERENCE_TIME*)&rtSampleTime);
		pSample->SetSyncPoint(TRUE);

#if(OUTPUTPLAYERDEBUG)
		_stprintf(g_buffer,TEXT("FillBuffer: Start:%u End:%u Duration:%f \n"),(long)rtStart,(long)rtSampleTime,(float)m_async._lastDuration/10000000);
		OutputDebugStr(g_buffer);
#endif
	}

	return NOERROR;
}

HRESULT	CSTAOutputpin::CheckMediaType(const CMediaType* inMediatype)
{
	CheckPointer(inMediatype,E_POINTER);
	CheckPointer(m_lpMediaType,E_INVALIDARG);
	CAutoLock lock(m_pFilter->pStateLock());

	if(inMediatype->majortype != m_lpMediaType->majortype || !inMediatype->IsFixedSize()) return S_FALSE;
	if(inMediatype->subtype == GUID_NULL || inMediatype->subtype != m_lpMediaType->subtype) return S_FALSE;
	return S_OK;
}

HRESULT CSTAOutputpin::DecideBufferSize(IMemAllocator * pAlloc,ALLOCATOR_PROPERTIES * ppropInputRequest)
{
	CheckPointer(pAlloc,E_POINTER);
	CheckPointer(ppropInputRequest,E_POINTER);
	CheckPointer(m_lpMediaType,E_INVALIDARG);
	CAutoLock lock(m_pFilter->pStateLock());

	HRESULT hr = NOERROR;
	ppropInputRequest->cBuffers = 1;
	DWORD size = (float)(m_file._datalensd * m_global._delay) / 1000;
	size = size - size % m_file._dataAlign;
	ppropInputRequest->cbBuffer = size;

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(ppropInputRequest,&Actual);
	if(hr != S_OK)
	{
		OutputDebugStr(TEXT("CSTAOutputpin:SetProperties failed! \n"));
		return hr;
	}

	if(Actual.cbBuffer < ppropInputRequest->cbBuffer){
		OutputDebugStr(TEXT("DecideBufferSize Failed2!"));
		return E_FAIL;
	}

	return hr;
}
bool CSTAOutputpin::IsOk()
{
	return (m_wmreader.IsOk() && m_file._init);
}

void CSTAOutputpin::UpdateFromSeek(void)
{
	if(ThreadExists())
	{
		DeliverBeginFlush();
		Stop();
		DeliverEndFlush();
		Run();
	}
}

HRESULT CSTAOutputpin::ChangeStart()
{
	if(m_rtStart > m_rtStop) return S_FALSE;

	if(m_async._changestart != m_rtStart)
	{
		m_async._changestart = m_rtStart;
		m_file._changestart = true;
		UpdateFromSeek();
	}
	//if(m_rtStart < m_file._duration && m_async._changestart != m_rtStart){
	//	OutputDebugStr(TEXT("====================================== Changestart =================================\n"));
	//	m_async._changestart = m_rtStart;
	//	m_wmreader._ringex.Close();
	//	WMStop();
	//	m_wmreader._ringex.Reset();
	//	m_async._start = 0;
	//	WMStart();
	//	UpdateFromSeek();
	//	m_wmreader._ringex.RingWaitFillEvent(m_file._datalensd * 2,500);
	//	m_async.Passage();

	//	/*static DWORD time = GetTickCount();
	//	m_wmreader._ringex.RingWaitFillEvent(m_file._datalensd,500);
	//	time = GetTickCount() - time;*/
	//}

	return NOERROR;
}

HRESULT CSTAOutputpin::ChangeStop(){
	return NOERROR;
}

HRESULT CSTAOutputpin::ChangeRate(){
	return NOERROR;
}

HRESULT CSTAOutputpin::OnThreadCreate(void){
	CAutoLock lock(&m_SharedState);
	return S_OK;
}

HRESULT CSTAOutputpin::OnThreadStartPlay(void){
	return DeliverNewSegment(m_rtStart, m_rtStop, m_dRateSeeking);
}

HRESULT CSTAOutputpin::Run()
{
	if(m_file._changestart)
	{
		m_file._changestart = false;
		m_wmreader._ringex.Close();
		WMStop();
		m_wmreader._ringex.Reset();
		m_async._start = 0;
		if(WMStart() == S_OK)
		{
			if(m_rtStart < m_rtStop) m_wmreader._ringex.RingWaitFillEvent((float)m_file._datalensd * 2.5,300);
			m_async.Passage();
		}
		return CSourceStream::Run();
	}
	return 0;
}
HRESULT CSTAOutputpin::DoBufferProcessingLoop(void)
{
	return CSourceStream::DoBufferProcessingLoop();
}
//重载CourceStream类函数
DWORD CSTAOutputpin::ThreadProc(void)
{
	return CSourceStream::ThreadProc();
}