#pragma once
/*#include <altbase.h>*/
#include <string>
#include <afxwin.h>
#include <streams.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)  do{if((x)) \
	(x)->Release(); (x)=NULL; }while(0)
#endif//SAFE_RELEASE

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) \
	if ((x) != NULL)      \
{                   \
	delete (x);        \
	(x) = NULL;        \
}
#endif//SAFE_DELETE

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
	if ((x) != NULL)            \
{                         \
	delete[] (x);            \
	(x) = NULL;              \
}
#endif//SAFE_ARRAY_DELETE

#ifndef SAFE_FREE
#define SAFE_FREE(x) \
	if ((x) != NULL)            \
{                         \
	free((x));            \
	(x) = NULL;              \
}
#endif//SAFE_FREE

#ifndef QI
#define QI(i)  (riid == __uuidof(i)) ? GetInterface((i*)this, ppv) :
#endif//QI
#ifndef QI2
#define QI2(i) (riid == IID_##i) ? GetInterface((i*)this, ppv) :
#endif//QI2

#ifndef BeginEnumPins
#define BeginEnumPins(pBaseFilter, pEnumPins, pPin)                                 \
{                                                                                   \
	CComPtr<IEnumPins> pEnumPins;                                                   \
	if (pBaseFilter && SUCCEEDED(pBaseFilter->EnumPins(&pEnumPins)))                \
	{                                                                               \
	for (CComPtr<IPin> pPin; S_OK == pEnumPins->Next(1, &pPin, 0); pPin = NULL) \
		{
#endif//BeginEnumPins

#ifndef EndEnumPins
#define EndEnumPins }}}
#endif//EndEnumPins

#ifndef PauseGraph
#define PauseGraph                                                                                         \
	CComQIPtr<IMediaControl> _pMC(m_pGraph);                                                               \
	OAFilterState _fs = -1;                                                                                \
	if (_pMC)                                                                                              \
	_pMC->GetState(1000, &_fs);                                                                        \
	if (_fs == State_Running)                                                                              \
	_pMC->Pause();                                                                                     \
	\
	HRESULT _hr = E_FAIL;                                                                                  \
	CComQIPtr<IMediaSeeking> _pMS((IUnknown*)(INonDelegatingUnknown*)m_pGraph);                            \
	REFERENCE_TIME _rtNow = 0;                                                                             \
	if (_pMS)                                                                                              \
	_hr = _pMS->GetCurrentPosition(&_rtNow);
#endif//PauseGraph

#ifndef ResumeGraph
#define ResumeGraph                                                                                        \
	if (SUCCEEDED(_hr) && _pMS && _fs != State_Stopped)                                                    \
	_hr = _pMS->SetPositions(&_rtNow, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning); \
	\
	if (_fs == State_Running && _pMS)                                                                      \
	_pMC->Run();
#endif//ResumeGraph


class CPinInfo : public PIN_INFO
{
public:
	CPinInfo() {pFilter = NULL;}
	~CPinInfo() {
		if (pFilter) {
			pFilter->Release();
		}
	}
};

extern CStringW GetPinName(IPin* pPin);
extern IPin* GetUpStreamPin(IBaseFilter* pBF, IPin* pInputPin = NULL);
extern IBaseFilter*  GetFilterFromPin(IPin* pPin);
extern int CountPins(IBaseFilter* pBF, int& nIn, int& nOut, int& nInC, int& nOutC);
extern bool IsSplitter(IBaseFilter* pBF, bool fCountConnectedOnly= false);
extern IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir= PINDIR_INPUT);
extern bool IsAudioWaveRenderer(IBaseFilter* pBF);
extern CLSID GetCLSID(IPin* pPin);
extern CLSID GetCLSID(IBaseFilter* pBF);
extern IPin* GetFirstDisconnectedPin(IBaseFilter* pBF, PIN_DIRECTION dir);

template <class T>
static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr)
{
	*phr = S_OK;
	CUnknown* punk = new T(lpunk, phr);
	if (punk == NULL) {
		*phr = E_OUTOFMEMORY;
	}
	return punk;
}
