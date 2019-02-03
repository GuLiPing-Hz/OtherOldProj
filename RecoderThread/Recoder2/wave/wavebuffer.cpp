// WaveBuffer.cpp: implementation of the CWaveBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveBuffer.h"
//#include <loghelp.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CWaveBuffer::CWaveBuffer(DWORD bufferSize) 
: m_dwNum(0)
, m_pBuffer(NULL)
, m_nSampleSize(0)
, m_uCurPos(0)
, m_uBufferSize(bufferSize)
{
	m_pBuffer = new char[m_uBufferSize];
	memset(m_pBuffer,0,m_uBufferSize);
}

//////////////////////////////////////////////////////////////////////
CWaveBuffer::~CWaveBuffer()
{
	m_dwNum = 0L;
	if (m_pBuffer)
	{
		delete [] m_pBuffer;
		m_pBuffer = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
void* CWaveBuffer::GetBuffer() const
{
	return m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
DWORD CWaveBuffer::GetNumSamples() const
{
	return m_dwNum;
}

//////////////////////////////////////////////////////////////////////
void CWaveBuffer::CopyBuffer(void* pBuffer, DWORD dwNumSamples, int nSize)
{
	assert(dwNumSamples );
	assert(m_nSampleSize = nSize);
	DWORD addSize = dwNumSamples * nSize;
	assert(addSize <= m_uBufferSize);
	//////////////////////////////////////////////////////////////////////////
	//若pBuffer比m_pBuffer短，则m_pBuffer中多出部分以 0 补足
	int minLen = min(addSize, m_uBufferSize);
	memset(m_pBuffer,0, m_uBufferSize);
	memcpy(m_pBuffer, pBuffer, minLen);
	m_uCurPos = minLen;
	m_dwNum = dwNumSamples;
}

void CWaveBuffer::ResetBuffer()
{
	m_dwNum = 0;
	m_uCurPos = 0;
}

void CWaveBuffer::AddBuffer(void* pBuffer, DWORD dwNumSamples, int nSize )
{
	assert(dwNumSamples*nSize >= 0);
	assert(m_nSampleSize == nSize);

	DWORD uNewBuffer = dwNumSamples*nSize;
	DWORD uNeedSize = uNewBuffer + m_uCurPos;
	if ( uNeedSize > m_uBufferSize)
	{
		assert(m_pBuffer);
		char* pTemp = (char*) m_pBuffer;
		while(m_uBufferSize < uNeedSize)
		{
			m_uBufferSize <<= 1;
		}
		//LOG_IFN1(m_uBufferSize);
		m_pBuffer = new char[m_uBufferSize];
		if (m_pBuffer == NULL)
		{
			MessageBoxA(NULL,"ERROR","wave buffer new error!! please check the memory free",MB_OK);
			//LOG_ERN0("wave buffer new error!! please check the memory free");
			m_pBuffer = pTemp;
			return ;
		}

		memset(m_pBuffer,0,m_uBufferSize);
		memcpy(m_pBuffer,pTemp,m_uCurPos);
		delete [] pTemp;
	}
	memcpy(((char*)m_pBuffer)+m_uCurPos,pBuffer,uNewBuffer);
	m_uCurPos = uNeedSize;
	m_dwNum += dwNumSamples;
}

//////////////////////////////////////////////////////////////////////
void CWaveBuffer::SetNumSamples(DWORD dwNumSamples, int nSize)
{
	assert(dwNumSamples >= 0);
	assert(nSize > 0);

	void* pBuffer = NULL;

	m_dwNum = dwNumSamples;
	m_nSampleSize = nSize;
	DWORD uNeedBuffer = m_dwNum*m_nSampleSize;
	if (uNeedBuffer > m_uBufferSize)
	{
		assert(m_pBuffer);
		char* pTemp = (char*) m_pBuffer;
		while(m_uBufferSize < uNeedBuffer)
		{
			m_uBufferSize <<= 2;
		}
		m_pBuffer = new char[m_uBufferSize];
		delete [] pTemp;
	}	
	//SetBuffer(pBuffer, dwNumSamples, nSize);
}

//////////////////////////////////////////////////////////////////////
// void CWaveBuffer::SetBuffer(void *pBuffer, DWORD dwNumSamples, int nSize)
// {
// 	ASSERT(dwNumSamples >= 0);
// 	ASSERT(nSize);
// 
// 	//sunshuai修改，06-1-22
// 	if(m_pBuffer != NULL)
// 	{
// 		delete m_pBuffer;
// 	}
// 
// 	m_pBuffer = pBuffer;
// 	m_dwNum = dwNumSamples;
// 	m_nSampleSize = nSize;
// }

//////////////////////////////////////////////////////////////////////
int CWaveBuffer::GetSampleSize() const
{
	return m_nSampleSize;
}


