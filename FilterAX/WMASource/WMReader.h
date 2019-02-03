#pragma once
#include <wmsdk.h>
#include <stdio.h>
#include <string>
#include <streams.h>

#define SAMPLEBUFFERLEN 3
#define SAMPLEBUFFERSIZE (100*1024)

typedef enum _eSampleFlags
{
	SF_NOFLAG,//普通帧
	SF_KEYFRAME,//关键帧
	SF_DISCONTINUITY,//有断点，需要Seek（a network loss, or other reason）
	SF_DATALOSS//跟前一个Sample之间有数据丢失
}eSampleFlags;

typedef enum _eReaderState
{
	RS_Running,
	RS_Pause,
	RS_Stop
}eReaderState;

typedef struct _SampleBuffer
{
	BYTE*						pData;//Sample 数据
	DWORD					cbData;//数据长度
	REFERENCE_TIME	start_tm;//Sample start tm
	REFERENCE_TIME	end_tm;//Sample end tm
	eSampleFlags			flags;//标志
}SampleBuffer;

class CWMReader : public IWMReaderCallback/*,public IWMReaderCallbackAdvanced*/
{
public:
	CWMReader(std::wstring file,HRESULT* phr,int nStreamNum);
	virtual ~CWMReader(void);

	void releaseReader();
	//IUnknown
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject);
	//IWMReaderCallback
	virtual STDMETHODIMP OnSample( DWORD dwOutputNum,QWORD cnsSampleTime,QWORD cnsSampleDuration
		,DWORD dwFlags,INSSBuffer *pSample,void *pvContext);
	//IWMStatusCallback
	virtual STDMETHODIMP OnStatus( WMT_STATUS Status, HRESULT hr, WMT_ATTR_DATATYPE dwType,BYTE *pValue,void *pvContext);

	//control 
	BOOL 		controlOpen(std::wstring file = L"",BOOL bAudio = TRUE);
	BOOL 		controlStart(QWORD nStartTime=WM_START_CURRENTPOSITION,float fRate=1.0f);
	BOOL 		controlPause();
	BOOL 		controlResume();
	BOOL 		controlStop();
	BOOL 		controlClose();

	HRESULT					selectStream(int nStreamNumber);
	BOOL 						isSeekble()const {return m_bSeekable;}
	BOOL 						isStridable()const {return m_bStridable;}
	BOOL 						isEOF()const {return m_bEOF;}
	BOOL 						isBroadcast()const {return m_bBroadcast;}
	BOOL						isMultiStream()const{return m_bMultStream;}
	QWORD 					getFileDuration()const {return m_nDuration;}
	DWORD					getBitRate()const {return m_uBitRate;}
	bool							getCurrentSample(SampleBuffer& sb);
	WM_MEDIA_TYPE*	getMT(){return m_pMT;}
	void							resetBufferUse(){m_uUsedBuffer=0;m_uInCurrentIndex=0;m_uOutCurrentIndex=0;}
	eReaderState			getReaderState(){return m_eState;}
protected:
	void			setSyncEvent(HRESULT hr);
	HRESULT	setAttribute(LPCWSTR pwszName,WMT_ATTR_DATATYPE eType, BYTE ** ppbValue,WORD cbLength);
	HRESULT	getHeaderAttribute(LPCWSTR pwszName, BYTE** ppbValue,WORD wStreamNum = 0);
	HRESULT	retrieveAttributes(void);
	// To get the connection bandwidth before receiving a sample
	HRESULT	detectBandwidth(DWORD& bandwidth);
	HRESULT	getAVOutput(BOOL bAudio = TRUE);
	char*		unicodeToAnsi(wchar_t* wszString);
	
private:
	void			initSampleBuffer();
	void			uninitSampleBuffer();
private:
	LONG								m_lRef;
	
	eReaderState					m_eState;
	WORD								m_uOutCurrentIndex;
	WORD								m_uInCurrentIndex;
	LONG								m_uUsedBuffer;
	SampleBuffer					m_sSampleBuffer[SAMPLEBUFFERLEN];
	int									m_StreamNum;
	DWORD							m_uAVNumOut;
	DWORD							m_uTotalStream;

	IWMReader*					m_pReader;
	IWMReaderAdvanced*	m_pReaderAdvanced;      // 提供了选择音轨的接口
	IWMHeaderInfo *			m_pHeaderInfo;
	
	BOOL								m_bSeekable;   
	BOOL								m_bStridable; //  if file can fast forward and rewind the content
	BOOL								m_bEOF;        // TRUE if EOF is reached or we need to stop
	BOOL								m_bMultStream;
	BOOL								m_bBroadcast;
	QWORD							m_nDuration;
	DWORD							m_uBitRate;
	QWORD							m_nTimeElapsed;

	std::string							m_strLanguage;
	int									m_nLanguageLen;

	std::wstring						m_strFile;

	HRESULT							m_hrSync;
	HANDLE							m_hSyncEv;

	WM_MEDIA_TYPE*			m_pMT;

};
