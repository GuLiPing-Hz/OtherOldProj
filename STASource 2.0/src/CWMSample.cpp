//Download by http://www.NewXing.com
//
// CWMSample.cpp
//
/**
 ** Copyright (C) 2005 EnjoyView Inc., all rights reserved.
 **           Your View, Our Passion. Just Enjoy It!
 **
 **            http://spaces.msn.com/members/jemylu
 **
 **/

/*************************************************************************/

#include "stdafx.h"
#include "CSampleAdmin.h"
#include "CWMSample.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
CWMSample::CWMSample(CSampleAdmin * inAdmin) : mAdmin(inAdmin), m_cRef(0)
{
	mBuffer   = NULL;
	mBufSize  = 0;
	mDataSize = 0;
}

CWMSample::~CWMSample()
{
	if (mBuffer)
	{
		delete[] mBuffer;
		mBuffer = NULL;
	}
}

HRESULT CWMSample::VerifyBufferSize(DWORD inRequiredSize)
{
	if (inRequiredSize > mBufSize)
	{
		// Re-allocate the buffer
		if (mBuffer)
		{
			delete[] mBuffer;
			mBuffer = NULL;
		}

		mBuffer   = new BYTE[inRequiredSize];
		mBufSize  = inRequiredSize;
		mDataSize = 0;
	}

	return S_OK;	
}

// --- IUnknown methods --- 
HRESULT STDMETHODCALLTYPE CWMSample::QueryInterface(REFIID riid,
							void __RPC_FAR *__RPC_FAR *ppvObject) 
{
	if ((IID_INSSBuffer== riid) || (IID_IUnknown == riid))
	{
		*ppvObject = static_cast<INSSBuffer*> (this);
		AddRef();
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CWMSample::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CWMSample::Release()
{
	if (0 == InterlockedDecrement(&m_cRef))
	{
	//	delete this;
	//	return 0;

		// Attention here:
		// Don't delete the sample object. 
		// We should return it to the list for reuse.
		mAdmin->Reuse(this);
		return 0;
	}
	return m_cRef;
}

// --- INSSBuffer methods --- 
HRESULT STDMETHODCALLTYPE CWMSample::GetLength(/* [out] */ DWORD *pdwLength)
{
	*pdwLength = mDataSize;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CWMSample::SetLength(/* [in] */ DWORD dwLength)
{
	mDataSize = dwLength;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CWMSample::GetMaxLength(/* [out] */ DWORD *pdwLength)
{
	*pdwLength = mBufSize;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CWMSample::GetBuffer(/* [out] */ BYTE **ppdwBuffer)
{
	*ppdwBuffer = mBuffer;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CWMSample::GetBufferAndLength(/* [out] */ BYTE **ppdwBuffer,
														/* [out] */ DWORD *pdwLength)
{
	*ppdwBuffer = mBuffer;
	*pdwLength  = mDataSize;
	return S_OK;
}
