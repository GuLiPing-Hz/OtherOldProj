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

#include "util.h"

class CNullRenderer : public CBaseRenderer
{
protected:
    virtual HRESULT DoRenderSample(IMediaSample* pSample) { return S_OK; }

public:
    CNullRenderer(REFCLSID clsid, TCHAR* pName, LPUNKNOWN pUnk, HRESULT* phr);
};



class __declspec(uuid("0C38BDFD-8C17-4E00-A344-F89397D3E22A"))
    CNullAudioRenderer : public CNullRenderer
{
protected:
    HRESULT CheckMediaType(const CMediaType* pmt);

public:
    CNullAudioRenderer(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("64A45125-7343-4772-9DA4-179FAC9D462C"))
    CNullUAudioRenderer : public CNullRenderer
{
protected:
    HRESULT CheckMediaType(const CMediaType* pmt);

public:
    CNullUAudioRenderer(LPUNKNOWN pUnk, HRESULT* phr);
};

// class __declspec(uuid("655D7613-C26C-4A25-BBBD-3C9C516122CC"))
//     CNullTextRenderer : public CBaseFilter, public CCritSec
// {
//     class CTextInputPin : public CBaseInputPin
//     {
//     public:
//         CTextInputPin(CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
//             : CBaseInputPin(NAME("CTextInputPin"), pFilter, pLock, phr, L"In") {}
//         HRESULT CheckMediaType(const CMediaType* pmt);
//     };
// 
//     CAutoPtr<CTextInputPin> m_pInput;
// 
// public:
//     CNullTextRenderer(LPUNKNOWN pUnk, HRESULT* phr);
//     int GetPinCount() { return (int)!!m_pInput; }
//     CBasePin* GetPin(int n) { return n == 0 ? (CBasePin*)m_pInput : NULL; }
// };
