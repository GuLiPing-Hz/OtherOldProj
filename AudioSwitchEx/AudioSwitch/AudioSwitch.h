#ifndef AUDIOSWITCH__H__
#define AUDIOSWITCH__H__
#include "util.h"
#include <InitGuid.h>
#include "StreamSwitch.h"

#define AudioSwitcherName L"TAudioSwitcher"

interface __declspec(uuid("CEDB2890-53AE-4231-91A3-B0AAFCD1DBDE"))
IAudioSwitcherFilter :
public IUnknown {
	STDMETHOD(GetInputSpeakerConfig)(DWORD * pdwChannelMask) = 0;
	STDMETHOD(GetSpeakerConfig)(bool * pfCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]) = 0;
	STDMETHOD(SetSpeakerConfig)(bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]) = 0;
	STDMETHOD_(int, GetNumberOfInputChannels)() = 0;
	STDMETHOD_(bool, IsDownSamplingTo441Enabled)() = 0;
	STDMETHOD(EnableDownSamplingTo441)(bool fEnable) = 0;
	STDMETHOD_(REFERENCE_TIME, GetAudioTimeShift)() = 0;
	STDMETHOD(SetAudioTimeShift)(REFERENCE_TIME rtAudioTimeShift) = 0;
	STDMETHOD(GetNormalizeBoost)(bool & fNormalize, bool & fNormalizeRecover, float & boost) = 0;
	STDMETHOD(SetNormalizeBoost)(bool fNormalize, bool fNormalizeRecover, float boost) = 0;
};

class AudioStreamResampler;
namespace soundtouch
{
	class SoundTouch;
}

class __declspec(uuid("4AC6D67F-AA1E-4b02-873C-C07C3F0E817F"))
CAudioSwitcherFilter : public CStreamSwitcherFilter, public IAudioSwitcherFilter /*, public IOS_Saturation*/
					   , public ISpecifyPropertyPages , public IOS_AudioSwitch, public IOS_ChangePitch
{
	typedef struct {
		DWORD Speaker, Channel;
	} ChMap;
	CAtlArray<ChMap> m_chs[18];

	bool m_fCustomChannelMapping;
	DWORD m_pSpeakerToChannelMap[18][18];
	bool m_fDownSampleTo441;
	REFERENCE_TIME m_rtAudioTimeShift;
	CAutoPtrArray<AudioStreamResampler> m_pResamplers;
	double m_sample_max;
	bool m_fNormalize, m_fNormalizeRecover;
	float m_boost_mul;

	REFERENCE_TIME m_rtNextStart, m_rtNextStop;
	//////////////////////////////////////////////////////////////////////////add by glp
private:
	CCritSec									m_csShared;
	//定义属性页获取，操作的值
	long											m_lSaturation;
	//change pitch
	bool											m_bInitSoundPitch;
	//当前的音调值
	int											m_nPitch;
	soundtouch::SoundTouch *		m_pSoundTouch;
	int											m_nNumElems;
	float *										m_pFloatBuffer;
	bool											m_bNeedChangePitch;
	CCritSec									m_csPitchChange;
	//////////////////////////////////////////////////////////////////////////
protected:
	HRESULT SetupSoundPitch(soundtouch::SoundTouch *pSoundTouch, WAVEFORMATEX * wfeout );
public:
	CAudioSwitcherFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CAudioSwitcherFilter();

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	int							ChangeMediaSample(LISTMEDIASAMPLE list_MS,IMediaSample* &pIn);
	HRESULT					CheckMediaType(const CMediaType* pmt);
	HRESULT					Transform(IMediaSample* pIn, IMediaSample* pOut);
	virtual HRESULT		ChangePitch(IMediaSample* pOut);
	CMediaType			CreateNewOutputMediaType(CMediaType mt, long& cbBuffer);
	void							OnNewOutputMediaType(const CMediaType& mtIn, const CMediaType& mtOut);

	HRESULT DeliverEndFlush();
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	// IAudioSwitcherFilter
	STDMETHODIMP GetInputSpeakerConfig(DWORD* pdwChannelMask);
	STDMETHODIMP GetSpeakerConfig(bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]);
	STDMETHODIMP SetSpeakerConfig(bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]);
	STDMETHODIMP_(int) GetNumberOfInputChannels();
	STDMETHODIMP_(bool) IsDownSamplingTo441Enabled();
	STDMETHODIMP EnableDownSamplingTo441(bool fEnable);
	STDMETHODIMP_(REFERENCE_TIME) GetAudioTimeShift();
	STDMETHODIMP SetAudioTimeShift(REFERENCE_TIME rtAudioTimeShift);
	STDMETHODIMP GetNormalizeBoost(bool& fNormalize, bool& fNormalizeRecover, float& boost);
	STDMETHODIMP SetNormalizeBoost(bool fNormalize, bool fNormalizeRecover, float boost);

	// IAMStreamSelect
	STDMETHODIMP Enable(long lIndex, DWORD dwFlags);

	//ISpecifyPropertyPages add by glp
	//用于属性页的显示，保存classid
	STDMETHODIMP GetPages(CAUUID *pPages);
// 	//IOS_Saturation用于graphedit 
// 	STDMETHODIMP SetSaturation(long lSat);
// 	STDMETHODIMP GetSaturation(long *plSat);
	//IOS_AudioSwitch
	STDMETHODIMP SwitchATrack(BOOL bFirstAudio);
	STDMETHODIMP SetSTrackCopy(BOOL bCopy);
	STDMETHODIMP GetSTrackCopy(BOOL* bCopy);
	STDMETHODIMP SwitchSTrack(eSoundTrack eType);
	//IOS_CHANGEPITCH
	STDMETHODIMP ChangeCurPitch(INT nPitch);
};
#endif//AUDIOSWITCH__H__
