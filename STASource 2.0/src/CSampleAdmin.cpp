//Download by http://www.NewXing.com
//
// CSampleAdmin.cpp
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
#include "CWMSample.h"
#include "CSampleAdmin.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
CSampleAdmin::CSampleAdmin()
{
	mSampleCount = 0;
}

CSampleAdmin::~CSampleAdmin()
{
	Uninit();
}

HRESULT CSampleAdmin::Init(int inCount, DWORD inBufferSize)
{
	mSampleCount = inCount;

	// Allocate samples and add them to the list
	for (int i = 0; i < inCount; i++)
	{
		CWMSample * pSample = new CWMSample(this);
		pSample->VerifyBufferSize(inBufferSize);

		mSampleList.push_back(pSample);
	}

	return S_OK;
}

HRESULT CSampleAdmin::Uninit(void)
{
	// Check if all samples are back to the list
	if ( mSampleCount != mSampleList.size() )
	{
		return S_FALSE;
	}

	// Remove samples from the list and release memory

	CWMSample * pSample = NULL;
	list<CWMSample*>::iterator plist;
	for(plist = mSampleList.begin(); plist != mSampleList.end(); plist++)
	{
		pSample = *plist;
		delete pSample;
		pSample = NULL;
	}
	mSampleList.clear();

	mSampleCount = 0;

	return S_OK;
}

// Get a free sample from the list
HRESULT CSampleAdmin::GetEmptySample(DWORD inRequiredSize, INSSBuffer ** outBuffer)
{
	while (mSampleList.empty())
	{
		Sleep(100);
	}

	//从类表头取出一个空闲的sample后，将其移出列表头位置
	CWMSample * pSample = (CWMSample *) mSampleList.front();
	mSampleList.pop_front();

	if ( pSample )
	{
		// Verify the buffer size
		pSample->VerifyBufferSize(inRequiredSize);
		pSample->AddRef();  // !!!
		*outBuffer = (INSSBuffer *) pSample;
	}
	return S_OK;
}

HRESULT CSampleAdmin::Reuse(CWMSample * inSample)
{
	// Add the sample to the list again for later use
	//重新回到列表尾部
	mSampleList.push_back(inSample);
	//The sample comes back to the list.
	return S_OK;
}