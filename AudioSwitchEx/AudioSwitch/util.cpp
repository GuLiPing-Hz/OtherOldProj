#include "util.h"
#include "NullRenders.h"
#include <InitGuid.h>


IPin* GetFirstDisconnectedPin(IBaseFilter* pBF, PIN_DIRECTION dir)
{
	if (!pBF) {
		return NULL;
	}

	BeginEnumPins(pBF, pEP, pPin) {
		PIN_DIRECTION dir2;
		pPin->QueryDirection(&dir2);
		CComPtr<IPin> pPinTo;
		if (dir == dir2 && (S_OK != pPin->ConnectedTo(&pPinTo))) {
			IPin* pRet = pPin.Detach();
			pRet->Release();
			return pRet;
		}
	}
	EndEnumPins;

	return NULL;
}

CLSID GetCLSID(IBaseFilter* pBF)
{
	CLSID clsid = GUID_NULL;
	if (pBF) {
		pBF->GetClassID(&clsid);
	}
	return clsid;
}

CLSID GetCLSID(IPin* pPin)
{
	return GetCLSID(GetFilterFromPin(pPin));
}

DEFINE_GUID(CLSID_ReClock,
			0x9dc15360, 0x914c, 0x46b8, 0xb9, 0xdf, 0xbf, 0xe6, 0x7f, 0xd3, 0x6c, 0x6a);
bool IsAudioWaveRenderer(IBaseFilter* pBF)
{
	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);

	if (nInC > 0 && nOut == 0 && CComQIPtr<IBasicAudio>(pBF)) {
		BeginEnumPins(pBF, pEP, pPin) {
			AM_MEDIA_TYPE mt;
			if (S_OK != pPin->ConnectionMediaType(&mt)) {
				continue;
			}

			FreeMediaType(mt);

			return !!(mt.majortype == MEDIATYPE_Audio);
			/*&& mt.formattype == FORMAT_WaveFormatEx);*/
		}
		EndEnumPins;
	}

	CLSID clsid;
	memcpy(&clsid, &GUID_NULL, sizeof(clsid));
	pBF->GetClassID(&clsid);

	return (clsid == CLSID_DSoundRender || clsid == CLSID_AudioRender || clsid == CLSID_ReClock
		|| clsid == __uuidof(CNullAudioRenderer) || clsid == __uuidof(CNullUAudioRenderer));
}

IPin* GetUpStreamPin(IBaseFilter* pBF, IPin* pInputPin)
{
	BeginEnumPins(pBF, pEP, pPin) {
		if (pInputPin && pInputPin != pPin) {
			continue;
		}

		PIN_DIRECTION dir;
		CComPtr<IPin> pPinConnectedTo;
		if (SUCCEEDED(pPin->QueryDirection(&dir)) && dir == PINDIR_INPUT
			&& SUCCEEDED(pPin->ConnectedTo(&pPinConnectedTo))) {
				IPin* pRet = pPinConnectedTo.Detach();
				pRet->Release();
				return pRet;
		}
	}
	EndEnumPins;

	return NULL;
}

IBaseFilter* GetFilterFromPin(IPin* pPin)
{
	if (!pPin) {
		return NULL;
	}
	IBaseFilter* pBF = NULL;
	CPinInfo pi;
	if (pPin && SUCCEEDED(pPin->QueryPinInfo(&pi))) {
		pBF = pi.pFilter;
	}
	return pBF;
}

CStringW GetPinName(IPin* pPin)
{
	CStringW name;
	CPinInfo pi;
	if (pPin && SUCCEEDED(pPin->QueryPinInfo(&pi))) {
		name = pi.achName;
	}
	return name;
}

int CountPins(IBaseFilter* pBF, int& nIn, int& nOut, int& nInC, int& nOutC)
{
	nIn = nOut = 0;
	nInC = nOutC = 0;

	BeginEnumPins(pBF, pEP, pPin) {
		PIN_DIRECTION dir;
		if (SUCCEEDED(pPin->QueryDirection(&dir))) {
			CComPtr<IPin> pPinConnectedTo;
			pPin->ConnectedTo(&pPinConnectedTo);

			if (dir == PINDIR_INPUT) {
				nIn++;
				if (pPinConnectedTo) {
					nInC++;
				}
			} else if (dir == PINDIR_OUTPUT) {
				nOut++;
				if (pPinConnectedTo) {
					nOutC++;
				}
			}
		}
	}
	EndEnumPins;

	return (nIn + nOut);
}

bool IsSplitter(IBaseFilter* pBF, bool fCountConnectedOnly)
{
	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);
	return (fCountConnectedOnly ? nOutC > 1 : nOut > 1);
}

IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir)
{
	if (!pBF) {
		return NULL;
	}

	BeginEnumPins(pBF, pEP, pPin) {
		PIN_DIRECTION dir2;
		pPin->QueryDirection(&dir2);
		if (dir == dir2) {
			IPin* pRet = pPin.Detach();
			pRet->Release();
			return pRet;
		}
	}
	EndEnumPins;

	return NULL;
}

