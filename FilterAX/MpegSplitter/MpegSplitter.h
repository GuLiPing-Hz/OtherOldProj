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

#pragma once

#include "BaseSplitter/BaseSplitter.h"
#include "MpegSplitterFile.h"
#include "MpegSplitterSettingsWnd.h"
#include "ITrackInfo.h"

#define MpegSplitterName L"TMPEG Splitter"
#define MpegSourceName   L"TMPEG Source"

class CMpegSplitterOutputPin;
// {ADFE8EBD-CF53-425e-BD02-FA84FA6DF1C1}
// DEFINE_GUID(<<name>>, 
// 			0xadfe8ebd, 0xcf53, 0x425e, 0xbd, 0x2, 0xfa, 0x84, 0xfa, 0x6d, 0xf1, 0xc1);

class __declspec(uuid("ADFE8EBD-CF53-425e-BD02-FA84FA6DF1C1"))
    CMpegSplitterFilter
    : public CBaseSplitterFilter
    , public IAMStreamSelect
    //, public ISpecifyPropertyPages2
    , public IMpegSplitterFilter
{
	friend class CMpegSplitterOutputPin;

protected:
    HRESULT CreateOutputs(IAsyncReader* pAsyncReader);
    void    ReadClipInfo(LPCOLESTR pszFileName);

    virtual bool DemuxInit();
    virtual void DemuxSeek(REFERENCE_TIME rt,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL);
	virtual void DemuxOnlySeek(REFERENCE_TIME rt,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL);
	virtual void DemuxBeginFlush();
	virtual REFERENCE_TIME getStartOffset(CBaseSplitterOutputPin* pOut);
	virtual bool isEof();
    //virtual bool DemuxLoop();

    bool BuildPlaylist(LPCTSTR pszFileName, CAtlList<CHdmvClipInfo::PlaylistItem>& files);
    bool BuildChapters(LPCTSTR pszFileName, CAtlList<CHdmvClipInfo::PlaylistItem>& PlaylistItems
		, CAtlList<CHdmvClipInfo::PlaylistChapter>& Items);

    virtual HRESULT DemuxNextPacket(REFERENCE_TIME rtStartOffset,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL,Packet* pPacket=NULL);

public:
    CMpegSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsid = __uuidof(CMpegSplitterFilter));
    void SetPipo(bool bPipo) { m_pPipoBimbo = bPipo; };

    bool StreamIsTrueHD(const WORD pid);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP GetClassID(CLSID* pClsID);
    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);

    // CBaseFilter

    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);

    // IAMStreamSelect

    STDMETHODIMP Count(DWORD* pcStreams);
    STDMETHODIMP Enable(long lIndex, DWORD dwFlags);
    STDMETHODIMP Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid,
                      DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk);

    // ISpecifyPropertyPages2

//     STDMETHODIMP GetPages(CAUUID* pPages);
//     STDMETHODIMP CreatePage(const GUID& guid, IPropertyPage** ppPage);

    // IMpegSplitterFilter
    STDMETHODIMP Apply();

    STDMETHODIMP SetFastStreamChange(BOOL nValue);
    STDMETHODIMP_(BOOL) GetFastStreamChange();

    STDMETHODIMP SetForcedSub(BOOL nValue);
    STDMETHODIMP_(BOOL) GetForcedSub();

    STDMETHODIMP SetTrackPriority(BOOL nValue);
    STDMETHODIMP_(BOOL) GetTrackPriority();

    STDMETHODIMP SetAudioLanguageOrder(WCHAR* nValue);
    STDMETHODIMP_(WCHAR*) GetAudioLanguageOrder();

    STDMETHODIMP SetSubtitlesLanguageOrder(WCHAR* nValue);
    STDMETHODIMP_(WCHAR*) GetSubtitlesLanguageOrder();

    STDMETHODIMP SetVC1_GuidFlag(int nValue);
    STDMETHODIMP_(int) GetVC1_GuidFlag();

    STDMETHODIMP SetTrueHD(int nValue);
    STDMETHODIMP_(int) GetTrueHD();

    STDMETHODIMP SetAlternativeDuration(BOOL nValue);
    STDMETHODIMP_(BOOL) GetAlternativeDuration();

    STDMETHODIMP_(int) GetMPEGType();

private:
	REFERENCE_TIME m_rtPlaylistDuration;
	CString m_csAudioLanguageOrder, m_csSubtitlesLanguageOrder;
	bool m_useFastStreamChange, m_ForcedSub, m_TrackPriority, m_AlternativeDuration;
	int m_nVC1_GuidFlag, m_AC3CoreOnly;
	CCritSec m_csProps;

	REFERENCE_TIME	m_rtStartOffset;
	bool							m_pPipoBimbo;
	CHdmvClipInfo		m_ClipInfo;

protected:
	CAutoPtr<CMpegSplitterFile> m_pFile;
	CComQIPtr<ITrackInfo> pTI;
};

// {7CF70DC5-D501-4f7f-817A-1873C149E4DB}
// DEFINE_GUID(<<name>>, 
// 			0x7cf70dc5, 0xd501, 0x4f7f, 0x81, 0x7a, 0x18, 0x73, 0xc1, 0x49, 0xe4, 0xdb);
class __declspec(uuid("7CF70DC5-D501-4f7f-817A-1873C149E4DB"))
    CMpegSourceFilter : public CMpegSplitterFilter
{
public:
    CMpegSourceFilter(LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsid = __uuidof(CMpegSourceFilter));
};

class CMpegSplitterOutputPin : public CBaseSplitterOutputPin, protected CCritSec
{
    CAutoPtr<Packet> m_p;
    CAutoPtrList<Packet> m_pl;
    REFERENCE_TIME m_rtPrev, m_rtOffset, m_rtMaxShift;
    bool m_fHasAccessUnitDelimiters;
    bool m_bFilterDTSMA;
    bool DD_reset;
    bool m_bFlushed;
    int  m_type;

protected:
    HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    //HRESULT DeliverPacket(CAutoPtr<Packet> p);
	virtual HRESULT dealPacket(Packet* p);
    HRESULT DeliverEndFlush();

public:
    CMpegSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CSource* pFilter
		, HRESULT* phr, int type ,DWORD nTrackNum,CMediaType* pMT);
    virtual ~CMpegSplitterOutputPin();
    STDMETHODIMP Connect(IPin* pReceivePin, const AM_MEDIA_TYPE* pmt);
    void SetMaxShift(REFERENCE_TIME rtMaxShift) { m_rtMaxShift = rtMaxShift; };
};
