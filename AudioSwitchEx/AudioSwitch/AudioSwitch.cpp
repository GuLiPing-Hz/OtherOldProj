#include "AudioSwitch.h"
//#include "uguid.h"
#include <math.h>
#include <MMReg.h>
#include "Audio.h"
#include "GrayProp.h"
#include "moreguids.h"
#include <assert.h>
#include "SoundTouch/SoundTouch.h"

#define INT24_MAX       8388607
#define INT24_MIN     (-8388608)

DEFINE_GUID(KSDATAFORMAT_SUBTYPE_PCM,0x00000001,0x0000,0x0010,0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71);
DEFINE_GUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT,0x00000003,0x0000,0x0010,0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71);


const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_NULL}
	// Major type 主类型 
	// Minor type sub类型，可以为MEDIASUBTYPE_NULL
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, TRUE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesOut), sudPinTypesOut}
// pin的名字 Obsolete, not used.
// Is it rendered       输入pin有用，输出pin一般为FALSE 
// Is it an output      TRUE表示是输出pin，不然是输入pin 
// Can the filter create zero instances? 是否能不实例化 
// Does the filter create multiple instances?是否能创建多个同这样类型的pin 
// Obsolete. 
// Obsolete.
// Number of media types. 该pin支持的媒体类型数量 
// Pointer to media types.   该pin的媒体类型的描述 
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CAudioSwitcherFilter), AudioSwitcherName, MERIT_DO_NOT_USE, _countof(sudpPins), sudpPins }
// Filter CLSID   该filter的类标志 
// Filter name.   该filter的名字 
// Filter merit   该filter的Merit值 
// Number of pin types.   该filter的pin的数目 
// Pointer to pin information.   该filter的pin的描述 
};

CFactoryTemplate g_Templates[] = {
	// This entry is for the filter.
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CAudioSwitcherFilter>, NULL, &sudFilter[0]},
//filter的名字 
//对象的类标识   
//创建一个实例用的函数 
// NULL
//Pointer to filter information.filter的注册信息

	// This entry is for the property page.
	{ L"My Props",&__uuidof(CGrayProp),CGrayProp::CreateInstance, NULL, NULL},

};
int g_cTemplates = _countof(g_Templates);
//////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );
}

//
// Win32 DllEntryPoint
//
// extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
// 
// BOOL APIENTRY DllMain(HANDLE hModule, 
// 					  DWORD  dwReason, 
// 					  LPVOID lpReserved)
// {
// 	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
// }

//MFC DllEntryPoint
#include "FilterApp.h"

CFilterApp theApp;

// Microsoft C Compiler will give hundreds of warnings about
// unused inline functions in header files.  Try to disable them.
#pragma warning( disable:4514)

#ifdef _BIG_ENDIAN_
// big-endian CPU, swap bytes in 16 & 32 bit words

// helper-function to swap byte-order of 32bit integer
static inline int _swap32(int &dwData)
{
	dwData = ((dwData >> 24) & 0x000000FF) | 
		((dwData >> 8)  & 0x0000FF00) | 
		((dwData << 8)  & 0x00FF0000) | 
		((dwData << 24) & 0xFF000000);
	return dwData;
}   

// helper-function to swap byte-order of 16bit integer
static inline short _swap16(short &wData)
{
	wData = ((wData >> 8) & 0x00FF) | 
		((wData << 8) & 0xFF00);
	return wData;
}

// helper-function to swap byte-order of buffer of 16bit integers
static inline void _swap16Buffer(short *pData, int numWords)
{
	int i;

	for (i = 0; i < numWords; i ++)
	{
		pData[i] = _swap16(pData[i]);
	}
}

#else   // BIG_ENDIAN
// little-endian CPU, WAV file is ok as such

// dummy helper-function
static inline int _swap32(int &dwData)
{
	// do nothing
	return dwData;
}   

// dummy helper-function
static inline short _swap16(short &wData)
{
	// do nothing
	return wData;
}

// dummy helper-function
static inline void _swap16Buffer(short *pData, int numBytes)
{
	// do nothing
}

#endif  // BIG_ENDIAN

/// Convert from float to integer and saturate
static inline int saturate(float fvalue, float minval, float maxval)
{
	if (fvalue > maxval) 
	{
		fvalue = maxval;
	} 
	else if (fvalue < minval)
	{
		fvalue = minval;
	}
	return (int)fvalue;
}

static inline bool toFloat(IN unsigned char * temp,OUT float *buffer,IN int numElems,IN int bytesPerSample )
{
	// swap byte ordert & convert to float, depending on sample format
	switch (bytesPerSample)
	{
	case 1:
		{
			unsigned char *temp2 = (unsigned char*)temp;
			double conv = 1.0 / 128.0;
			for (int i = 0; i < numElems; i ++)
			{
				buffer[i] = (float)(temp2[i] * conv - 1.0);
			}
			break;
		}

	case 2:
		{
			short *temp2 = (short*)temp;
			double conv = 1.0 / 32768.0;
			for (int i = 0; i < numElems; i ++)
			{
				short value = temp2[i];
				buffer[i] = (float)(_swap16(value) * conv);
			}
			break;
		}

	case 3:
		{
			char *temp2 = (char *)temp;
			double conv = 1.0 / 8388608.0;
			for (int i = 0; i < numElems; i ++)
			{
				int value = *((int*)temp2);
				value = _swap32(value) & 0x00ffffff;             // take 24 bits
				value |= (value & 0x00800000) ? 0xff000000 : 0;  // extend minus sign bits
				buffer[i] = (float)(value * conv);
				temp2 += 3;
			}
			break;
		}

	case 4:
		{
			int *temp2 = (int *)temp;
			double conv = 1.0 / 2147483648.0;
			assert(sizeof(int) == 4);
			for (int i = 0; i < numElems; i ++)
			{
				int value = temp2[i];
				buffer[i] = (float)(_swap32(value) * conv);
			}
			break;
		}

	default:
		return false;
	}

	return true;
}

static inline bool toSampleType(OUT unsigned char * temp,IN float *buffer,IN int numElems,IN int bytesPerSample )
{
	switch (bytesPerSample)
	{
	case 1:
		{
			unsigned char *temp2 = (unsigned char *)temp;
			for (int i = 0; i < numElems; i ++)
			{
				temp2[i] = (unsigned char)saturate(buffer[i] * 128.0f + 128.0f, 0.0f, 255.0f);
			}
			break;
		}

	case 2:
		{
			short *temp2 = (short *)temp;
			for (int i = 0; i < numElems; i ++)
			{
				short value = (short)saturate(buffer[i] * 32768.0f, -32768.0f, 32767.0f);
				temp2[i] = _swap16(value);
			}
			break;
		}

	case 3:
		{
			char *temp2 = (char *)temp;
			for (int i = 0; i < numElems; i ++)
			{
				int value = saturate(buffer[i] * 8388608.0f, -8388608.0f, 8388607.0f);
				*((int*)temp2) = _swap32(value);
				temp2 += 3;
			}
			break;
		}

	case 4:
		{
			int *temp2 = (int *)temp;
			for (int i = 0; i < numElems; i ++)
			{
				int value = saturate(buffer[i] * 2147483648.0f, -2147483648.0f, 2147483647.0f);
				temp2[i] = _swap32(value);
			}
			break;
		}

	default:
		return false;
	}

	return true;
}

template<class T>
HRESULT GetPeer(CStreamSwitcherFilter* pFilter, T** ppT)
{
	*ppT = NULL;

	CBasePin* pPin = pFilter->GetInputPin();
	if (!pPin) {
		return E_NOTIMPL;
	}

	CComPtr<IPin> pConnected;
	if (FAILED(pPin->ConnectedTo(&pConnected))) {
		return E_NOTIMPL;
	}

	if (CComQIPtr<T> pT = pConnected) {
		*ppT = pT.Detach();
		return S_OK;
	}

	return E_NOTIMPL;
}
//
// CAudioSwitcherFilter
//

CAudioSwitcherFilter::CAudioSwitcherFilter(LPUNKNOWN lpunk, HRESULT* phr)
:CStreamSwitcherFilter(lpunk, phr, __uuidof(this))
,m_fCustomChannelMapping(false)
,m_fDownSampleTo441(false)
,m_rtAudioTimeShift(0)
,m_rtNextStart(0)
,m_rtNextStop(1)
,m_fNormalize(false)
,m_fNormalizeRecover(false)
,m_boost_mul(1)
,m_sample_max(0.1f)
,m_lSaturation(0)
,m_bInitSoundPitch(false)
,m_nPitch(0)
,m_pSoundTouch(NULL)
,m_nNumElems(0)
,m_pFloatBuffer(NULL)
,m_bNeedChangePitch(false)
{
	memset(m_pSpeakerToChannelMap, 0, sizeof(m_pSpeakerToChannelMap));

	if (phr) {
		if (FAILED(*phr)) {
			return;
		} else {
			*phr = S_OK;
		}
	}

	m_pSoundTouch = new soundtouch::SoundTouch();
}

CAudioSwitcherFilter::~CAudioSwitcherFilter()
{
	SAFE_DELETE(m_pSoundTouch);
}

STDMETHODIMP CAudioSwitcherFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI2(IOS_ChangePitch)
		QI2(IOS_AudioSwitch)
		QI2(ISpecifyPropertyPages)
		QI(IAudioSwitcherFilter)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAudioSwitcherFilter::CheckMediaType(const CMediaType* pmt)
{
	WAVEFORMATEX* pWF = NULL;
	if (pmt->formattype == FORMAT_WaveFormatEx && pmt->cbFormat >= sizeof(WAVEFORMATEX))
	{
		pWF = (WAVEFORMATEX*)pmt->pbFormat;
		if((pWF->nChannels > 2) && (pWF->wFormatTag != WAVE_FORMAT_EXTENSIBLE)) 
		{
			return VFW_E_INVALIDMEDIATYPE;    // stupid iviaudio tries to fool us
		}
	}

	
	return (pmt->majortype == MEDIATYPE_Audio
		&& pmt->formattype == FORMAT_WaveFormatEx
		&& (pWF->wBitsPerSample == 8
		|| pWF->wBitsPerSample == 16
		|| pWF->wBitsPerSample == 24
		|| pWF->wBitsPerSample == 32)
		&& (pWF->wFormatTag == WAVE_FORMAT_PCM
		|| pWF->wFormatTag == WAVE_FORMAT_IEEE_FLOAT
		|| pWF->wFormatTag == WAVE_FORMAT_DOLBY_AC3_SPDIF
		|| pWF->wFormatTag == WAVE_FORMAT_EXTENSIBLE))
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;
}

template<class T, class U, int Umin, int Umax>
__forceinline void mix(DWORD mask, int ch, int bps, BYTE* src, BYTE* dst)
{
	U sum = 0;

	for (int i = 0, j = ch; i < j; i++) {
		if (mask & (1 << i)) {
			sum += *(T*)&src[bps * i];
		}
	}

	if (sum < Umin) {
		sum = Umin;
	} else if (sum > Umax) {
		sum = Umax;
	}

	*(T*)dst = (T)sum;
}

template<>
__forceinline void mix<int, INT64, INT24_MIN, INT24_MAX>(DWORD mask, int ch, int bps, BYTE* src, BYTE* dst)
{
	INT64 sum = 0;

	for (int i = 0, j = ch; i < j; i++) {
		if (mask & (1 << i)) {
			int tmp;
			memcpy((BYTE*)&tmp + 1, &src[bps * i], 3);
			sum += tmp >> 8;
		}
	}

	sum = min(max(sum, INT24_MIN), INT24_MAX);

	memcpy(dst, (BYTE*)&sum, 3);
}

template<class T, class U, int Umin, int Umax>
__forceinline void mix4(DWORD mask, BYTE* src, BYTE* dst)
{
	U sum = 0;
	int bps = sizeof T;

	if (mask & (1 << 0)) {
		sum += *(T*)&src[bps * 0];
	}
	if (mask & (1 << 1)) {
		sum += *(T*)&src[bps * 1];
	}
	if (mask & (1 << 2)) {
		sum += *(T*)&src[bps * 2];
	}
	if (mask & (1 << 3)) {
		sum += *(T*)&src[bps * 3];
	}

	if (sum < Umin) {
		sum = Umin;
	} else if (sum > Umax) {
		sum = Umax;
	}

	*(T*)dst = (T)sum;
}

template<class T>
T clamp(double s, T smin, T smax)
{
	if (s < -1) {
		s = -1;
	} else if (s > 1) {
		s = 1;
	}
	T t = (T)(s * smax);
	if (t < smin) {
		t = smin;
	} else if (t > smax) {
		t = smax;
	}
	return t;
}

int CAudioSwitcherFilter::ChangeMediaSample(LISTMEDIASAMPLE list_MS,IMediaSample* &pIn)
{
	BYTE* pData;
	REFERENCE_TIME start_tm,stop_tm;
	pIn->GetPointer(&pData);
	pIn->GetTime(&start_tm,&stop_tm);
	AM_MEDIA_TYPE* pAM_MT;
	pIn->GetMediaType(&pAM_MT);

	long total_size = pIn->GetSize();
	long new_size = pIn->GetActualDataLength();
	if (!list_MS.empty())
	{
		new_size = 0;
		LISTMEDIASAMPLE::iterator i = list_MS.begin();
		REFERENCE_TIME new_start_tm = (*i).start_tm;
		while(i!=list_MS.end())
		{
			if (new_size+(*i).data_size>=total_size)
			{
				break;
			}
			memcpy(pData+new_size,(*i).pSample,(*i).data_size);
			new_size += (*i).data_size;
			i++;
		}

		pIn->SetMediaType(pAM_MT);
		pIn->SetTime(&new_start_tm,&stop_tm);
		pIn->SetActualDataLength(new_size);
	}
	DeleteMediaType(pAM_MT);

	return new_size;
}

HRESULT CAudioSwitcherFilter::SetupSoundPitch(soundtouch::SoundTouch *pSoundTouch, WAVEFORMATEX * wfeout )
{
	if ( NULL == wfeout ) {
		return S_FALSE;
	}

	pSoundTouch->setSampleRate(wfeout->nSamplesPerSec);
	pSoundTouch->setChannels(wfeout->nChannels);

	pSoundTouch->setTempoChange(0.0);
	pSoundTouch->setPitchSemiTones(m_nPitch);
	pSoundTouch->setRateChange(0.0);

	pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 0);
	pSoundTouch->setSetting(SETTING_USE_AA_FILTER, 1);

	return S_OK;
}

HRESULT	CAudioSwitcherFilter::ChangePitch(IMediaSample* pOut)
{
	if ( !pOut ) 
	{
		return E_INVALIDARG;
	}

	CStreamSwitcherOutputPin* pOutPin = GetOutputPin();
	if ( NULL == pOutPin ) 
	{
		return E_UNEXPECTED;
	}
	WAVEFORMATEX* wfeout = NULL;
	if (pOutPin->CurrentMediaType().formattype == FORMAT_WaveFormatEx && pOutPin->CurrentMediaType().cbFormat >= sizeof(WAVEFORMATEX))
	{
		wfeout = (WAVEFORMATEX*) pOutPin->CurrentMediaType().pbFormat;
		if ( NULL == wfeout || 0 >= wfeout->nChannels && 8 > wfeout->wBitsPerSample ) 
		{
			return E_INVALIDARG;
		}
	}
	else
	{//just do nothing
		return S_OK;
	}

	if ( !m_bInitSoundPitch ) 
	{
		SetupSoundPitch(m_pSoundTouch,wfeout);
		m_bInitSoundPitch = true;
	}

	if (m_bNeedChangePitch)
	{
		if ( m_pSoundTouch )
		{
			m_pSoundTouch->setPitchSemiTones( m_nPitch );
		}
		CAutoLock lock(&m_csPitchChange);
		m_bNeedChangePitch = false;
	}

	if ( m_nPitch != 0 ) {
		BYTE * pBuffer = NULL;
		pOut->GetPointer( &pBuffer );

		int bytesPerSample = (wfeout->wBitsPerSample)>>3;
		int l = min( pOut->GetActualDataLength(), pOut->GetSize() );

		int numElems = (l / bytesPerSample);
		if ( m_nNumElems < numElems ) 
		{
			m_nNumElems = numElems;
			SAFE_DELETE(m_pFloatBuffer);
			m_pFloatBuffer = new float[m_nNumElems];
		}

		// transform a chunk of samples
		if ( FAILED( toFloat( pBuffer, m_pFloatBuffer, numElems, bytesPerSample ) ) ) 
		{
			return E_FAIL;
		}

		int nSamples = numElems / wfeout->nChannels;
		// Feed the samples into SoundTouch processor
		m_pSoundTouch->putSamples(m_pFloatBuffer, nSamples);

		// Read ready samples from SoundTouch processor & write them output file.
		// NOTES:
		// - 'receiveSamples' doesn't necessarily return any samples at all
		//   during some rounds!
		// - On the other hand, during some round 'receiveSamples' may have more
		//   ready samples than would fit into 'sampleBuffer', and for this reason 
		//   the 'receiveSamples' call is iterated for as many times as it
		//   outputs samples.
		int nWritePosition = 0;
		int nReceiveSamples = 0;
		do 
		{
			nReceiveSamples = m_pSoundTouch->receiveSamples(m_pFloatBuffer+nWritePosition, nSamples);
			nSamples -= nReceiveSamples;
			nWritePosition += nReceiveSamples;
		} while (nReceiveSamples != 0);

		// transform a chunk of samples
		if ( FAILED( toSampleType( pBuffer, m_pFloatBuffer, numElems, bytesPerSample ) ) ) 
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CAudioSwitcherFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
	CStreamSwitcherInputPin* pInPin = GetInputPin();
	CStreamSwitcherOutputPin* pOutPin = GetOutputPin();
	if (!pInPin || !pOutPin) {
		return __super::Transform(pIn, pOut);
	}

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)pInPin->CurrentMediaType().pbFormat;
	WAVEFORMATEX* wfeout = (WAVEFORMATEX*)pOutPin->CurrentMediaType().pbFormat;
	WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)wfe;

	int bps = wfe->wBitsPerSample >> 3;


	REFERENCE_TIME rtStart, rtStop;
	if (SUCCEEDED(pIn->GetTime(&rtStart, &rtStop))) 
	{
// 		if (m_bChangePin)//如果切换了inputpin
// 		{
// 			if (m_bUsePin1)//这里尝试了下用保存下来的sample来做处理，但是效果不理想，所以其实现在这里没做什么
// 			{
// 				ChangeMediaSample(m_listMediaSample1,pIn);
// 			}
// 			else
// 			{
// 				ChangeMediaSample(m_listMediaSample2,pIn);
// 			}
// 			m_bChangePin = false;
// 		}
		rtStart += m_rtAudioTimeShift;
		rtStop += m_rtAudioTimeShift;
		pOut->SetTime(&rtStart, &rtStop);

		m_rtNextStart = rtStart;
		m_rtNextStop = rtStop;
	} else {
		pOut->SetTime(&m_rtNextStart, &m_rtNextStop);
	}
	
	int len = pIn->GetActualDataLength() / (bps * wfe->nChannels);//in采样点数量
	int lenout = int((UINT64)len * wfeout->nSamplesPerSec / wfe->nSamplesPerSec);//out采样点数量

	REFERENCE_TIME rtDur = 10000000i64 * len / wfe->nSamplesPerSec;

	m_rtNextStart += rtDur;
	m_rtNextStop += rtDur;

	if (pIn->IsDiscontinuity() == S_OK) {
		m_sample_max = 0.1f;
	}

	WORD tag = wfe->wFormatTag;
	bool fPCM = tag == WAVE_FORMAT_PCM || tag == WAVE_FORMAT_EXTENSIBLE && wfex->SubFormat == KSDATAFORMAT_SUBTYPE_PCM;
	bool fFloat = tag == WAVE_FORMAT_IEEE_FLOAT || tag == WAVE_FORMAT_EXTENSIBLE && wfex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	if (!fPCM && !fFloat) {
		return __super::Transform(pIn, pOut);
	}

	BYTE* pDataIn = NULL;
	BYTE* pDataOut = NULL;

	HRESULT hr;
	if (FAILED(hr = pIn->GetPointer(&pDataIn))) {
		return hr;
	}
	if (FAILED(hr = pOut->GetPointer(&pDataOut))) {
		return hr;
	}

	if (!pDataIn || !pDataOut || len < 0 || lenout < 0) {
		return S_FALSE;
	}
	// len = 0 doesn't mean it's failed, return S_OK otherwise might screw the sound
	if (len == 0) {
		pOut->SetActualDataLength(0);
		return S_OK;
	}

	if (m_fCustomChannelMapping) 
	{
		size_t channelsCount = m_chs[wfe->nChannels - 1].GetCount();
		if (channelsCount > 0 && wfeout->nChannels <= channelsCount) {
			for (int i = 0; i < wfeout->nChannels; i++) {
				DWORD mask = m_chs[wfe->nChannels - 1][i].Channel;

				BYTE* src = pDataIn;
				BYTE* dst = &pDataOut[bps * i];

				int srcstep = bps * wfe->nChannels;
				int dststep = bps * wfeout->nChannels;
				int channels = min(18, wfe->nChannels);
				if (fPCM && wfe->wBitsPerSample == 8) {
					for (int k = 0; k < len; k++, src += srcstep, dst += dststep) {
						mix<unsigned char, INT64, 0, UCHAR_MAX>(mask, channels, bps, src, dst);
					}
				} else if (fPCM && wfe->wBitsPerSample == 16) {
					if (wfe->nChannels != 4 || wfeout->nChannels != 4) {
						for (int k = 0; k < len; k++, src += srcstep, dst += dststep) {
							mix<short, INT64, SHRT_MIN, SHRT_MAX>(mask, channels, bps, src, dst);
						}
					} else { // most popular channels count
						for (int k = 0; k < len; k++, src += srcstep, dst += dststep) {
							mix4<short, INT64, SHRT_MIN, SHRT_MAX>(mask, src, dst);
						}
					}
				} else if (fPCM && wfe->wBitsPerSample == 24) {
					for (int k = 0; k < len; k++, src += srcstep, dst += dststep) {
						mix<int, INT64, INT24_MIN, INT24_MAX>(mask, channels, bps, src, dst);
					}
				} else if (fPCM && wfe->wBitsPerSample == 32) {
					for (int k = 0; k < len; k++, src += srcstep, dst += dststep) {
						mix<int, __int64, INT_MIN, INT_MAX>(mask, channels, bps, src, dst);
					}
				} else if (fFloat && wfe->wBitsPerSample == 32) {
					for (int k = 0; k < len; k++, src += srcstep, dst += dststep) {
						mix < float, double, -1, 1 > (mask, channels, bps, src, dst);
					}
				} else if (fFloat && wfe->wBitsPerSample == 64) {
					for (int k = 0; k < len; k++, src += srcstep, dst += dststep) {
						mix < double, double, -1, 1 > (mask, channels, bps, src, dst);
					}
				}
			}
		}
		else 
		{
			BYTE* pDataOut = NULL;
			HRESULT hr2;
			if (FAILED(hr2 = pOut->GetPointer(&pDataOut)) || !pDataOut) {
				return hr2;
			}
			memset(pDataOut, 0, pOut->GetSize());
		}
	} 
	else 
	{
		HRESULT hr2;
		if (S_OK != (hr2 = __super::Transform(pIn, pOut))) {
			return hr2;
		}
	}

	if (m_fDownSampleTo441
		&& wfe->nSamplesPerSec > 44100 && wfeout->nSamplesPerSec == 44100
		&& wfe->wBitsPerSample <= 16 && fPCM) {
			if (BYTE* buff = DEBUG_NEW BYTE[len * bps]) {
				for (int ch = 0; ch < wfeout->nChannels; ch++) {
					memset(buff, 0, len * bps);

					for (int i = 0; i < len; i++) {
						memcpy(buff + i * bps, (char*)pDataOut + (ch + i * wfeout->nChannels)*bps, bps);
					}

					m_pResamplers[ch]->Downsample(buff, len, buff, lenout);

					for (int i = 0; i < lenout; i++) {
						memcpy((char*)pDataOut + (ch + i * wfeout->nChannels)*bps, buff + i * bps, bps);
					}
				}

				delete [] buff;
			}
	}

	if (m_fNormalize || m_boost_mul > 1) {
		int samples = lenout * wfeout->nChannels;

		if (double* buff = DEBUG_NEW double[samples]) {
			for (int i = 0; i < samples; i++) {
				if (fPCM && wfe->wBitsPerSample == 8) {
					buff[i] = (double)((BYTE*)pDataOut)[i] / UCHAR_MAX;
				} else if (fPCM && wfe->wBitsPerSample == 16) {
					buff[i] = (double)((short*)pDataOut)[i] / SHRT_MAX;
				} else if (fPCM && wfe->wBitsPerSample == 24) {
					int tmp;
					memcpy(((BYTE*)&tmp) + 1, &pDataOut[i * 3], 3);
					buff[i] = (float)(tmp >> 8) / ((1 << 23) - 1);
				} else if (fPCM && wfe->wBitsPerSample == 32) {
					buff[i] = (double)((int*)pDataOut)[i] / INT_MAX;
				} else if (fFloat && wfe->wBitsPerSample == 32) {
					buff[i] = (double)((float*)pDataOut)[i];
				} else if (fFloat && wfe->wBitsPerSample == 64) {
					buff[i] = ((double*)pDataOut)[i];
				}
			}

			double sample_mul = 1;

			if (m_fNormalize) {
				for (int i = 0; i < samples; i++) {
					double s = buff[i];
					if (s < 0) {
						s = -s;
					}
					if (s > 1) {
						s = 1;
					}
					if (m_sample_max < s) {
						m_sample_max = s;
					}
				}

				sample_mul = 1.0f / m_sample_max;

				if (m_fNormalizeRecover) {
					m_sample_max -= 1.0 * rtDur / 200000000; // -5%/sec
				}
				if (m_sample_max < 0.1) {
					m_sample_max = 0.1;
				}
			}

			if (m_boost_mul > 1) {
				sample_mul *= m_boost_mul;
			}

			for (int i = 0; i < samples; i++) {
				double s = buff[i] * sample_mul;

				if (fPCM && wfe->wBitsPerSample == 8) {
					((BYTE*)pDataOut)[i] = clamp<BYTE>(s, 0, UCHAR_MAX);
				} else if (fPCM && wfe->wBitsPerSample == 16) {
					((short*)pDataOut)[i] = clamp<short>(s, SHRT_MIN, SHRT_MAX);
				} else if (fPCM && wfe->wBitsPerSample == 24)  {
					int tmp = clamp<int>(s, -1 << 23, (1 << 23) - 1);
					memcpy(&pDataOut[i * 3], &tmp, 3);
				} else if (fPCM && wfe->wBitsPerSample == 32) {
					((int*)pDataOut)[i] = clamp<int>(s, INT_MIN, INT_MAX);
				} else if (fFloat && wfe->wBitsPerSample == 32) {
					((float*)pDataOut)[i] = clamp<float>(s, -1, +1);
				} else if (fFloat && wfe->wBitsPerSample == 64) {
					((double*)pDataOut)[i] = clamp<double>(s, -1, +1);
				}
			}

			delete [] buff;
		}
	}

	pOut->SetActualDataLength(lenout * bps * wfeout->nChannels);

	return S_OK;
}

CMediaType CAudioSwitcherFilter::CreateNewOutputMediaType(CMediaType mt, long& cbBuffer)
{
	CStreamSwitcherInputPin* pInPin = GetInputPin();
	CStreamSwitcherOutputPin* pOutPin = GetOutputPin();
	if (!pInPin || !pOutPin || ((WAVEFORMATEX*)mt.pbFormat)->wFormatTag == WAVE_FORMAT_DOLBY_AC3_SPDIF) {
		return __super::CreateNewOutputMediaType(mt, cbBuffer);
	}

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)pInPin->CurrentMediaType().pbFormat;

	if (m_fCustomChannelMapping) {
		m_chs[wfe->nChannels - 1].RemoveAll();

		DWORD mask = DWORD((__int64(1) << wfe->nChannels) - 1);
		for (int i = 0; i < 18; i++) {
			if (m_pSpeakerToChannelMap[wfe->nChannels - 1][i]&mask) {
				ChMap cm = {1 << i, m_pSpeakerToChannelMap[wfe->nChannels - 1][i]};
				m_chs[wfe->nChannels - 1].Add(cm);
			}
		}

		if (m_chs[wfe->nChannels - 1].GetCount() > 0) {
			mt.ReallocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));
			WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)mt.pbFormat;
			wfex->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
			wfex->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			wfex->Samples.wValidBitsPerSample = wfe->wBitsPerSample;
			wfex->SubFormat =
				wfe->wFormatTag == WAVE_FORMAT_PCM ? KSDATAFORMAT_SUBTYPE_PCM :
				wfe->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT :
				wfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? ((WAVEFORMATEXTENSIBLE*)wfe)->SubFormat :
				KSDATAFORMAT_SUBTYPE_PCM; // can't happen

			wfex->dwChannelMask = 0;
			for (size_t i = 0; i < m_chs[wfe->nChannels - 1].GetCount(); i++) {
				wfex->dwChannelMask |= m_chs[wfe->nChannels - 1][i].Speaker;
			}

			wfex->Format.nChannels = (WORD)m_chs[wfe->nChannels - 1].GetCount();
			wfex->Format.nBlockAlign = wfex->Format.nChannels * wfex->Format.wBitsPerSample >> 3;
			wfex->Format.nAvgBytesPerSec = wfex->Format.nBlockAlign * wfex->Format.nSamplesPerSec;
		}
	}

	WAVEFORMATEX* wfeout = (WAVEFORMATEX*)mt.pbFormat;

	if (m_fDownSampleTo441) {
		if (wfeout->nSamplesPerSec > 44100 && wfeout->wBitsPerSample <= 16) {
			wfeout->nSamplesPerSec = 44100;
			wfeout->nAvgBytesPerSec = wfeout->nBlockAlign * wfeout->nSamplesPerSec;
		}
	}

	int bps = wfe->wBitsPerSample >> 3;
	int len = cbBuffer / (bps * wfe->nChannels);
	int lenout = len * wfeout->nSamplesPerSec / wfe->nSamplesPerSec;
	cbBuffer = lenout * bps * wfeout->nChannels;

	//  mt.lSampleSize = (ULONG)max(mt.lSampleSize, wfe->nAvgBytesPerSec * rtLen / 10000000i64);
	//  mt.lSampleSize = (mt.lSampleSize + (wfe->nBlockAlign-1)) & ~(wfe->nBlockAlign-1);

	return mt;
}

void CAudioSwitcherFilter::OnNewOutputMediaType(const CMediaType& mtIn, const CMediaType& mtOut)
{
	const WAVEFORMATEX* wfe = (WAVEFORMATEX*)mtIn.pbFormat;
	const WAVEFORMATEX* wfeout = (WAVEFORMATEX*)mtOut.pbFormat;

	m_pResamplers.RemoveAll();
	for (int i = 0; i < wfeout->nChannels; i++) {
		CAutoPtr<AudioStreamResampler> pResampler;
		pResampler.Attach(DEBUG_NEW AudioStreamResampler(wfeout->wBitsPerSample >> 3, wfe->nSamplesPerSec, wfeout->nSamplesPerSec, true));
		m_pResamplers.Add(pResampler);
	}

	TRACE(_T("CAudioSwitcherFilter::OnNewOutputMediaType\n"));
	m_sample_max = 0.1f;
}

HRESULT CAudioSwitcherFilter::DeliverEndFlush()
{
	m_sample_max = 0.1f;
	return __super::DeliverEndFlush();
}

HRESULT CAudioSwitcherFilter::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	m_sample_max = 0.1f;
	return __super::DeliverNewSegment(tStart, tStop, dRate);
}

// IAudioSwitcherFilter

STDMETHODIMP CAudioSwitcherFilter::GetInputSpeakerConfig(DWORD* pdwChannelMask)
{
	if (!pdwChannelMask) {
		return E_POINTER;
	}

	*pdwChannelMask = 0;

	CStreamSwitcherInputPin* pInPin = GetInputPin();
	if (!pInPin || !pInPin->IsConnected()) {
		return E_UNEXPECTED;
	}

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)pInPin->CurrentMediaType().pbFormat;

	if (wfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)wfe;
		*pdwChannelMask = wfex->dwChannelMask;
	} else {
		*pdwChannelMask = 0/*wfe->nChannels == 1 ? 4 : wfe->nChannels == 2 ? 3 : 0*/;
	}

	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::GetSpeakerConfig(bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18])
{
	if (pfCustomChannelMapping) {
		*pfCustomChannelMapping = m_fCustomChannelMapping;
	}
	memcpy(pSpeakerToChannelMap, m_pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap));

	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::SetSpeakerConfig(bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18])
{
	if (m_State == State_Stopped
		|| m_fCustomChannelMapping != fCustomChannelMapping
		|| memcmp(m_pSpeakerToChannelMap, pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap))) {
			PauseGraph;

			CStreamSwitcherInputPin* pInput = GetInputPin();

			SelectInput(NULL);

			m_fCustomChannelMapping = fCustomChannelMapping;
			memcpy(m_pSpeakerToChannelMap, pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap));

			SelectInput(pInput);

			ResumeGraph;
	}

	return S_OK;
}

STDMETHODIMP_(int) CAudioSwitcherFilter::GetNumberOfInputChannels()
{
	CStreamSwitcherInputPin* pInPin = GetInputPin();
	return pInPin ? ((WAVEFORMATEX*)pInPin->CurrentMediaType().pbFormat)->nChannels : 0;
}

STDMETHODIMP_(bool) CAudioSwitcherFilter::IsDownSamplingTo441Enabled()
{
	return m_fDownSampleTo441;
}

STDMETHODIMP CAudioSwitcherFilter::EnableDownSamplingTo441(bool fEnable)
{
	if (m_fDownSampleTo441 != fEnable) {
		PauseGraph;
		m_fDownSampleTo441 = fEnable;
		ResumeGraph;
	}

	return S_OK;
}

STDMETHODIMP_(REFERENCE_TIME) CAudioSwitcherFilter::GetAudioTimeShift()
{
	return m_rtAudioTimeShift;
}

STDMETHODIMP CAudioSwitcherFilter::SetAudioTimeShift(REFERENCE_TIME rtAudioTimeShift)
{
	m_rtAudioTimeShift = rtAudioTimeShift;
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::GetNormalizeBoost(bool& fNormalize, bool& fNormalizeRecover, float& boost_dB)
{
	fNormalize = m_fNormalize;
	fNormalizeRecover = m_fNormalizeRecover;
	boost_dB = 20 * log10(m_boost_mul);
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::SetNormalizeBoost(bool fNormalize, bool fNormalizeRecover, float boost_dB)
{
	if (m_fNormalize != fNormalize) {
		m_sample_max = 0.1f;
	}
	m_fNormalize = fNormalize;
	m_fNormalizeRecover = fNormalizeRecover;
	m_boost_mul = pow(10.0f, boost_dB / 20);
	return S_OK;
}

// IAMStreamSelect

STDMETHODIMP CAudioSwitcherFilter::Enable(long lIndex, DWORD dwFlags)
{
	HRESULT hr = __super::Enable(lIndex, dwFlags);
	if (S_OK == hr) {
		m_sample_max = 0.1f;
	}
	return hr;
}

STDMETHODIMP CAudioSwitcherFilter::ChangeCurPitch(INT nPitch)
{
	if ( m_nPitch != nPitch ) 
	{
		m_nPitch = nPitch;
		CAutoLock lock(&m_csPitchChange);
		m_bNeedChangePitch = true;
	}
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::SwitchSTrack(eSoundTrack eType)
{
	__super::ChangeSoundTrack(eType);
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::GetSTrackCopy(BOOL* bCopy)
{
	CheckPointer(bCopy,E_POINTER);
	*bCopy = m_bCopySTrack;
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::SetSTrackCopy(BOOL bCopy)
{
	m_bCopySTrack = bCopy;
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::SwitchATrack(BOOL bFirstAudio)
{
	if (!m_pInput2->IsConnected() || !m_pInput1->IsConnected())
	{
		return E_FAIL;
	}
	CAutoLock lock(&m_csInputPin);
	m_bUsePin1 = bFirstAudio;
	m_bChangePin = true;
	m_bBeginFlush = true;
	m_bEndFlush = true;

	REFERENCE_TIME rtNewStart = 0;
	IMediaSeeking* pMS = NULL;
	if (false/*m_pGraph*/)
	{
		HRESULT hr = m_pGraph->QueryInterface(IID_IMediaSeeking,(void**)&pMS);
		if (SUCCEEDED(hr))
		{
			hr = pMS->GetCurrentPosition(&(m_rtNewStart));
			rtNewStart = m_rtNewStart/*-m_rtSampleDelta*/;
			if (SUCCEEDED(hr))
			{
				char buf[260] = {0};
				sprintf(buf,"rtNewStart %lld m_rtSampleDelta %lld\n",rtNewStart,m_rtSampleDelta);
				rtNewStart /*+= m_rtSampleDelta*/;
				OutputDebugStringA(buf);
				hr = pMS->SetPositions(&rtNewStart,AM_SEEKING_AbsolutePositioning,NULL,AM_SEEKING_NoPositioning);
			}
		}
	}
	SAFE_RELEASE(pMS);
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL) return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(pPages->cElems*sizeof(GUID));
	if (pPages->pElems == NULL) 
	{
		return E_OUTOFMEMORY;
	}
	pPages->pElems[0] = __uuidof(CGrayProp);
	return S_OK;
}



