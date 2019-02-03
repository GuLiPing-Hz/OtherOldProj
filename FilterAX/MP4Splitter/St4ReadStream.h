#pragma once

#include "basesplitter/BaseSplitter.h"

#include "thirdparty/Bento4/Core/Ap4.h"
#include "thirdparty/Bento4/Core/Ap4File.h"
#include "thirdparty/Bento4/Core/Ap4ByteStream.h"


class St4ReadStream : public AP4_ByteStream
{
public:
	St4ReadStream(CBaseSplitterFile* pFile);
	virtual ~St4ReadStream(void);

	virtual void AddReference();
	virtual void Release();

	AP4_Result Read(void* buffer, AP4_Size bytesToRead, AP4_Size* bytesRead);
	AP4_Result Write(const void* buffer, AP4_Size bytesToWrite, AP4_Size* bytesWritten);
	AP4_Result Seek(AP4_Offset offset);
	AP4_Result Tell(AP4_Offset& offset);
	AP4_Result GetSize(AP4_Size& size);

private:
	long							m_nRef;
	CBaseSplitterFile*	m_pFile;
};
