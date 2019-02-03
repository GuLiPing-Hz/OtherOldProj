#include "stdafx.h"
#include "St4ReadStream.h"
#include <assert.h>

St4ReadStream::St4ReadStream(CBaseSplitterFile* pFile)
:m_nRef(1)
,m_pFile(pFile)
{
	assert(m_pFile != NULL);
}

St4ReadStream::~St4ReadStream(void)
{
	assert(m_nRef == 0);
}

void St4ReadStream::AddReference()
{
	InterlockedIncrement(&m_nRef);
}
void St4ReadStream::Release()
{
	InterlockedDecrement(&m_nRef);
	if(m_nRef == 0)
	{
		delete this;
	}
}

AP4_Result St4ReadStream::Read(void* buffer, AP4_Size bytesToRead, AP4_Size* bytesRead)
{
	if(m_pFile->GetPos() < 16)
	{//这个16为文件开头加密字符组  39-00-31-00-35-00-38-00-00-00-00-00-00-00-00-00
		m_pFile->Seek(16);
	}
	__int64 bytesAvail = m_pFile->GetRemaining()-16;
	//这个16为文件最后的的加密字符组
	//61-00-61-00-63-00-00-00-00-00-00-00-00-00-00-00

	if (bytesAvail < (LONGLONG)bytesToRead) {
		if (bytesRead) {
			*bytesRead = bytesAvail;
		}
		bytesToRead = bytesAvail;
	}

	if (bytesAvail == 0) {
		return AP4_ERROR_EOS;
	}

	if (FAILED(m_pFile->ByteRead((BYTE*)buffer, bytesToRead))) {
		if (bytesRead) {
			*bytesRead = 0;
		}
		return AP4_ERROR_READ_FAILED;
	}

	if (bytesRead) {
		*bytesRead = bytesToRead;
	}

	return AP4_SUCCESS;

}
AP4_Result St4ReadStream::Write(const void* buffer, AP4_Size bytesToWrite, AP4_Size* bytesWritten)
{
	return AP4_ERROR_WRITE_FAILED;
}
AP4_Result St4ReadStream::Seek(AP4_Offset offset)
{
	AP4_Offset realoffset = offset+16;
	m_pFile->Seek(realoffset);
	return m_pFile->GetPos() == realoffset ? AP4_SUCCESS : AP4_FAILURE;
}
AP4_Result St4ReadStream::Tell(AP4_Offset& offset)
{
	if((m_pFile->GetPos()+16)>m_pFile->GetLength())
	{
		m_pFile->Seek(m_pFile->GetLength()-16);
	}
	if (m_pFile->GetPos() < 16)
	{
		m_pFile->Seek(16);
	}
	offset = (AP4_Offset)(m_pFile->GetPos()-16);
	return AP4_SUCCESS;
}
AP4_Result St4ReadStream::GetSize(AP4_Size& size)
{
	size = (AP4_Size)(m_pFile->GetLength()-32);
	return AP4_SUCCESS;
}
