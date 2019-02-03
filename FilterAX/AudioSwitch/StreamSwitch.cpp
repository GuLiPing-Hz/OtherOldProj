/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "StreamSwitch.h"
#include <InitGuid.h>
#include "moreguids.h"

int g_nDebug = 10;

#ifdef STANDALONE_FILTER
#include <InitGuid.h>
#endif
// #include "moreuuids.h"

#define BLOCKSTREAM

//
// CStreamSwitcherPassThru
//

CStreamSwitcherPassThru::CStreamSwitcherPassThru(LPUNKNOWN pUnk, HRESULT* phr, CStreamSwitcherFilter* pFilter)
    : CMediaPosition(NAME("CStreamSwitcherPassThru"), pUnk)
    , m_pFilter(pFilter)
{
}

STDMETHODIMP CStreamSwitcherPassThru::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);
    *ppv = NULL;

    return
        QI(IMediaSeeking)
        CMediaPosition::NonDelegatingQueryInterface(riid, ppv);
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


#define CallPeerSeeking(call)                               \
    CComPtr<IMediaSeeking> pMS;                             \
    if (FAILED(GetPeer(m_pFilter, &pMS)))                   \
        return E_NOTIMPL;                                   \
    return pMS->##call;

#define CallPeer(call)                                      \
    CComPtr<IMediaPosition> pMP;                            \
    if (FAILED(GetPeer(m_pFilter, &pMP)))                   \
        return E_NOTIMPL;                                   \
    return pMP->##call;

#define CallPeerSeekingAll(call)                            \
    HRESULT hr = E_NOTIMPL;                                 \
    POSITION pos = m_pFilter->m_pInputs.GetHeadPosition();  \
    while (pos)                                             \
    {                                                       \
        CBasePin* pPin = m_pFilter->m_pInputs.GetNext(pos); \
        CComPtr<IPin> pConnected;                           \
        if (FAILED(pPin->ConnectedTo(&pConnected)))         \
            continue;                                       \
        if (CComQIPtr<IMediaSeeking> pMS = pConnected)      \
        {                                                   \
            HRESULT hr2 = pMS->call;                        \
            if (pPin == m_pFilter->GetInputPin())           \
                hr = hr2;                                   \
        }                                                   \
    }                                                       \
    return hr;

#define CallPeerAll(call)                                   \
    HRESULT hr = E_NOTIMPL;                                 \
    POSITION pos = m_pFilter->m_pInputs.GetHeadPosition();  \
    while (pos)                                             \
    {                                                       \
        CBasePin* pPin = m_pFilter->m_pInputs.GetNext(pos); \
        CComPtr<IPin> pConnected;                           \
        if (FAILED(pPin->ConnectedTo(&pConnected)))         \
            continue;                                       \
        if (CComQIPtr<IMediaPosition> pMP = pConnected)     \
        {                                                   \
            HRESULT hr2 = pMP->call;                        \
            if (pPin == m_pFilter->GetInputPin())           \
                hr = hr2;                                   \
        }                                                   \
    }                                                       \
    return hr;


// IMediaSeeking

STDMETHODIMP CStreamSwitcherPassThru::GetCapabilities(DWORD* pCaps)
{
    CallPeerSeeking(GetCapabilities(pCaps));
}

STDMETHODIMP CStreamSwitcherPassThru::CheckCapabilities(DWORD* pCaps)
{
    CallPeerSeeking(CheckCapabilities(pCaps));
}

STDMETHODIMP CStreamSwitcherPassThru::IsFormatSupported(const GUID* pFormat)
{
    CallPeerSeeking(IsFormatSupported(pFormat));
}

STDMETHODIMP CStreamSwitcherPassThru::QueryPreferredFormat(GUID* pFormat)
{
    CallPeerSeeking(QueryPreferredFormat(pFormat));
}

STDMETHODIMP CStreamSwitcherPassThru::SetTimeFormat(const GUID* pFormat)
{
    CallPeerSeeking(SetTimeFormat(pFormat));
}

STDMETHODIMP CStreamSwitcherPassThru::GetTimeFormat(GUID* pFormat)
{
    CallPeerSeeking(GetTimeFormat(pFormat));
}

STDMETHODIMP CStreamSwitcherPassThru::IsUsingTimeFormat(const GUID* pFormat)
{
    CallPeerSeeking(IsUsingTimeFormat(pFormat));
}

STDMETHODIMP CStreamSwitcherPassThru::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
    CallPeerSeeking(ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat));
}

STDMETHODIMP CStreamSwitcherPassThru::SetPositions(LONGLONG* pCurrent, DWORD CurrentFlags, LONGLONG* pStop, DWORD StopFlags)
{
	DWORD StopPosBits = StopFlags & AM_SEEKING_PositioningBitsMask;
	DWORD StartPosBits = CurrentFlags & AM_SEEKING_PositioningBitsMask;

	if(StopFlags) 
	{
		CheckPointer(pStop, E_POINTER);

		// accept only relative, incremental, or absolute positioning
		if(StopPosBits != StopFlags) 
		{
			return E_INVALIDARG;
		}
	}

	if(CurrentFlags) {
		CheckPointer(pCurrent, E_POINTER);
		if(StartPosBits != AM_SEEKING_AbsolutePositioning &&
			StartPosBits != AM_SEEKING_RelativePositioning)
		{
				return E_INVALIDARG;
		}
	}

	// scope for autolock
	{
		CAutoLock lock(m_pFilter->m_pLock);

		// set start position
		if(StartPosBits == AM_SEEKING_AbsolutePositioning)
		{
			m_pFilter->m_rtStart = *pCurrent;
		}
		else if(StartPosBits == AM_SEEKING_RelativePositioning)
		{
			m_pFilter->m_rtStart += *pCurrent;
		}

		// set stop position
		if(StopPosBits == AM_SEEKING_AbsolutePositioning)
		{
			m_pFilter->m_rtStop = *pStop;
		}
		else if(StopPosBits == AM_SEEKING_IncrementalPositioning)
		{
			m_pFilter->m_rtStop = m_pFilter->m_rtStart + *pStop;
		}
		else if(StopPosBits == AM_SEEKING_RelativePositioning)
		{
			m_pFilter->m_rtStop = m_pFilter->m_rtStop + *pStop;
		}
	}

    CallPeerSeekingAll(SetPositions(pCurrent, CurrentFlags, pStop, StopFlags));
}

STDMETHODIMP CStreamSwitcherPassThru::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
    CallPeerSeeking(GetPositions(pCurrent, pStop));
}

STDMETHODIMP CStreamSwitcherPassThru::GetCurrentPosition(LONGLONG* pCurrent)
{
    CallPeerSeeking(GetCurrentPosition(pCurrent));
}

STDMETHODIMP CStreamSwitcherPassThru::GetStopPosition(LONGLONG* pStop)
{
    CallPeerSeeking(GetStopPosition(pStop));
}

STDMETHODIMP CStreamSwitcherPassThru::GetDuration(LONGLONG* pDuration)
{
    CallPeerSeeking(GetDuration(pDuration));
}

STDMETHODIMP CStreamSwitcherPassThru::GetPreroll(LONGLONG* pllPreroll)
{
    CallPeerSeeking(GetPreroll(pllPreroll));
}

STDMETHODIMP CStreamSwitcherPassThru::GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest)
{
    CallPeerSeeking(GetAvailable(pEarliest, pLatest));
}

STDMETHODIMP CStreamSwitcherPassThru::GetRate(double* pdRate)
{
    CallPeerSeeking(GetRate(pdRate));
}

STDMETHODIMP CStreamSwitcherPassThru::SetRate(double dRate)
{
    if (0.0 == dRate) {
        return E_INVALIDARG;
    }
    CallPeerSeekingAll(SetRate(dRate));
}

// IMediaPosition

STDMETHODIMP CStreamSwitcherPassThru::get_Duration(REFTIME* plength)
{
    CallPeer(get_Duration(plength));
}

STDMETHODIMP CStreamSwitcherPassThru::get_CurrentPosition(REFTIME* pllTime)
{
    CallPeer(get_CurrentPosition(pllTime));
}

STDMETHODIMP CStreamSwitcherPassThru::put_CurrentPosition(REFTIME llTime)
{
    CallPeerAll(put_CurrentPosition(llTime));
}

STDMETHODIMP CStreamSwitcherPassThru::get_StopTime(REFTIME* pllTime)
{
    CallPeer(get_StopTime(pllTime));
}

STDMETHODIMP CStreamSwitcherPassThru::put_StopTime(REFTIME llTime)
{
    CallPeerAll(put_StopTime(llTime));
}

STDMETHODIMP CStreamSwitcherPassThru::get_PrerollTime(REFTIME* pllTime)
{
    CallPeer(get_PrerollTime(pllTime));
}

STDMETHODIMP CStreamSwitcherPassThru::put_PrerollTime(REFTIME llTime)
{
    CallPeerAll(put_PrerollTime(llTime));
}

STDMETHODIMP CStreamSwitcherPassThru::get_Rate(double* pdRate)
{
    CallPeer(get_Rate(pdRate));
}

STDMETHODIMP CStreamSwitcherPassThru::put_Rate(double dRate)
{
    if (0.0 == dRate) {
        return E_INVALIDARG;
    }
    CallPeerAll(put_Rate(dRate));
}

STDMETHODIMP CStreamSwitcherPassThru::CanSeekForward(LONG* pCanSeekForward)
{
    CallPeer(CanSeekForward(pCanSeekForward));
}

STDMETHODIMP CStreamSwitcherPassThru::CanSeekBackward(LONG* pCanSeekBackward)
{
    CallPeer(CanSeekBackward(pCanSeekBackward));
}

//
// CStreamSwitcherAllocator
//

CStreamSwitcherAllocator::CStreamSwitcherAllocator(CStreamSwitcherInputPin* pPin, HRESULT* phr)
    : CMemAllocator(NAME("CStreamSwitcherAllocator"), NULL, phr)
    , m_pPin(pPin)
    , m_fMediaTypeChanged(false)
{
    ASSERT(phr);
    ASSERT(pPin);
}

#ifdef _DEBUG
CStreamSwitcherAllocator::~CStreamSwitcherAllocator()
{
    ASSERT(m_bCommitted == FALSE);
}
#endif

STDMETHODIMP_(ULONG) CStreamSwitcherAllocator::NonDelegatingAddRef()
{
    return m_pPin->m_pFilter->AddRef();
}

STDMETHODIMP_(ULONG) CStreamSwitcherAllocator::NonDelegatingRelease()
{
    return m_pPin->m_pFilter->Release();
}

STDMETHODIMP CStreamSwitcherAllocator::GetBuffer(
    IMediaSample** ppBuffer,
    REFERENCE_TIME* pStartTime, REFERENCE_TIME* pEndTime,
    DWORD dwFlags)
{
    HRESULT hr = VFW_E_NOT_COMMITTED;

    if (!m_bCommitted) {
        return hr;
    }
    /*
    TRACE(_T("CStreamSwitcherAllocator::GetBuffer m_pPin->m_evBlock.Wait() + %x\n"), this);
        m_pPin->m_evBlock.Wait();
    TRACE(_T("CStreamSwitcherAllocator::GetBuffer m_pPin-> .Wait() - %x\n"), this);
    */
    if (m_fMediaTypeChanged) {
        if (!m_pPin || !m_pPin->m_pFilter) {
            return hr;
        }

        CStreamSwitcherOutputPin* pOut = (static_cast<CStreamSwitcherFilter*>(m_pPin->m_pFilter))->GetOutputPin();
        if (!pOut || !pOut->CurrentAllocator()) {
            return hr;
        }

        ALLOCATOR_PROPERTIES Properties, Actual;
        if (FAILED(pOut->CurrentAllocator()->GetProperties(&Actual))) {
            return hr;
        }
        if (FAILED(GetProperties(&Properties))) {
            return hr;
        }

        if (!m_bCommitted || Properties.cbBuffer < Actual.cbBuffer) {
            Properties.cbBuffer = Actual.cbBuffer;
            if (FAILED(Decommit())) {
                return hr;
            }
            if (FAILED(SetProperties(&Properties, &Actual))) {
                return hr;
            }
            if (FAILED(Commit())) {
                return hr;
            }
            ASSERT(Actual.cbBuffer >= Properties.cbBuffer);
            if (Actual.cbBuffer < Properties.cbBuffer) {
                return hr;
            }
        }
    }

    hr = CMemAllocator::GetBuffer(ppBuffer, pStartTime, pEndTime, dwFlags);

    if (m_fMediaTypeChanged && SUCCEEDED(hr)) {
        (*ppBuffer)->SetMediaType(&m_mt);
        m_fMediaTypeChanged = false;
    }

    return hr;
}

void CStreamSwitcherAllocator::NotifyMediaType(const CMediaType& mt)
{
    CopyMediaType(&m_mt, &mt);
    m_fMediaTypeChanged = true;
}


//
// CStreamSwitcherInputPin
//

CStreamSwitcherInputPin::CStreamSwitcherInputPin(CStreamSwitcherFilter* pFilter, HRESULT* phr, LPCWSTR pName)
    : CBaseInputPin(NAME("CStreamSwitcherInputPin"), pFilter, &pFilter->m_csState, phr, pName)
    , m_Allocator(this, phr)
    , m_bSampleSkipped(FALSE)
    , m_bQualityChanged(FALSE)
    , m_bUsingOwnAllocator(FALSE)
    , m_evBlock(TRUE)
    , m_fCanBlock(false)
    , m_hNotifyEvent(NULL)
	,m_bRunning(true)
	,m_iGS(NULL)
	,m_bSwitchInit(false)
{
    m_bCanReconnectWhenActive = TRUE;
}

CStreamSwitcherInputPin::~CStreamSwitcherInputPin()
{
	SAFE_RELEASE(m_iGS);
}

class __declspec(uuid("138130AF-A79B-45D5-B4AA-87697457BA87"))
        NeroAudioDecoder {};

STDMETHODIMP CStreamSwitcherInputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(IStreamSwitcherInputPin)
        IsConnected() && GetCLSID(GetFilterFromPin(GetConnected())) == __uuidof(NeroAudioDecoder) && QI(IPinConnection)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// IPinConnection

STDMETHODIMP CStreamSwitcherInputPin::DynamicQueryAccept(const AM_MEDIA_TYPE* pmt)
{
    return QueryAccept(pmt);
}

STDMETHODIMP CStreamSwitcherInputPin::NotifyEndOfStream(HANDLE hNotifyEvent)
{
    if (m_hNotifyEvent) {
        SetEvent(m_hNotifyEvent);
    }
    m_hNotifyEvent = hNotifyEvent;
    return S_OK;
}

STDMETHODIMP CStreamSwitcherInputPin::IsEndPin()
{
    return S_OK;
}

STDMETHODIMP CStreamSwitcherInputPin::DynamicDisconnect()
{
    CAutoLock cAutoLock(&m_csReceive);
    Disconnect();
    return S_OK;
}

// IStreamSwitcherInputPin

STDMETHODIMP_(bool) CStreamSwitcherInputPin::IsActive()
{
    return (this == (static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetInputPin());
}

//

HRESULT CStreamSwitcherInputPin::QueryAcceptDownstream(const AM_MEDIA_TYPE* pmt)
{
    HRESULT hr = S_OK;

    CStreamSwitcherOutputPin* pOut = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetOutputPin();

    if (pOut && pOut->IsConnected()) {
        if (CComPtr<IPinConnection> pPC = pOut->CurrentPinConnection()) {
            hr = pPC->DynamicQueryAccept(pmt);
            if (hr == S_OK) {
                return S_OK;
            }
        }

        hr = pOut->GetConnected()->QueryAccept(pmt);
    }

    return hr;
}

bool CStreamSwitcherInputPin::FindMyInterface(REFIID riid,void** ppObj)
{
	CComPtr<IPin> pPin = this;
	HRESULT hr;
	do 
	{
		CComPtr<IPin> pConneted;
		hr = pPin->ConnectedTo(&pConneted);
		if (FAILED(hr))
		{
			return false;
		}
		hr = pConneted->QueryInterface(riid,(void**)ppObj);
		if (SUCCEEDED(hr))
		{
			return true;
		}

		PIN_INFO pin_info;
		hr = pConneted->QueryPinInfo(&pin_info);
		if (FAILED(hr))
		{
			return false;
		}
		pPin = GetFirstPin(pin_info.pFilter);
		SAFE_RELEASE(pin_info.pFilter);
	} while (pPin);

	return false;
}

bool CStreamSwitcherInputPin::FindPosInterface()
{
	SAFE_RELEASE(m_iGS);
	if (!IsConnected())
	{
		return false;
	}

	return FindMyInterface(__uuidof(IGSeekPosition),(void**)&m_iGS);
}

void CStreamSwitcherInputPin::Block(bool fBlock)
{
    if (fBlock) {
        m_evBlock.Reset();
    } else {
        m_evBlock.Set();
    }
}

HRESULT CStreamSwitcherInputPin::InitializeOutputSample(IMediaSample* pInSample, IMediaSample** ppOutSample)
{
    if (!pInSample || !ppOutSample) {
        return E_POINTER;
    }

    CStreamSwitcherOutputPin* pOut = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetOutputPin();
    ASSERT(pOut->GetConnected());

    CComPtr<IMediaSample> pOutSample;

    DWORD dwFlags = m_bSampleSkipped ? AM_GBF_PREVFRAMESKIPPED : 0;

    if (!(m_SampleProps.dwSampleFlags & AM_SAMPLE_SPLICEPOINT)) {
        dwFlags |= AM_GBF_NOTASYNCPOINT;
    }

    HRESULT hr = pOut->GetDeliveryBuffer(&pOutSample
                                         , m_SampleProps.dwSampleFlags & AM_SAMPLE_TIMEVALID ? &m_SampleProps.tStart : NULL
                                         , m_SampleProps.dwSampleFlags & AM_SAMPLE_STOPVALID ? &m_SampleProps.tStop : NULL
                                         , dwFlags);

    if (FAILED(hr)) {
        return hr;
    }

    if (!pOutSample) {
        return E_FAIL;
    }

    if (CComQIPtr<IMediaSample2> pOutSample2 = pOutSample) {
        AM_SAMPLE2_PROPERTIES OutProps;
        EXECUTE_ASSERT(SUCCEEDED(pOutSample2->GetProperties(FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, tStart), (PBYTE)&OutProps)));
        OutProps.dwTypeSpecificFlags = m_SampleProps.dwTypeSpecificFlags;
        OutProps.dwSampleFlags =
            (OutProps.dwSampleFlags & AM_SAMPLE_TYPECHANGED) |
            (m_SampleProps.dwSampleFlags & ~AM_SAMPLE_TYPECHANGED);

        OutProps.tStart = m_SampleProps.tStart;
        OutProps.tStop  = m_SampleProps.tStop;
        OutProps.cbData = FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId);

        hr = pOutSample2->SetProperties(FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId), (PBYTE)&OutProps);
        if (m_SampleProps.dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY) {
            m_bSampleSkipped = FALSE;
        }
    } else {
        if (m_SampleProps.dwSampleFlags & AM_SAMPLE_TIMEVALID) {
            pOutSample->SetTime(&m_SampleProps.tStart, &m_SampleProps.tStop);
        }

        if (m_SampleProps.dwSampleFlags & AM_SAMPLE_SPLICEPOINT) {
            pOutSample->SetSyncPoint(TRUE);
        }

        if (m_SampleProps.dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY) {
            pOutSample->SetDiscontinuity(TRUE);
            m_bSampleSkipped = FALSE;
        }

        LONGLONG MediaStart, MediaEnd;
        if (pInSample->GetMediaTime(&MediaStart, &MediaEnd) == NOERROR) {
            pOutSample->SetMediaTime(&MediaStart, &MediaEnd);
        }
    }

    *ppOutSample = pOutSample.Detach();

    return S_OK;
}

// pure virtual

HRESULT CStreamSwitcherInputPin::CheckMediaType(const CMediaType* pmt)
{
	CStreamSwitcherFilter* pSSFilter = static_cast<CStreamSwitcherFilter*>(m_pFilter);
    HRESULT hr =  pSSFilter->CheckMediaType(pmt);

// 	if (SUCCEEDED(hr))
// 	{
// 		WAVEFORMATEX* pWF = NULL;
// 		if (pmt->formattype == FORMAT_WaveFormatEx && pmt->cbFormat >= sizeof(WAVEFORMATEX))
// 		{
// 			pWF = (WAVEFORMATEX*)pmt->pbFormat;
// 			if (pWF->wFormatTag == WAVE_FORMAT_PCM)
// 			{
// 				m_wave.BuildFormat(pWF->nChannels,pWF->nSamplesPerSec,pWF->wBitsPerSample);
// 				pSSFilter->m_wave.BuildFormat(pWF->nChannels,pWF->nSamplesPerSec,pWF->wBitsPerSample);
// 			}
// 		}
// 	}

	return hr;
}

// virtual

HRESULT CStreamSwitcherInputPin::CheckConnect(IPin* pPin)
{
    return (IPin*)(static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetOutputPin() == pPin
           ? E_FAIL
           : __super::CheckConnect(pPin);
}

HRESULT CStreamSwitcherInputPin::BreakConnect()
{
	m_bSwitchInit = false;
	SAFE_RELEASE(m_iGS);
	return __super::BreakConnect();
}

HRESULT CStreamSwitcherInputPin::CompleteConnect(IPin* pReceivePin)
{
    HRESULT hr = __super::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

    (static_cast<CStreamSwitcherFilter*>(m_pFilter))->CompleteConnect(PINDIR_INPUT, this, pReceivePin);

	if (!m_bSwitchInit)
	{
		m_bSwitchInit = FindPosInterface();
	}

    m_fCanBlock = false;
    bool fForkedSomewhere = false;

    CStringW fileName;
    CStringW pinName;

    IPin* pPin = (IPin*)this;
    IBaseFilter* pBF = (IBaseFilter*)m_pFilter;

    pPin = GetUpStreamPin(pBF, pPin);
    if (pPin)
	{
        pBF = GetFilterFromPin(pPin);
    }
    while (pPin && pBF) 
	{
        if (IsSplitter(pBF)) 
		{
            pinName = GetPinName(pPin);
        }

        CLSID clsid = GetCLSID(pBF);
        if (clsid == CLSID_AviSplitter || clsid == CLSID_OggSplitter) 
		{
            m_fCanBlock = true;
        }

        int nIn, nOut, nInC, nOutC;
        CountPins(pBF, nIn, nOut, nInC, nOutC);
        fForkedSomewhere = fForkedSomewhere || nIn > 1 || nOut > 1;

        pPin = GetFirstPin(pBF);

        pPin = GetUpStreamPin(pBF, pPin);
        if (pPin)
		{
            pBF = GetFilterFromPin(pPin);
        }
    }

    if (!fForkedSomewhere) {
        m_fCanBlock = true;
    }

    m_hNotifyEvent = NULL;

    return S_OK;
}

HRESULT CStreamSwitcherInputPin::Active()
{
	m_bRunning = true;
	//OutputDebugStringA(m_bRunning?"Active Running\n":"Active Stopping\n");
	(static_cast<CStreamSwitcherFilter*>(m_pFilter))->m_rtNewStart = 0;
    return __super::Active();
}

HRESULT CStreamSwitcherInputPin::Inactive()
{
	m_bRunning = false;
	//OutputDebugStringA(m_bRunning?"Inactive Running\n":"Inactive Stopping\n");
	HRESULT hr = __super::Inactive();
	(static_cast<CStreamSwitcherFilter*>(m_pFilter))->ClearList();
    return hr;
}

// IPin

STDMETHODIMP CStreamSwitcherInputPin::QueryAccept(const AM_MEDIA_TYPE* pmt)
{
    HRESULT hr = __super::QueryAccept(pmt);
    if (S_OK != hr) {
        return hr;
    }

    return QueryAcceptDownstream(pmt);
}

STDMETHODIMP CStreamSwitcherInputPin::ReceiveConnection(IPin* pConnector, const AM_MEDIA_TYPE* pmt)
{
    // FIXME: this locked up once
    //    CAutoLock cAutoLock(&((CStreamSwitcherFilter*)m_pFilter)->m_csReceive);

    HRESULT hr;
    if (S_OK != (hr = QueryAcceptDownstream(pmt))) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if (m_Connected && m_Connected != pConnector) {
        return VFW_E_ALREADY_CONNECTED;
    }

    SAFE_RELEASE(m_Connected);

    return SUCCEEDED(__super::ReceiveConnection(pConnector, pmt)) ? S_OK : E_FAIL;
}

STDMETHODIMP CStreamSwitcherInputPin::GetAllocator(IMemAllocator** ppAllocator)
{
    CheckPointer(ppAllocator, E_POINTER);

    if (m_pAllocator == NULL) {
        (m_pAllocator = &m_Allocator)->AddRef();
    }

    (*ppAllocator = m_pAllocator)->AddRef();

    return NOERROR;
}

STDMETHODIMP CStreamSwitcherInputPin::NotifyAllocator(IMemAllocator* pAllocator, BOOL bReadOnly)
{
    HRESULT hr = __super::NotifyAllocator(pAllocator, bReadOnly);
    if (FAILED(hr)) {
        return hr;
    }

    m_bUsingOwnAllocator = (pAllocator == (IMemAllocator*)&m_Allocator);

    return S_OK;
}

STDMETHODIMP CStreamSwitcherInputPin::BeginFlush()
{
    CAutoLock cAutoLock(&(static_cast<CStreamSwitcherFilter*>(m_pFilter))->m_csState);
	m_bRunning = false;
	OutputDebugStringA(m_bRunning?"BeginFlush Running\n":"BeginFlush Stopping\n");
    HRESULT hr;
    CStreamSwitcherFilter* pSSF = static_cast<CStreamSwitcherFilter*>(m_pFilter);

    CStreamSwitcherOutputPin* pOut = pSSF->GetOutputPin();
    if (!IsConnected() || !pOut || !pOut->IsConnected()) {
        return VFW_E_NOT_CONNECTED;
    }

	if (IsActive())
	{
		if (FAILED(hr = __super::BeginFlush())) {
			return hr;
		}

		return pSSF->DeliverBeginFlush();
	}
	else
	{
		Block(false);
		return  S_OK;
	}
}

STDMETHODIMP CStreamSwitcherInputPin::EndFlush()
{
    CAutoLock cAutoLock(&(static_cast<CStreamSwitcherFilter*>(m_pFilter))->m_csState);
	m_bRunning = true;
	OutputDebugStringA(m_bRunning?"EndFlush Running\n":"EndFlush Stopping\n");
    HRESULT hr;
    CStreamSwitcherFilter* pSSF = static_cast<CStreamSwitcherFilter*>(m_pFilter);

    CStreamSwitcherOutputPin* pOut = pSSF->GetOutputPin();
    if (!IsConnected() || !pOut || !pOut->IsConnected()) {
        return VFW_E_NOT_CONNECTED;
    }

	if (IsActive())
	{
		if (FAILED(hr = __super::EndFlush())) {
			return hr;
		}

		return pSSF->DeliverEndFlush();
	}
	else
	{
		Block(true);
		return S_OK;
	}
}

STDMETHODIMP CStreamSwitcherInputPin::EndOfStream()
{
    CAutoLock cAutoLock(&m_csReceive);
	m_bRunning = false;
	OutputDebugStringA(m_bRunning?"EndOfStream Running\n":"EndOfStream Stopping\n");
    CStreamSwitcherFilter* pSSF = static_cast<CStreamSwitcherFilter*>(m_pFilter);

    CStreamSwitcherOutputPin* pOut = pSSF->GetOutputPin();
    if (!IsConnected() || !pOut || !pOut->IsConnected()) {
        return VFW_E_NOT_CONNECTED;
    }

    if (m_hNotifyEvent) {
        SetEvent(m_hNotifyEvent), m_hNotifyEvent = NULL;
        return S_OK;
    }


// 	if (IsActive())
// 	{
// 		m_wave.Save("F:\\active.wav");
// 	}
// 	else
// 	{
// 		m_wave.Save("F:\\inactive.wav");	
// 	}

    return IsActive() ? pSSF->DeliverEndOfStream() : S_OK;
}

// IMemInputPin

STDMETHODIMP CStreamSwitcherInputPin::Receive(IMediaSample* pSample)
{
	CStreamSwitcherFilter* pSSFilter = static_cast<CStreamSwitcherFilter*>(m_pFilter);

    AM_MEDIA_TYPE* pmt = NULL;
    if (SUCCEEDED(pSample->GetMediaType(&pmt)) && pmt) {
        const CMediaType mt(*pmt);
        DeleteMediaType(pmt), pmt = NULL;
        SetMediaType(&mt);
    }

	//保存两个inputPin的MediaSample数据
	//(static_cast<CStreamSwitcherFilter*>(m_pFilter))->SaveMediaSample(this,pSample);

	
#ifdef BLOCKSTREAM
    if (m_fCanBlock) {
        //m_evBlock.Wait();
    }
#endif
	char buf[260] = {0};
	REFERENCE_TIME rt_start,rt_stop;
	pSample->GetTime(&rt_start,&rt_stop);
	long cbBuffer = pSample->GetActualDataLength();
	BYTE* pByte = NULL;
	pSample->GetPointer(&pByte);

	CAutoLock lock(&(pSSFilter->m_csChangePin));

    if (!IsActive()) 
	{
// 		sprintf_s(buf,"InActive rtStart : %lld,rtStop :%lld,len : %d\n",rt_start,rt_stop,cbBuffer);
// 		OutputDebugStringA(buf);
		if (pSSFilter->m_bSwitchInit)
		{
			pSSFilter->m_csChangePin.Unlock();
			// 如果未渲染的那一路比渲染的那一路时间快的话，等待
			while(m_bRunning/*&& (rt_start-((static_cast<CStreamSwitcherFilter*>(m_pFilter))->m_rtStreamStart) >= 100000)*/)
			{
				Sleep(1);
				if (IsActive())
				{
					pSSFilter->m_csChangePin.Lock();
					return S_OK;
				}
			}
			pSSFilter->m_csChangePin.Lock();
			return S_OK;
		}
		else
		{
// 			sprintf_s(buf,259,"threadid %d,noactive return fail\n",GetCurrentThreadId());
// 			OutputDebugStringA(buf);
			return E_FAIL;
		}
    }
	else
	{
		if (pSSFilter->m_bSwitchInit)
		{
			if (pSSFilter->m_bTimeVaild)
			{
				if (pSSFilter->m_bControlFirst)
				{
					if ((rt_start - pSSFilter->m_rtStreamStart) < 0 )
					{
						return S_OK;
					}
					else
					{
						if (pSSFilter->m_bChangePin)
						{
							pSSFilter->m_bControlFirst = FALSE;
							pSSFilter->m_bChangePin = false;
						}
					}
				}
				else
				{
					if (pSSFilter->m_bChangePin)
					{
						if ((rt_start - pSSFilter->m_rtStreamStart) <= 0 )
						{
							return S_OK;
						}
						else
						{
							pSSFilter->m_bChangePin = false;
						}
					}
				}
			}
			else
			{
				pSSFilter->m_bTimeVaild = true;
			}
		}

		//if (g_nDebug-- > 0)
		{
// 			sprintf_s(buf,"  Active rtStart : %lld,rtStop :%lld,len : %d\n",rt_start,rt_stop,cbBuffer);
// 			OutputDebugStringA(buf);
		}
	}

// 	m_wave.AddBuffer(pByte,cbBuffer/pSSFilter->m_wave.GetFormat().nBlockAlign);
// 	return S_OK;

// 	pSSFilter->m_wave.AddBuffer(pByte,cbBuffer/pSSFilter->m_wave.GetFormat().nBlockAlign);
	//实时更新当前的渲染时间，用于控制另一个inputpin的sleep时间
	pSSFilter->UpdateCurrentTime(rt_start,rt_stop);

	CAutoLock cAutoLock(&m_csReceive);

    CStreamSwitcherOutputPin* pOut = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetOutputPin();
    ASSERT(pOut->GetConnected());

    HRESULT hr = __super::Receive(pSample);
    if (S_OK != hr) {
        return hr;
    }

    if (m_SampleProps.dwStreamId != AM_STREAM_MEDIA) {
        return pOut->Deliver(pSample);
    }

    ALLOCATOR_PROPERTIES props, actual;
    hr = m_pAllocator->GetProperties(&props);
    hr = pOut->CurrentAllocator()->GetProperties(&actual);

    CMediaType mtOut = m_mt;
    mtOut = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->CreateNewOutputMediaType(mtOut, cbBuffer);

    bool fTypeChanged = false;

    if (mtOut != pOut->CurrentMediaType() || cbBuffer > actual.cbBuffer) 
	{
        fTypeChanged = true;

        m_SampleProps.dwSampleFlags |= AM_SAMPLE_TYPECHANGED/*|AM_SAMPLE_DATADISCONTINUITY|AM_SAMPLE_TIMEDISCONTINUITY*/;

        if (props.cBuffers < 8 && mtOut.majortype == MEDIATYPE_Audio) 
		{
            props.cBuffers = 8;
        }

        props.cbBuffer = cbBuffer;

        if (actual.cbAlign != props.cbAlign
                || actual.cbPrefix != props.cbPrefix
                || actual.cBuffers < props.cBuffers
                || actual.cbBuffer < props.cbBuffer) 
		{
            hr = pOut->DeliverBeginFlush();
            hr = pOut->DeliverEndFlush();
            hr = pOut->CurrentAllocator()->Decommit();
            hr = pOut->CurrentAllocator()->SetProperties(&props, &actual);
            hr = pOut->CurrentAllocator()->Commit();
        }
    }

	pSSFilter->m_csChangePin.Unlock();
    CComPtr<IMediaSample> pOutSample;
    if (FAILED(InitializeOutputSample(pSample, &pOutSample)))
	{
		pSSFilter->m_csChangePin.Lock();
        return E_FAIL;
    }
	pSSFilter->m_csChangePin.Lock();
// 	if (pSSFilter->m_bChangePin)
// 	{
// 		pSSFilter->m_bChangePin = false;
// 		return E_FAIL;
// 	}

    pmt = NULL;
    if (SUCCEEDED(pOutSample->GetMediaType(&pmt)) && pmt) 
	{
        const CMediaType mt(*pmt);
        DeleteMediaType(pmt), pmt = NULL;
        // TODO
        ASSERT(0);
    }

    if (fTypeChanged) 
	{
        pOut->SetMediaType(&mtOut);
        (static_cast<CStreamSwitcherFilter*>(m_pFilter))->OnNewOutputMediaType(m_mt, mtOut);
        pOutSample->SetMediaType(&mtOut);
    }

    // Transform
    hr = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->Transform(pSample, pOutSample);

    if (S_OK == hr) 
	{
		pSSFilter->m_csChangePin.Unlock();
        hr = pOut->Deliver(pOutSample);
        m_bSampleSkipped = FALSE;
		pSSFilter->m_csChangePin.Lock();
    } 
	else if (S_FALSE == hr)
	{
        hr = S_OK;
        pOutSample = NULL;
        m_bSampleSkipped = TRUE;

        if (!m_bQualityChanged) 
		{
            m_pFilter->NotifyEvent(EC_QUALITY_CHANGE, 0, 0);
            m_bQualityChanged = TRUE;
        }
    }
    return hr;
}

STDMETHODIMP CStreamSwitcherInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    if (!IsConnected()) {
        return S_OK;
    }

	if (!IsActive())
	{
		return S_OK;
	}

    CAutoLock cAutoLock(&m_csReceive);

    CStreamSwitcherFilter* pSSF = static_cast<CStreamSwitcherFilter*>(m_pFilter);

    CStreamSwitcherOutputPin* pOut = pSSF->GetOutputPin();
    if (!pOut || !pOut->IsConnected()) 
	{
        return VFW_E_NOT_CONNECTED;
    }
    return pSSF->DeliverNewSegment(tStart, tStop, dRate);
}

//
// CStreamSwitcherOutputPin
//

CStreamSwitcherOutputPin::CStreamSwitcherOutputPin(CStreamSwitcherFilter* pFilter, HRESULT* phr)
    : CBaseOutputPin(NAME("CStreamSwitcherOutputPin"), pFilter, &pFilter->m_csState, phr, L"Out")
{
    //  m_bCanReconnectWhenActive = TRUE;
}

STDMETHODIMP CStreamSwitcherOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);
    ValidateReadWritePtr(ppv, sizeof(PVOID));
    *ppv = NULL;

    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) 
	{
        if (m_pStreamSwitcherPassThru == NULL)
		{
            HRESULT hr = S_OK;
            m_pStreamSwitcherPassThru = (IUnknown*)(INonDelegatingUnknown*)
                                        DEBUG_NEW CStreamSwitcherPassThru(GetOwner(), &hr, static_cast<CStreamSwitcherFilter*>(m_pFilter));

            if (!m_pStreamSwitcherPassThru)
			{
                return E_OUTOFMEMORY;
            }
            if (FAILED(hr)) 
			{
                return hr;
            }
        }

        return m_pStreamSwitcherPassThru->QueryInterface(riid, ppv);
    }
    /*
        else if (riid == IID_IStreamBuilder)
        {
            return GetInterface((IStreamBuilder*)this, ppv);
        }
    */
    return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CStreamSwitcherOutputPin::QueryAcceptUpstream(const AM_MEDIA_TYPE* pmt)
{
    HRESULT hr = S_FALSE;

    CStreamSwitcherInputPin* pIn = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetInputPin();

    if (pIn && pIn->IsConnected() && (pIn->IsUsingOwnAllocator() || pIn->CurrentMediaType() == *pmt)) 
	{
        if (CComQIPtr<IPin> pPinTo = pIn->GetConnected()) 
		{
            if (S_OK != (hr = pPinTo->QueryAccept(pmt))) 
			{
                return VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
		else 
		{
            return E_FAIL;
        }
    }

    return hr;
}

// pure virtual

HRESULT CStreamSwitcherOutputPin::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
    CStreamSwitcherInputPin* pIn = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetInputPin();
    if (!pIn || !pIn->IsConnected()) 
	{
        return E_UNEXPECTED;
    }

    CComPtr<IMemAllocator> pAllocatorIn;
    pIn->GetAllocator(&pAllocatorIn);
    if (!pAllocatorIn) 
	{
        return E_UNEXPECTED;
    }

    HRESULT hr;
    if (FAILED(hr = pAllocatorIn->GetProperties(pProperties)))
	{
        return hr;
    }

    if (/*pProperties->cBuffers < 8 &&*/ pIn->CurrentMediaType().majortype == MEDIATYPE_Audio) {
        //pProperties->cBuffers = 8;
    }
	pProperties->cBuffers = 1;

    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    return (pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer? E_FAIL: NOERROR);
}

// virtual

class __declspec(uuid("AEFA5024-215A-4FC7-97A4-1043C86FD0B8"))
    MatrixMixer {};
class __declspec(uuid("C1F400A0-3F08-11d3-9F0B-006008039E37"))
	SampleGrabber {};
class __declspec(uuid("42D96541-CCA9-4A93-967E-42A2F68E028D"))
	Vocaltrans{};
class __declspec(uuid("6A08CF80-0E18-11CF-A24D-0020AFD79767"))
	ACMWrapper{};

HRESULT CStreamSwitcherOutputPin::Inactive()
{
	//(static_cast<CStreamSwitcherFilter*>(m_pFilter))->m_wave.Save("F:\\Switch.wav");

	return __super::Inactive();
}

HRESULT CStreamSwitcherOutputPin::CheckConnect(IPin* pPin)
{
//     CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPin);

//     return 
//         IsAudioWaveRenderer(pBF) || GetCLSID(pBF) == __uuidof(MatrixMixer) || GetCLSID(pBF) == __uuidof(SampleGrabber)//对SampleGrabber进行支持
// 		|| GetCLSID(pBF) == __uuidof(Vocaltrans)//对Vocaltrans进行支持
//         ? __super::CheckConnect(pPin)
//         : E_FAIL;

     return CBaseOutputPin::CheckConnect(pPin);
}

HRESULT CStreamSwitcherOutputPin::BreakConnect()
{
    m_pPinConnection = NULL;
    return __super::BreakConnect();
}

HRESULT CStreamSwitcherOutputPin::CompleteConnect(IPin* pReceivePin)
{
    m_pPinConnection = CComQIPtr<IPinConnection>(pReceivePin);
	(static_cast<CStreamSwitcherFilter*>(m_pFilter))->CompleteConnect(PINDIR_OUTPUT, this, pReceivePin);
    return __super::CompleteConnect(pReceivePin);
}

HRESULT CStreamSwitcherOutputPin::CheckMediaType(const CMediaType* pmt)
{
    return (static_cast<CStreamSwitcherFilter*>(m_pFilter))->CheckMediaType(pmt);
}

HRESULT CStreamSwitcherOutputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
    CStreamSwitcherInputPin* pIn = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetInputPin();
    if (!pIn || !pIn->IsConnected()) {
        return E_UNEXPECTED;
    }

    CComPtr<IEnumMediaTypes> pEM;
    if (FAILED(pIn->GetConnected()->EnumMediaTypes(&pEM))) {
        return VFW_S_NO_MORE_ITEMS;
    }

    if (iPosition > 0 && FAILED(pEM->Skip(iPosition))) {
        return VFW_S_NO_MORE_ITEMS;
    }

    AM_MEDIA_TYPE* tmp = NULL;
    if (S_OK != pEM->Next(1, &tmp, NULL) || !tmp) {
        return VFW_S_NO_MORE_ITEMS;
    }

    CopyMediaType(pmt, tmp);
    DeleteMediaType(tmp);

    return S_OK;
}

// IPin

STDMETHODIMP CStreamSwitcherOutputPin::QueryAccept(const AM_MEDIA_TYPE* pmt)
{
    HRESULT hr = __super::QueryAccept(pmt);
    if (S_OK != hr) {
        return hr;
    }

    return QueryAcceptUpstream(pmt);
}

// IQualityControl

STDMETHODIMP CStreamSwitcherOutputPin::Notify(IBaseFilter* pSender, Quality q)
{
    CStreamSwitcherInputPin* pIn = (static_cast<CStreamSwitcherFilter*>(m_pFilter))->GetInputPin();
    if (!pIn || !pIn->IsConnected()) {
        return VFW_E_NOT_CONNECTED;
    }
    return pIn->PassNotify(q);
}

// IStreamBuilder

STDMETHODIMP CStreamSwitcherOutputPin::Render(IPin* ppinOut, IGraphBuilder* pGraph)
{
    CComPtr<IBaseFilter> pBF;
    pBF.CoCreateInstance(CLSID_DSoundRender);
    if (!pBF || FAILED(pGraph->AddFilter(pBF, L"Default DirectSound Device"))) {
        return E_FAIL;
    }

    if (FAILED(pGraph->ConnectDirect(ppinOut, GetFirstDisconnectedPin(pBF, PINDIR_INPUT), NULL))) {
        pGraph->RemoveFilter(pBF);
        return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CStreamSwitcherOutputPin::Backout(IPin* ppinOut, IGraphBuilder* pGraph)
{
    return S_OK;
}

//
// CStreamSwitcherFilter
//

CStreamSwitcherFilter::CStreamSwitcherFilter(LPUNKNOWN lpunk, HRESULT* phr, const CLSID& clsid)
    :CBaseFilter(NAME("CStreamSwitcherFilter"), lpunk, &m_csState, clsid)
	,m_rtStart(0)
	,m_rtStop(0)
	,m_rtStreamStart(0)
	,m_rtStreamStop(0)
	,m_bChangePin(false)
	,m_bUsePin1(TRUE)
	,m_bControlFirst(FALSE)
	,m_uInputPin(0)
	,m_eSoundTrack(Stereo)
	,m_bCopySTrack(TRUE)
	,m_rtSampleDelta(0)
	,m_rtNewStart(0)
	,m_bSwitchInit(false)
	,m_bTimeVaild(true)
{
    if (phr) {
        *phr = S_OK;
    }
	timeBeginPeriod(1);
    HRESULT hr = S_OK;

    do {
        CAutoPtr<CStreamSwitcherInputPin> pInput1;
		CAutoPtr<CStreamSwitcherInputPin> pInput2;
        CAutoPtr<CStreamSwitcherOutputPin> pOutput;

        hr = S_OK;
        pInput1.Attach(DEBUG_NEW CStreamSwitcherInputPin(this, &hr, L"Audio Line 1"));
        if (!pInput1 || FAILED(hr)) {
            break;
        }
		IncrementPinVersion();

		hr = S_OK;
		pInput2.Attach(DEBUG_NEW CStreamSwitcherInputPin(this, &hr, L"Audio Line 2"));
		if (!pInput2 || FAILED(hr)) {
			break;
		}
		IncrementPinVersion();
        hr = S_OK;
        pOutput.Attach(DEBUG_NEW CStreamSwitcherOutputPin(this, &hr));
        if (!pOutput || FAILED(hr)) {
            break;
        }
		IncrementPinVersion();
        CAutoLock cAutoLock(&m_csPins);

        m_pInputs.AddHead(m_pInput1 = pInput1.Detach());
		m_pInputs.AddTail(m_pInput2 = pInput2.Detach());
        m_pOutput = pOutput.Detach();
		
		//set the input pin number
		//m_PinVersion = 2;
		m_uInputPin = 2;

        return;
    } while (false);

    if (phr) {
        *phr = E_FAIL;
    }
}

CStreamSwitcherFilter::~CStreamSwitcherFilter()
{
    CAutoLock cAutoLock(&m_csPins);
	timeEndPeriod(1);
    POSITION pos = m_pInputs.GetHeadPosition();
    while (pos) {
        delete m_pInputs.GetNext(pos);
    }
    m_pInputs.RemoveAll();
    m_pInput1 = NULL;
	m_pInput2 = NULL;

    delete m_pOutput;
    m_pOutput = NULL;

	ClearList();
}

STDMETHODIMP CStreamSwitcherFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(IAMStreamSelect)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

//
void CStreamSwitcherFilter::SaveMediaSample(LISTMEDIASAMPLE& list_MS,IMediaSample* pMS)
{
	if (list_MS.size() < 10)
	{
		long size = pMS->GetActualDataLength();
		OS_MedaiSample OS_MS;
		OS_MS.data_size = size;
		OS_MS.pSample = new BYTE[size];
		BYTE* pSample = NULL;
		pMS->GetPointer(&pSample);
		memcpy(OS_MS.pSample,pSample,size);
		REFERENCE_TIME start_tm,stop_tm;
		pMS->GetTime(&start_tm,&stop_tm);
		OS_MS.start_tm = start_tm;
		OS_MS.stop_tm = stop_tm;
		list_MS.push_back(OS_MS);
	}
	else
	{
		LISTMEDIASAMPLE::iterator i = list_MS.begin();
		if ((*i).pSample)
		{
			delete ((*i).pSample);
		}
		list_MS.pop_front();

		long size = pMS->GetActualDataLength();
		OS_MedaiSample OS_MS;
		OS_MS.data_size = size;
		OS_MS.pSample = new BYTE[size];
		BYTE* pSample = NULL;
		pMS->GetPointer(&pSample);
		memcpy(OS_MS.pSample,pSample,size);
		REFERENCE_TIME start_tm,stop_tm;
		pMS->GetTime(&start_tm,&stop_tm);
		OS_MS.start_tm = start_tm;
		OS_MS.stop_tm = stop_tm;
		list_MS.push_back(OS_MS);
	}
}

void CStreamSwitcherFilter::SaveMediaSample(CStreamSwitcherInputPin* pInPin,IMediaSample* pMS)
{
	CAutoLock lock(&m_csMedia);

	if ((pInPin == m_pInput1) && (m_pInput1!=NULL))
	{
		SaveMediaSample(m_listMediaSample1,pMS);
	}
	else if ((pInPin == m_pInput2) && (m_pInput2!=NULL))
	{
		SaveMediaSample(m_listMediaSample2,pMS);
	}
}

void CStreamSwitcherFilter::UpdateCurrentTime(const REFERENCE_TIME rtStart,const REFERENCE_TIME rtStop)
{
// 	char buf[260] = {0};
// 	sprintf_s(buf,259,"UpdateCurrentTime rtStart : %lld\n",rtStart);
// 	OutputDebugStringA(buf);
	m_rtStreamStart = rtStart;
	m_rtStreamStop = rtStop;
	m_rtSampleDelta = m_rtStreamStop - m_rtStreamStart;
}

int CStreamSwitcherFilter::GetPinCount()
{
    CAutoLock cAutoLock(&m_csPins);

    return (1 + (int)m_pInputs.GetCount());
}

CBasePin* CStreamSwitcherFilter::GetPin(int n)
{
    CAutoLock cAutoLock(&m_csPins);

    if (n < 0 || n >= GetPinCount()) {
        return NULL;
    } else if (n == 0) {
        return m_pOutput;
    } else {
        return m_pInputs.GetAt(m_pInputs.FindIndex(n - 1));
    }
}

int CStreamSwitcherFilter::GetConnectedInputPinCount()
{
    CAutoLock cAutoLock(&m_csPins);

    int nConnected = 0;
    POSITION pos = m_pInputs.GetHeadPosition();

    while (pos) {
        if (m_pInputs.GetNext(pos)->IsConnected()) {
            nConnected++;
        }
    }

    return nConnected;
}

CStreamSwitcherInputPin* CStreamSwitcherFilter::GetConnectedInputPin(int n)
{
    if (n >= 0) {
        POSITION pos = m_pInputs.GetHeadPosition();
        while (pos) {
            CStreamSwitcherInputPin* pPin = m_pInputs.GetNext(pos);
            if (pPin->IsConnected()) {
                if (n == 0) {
                    return pPin;
                }
                n--;
            }
        }
    }

    return NULL;
}

CStreamSwitcherInputPin* CStreamSwitcherFilter::GetInputPin()
{
	CAutoLock lock(&m_csPins);

	if (m_bUsePin1)
	{
		return m_pInput1;
	}
	else
	{
		return m_pInput2;
	}
}

CStreamSwitcherOutputPin* CStreamSwitcherFilter::GetOutputPin()
{
    return m_pOutput;
}

void CStreamSwitcherFilter::ClearList()
{
// 	m_rtStart = 0;
// 	m_rtStop = 0;
	CAutoLock lock(&m_csMedia);

	ClearList(m_listMediaSample1);
	ClearList(m_listMediaSample2);
}

//
void CStreamSwitcherFilter::ClearList(LISTMEDIASAMPLE& list_MS)
{
	LISTMEDIASAMPLE::iterator i;
	for (i=list_MS.begin();i!=list_MS.end();i++)
	{
		if ((*i).pSample)
		{
			delete ((*i).pSample);
		}
	}
	list_MS.clear();
}

HRESULT CStreamSwitcherFilter::CompleteConnect(PIN_DIRECTION dir, CBasePin* pPin, IPin* pReceivePin)
{
    if (dir == PINDIR_INPUT) {
        CAutoLock cAutoLock(&m_csPins);

        int nConnected = GetConnectedInputPinCount();

        if (nConnected < 2) {
			return S_OK;
        }
		//可以增加3个及以上inputpin，暂时不需要
//         if ((size_t)nConnected == m_pInputs.GetCount()) {
//             CStringW name;
//             name.Format(L"Audio Line %d", ++m_uInputPin/*m_PinVersion*/);
// 			IncrementPinVersion();
// 
//             HRESULT hr = S_OK;
//             CStreamSwitcherInputPin* pPin = DEBUG_NEW CStreamSwitcherInputPin(this, &hr, name);
//             if (!pPin || FAILED(hr)) {
//                 SAFE_DELETE(pPin);
//                 return E_FAIL;
//             }
//             m_pInputs.AddTail(pPin);
//         }
    }
	else
	{
		m_bSwitchInit = m_pInput1->m_bSwitchInit && m_pInput2->m_bSwitchInit;
	}
    return S_OK;
}

// this should be very thread safe, I hope it is, it must be... :)

void CStreamSwitcherFilter::SelectInput(CStreamSwitcherInputPin* pInput)
{
    // make sure no input thinks it is active
	if (m_bUsePin1)
	{
		m_pInput1 = NULL;
	}
	else
	{
		m_pInput2 = NULL;
	}

    // release blocked GetBuffer in our own allocator & block all Receive
    POSITION pos = m_pInputs.GetHeadPosition();
    while (pos) {
        CStreamSwitcherInputPin* pPin = m_pInputs.GetNext(pos);
        pPin->Block(false);
        // a few Receive calls can arrive here, but since m_pInput == NULL neighter of them gets delivered
        pPin->Block(true);
    }

    // this will let waiting GetBuffer() calls go on inside our Receive()
    if (m_pOutput) {
        m_pOutput->DeliverBeginFlush();
        m_pOutput->DeliverEndFlush();
    }

    if (!pInput) {
        return;
    }

	if (m_bUsePin1)
	{
		// set new input
		m_pInput1 = pInput;

		// let it go
		m_pInput1->Block(false);
	}
	else
	{
		// set new input
		m_pInput2 = pInput;

		// let it go
		m_pInput2->Block(false);
	}
}

bool CStreamSwitcherFilter::ChangeNextPinMT(CMediaType &mt)
{
	HRESULT hr;
	IPin* pInput;
	GetOutputPin()->ConnectedTo(&pInput);
	if (pInput)
	{
		hr = pInput->QueryAccept((AM_MEDIA_TYPE*)&mt);
		if (hr == S_FALSE)
		{
			pInput->Release();
			return false;
		}
		pInput->Release();
	}
	else
	{
		return false;
	}
	return SUCCEEDED(GetOutputPin()->SetMediaType(&mt));
}

HRESULT CStreamSwitcherFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
    BYTE* pDataIn  = NULL;
    BYTE* pDataOut = NULL;
	CMediaType mt = GetInputPin()->CurrentMediaType();//获得当前input pin 的媒体类型
	WAVEFORMATEX* wfm = NULL;

	if (mt.subtype == FOURCCMap(WAVE_FORMAT_PCM)&&mt.formattype == FORMAT_WaveFormatEx && mt.cbFormat >= sizeof(WAVEFORMATEX))//判断是否是WAVEFORMATEX
	{
		wfm = reinterpret_cast<WAVEFORMATEX*>(mt.Format());
	}
	
    HRESULT hr;
    if (FAILED(hr = pIn->GetPointer(&pDataIn))) {
        return hr;
    }
    if (FAILED(hr = pOut->GetPointer(&pDataOut))) {
        return hr;
    }

    long len  = pIn->GetActualDataLength();
    long size = pOut->GetSize();

    if (!pDataIn || !pDataOut /*|| len > size || len <= 0*/) {
        return S_FALSE;    // FIXME
    }
	long minLen = min(len, size);
	if (wfm && (wfm->nChannels == 2))
	{
		switch(m_eSoundTrack)
		{
		case Stereo://只拷贝下
			{
				memcpy(pDataOut, pDataIn,minLen );
				pOut->SetActualDataLength(minLen);
				goto NextTF;
			}
		case MonoL:
			{
				WORD wBytesPerSample = wfm->wBitsPerSample/8;

				if (m_bCopySTrack)
				{
					long tmpCopied = 0;
					long tmpLenIn = 0;//左声道开始
					long tmpLenOut = 0;
					long totalCopy = minLen / wfm->nBlockAlign;
					while (tmpCopied < totalCopy)
					{
						memcpy(pDataOut+tmpLenOut,pDataIn+tmpLenIn,wBytesPerSample);//左声道拷贝左声道数据
						tmpLenOut += wBytesPerSample;
						memcpy(pDataOut+tmpLenOut,pDataIn+tmpLenIn,wBytesPerSample);//右声道拷贝左声道数据
						tmpLenOut += wBytesPerSample;
						tmpLenIn += wBytesPerSample*2;
						tmpCopied ++;
					}
					pOut->SetActualDataLength(minLen);
				}
				else
				{

					long tmpCopied = 0;
					long tmpLenIn = 0;//左声道开始,
					long tmpLenOut = 0;
					long totalCopy = minLen / wfm->nBlockAlign;
					while (tmpCopied < totalCopy)
					{
						memcpy(pDataOut+tmpLenOut,pDataIn+tmpLenIn,wBytesPerSample);//拷贝进左声道
						tmpLenOut += wBytesPerSample;

						static unsigned char nChar = 128;
						static unsigned short nShort = 32768;
						if (wBytesPerSample == 1)
						{
							memcpy(pDataOut+tmpLenOut,&nChar,wBytesPerSample);//左声道置为0
						}
						else if (wBytesPerSample == 2)
						{
							memcpy(pDataOut+tmpLenOut,&nShort,wBytesPerSample);//左声道置为0
						}
						tmpLenOut += wBytesPerSample;
						tmpLenIn += wBytesPerSample*2;
						tmpCopied ++;
					}
					pOut->SetActualDataLength(totalCopy*wfm->nBlockAlign);
				}
				goto NextTF;
			}
		case MonoR:
			{
				WORD wBytesPerSample = wfm->wBitsPerSample/8;

				if (m_bCopySTrack)
				{
					long tmpCopied = 0;
					long tmpLenIn = wBytesPerSample;//右声道开始
					long tmpLenOut = 0;
					long totalCopy = minLen / wfm->nBlockAlign;
					while (tmpCopied < totalCopy)
					{
						memcpy(pDataOut+tmpLenOut,pDataIn+tmpLenIn,wBytesPerSample);//右声道拷贝到左声道
						tmpLenOut += wBytesPerSample;
						memcpy(pDataOut+tmpLenOut,pDataIn+tmpLenIn,wBytesPerSample);//右声道拷贝到右声道
						tmpLenOut += wBytesPerSample;
						tmpLenIn += wBytesPerSample*2;
						tmpCopied ++;
					}
					pOut->SetActualDataLength(minLen);
				}
				else
				{
					long tmpCopied = 0;
					long tmpLenIn = wBytesPerSample;//右声道开始
					long tmpLenOut = 0;
					long totalCopy = minLen / wfm->nBlockAlign;
					while (tmpCopied < totalCopy)
					{
						static unsigned char nChar = 128;
						static unsigned short nShort = 32768;
						if (wBytesPerSample == 1)
						{
							memcpy(pDataOut+tmpLenOut,&nChar,wBytesPerSample);//左声道置为0
						}
						else if (wBytesPerSample == 2)
						{
							memcpy(pDataOut+tmpLenOut,&nShort,wBytesPerSample);//左声道置为0
						}
						tmpLenOut += wBytesPerSample;

						memcpy(pDataOut+tmpLenOut,pDataIn+tmpLenIn,wBytesPerSample);//拷贝进右声道
						tmpLenOut += wBytesPerSample;

						tmpLenIn += wBytesPerSample*2;
						tmpCopied ++;
					}
					pOut->SetActualDataLength(totalCopy*wfm->nBlockAlign);
				}
				goto NextTF;
			}
		}
	}

rejectmt:
	memcpy(pDataOut, pDataIn, minLen);
	pOut->SetActualDataLength(minLen);

NextTF:
	return ChangePitch(pOut);
}

CMediaType CStreamSwitcherFilter::CreateNewOutputMediaType(CMediaType mt, long& cbBuffer)
{
    return mt;
}

HRESULT CStreamSwitcherFilter::DeliverEndOfStream()
{
    return m_pOutput ? m_pOutput->DeliverEndOfStream() : E_FAIL;
}

HRESULT CStreamSwitcherFilter::DeliverBeginFlush()
{
	OutputDebugString(__TEXT("CStreamSwitcherFilter  GLP DeliverBeginFlush\n"));
    return m_pOutput ? m_pOutput->DeliverBeginFlush() : E_FAIL;
}

HRESULT CStreamSwitcherFilter::DeliverEndFlush()
{
	OutputDebugString(__TEXT("CStreamSwitcherFilter GLP DeliverEndFlush\n"));
    return m_pOutput ? m_pOutput->DeliverEndFlush() : E_FAIL;
}

HRESULT CStreamSwitcherFilter::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	char buf[260] = {0};
	static int nGLP;
	sprintf_s(buf,"DeliverNewSegment tStart %lld,tStop %lld,dRate %f\n***********************************************************%d\n"
		,tStart,tStop,dRate,++nGLP);
	OutputDebugStringA(buf);
	m_bTimeVaild = false; 
    return m_pOutput ? m_pOutput->DeliverNewSegment(tStart, tStop, dRate) : E_FAIL;
}

STDMETHODIMP CStreamSwitcherFilter::Pause()
{
	HRESULT hr = __super::Pause();
	if (SUCCEEDED(hr))//用于在用GraphEdit调试的时候。一般手动连接的话，初始化都已经完成了。
	{
		m_bSwitchInit = m_pInput1->m_bSwitchInit && m_pInput2->m_bSwitchInit;
	}

	return hr;
}
// IAMStreamSelect

STDMETHODIMP CStreamSwitcherFilter::Count(DWORD* pcStreams)
{
    if (!pcStreams) {
        return E_POINTER;
    }

    CAutoLock cAutoLock(&m_csPins);

    *pcStreams = GetConnectedInputPinCount();

    return S_OK;
}

STDMETHODIMP CStreamSwitcherFilter::Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid, DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk)
{
    CAutoLock cAutoLock(&m_csPins);

    CBasePin* pPin = GetConnectedInputPin(lIndex);
    if (!pPin) {
        return E_INVALIDARG;
    }

    if (ppmt) {
        *ppmt = CreateMediaType(&m_pOutput->CurrentMediaType());
    }

    if (pdwFlags) {
        *pdwFlags = (m_pInput1 == pPin) ? AMSTREAMSELECTINFO_EXCLUSIVE : 0;
    }

    if (plcid) {
        *plcid = 0;
    }

    if (pdwGroup) {
        *pdwGroup = 0;
    }

    if (ppszName) {
        *ppszName = (WCHAR*)CoTaskMemAlloc((wcslen(pPin->Name()) + 1) * sizeof(WCHAR));
        if (*ppszName) {
            wcscpy_s(*ppszName, wcslen(pPin->Name()) + 1, pPin->Name());
        }
    }

    if (ppObject) {
        *ppObject = NULL;
    }

    if (ppUnk) {
        *ppUnk = NULL;
    }

    return S_OK;
}

STDMETHODIMP CStreamSwitcherFilter::Enable(long lIndex, DWORD dwFlags)
{
    if (dwFlags != AMSTREAMSELECTENABLE_ENABLE) {
        return E_NOTIMPL;
    }

    PauseGraph;

    CStreamSwitcherInputPin* pNewInput = GetConnectedInputPin(lIndex);
    if (!pNewInput) {
        return E_INVALIDARG;
    }

    SelectInput(pNewInput);

    ResumeGraph;

    return S_OK;
}
