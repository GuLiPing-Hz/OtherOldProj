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

#include "stdafx.h"
#include "MP4SplitterFile.h"
#include "Ap4AsyncReaderStream.h"
#include "St4ReadStream.h"

const BYTE ENCHEAD[] = {0x39,0x00,0x31,0x00,0x35,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

CMP4SplitterFile::CMP4SplitterFile(IAsyncReader* pReader, HRESULT& hr)
    : CBaseSplitterFileEx(pReader, hr, DEFAULT_CACHE_LENGTH, false)
    , m_pAp4File(NULL)
{
    if (FAILED(hr)) {
        return;
    }

    hr = Init();
}

CMP4SplitterFile::~CMP4SplitterFile()
{
	SAFE_DELETE(m_pAp4File);
}

void* /* AP4_Movie* */ CMP4SplitterFile::GetMovie()
{
    ASSERT(m_pAp4File);
    return m_pAp4File ? ((AP4_File*)m_pAp4File)->GetMovie() : NULL;
}

HRESULT CMP4SplitterFile::Init()
{
    Seek(0);

	SAFE_DELETE(m_pAp4File);

	BYTE enc[16] = {0};
	if(FAILED(ByteRead(enc,16)))
	{
		return E_FAIL;
	}

    AP4_ByteStream* stream = NULL;
	if(memcmp(enc,ENCHEAD,sizeof(ENCHEAD)) == 0)
	{
		stream = DEBUG_NEW St4ReadStream(this);
	}
	else
	{
		stream = DEBUG_NEW AP4_AsyncReaderStream(this);
	}
	Seek(0);
    m_pAp4File = DEBUG_NEW AP4_File(*stream);

    AP4_Movie* movie = ((AP4_File*)m_pAp4File)->GetMovie();

    stream->Release();

    return movie ? S_OK : E_FAIL;
}
