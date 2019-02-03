#pragma once
#include <string>
#include "WMReader.h"
#include "GrayProp.h"

class CWMAOutputPin;
class CCWMAPassThru : public CSourceSeeking
{
public:
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	CCWMAPassThru(LPUNKNOWN lpunk, HRESULT *phr,CWMAOutputPin* owner);
	~CCWMAPassThru();
	//PURE
	virtual HRESULT ChangeStart();
	virtual HRESULT ChangeStop();
	virtual HRESULT ChangeRate();

	REFERENCE_TIME		getDuration(){return m_rtDuration;}
	REFERENCE_TIME		getStartRT(){return m_rtStart;}
	REFERENCE_TIME		getStopRT(){return m_rtStop;}
	double							getRateSeeking(){return m_dRateSeeking;}

private:
	CWMAOutputPin* m_pOwner;
};
// {19D812B8-095B-493a-81E3-765C529B6ABB}
// DEFINE_GUID(<<name>>, 
// 			0x19d812b8, 0x95b, 0x493a, 0x81, 0xe3, 0x76, 0x5c, 0x52, 0x9b, 0x6a, 0xbb);
interface __declspec(uuid("19D812B8-095B-493a-81E3-765C529B6ABB"))
IGSeekPosition : public IUnknown
{
	virtual HRESULT setNewPosition(REFERENCE_TIME rtCur) = 0;
};

class CWMAOutputPin : public CSourceStream,public IGSeekPosition
{
public:
	friend class CCWMAPassThru;

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	CWMAOutputPin(HRESULT* phr,CSource* ps,LPCWSTR lpPinName,CWMReader* pWMReader=NULL);
	~CWMAOutputPin();

	//IGSeekPosition
	virtual HRESULT setNewPosition(REFERENCE_TIME rtCur);
	//CBaseOutputPin
	virtual HRESULT DecideBufferSize(IMemAllocator * pAlloc,ALLOCATOR_PROPERTIES * ppropInputRequest);
	HRESULT Active(void);
	HRESULT Inactive();
	//CSourceStream
	virtual HRESULT FillBuffer(IMediaSample *pSamp);
	virtual HRESULT CheckMediaType(const CMediaType *pMediaType);
	virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);  // List pos. 0-n
	virtual HRESULT OnThreadStartPlay(void);
	// Quality control notifications sent to us
	STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

	void							setWMAOutputPin(CWMReader* pWMReader);
	AM_MEDIA_TYPE*	getMT();
	void							updateFromSeek(void);
private:
	CWMReader*			m_pWMReader;
	CCWMAPassThru*	m_pPassThru;
};

class __declspec(uuid("3A087870-9633-427d-9341-D1487C8C7FD0"))
CWMASource : public CSource , public IFileSourceFilter , public ISpecifyPropertyPages
{
	friend class CWMAOutputPin;

	CWMASource(LPUNKNOWN lpunk, HRESULT *phr);
	~CWMASource(void);

public:
 	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) ;

	//IFileSourceFilter
	virtual STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
	virtual STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);
	//ISpecifyPropertyPages
	virtual HRESULT STDMETHODCALLTYPE GetPages( CAUUID *pPages);

	HRESULT					CheckMediaType(const CMediaType* pmt);
private:
	std::wstring m_strFile;
};
