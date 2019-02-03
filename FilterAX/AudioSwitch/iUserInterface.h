#pragma once

#ifndef IUSERINTERFACE__H__
#define IUSERINTERFACE__H__
#include <objbase.h>

interface __declspec(uuid("19D812B8-095B-493a-81E3-765C529B6ABB"))
IGSeekPosition : public IUnknown
{
	virtual HRESULT setNewPosition(REFERENCE_TIME rtCur) = 0;
};

interface __declspec(uuid("01BE6031-0672-41E5-A056-1D8CC14E9334"))
IAudioDecReset : public IUnknown
{
	STDMETHOD(reset) () PURE;
};

#endif
