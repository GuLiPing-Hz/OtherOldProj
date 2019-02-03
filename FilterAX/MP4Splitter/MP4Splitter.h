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

#include <atlbase.h>
#include <atlcoll.h>
#include "MP4SplitterFile.h"
#include "BaseSplitter/BaseSplitter.h"
#include <map>

#define MP4SplitterName L"TMP4/MOV Splitter"
#define MP4SourceName   L"TMP4/MOV Source"
#define MPEG4SplitterName L"TMPEG4 Video Splitter"
#define MPEG4SourceName L"TMPEG4 Video Source"

typedef std::map<DWORD,REFERENCE_TIME> MAPDWRT;

class CMP4SplitterOutputPin : public CBaseSplitterOutputPin, protected CCritSec
{
public:
	CMP4SplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CSource* pFilter, HRESULT* phr,DWORD nTrackNum,CMediaType* pMT);
	virtual ~CMP4SplitterOutputPin();

	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
	//HRESULT DeliverPacket(CAutoPtr<Packet> p);
	virtual HRESULT dealPacket(Packet* p);
	HRESULT DeliverEndFlush();
};

// {9649CBA0-2D87-4058-9CF7-5E95980EE99B}
// DEFINE_GUID(<<name>>, 
// 			0x9649cba0, 0x2d87, 0x4058, 0x9c, 0xf7, 0x5e, 0x95, 0x98, 0xe, 0xe9, 0x9b);
class __declspec(uuid("9649CBA0-2D87-4058-9CF7-5E95980EE99B"))
    CMP4SplitterFilter : public CBaseSplitterFilter
{
	friend class CMP4SplitterOutputPin;

protected:
    CAutoPtr<CMP4SplitterFile> m_pFile;
    virtual HRESULT CreateOutputs(IAsyncReader* pAsyncReader);
    virtual bool DemuxInit();
    //virtual void DemuxSeek(REFERENCE_TIME rt);
	virtual void DemuxSeek(REFERENCE_TIME rt,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL);
	virtual void DemuxOnlySeek(REFERENCE_TIME rt,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL);
	virtual REFERENCE_TIME getStartOffset(CBaseSplitterOutputPin* pOut);
	virtual bool isEof();
	virtual HRESULT DemuxNextPacket(REFERENCE_TIME rtStartOffset,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL,Packet* pPacket=NULL);

public:
    CMP4SplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
    virtual ~CMP4SplitterFilter();

	// CBaseFilter
    STDMETHODIMP_(HRESULT) QueryFilterInfo(FILTER_INFO* pInfo);

    // IKeyFrameInfo
    STDMETHODIMP_(HRESULT) GetKeyFrameCount(UINT& nKFs);
    STDMETHODIMP_(HRESULT) GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs);

protected:
	struct trackpos {
		DWORD /*AP4_Ordinal*/ index;
		unsigned __int64 /*AP4_TimeStamp*/ ts;
	};
	//该map保存相应id的track pos 数据，例如如果有两路音轨，则有2个id，每次读取之后记录track pos
	CAtlMap<DWORD, trackpos>	m_trackpos;
	CSize											m_framesize;

	MAPDWRT									m_mapStartOffset;
	REFERENCE_TIME						m_rtStartOffset;
	
};

// {E858F704-4B2A-47d8-A2CF-9BF7B57807D5}
// DEFINE_GUID(<<name>>, 
// 			0xe858f704, 0x4b2a, 0x47d8, 0xa2, 0xcf, 0x9b, 0xf7, 0xb5, 0x78, 0x7, 0xd5);
class __declspec(uuid("E858F704-4B2A-47d8-A2CF-9BF7B57807D5"))
    CMP4SourceFilter : public CMP4SplitterFilter
{
public:
    CMP4SourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

// for raw mpeg4 elementary streams:
// {CD9BE351-C189-4a21-9D3D-F24C24ABD417}
// DEFINE_GUID(<<name>>, 
// 			0xcd9be351, 0xc189, 0x4a21, 0x9d, 0x3d, 0xf2, 0x4c, 0x24, 0xab, 0xd4, 0x17);
class __declspec(uuid("CD9BE351-C189-4a21-9D3D-F24C24ABD417"))
    CMPEG4VideoSplitterFilter : public CBaseSplitterFilter
{
    int NextStartCode();
    void SkipUserData();

protected:
    CAutoPtr<CBaseSplitterFileEx> m_pFile;
    virtual HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

    virtual bool DemuxInit();
	virtual void DemuxSeek(REFERENCE_TIME rt,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL);
	virtual void DemuxOnlySeek(REFERENCE_TIME rt,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL);
	virtual REFERENCE_TIME getStartOffset(CBaseSplitterOutputPin* pOut);
	virtual bool isEof();
	virtual HRESULT DemuxNextPacket(REFERENCE_TIME rtStartOffset,CBaseSplitterOutputPin* pOut,__int64* pInt64=NULL,Packet* pPacket=NULL);
public:
    CMPEG4VideoSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);

private:
	__int64 m_seqhdrsize;
public:
	REFERENCE_TIME m_rtMpeg4;
	REFERENCE_TIME m_atpf;
};

// {2491B7BD-B9CA-488e-9BD4-17819CC4C70C}
// DEFINE_GUID(<<name>>, 
// 			0x2491b7bd, 0xb9ca, 0x488e, 0x9b, 0xd4, 0x17, 0x81, 0x9c, 0xc4, 0xc7, 0xc);
class __declspec(uuid("2491B7BD-B9CA-488e-9BD4-17819CC4C70C"))
    CMPEG4VideoSourceFilter : public CMPEG4VideoSplitterFilter
{
public:
    CMPEG4VideoSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};
