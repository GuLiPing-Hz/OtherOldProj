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

// #include "stdafx.h"
#include "NullRenders.h"
#include "moreguids.h"

#define USE_DXVA

#ifdef USE_DXVA

// #include <d3d9.h>
// #include <dxva.h>
// #include <dxva2api.h>   // DXVA2
// #include <evr.h>
// #include <mfapi.h>      // API Media Foundation
// #include <Mferror.h>

// dxva.dll


#endif // USE_DXVA

//
// CNullRenderer
//

CNullRenderer::CNullRenderer(REFCLSID clsid, TCHAR* pName, LPUNKNOWN pUnk, HRESULT* phr)
    : CBaseRenderer(clsid, pName, pUnk, phr)
{
}

//
// CNullAudioRenderer
//

CNullAudioRenderer::CNullAudioRenderer(LPUNKNOWN pUnk, HRESULT* phr)
    : CNullRenderer(__uuidof(this), NAME("Null Audio Renderer"), pUnk, phr)
{
}

HRESULT CNullAudioRenderer::CheckMediaType(const CMediaType* pmt)
{
    return pmt->majortype == MEDIATYPE_Audio
           || pmt->majortype == MEDIATYPE_Midi
           || pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO
           || pmt->subtype == MEDIASUBTYPE_DOLBY_AC3
           || pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO
           || pmt->subtype == MEDIASUBTYPE_DTS
           || pmt->subtype == MEDIASUBTYPE_SDDS
           || pmt->subtype == MEDIASUBTYPE_MPEG1AudioPayload
           || pmt->subtype == MEDIASUBTYPE_MPEG1Audio
           ? S_OK
           : E_FAIL;
}

//
// CNullUAudioRenderer
//

CNullUAudioRenderer::CNullUAudioRenderer(LPUNKNOWN pUnk, HRESULT* phr)
    : CNullRenderer(__uuidof(this), NAME("Null Audio Renderer (Uncompressed)"), pUnk, phr)
{
}

HRESULT CNullUAudioRenderer::CheckMediaType(const CMediaType* pmt)
{
    return pmt->majortype == MEDIATYPE_Audio
           && (pmt->subtype == MEDIASUBTYPE_PCM
               || pmt->subtype == MEDIASUBTYPE_IEEE_FLOAT
               || pmt->subtype == MEDIASUBTYPE_DRM_Audio
               || pmt->subtype == MEDIASUBTYPE_DOLBY_AC3_SPDIF
               || pmt->subtype == MEDIASUBTYPE_RAW_SPORT
               || pmt->subtype == MEDIASUBTYPE_SPDIF_TAG_241h)
           ? S_OK
           : E_FAIL;
}

//
// CNullTextRenderer
//

// HRESULT CNullTextRenderer::CTextInputPin::CheckMediaType(const CMediaType* pmt)
// {
//     return pmt->majortype == MEDIATYPE_Text
//            || pmt->majortype == MEDIATYPE_ScriptCommand
//            || pmt->majortype == MEDIATYPE_Subtitle
//            || pmt->subtype == MEDIASUBTYPE_DVD_SUBPICTURE
//            || pmt->subtype == MEDIASUBTYPE_CVD_SUBPICTURE
//            || pmt->subtype == MEDIASUBTYPE_SVCD_SUBPICTURE
//            ? S_OK
//            : E_FAIL;
// }
// 
// CNullTextRenderer::CNullTextRenderer(LPUNKNOWN pUnk, HRESULT* phr)
//     : CBaseFilter(NAME("CNullTextRenderer"), pUnk, this, __uuidof(this), phr)
// {
//     m_pInput.Attach(DEBUG_NEW CTextInputPin(this, this, phr));
// }
