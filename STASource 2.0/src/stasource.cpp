#include "stdafx.h"
#include "stasource.h"

CSTASource::CSTASource(LPUNKNOWN lpunk, HRESULT *phr):
	CSource(NAME("STASource Filter"), lpunk, CLSID_STASOURCE)
{
	m_lpPin = new CSTAOutputpin(phr,this,TEXT("Output"),1);
	m_lpPin2 = new CSTAOutputpin(phr,this,TEXT("Output"),2);
	if(m_lpPin == NULL&&m_lpPin2 == NULL)
		*phr = E_OUTOFMEMORY;
}

CSTASource::~CSTASource()
{
	
}

CUnknown* CSTASource::CreateInstance(LPUNKNOWN lpUnk,HRESULT* lphr)
{
	CSTASource* lpSource = new CSTASource(lpUnk,lphr);
	if(lpSource == NULL) *lphr = E_OUTOFMEMORY;
	return lpSource;
}

HRESULT CSTASource::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv, E_POINTER);
	if (riid == IID_IFileSourceFilter)
	{
		return GetInterface((IFileSourceFilter *) this, ppv);
	}
	else if (riid == IID_ISTASOURCE)
	{
		return GetInterface((ISTASource*) this, ppv);
	}
	else
	{
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}
}

HRESULT CSTASource::Load(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt)
{
	CheckPointer(m_lpPin,E_POINTER);
	m_lpPin->Load(pszFileName);
	CheckPointer(m_lpPin2,E_POINTER);
	m_lpPin2->Load(pszFileName);
	return S_OK;
}

HRESULT CSTASource::GetCurFile(LPOLESTR *ppszFileName,AM_MEDIA_TYPE *pmt)
{
	CheckPointer(m_lpPin,E_POINTER);
	return m_lpPin->GetCurFile(ppszFileName,pmt);
}

HRESULT CSTASource::GetLanguageLen(DWORD& dwlen)
{
	CheckPointer(m_lpPin,E_POINTER);
	return m_lpPin->GetLanguageLen(dwlen);
}

HRESULT CSTASource::GetLanguage(LPTSTR language)
{
	CheckPointer(m_lpPin,E_POINTER);
	return m_lpPin->GetLanguage(language);
}

HRESULT CSTASource::IsMultiStream(bool& multistream)
{
	CheckPointer(m_lpPin,E_POINTER);
	return m_lpPin->IsMultiStream(multistream);
}

HRESULT CSTASource::SetStream(DWORD number)
{
	CheckPointer(m_lpPin,E_POINTER);
	return m_lpPin->SetStream(number);
}

HRESULT CSTASource::SetDelayTime(DWORD time)
{
	CheckPointer(m_lpPin,E_POINTER);
	CheckPointer(m_lpPin2,E_POINTER);
	return (m_lpPin->SetDelayTime(time) & m_lpPin2->SetDelayTime(time));
}
HRESULT CSTASource::SetPause()
{
	HRESULT hr;
  	hr = m_lpPin->WMPause();
	hr|= m_lpPin2->WMPause();
	return hr;
}
HRESULT CSTASource::SetPlay()
{
// 	HRESULT hr;
// 	hr = m_lpPin->Run();
// 	hr|= m_lpPin2->Run();
	return 0;
}