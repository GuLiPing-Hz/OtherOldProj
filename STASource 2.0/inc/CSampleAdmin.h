//Download by http://www.NewXing.com
//
// CSampleAdmin.h
//
/**
 ** Copyright (C) 2005 EnjoyView Inc., all rights reserved.
 **           Your View, Our Passion. Just Enjoy It!
 **
 **            http://spaces.msn.com/members/jemylu
 **
 **/

/*************************************************************************/

#ifndef __H_CSampleAdmin__
#define __H_CSampleAdmin__
#include <list>
using namespace std;
class CWMSample;
class CSampleAdmin
{
private:
	list< CWMSample* >	mSampleList;
	int			mSampleCount;

public:
	CSampleAdmin();
	~CSampleAdmin();

	HRESULT Init(int inCount, DWORD inBufferSize);
	HRESULT Uninit(void);

	HRESULT GetEmptySample(DWORD inRequiredSize, INSSBuffer ** outBuffer);
	HRESULT Reuse(CWMSample * inSample);
};

#endif // __H_CSampleAdmin__