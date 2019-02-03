//Download by http://www.NewXing.com
//
// CWMSample.h
//
/**
 ** Copyright (C) 2005 EnjoyView Inc., all rights reserved.
 **           Your View, Our Passion. Just Enjoy It!
 **
 **            http://spaces.msn.com/members/jemylu
 **
 **/

/*************************************************************************/

#ifndef __H_CWMSample__
#define __H_CWMSample__

class CSampleAdmin;
class CWMSample : public INSSBuffer
{
private:
	LONG	m_cRef; 

	CSampleAdmin *	mAdmin; // Manage a sample list

	BYTE *	mBuffer;
	DWORD	mBufSize;    // Buffer size
	DWORD	mDataSize;   // Valid data size in the buffer

public:
	CWMSample(CSampleAdmin * inAdmin);
	~CWMSample();

	HRESULT VerifyBufferSize(DWORD inRequiredSize);

	// --- IUnknown methods --- 
	HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */ REFIID riid,
					/* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject ); 
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	// --- INSSBuffer methods --- 
	HRESULT STDMETHODCALLTYPE GetLength(/* [out] */ DWORD *pdwLength);
	HRESULT STDMETHODCALLTYPE SetLength(/* [in] */ DWORD dwLength); 
	HRESULT STDMETHODCALLTYPE GetMaxLength(/* [out] */ DWORD *pdwLength);   
	HRESULT STDMETHODCALLTYPE GetBuffer(/* [out] */ BYTE **ppdwBuffer); 
	HRESULT STDMETHODCALLTYPE GetBufferAndLength(/* [out] */ BYTE **ppdwBuffer,
				/* [out] */ DWORD *pdwLength);
};

#endif // __H_CWMSample__