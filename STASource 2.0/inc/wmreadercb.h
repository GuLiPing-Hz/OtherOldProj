#pragma once
#include "ringex.h"
#include "FastDelegate.h"

//#define _TRY_SAMPLE_OPTIMIZATION_

#ifdef _TRY_SAMPLE_OPTIMIZATION_
#include "CSampleAdmin.h"
#endif 
/**
	@file wmreadercb.h
	@brief wmreader 的数据与状态回调类
*/

typedef fastdelegate::FastDelegate1<HRESULT,void>	WMCBFUNC;

class CWMReaderCallBack:public IWMReaderCallback,public IWMReaderCallbackAdvanced
{
public:
	CWMReaderCallBack();
	~CWMReaderCallBack();

	bool	Initialize(WMCBFUNC eofunc,HANDLE asyncevent,CRingEx* ring,IWMReader* pReader);
	void	SetOutputNumber(DWORD outputno){	m_dwOoutputNum = outputno;	}
	DWORD	GetOutputNumber(){ return	m_dwOoutputNum ;	}

	HRESULT STDMETHODCALLTYPE	QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject ); 
	ULONG STDMETHODCALLTYPE		AddRef();
	ULONG STDMETHODCALLTYPE		Release();

	// --- IWMReaderCallback methods --- 
	HRESULT STDMETHODCALLTYPE	OnStatus(WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext);
	HRESULT STDMETHODCALLTYPE	OnSample(DWORD dwOutputNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR *pSample,void __RPC_FAR *pvContext );

	// --- IWMReaderCallbackAdvanced methods --- 
	virtual HRESULT STDMETHODCALLTYPE OnStreamSample(WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext); ///< 1
	virtual HRESULT STDMETHODCALLTYPE OnTime(QWORD qwCurrentTime,void __RPC_FAR * pvContext);	///< 1
	virtual HRESULT STDMETHODCALLTYPE OnStreamSelection(WORD wStreamCount,WORD __RPC_FAR * pStreamNumbers,WMT_STREAM_SELECTION __RPC_FAR * pSelections,void __RPC_FAR * pvContext); ///< 1
	virtual HRESULT STDMETHODCALLTYPE OnOutputPropsChanged(DWORD dwOutputNum,WM_MEDIA_TYPE __RPC_FAR * pMediaType,void __RPC_FAR * pvContext );	///< 1
	virtual HRESULT STDMETHODCALLTYPE AllocateForOutput(DWORD dwOutputNum,DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext);
	virtual HRESULT STDMETHODCALLTYPE AllocateForStream(WORD wStreamNum,DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext);

	bool	IsEof()
	{
		return m_bEof;
	}

	HRESULT	InitSampleAdmin(int inCount, DWORD inBufferSize);
	HRESULT UninitSampleAdmin(void);

private:
	IWMReader*	m_pReader;
	LONG			m_cRef;        // reference count
	bool			m_bEof;
	CRingEx*		m_lpRingex;
	DWORD			m_dwOoutputNum;
	HANDLE			m_asyncevnet;
	WMCBFUNC		m_eof;			///< 播放结束后的回调
#ifdef _TRY_SAMPLE_OPTIMIZATION_
	CSampleAdmin	mSampleAdmin;	
#endif 

};