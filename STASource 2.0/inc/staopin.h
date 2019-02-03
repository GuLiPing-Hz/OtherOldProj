#pragma once
#include "stastruct.h"
/**
	@file staopin
	@brief STASource outputpin
*/

class CSTAOutputpin:public CSourceStream,public CSourceSeeking
{
public:
	CSTAOutputpin(HRESULT *phr, CSource* pFilter, LPCWSTR pPinName,WORD m_nAudioStreamIndex);
	virtual ~CSTAOutputpin();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	virtual HRESULT ChangeStart();
	virtual HRESULT ChangeStop();
	virtual HRESULT ChangeRate();
	virtual HRESULT OnThreadCreate(void);
	virtual HRESULT OnThreadStartPlay(void);

	virtual HRESULT FillBuffer(IMediaSample * pSample);
	virtual HRESULT	CheckMediaType(const CMediaType* inMediatype);
	virtual HRESULT DecideBufferSize(IMemAllocator * pAlloc,ALLOCATOR_PROPERTIES * ppropInputRequest);
	virtual HRESULT Active(void);
	virtual HRESULT Inactive();

	virtual HRESULT Run();

	HRESULT GetMediaType(int iPosition, CMediaType *pmt);
	HRESULT SetMediaType(const CMediaType *pMediaType);

	HRESULT	Load(LPCOLESTR pszFileName);
	HRESULT GetCurFile(LPOLESTR *ppszFileName,AM_MEDIA_TYPE *pmt);

	static unsigned WINAPI ThreadProc1(LPVOID lpParameter){		//Note:线程构造时指定的执行函数
		CSTAOutputpin* tempThread = reinterpret_cast<CSTAOutputpin*>(lpParameter);
		tempThread->Execute();
		return S_OK;
	}
	void	Execute(void);

	HRESULT	FillHeader();
	void	MediaEof(HRESULT hr);

	HRESULT	WMStart();
	HRESULT	WMPause();
	HRESULT	WMResume();
	HRESULT	WMStop();
	HRESULT	WMClose();
	virtual HRESULT DoBufferProcessingLoop(void);
	virtual DWORD ThreadProc(void); 

	virtual HRESULT STDMETHODCALLTYPE GetLanguageLen(DWORD& dwlen);
	virtual HRESULT STDMETHODCALLTYPE GetLanguage(LPWSTR language);
	virtual HRESULT STDMETHODCALLTYPE IsMultiStream(bool& multistream);
	virtual HRESULT STDMETHODCALLTYPE SetStream(DWORD number);
	virtual HRESULT STDMETHODCALLTYPE SetDelayTime(DWORD time);

	static DWORD WINAPI WmCloseThreadFunc( LPVOID pParam );
private:
	HRESULT Parsethefile(LPCTSTR pszFileName);
	HRESULT RetrieveAttributes(void);
	HRESULT GetAVOutput();
	HRESULT	InitBuffer();
	bool	IsOk();
	void	UpdateFromSeek(void);

private:
	CCritSec		m_SharedState;
	AM_MEDIA_TYPE*	m_lpMediaType;
	tag_file		m_file;
	tag_thread		m_thread;
	tag_wmrender	m_wmreader;
	tag_global		m_global;
	tag_asynctime	m_async;
	WORD			m_nAudioStreamIndex;
public:
	HANDLE			m_hEvent;
	bool			m_bQuit;
};