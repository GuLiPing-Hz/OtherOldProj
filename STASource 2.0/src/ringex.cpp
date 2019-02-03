#include "stdafx.h"
#include "ringex.h"

namespace Burning
{
//CRingPos类
////////////////////////////////////////////////////////////////////////// 
CRingPos::CRingPos()
{
	m_nPos = 0;
	m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_bOk = (m_hEvent != NULL);
}

CRingPos::~CRingPos()
{
	RingPosSetEvent();
	if ( m_hEvent != NULL )
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
}

bool CRingPos::IsOk()
{	
	return m_bOk;	
}

void CRingPos::clear()
{
	m_nPos = m_nRound = 0;
	ResetEvent(m_hEvent);
}

void CRingPos::RingPosWaitEvent( DWORD time )
{
	WaitForSingleObject(m_hEvent,time);
	ResetEvent(m_hEvent);
}

void CRingPos::RingPosSetEvent()
{
	SetEvent(m_hEvent);
}
//////////////////////////////////////////////////////////////////////////


//CRingEx类
//////////////////////////////////////////////////////////////////////////
CRingEx::CRingEx()
		:m_size(0),
		m_buffer(NULL),
		m_bQuit(false)
{
	m_fillevent = CreateEvent(NULL,FALSE,FALSE,NULL);
}
CRingEx::~CRingEx()
{
	m_bQuit = true;
	SafeComMemFree(m_buffer);
	SafeCloseHandle(m_fillevent);
}
void CRingEx::Close()
{
	m_write.RingPosSetEvent();
	m_read.RingPosSetEvent();
	m_bQuit = true;
}
bool CRingEx::Initialize(const unsigned int size)
{
	m_size = size;
	SafeComMemFree(m_buffer);
	m_buffer = (byte*)CoTaskMemAlloc(m_size);
	if(m_buffer == NULL) return false;
	Reset();
	return true;
}

unsigned int CRingEx::SetData(const byte* data,const unsigned int size,IWMReader* pReader)
{
	ASSERT( pReader != NULL);

	unsigned int write	= 0;	///< 能够写入的数据长度
	unsigned int length = 0;	///< 需要写入的数据长度
	unsigned int surplus= size;	///< 剩余写入的数据长度
	unsigned int writed = 0;	///< 已写入数据长度

	bool bPaused = false;

	while(surplus > 0 && !m_bQuit)
	{
		{
			Lockerguard locker(&m_locker);	///< 同步锁
			write = m_size - Length();
			length = min(write,surplus);

			if(m_write.m_nPos + length > m_size)
			{
				unsigned int first = m_size - m_write.m_nPos;
				memcpy(&m_buffer[m_write.m_nPos],&data[writed],first);
				m_write.m_nPos = (m_write.m_nPos + first) % m_size;
				writed += first;
				length -= first;
				++m_write.m_nRound;
			}

			if(length > 0)
			{
				memcpy(&m_buffer[m_write.m_nPos],&data[writed],length);
				m_write.m_nPos = (m_write.m_nPos + length) % m_size;
				writed += length;
			}
		}

		if(m_write.m_nPos == 0)
		{	
			++m_write.m_nRound;	
		}
		surplus = size - writed;

		if(surplus > 0)
		{
			pReader->Pause();
			bPaused = true;
			m_write.RingPosWaitEvent();
		}
	}

	if (bPaused)
		pReader->Resume();

	if(Length() >= m_size || m_bQuit)
	{
		SetEvent(m_fillevent);
	}

	return writed;
}

unsigned int CRingEx::Read(byte* data,const unsigned int& size)
{
	if(m_bQuit) return 0;
	unsigned int read	= 0;	///< 能够写入的数据长度
	unsigned int readed = 0;	///< 已读写数据

	{
		Lockerguard locker(&m_locker);
		read = Length();
		if(read <= 0) return 0;
		read = min(read,size);

		if(m_read.m_nPos + read > m_size)
		{
			unsigned int first = (m_size - m_read.m_nPos);
			memcpy(&data[readed],&m_buffer[m_read.m_nPos],first);
			m_read.m_nPos = (m_read.m_nPos + first) % m_size;
			readed += first;
			read -= first;
			++m_read.m_nRound;
		}

		if(read > 0)
		{
			memcpy(&data[readed],&m_buffer[m_read.m_nPos],read);
			m_read.m_nPos = (m_read.m_nPos + read) % m_size;
			readed += read;
		}
	}
	if(m_read.m_nPos == 0){	++m_read.m_nRound;	}
	m_write.RingPosSetEvent();

	return readed;
}

void CRingEx::Reset()
{
	ZeroMemory(m_buffer,m_size);
	m_bQuit = false;		///< 不允许删除,标记不需要强制退出.
	m_write.clear();
	m_read.clear();
	ResetEvent(m_fillevent);
	m_fillsize = 0;
}

void CRingEx::RingWaitFillEvent(unsigned int fillsize,DWORD time)
{
	m_fillsize = fillsize;
	WaitForSingleObject(m_fillevent,time);
	ResetEvent(m_fillevent);
}

unsigned int CRingEx::Length()
{
	if(m_bQuit) 
		return 0;

	Lockerguard locker(&m_locker);

	if(m_write.m_nPos == m_read.m_nPos)
	{
		if(m_write.m_nRound > m_read.m_nRound)
		{
			return m_size;
		}
		return 0;
	}
	if(m_write.m_nPos > m_read.m_nPos)
	{
		return abs(m_write.m_nPos - m_read.m_nPos);
	}
	return m_size - m_read.m_nPos + m_write.m_nPos;
}
//////////////////////////////////////////////////////////////////////////
}