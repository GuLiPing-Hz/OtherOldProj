#pragma once
#include "istasource.h"
#include "staopin.h"
/**
	@file stasource.h
	@brief source filter
*/

class CSTASource:public CSource,public IFileSourceFilter,public ISTASource{
public:
	CSTASource(LPUNKNOWN lpunk, HRESULT *phr);
	virtual ~CSTASource();

	static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpUnk,HRESULT* lphr); ///< ´´½¨ÊµÀý

	DECLARE_IUNKNOWN;
	/*STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
		return GetOwner()->QueryInterface(riid,ppv);
	};
	STDMETHODIMP_(ULONG) AddRef() {
		return GetOwner()->AddRef();
	};
	STDMETHODIMP_(ULONG) Release() {
		return GetOwner()->Release();
	};*/
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	virtual HRESULT STDMETHODCALLTYPE Load(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt);
	virtual HRESULT STDMETHODCALLTYPE GetCurFile(LPOLESTR *ppszFileName,AM_MEDIA_TYPE *pmt);

	virtual HRESULT STDMETHODCALLTYPE GetLanguageLen(DWORD& dwlen);
	virtual HRESULT STDMETHODCALLTYPE GetLanguage(LPTSTR language);
	virtual HRESULT STDMETHODCALLTYPE IsMultiStream(bool& multistream);
	virtual HRESULT STDMETHODCALLTYPE SetStream(DWORD number);
	virtual HRESULT STDMETHODCALLTYPE SetDelayTime(DWORD time);
	virtual HRESULT STDMETHODCALLTYPE SetPause();
	virtual HRESULT STDMETHODCALLTYPE SetPlay();

private:
	CSTAOutputpin*		m_lpPin;
	CSTAOutputpin*		m_lpPin2;
};