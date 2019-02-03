// Wave.cpp: implementation of the CWave class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Wave.h"
#include <fstream>
#include <vector>

 
//////////////////////////////////////////////////////////////////////
CWave::CWave()
{
	ZeroMemory((void*)&m_pcmWaveFormat, sizeof(m_pcmWaveFormat));
	m_pcmWaveFormat.wFormatTag = 1;
}

//////////////////////////////////////////////////////////////////////
CWave::CWave(const CWave &copy)
{
	m_pcmWaveFormat = copy.GetFormat();
	m_buffer.SetNumSamples( copy.GetNumSamples(), copy.GetFormat().nBlockAlign ) ;
	m_buffer.CopyBuffer( copy.GetBuffer(), copy.GetNumSamples(), copy.GetFormat().nBlockAlign );
}

//////////////////////////////////////////////////////////////////////
CWave& CWave::operator =(const CWave& wave)
{
	if (&wave != this) {
		m_pcmWaveFormat = wave.GetFormat();
		m_buffer.SetNumSamples( wave.GetNumSamples(), wave.GetFormat().nBlockAlign );
		m_buffer.CopyBuffer( wave.GetBuffer(), wave.GetNumSamples(), wave.GetFormat().nBlockAlign );
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
CWave::~CWave()
{
}

//////////////////////////////////////////////////////////////////////
void CWave::BuildFormat(WORD nChannels, DWORD nFrequency, WORD nBits)
{
	m_pcmWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_pcmWaveFormat.nChannels = nChannels;
	m_pcmWaveFormat.nSamplesPerSec = nFrequency;
	m_pcmWaveFormat.nAvgBytesPerSec = nFrequency * nChannels * nBits / 8;
	m_pcmWaveFormat.nBlockAlign = nChannels * nBits / 8;
	m_pcmWaveFormat.wBitsPerSample = nBits;
	m_buffer.SetNumSamples(0L, m_pcmWaveFormat.nBlockAlign);
}	

//////////////////////////////////////////////////////////////////////
//inline WAVEFORMATEX CWave::GetFormat() const
WAVEFORMATEX CWave::GetFormat() const
{
	return m_pcmWaveFormat;
}

//////////////////////////////////////////////////////////////////////
/*
void CWave::Serialize( CArchive& archive )
{
	CFile* f = archive.GetFile();
	if (archive.IsLoading())
		Load(f);
	else
		Save(f);
}
*/
//////////////////////////////////////////////////////////////////////
void CWave::Load(const char* strFile)
{
	FILE* fp = fopen(strFile,"rb");
	if (fp)
	{
		Load(fp);
		fclose(fp);
	}
}

void CWave::Load(FILE* f)
{
	char szTmp[10];
	WAVEFORMATEX pcmWaveFormat;
	ZeroMemory(szTmp, 10);
	//f->Read(szTmp, 4 * sizeof(char)) ;
	fread(szTmp,1,4,f);
	if (strncmp(szTmp, ("RIFF"), 4) != 0) 
	{
		OutputDebugStringA("RIFF error\n");
		return ;
	}
	DWORD dwFileSize/* = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign + 36*/ ;
	//f->Read(&dwFileSize, sizeof(dwFileSize)) ;
	fread(&dwFileSize,1,sizeof(dwFileSize),f);
	ZeroMemory(szTmp, 10);
	//f->Read(szTmp, 8 * sizeof(char)) ;
	fread(szTmp,1,8,f);
	if (strncmp(szTmp, ("WAVEfmt "), 8) != 0) 
	{
		OutputDebugStringA("WAVEfmt  error\n");
		return ;
	}
	DWORD dwFmtSize; //= 16L, 不一定是16!
	//f->Read(&dwFmtSize, sizeof(dwFmtSize)) ;
	fread(&dwFmtSize,1,sizeof(dwFmtSize),f);
	//f->Read(&pcmWaveFormat.wFormatTag, sizeof(pcmWaveFormat.wFormatTag)) ;
	fread(&pcmWaveFormat.wFormatTag,1,sizeof(pcmWaveFormat.wFormatTag),f);
	//f->Read(&pcmWaveFormat.nChannels, sizeof(pcmWaveFormat.nChannels)) ;
	fread(&pcmWaveFormat.nChannels,1, sizeof(pcmWaveFormat.nChannels),f);
	//f->Read(&pcmWaveFormat.nSamplesPerSec, sizeof(pcmWaveFormat.nSamplesPerSec)) ;
	fread(&pcmWaveFormat.nSamplesPerSec,1, sizeof(pcmWaveFormat.nSamplesPerSec),f);
	//f->Read(&pcmWaveFormat.nAvgBytesPerSec, sizeof(pcmWaveFormat.nAvgBytesPerSec)) ;
	fread(&pcmWaveFormat.nAvgBytesPerSec,1, sizeof(pcmWaveFormat.nAvgBytesPerSec),f);
	//f->Read(&pcmWaveFormat.nBlockAlign, sizeof(pcmWaveFormat.nBlockAlign)) ;
	fread(&pcmWaveFormat.nBlockAlign,1, sizeof(pcmWaveFormat.nBlockAlign),f);
	//f->Read(&pcmWaveFormat.wBitsPerSample, sizeof(pcmWaveFormat.wBitsPerSample)) ;
	fread(&pcmWaveFormat.wBitsPerSample,1, sizeof(pcmWaveFormat.wBitsPerSample),f);

	//文件中读入的可能有误
	pcmWaveFormat.nAvgBytesPerSec = pcmWaveFormat.nSamplesPerSec 
		* pcmWaveFormat.nChannels * pcmWaveFormat.wBitsPerSample / 8;
	pcmWaveFormat.nBlockAlign = pcmWaveFormat.nChannels 
		* pcmWaveFormat.wBitsPerSample / 8;

	//跳过format chunk中的可能剩余部分
	//前面已经读了的format中的字节数是 16
	int bytes_need_to_jump;
	bytes_need_to_jump = dwFmtSize - 16;
	if ( bytes_need_to_jump != 0)
	{
		BYTE temp;
		for (int i = 0; i < bytes_need_to_jump; i++)
		{
			//f->Read(&temp, sizeof(BYTE));
			fread(&temp,1,sizeof(temp),f);
		}
	}

	//跳过非data的chunk
	char idTemp[5];//,
	memset(idTemp,0,5);
	//f->Read(idTemp, 4 * sizeof(BYTE));
	fread(idTemp,1,4,f);
	while ( strncmp(idTemp, "data", 4) != 0 )
	{
		DWORD chunk_size;
		//f->Read(&chunk_size, sizeof(DWORD));
		fread(&chunk_size,1,sizeof(DWORD),f);
		BYTE temp;
		for (int i = 0; i < (int)chunk_size; i++)
		{
			//f->Read(&temp, sizeof(BYTE));
			fread(&temp,1,1,f);
		}
		//f->Read(idTemp, 4 * sizeof(BYTE));
		fread(idTemp,1,4,f);
	}

	strncpy(szTmp, idTemp, 4);
	if (strncmp(szTmp, ("data"), 4) != 0) 
	{
		OutputDebugStringA("data  error\n");
		return ;
	}
	m_pcmWaveFormat = pcmWaveFormat;
	DWORD dwNum;
	//f->Read(&dwNum, sizeof(dwNum)) ;
	fread(&dwNum,1,sizeof(dwNum),f);
	m_buffer.SetNumSamples(dwNum / pcmWaveFormat.nBlockAlign, pcmWaveFormat.nBlockAlign);
	//f->Read(m_buffer.GetBuffer(), dwNum ) ;
	fread(m_buffer.GetBuffer(),1,dwNum,f);
}

void CWave::Save(const char* strFile)
{
	FILE* fp = fopen(strFile,"wb");
	if (fp)
	{
		Save(fp);
		fclose(fp);
	}
}

void CWave::Save(FILE *f)
{
	assert(f);

	if ( m_buffer.GetNumSamples() > 0 )
	{
		SaveWaveHeader(f,m_pcmWaveFormat.nChannels);
		// 		f->Write("RIFF", 4) ;
		// 		DWORD dwFileSize = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign + 36 ;
		// 		f->Write(&dwFileSize, sizeof(dwFileSize)) ;
		// 		f->Write("WAVEfmt ", 8) ;
		// 		DWORD dwFmtSize = 16L;
		// 		f->Write(&dwFmtSize, sizeof(dwFmtSize)) ;
		// 		f->Write(&m_pcmWaveFormat.wFormatTag, sizeof(m_pcmWaveFormat.wFormatTag)) ;
		// 		f->Write(&m_pcmWaveFormat.nChannels, sizeof(m_pcmWaveFormat.nChannels)) ;
		// 		f->Write(&m_pcmWaveFormat.nSamplesPerSec, sizeof(m_pcmWaveFormat.nSamplesPerSec)) ;
		// 		f->Write(&m_pcmWaveFormat.nAvgBytesPerSec, sizeof(m_pcmWaveFormat.nAvgBytesPerSec)) ;
		// 		f->Write(&m_pcmWaveFormat.nBlockAlign, sizeof(m_pcmWaveFormat.nBlockAlign)) ;
		// 		f->Write(&m_pcmWaveFormat.wBitsPerSample, sizeof(m_pcmWaveFormat.wBitsPerSample)) ;
		// 		f->Write("data", 4) ;
		DWORD dwNum = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign;
		// 		f->Write(&dwNum, sizeof(dwNum));
		//f->Write(m_buffer.GetBuffer(), dwNum);
		fwrite(m_buffer.GetBuffer(),1,dwNum,f);
	}
}

void CWave::LoadSegment(const char* strFile, const double secLen)
{
	FILE* fp = fopen(strFile,"rb");
	if (fp)
	{
		LoadSegment(fp,secLen);
		fclose(fp);
	}
}
void CWave::LoadSegment(FILE* f, const double secLen)
{
	char szTmp[10];
	WAVEFORMATEX pcmWaveFormat;
	ZeroMemory(szTmp, 10);
	//f->Read(szTmp, 4 * sizeof(char)) ;
	fread(szTmp,1,4,f);
	if (strncmp(szTmp, ("RIFF"), 4) != 0) 
	{
		OutputDebugStringA("RIFF error\n");
		return ;
	}
	DWORD dwFileSize/* = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign + 36*/ ;
	//f->Read(&dwFileSize, sizeof(dwFileSize)) ;
	fread(&dwFileSize,1,sizeof(dwFileSize),f);
	ZeroMemory(szTmp, 10);
	//f->Read(szTmp, 8 * sizeof(char)) ;
	fread(szTmp,1,8,f);
	if (strncmp(szTmp, ("WAVEfmt "), 8) != 0) 
	{
		OutputDebugStringA("WAVEfmt  error\n");
		return ;
	}
	DWORD dwFmtSize; //= 16L, 不一定是16!
	//f->Read(&dwFmtSize, sizeof(dwFmtSize)) ;
	fread(&dwFmtSize,1,sizeof(dwFmtSize),f);
	//f->Read(&pcmWaveFormat.wFormatTag, sizeof(pcmWaveFormat.wFormatTag)) ;
	fread(&pcmWaveFormat.wFormatTag,1,sizeof(pcmWaveFormat.wFormatTag),f);
	//f->Read(&pcmWaveFormat.nChannels, sizeof(pcmWaveFormat.nChannels)) ;
	fread(&pcmWaveFormat.nChannels,1, sizeof(pcmWaveFormat.nChannels),f);
	//f->Read(&pcmWaveFormat.nSamplesPerSec, sizeof(pcmWaveFormat.nSamplesPerSec)) ;
	fread(&pcmWaveFormat.nSamplesPerSec,1, sizeof(pcmWaveFormat.nSamplesPerSec),f);
	//f->Read(&pcmWaveFormat.nAvgBytesPerSec, sizeof(pcmWaveFormat.nAvgBytesPerSec)) ;
	fread(&pcmWaveFormat.nAvgBytesPerSec,1, sizeof(pcmWaveFormat.nAvgBytesPerSec),f);
	//f->Read(&pcmWaveFormat.nBlockAlign, sizeof(pcmWaveFormat.nBlockAlign)) ;
	fread(&pcmWaveFormat.nBlockAlign,1, sizeof(pcmWaveFormat.nBlockAlign),f);
	//f->Read(&pcmWaveFormat.wBitsPerSample, sizeof(pcmWaveFormat.wBitsPerSample)) ;
	fread(&pcmWaveFormat.wBitsPerSample,1, sizeof(pcmWaveFormat.wBitsPerSample),f);

	//文件中读入的可能有误
	pcmWaveFormat.nAvgBytesPerSec = pcmWaveFormat.nSamplesPerSec 
		* pcmWaveFormat.nChannels * pcmWaveFormat.wBitsPerSample / 8;
	pcmWaveFormat.nBlockAlign = pcmWaveFormat.nChannels 
		* pcmWaveFormat.wBitsPerSample / 8;

	//跳过format chunk中的可能剩余部分
	//前面已经读了的format中的字节数是 16
	int bytes_need_to_jump;
	bytes_need_to_jump = dwFmtSize - 16;
	if ( bytes_need_to_jump != 0)
	{
		BYTE temp;
		for (int i = 0; i < bytes_need_to_jump; i++)
		{
			//f->Read(&temp, sizeof(BYTE));
			fread(&temp,1,sizeof(temp),f);
		}
	}

	//跳过非data的chunk
	char idTemp[5];//,
	memset(idTemp,0,5);
	//f->Read(idTemp, 4 * sizeof(BYTE));
	fread(idTemp,1,4,f);
	while ( strncmp(idTemp, "data", 4) != 0 )
	{
		DWORD chunk_size;
		//f->Read(&chunk_size, sizeof(DWORD));
		fread(&chunk_size,1,sizeof(DWORD),f);
		BYTE temp;
		for (int i = 0; i < (int)chunk_size; i++)
		{
			//f->Read(&temp, sizeof(BYTE));
			fread(&temp,1,1,f);
		}
		//f->Read(idTemp, 4 * sizeof(BYTE));
		fread(idTemp,1,4,f);
	}

	strncpy(szTmp, idTemp, 4);
	if (strncmp(szTmp, ("data"), 4) != 0) 
	{
		OutputDebugStringA("data  error\n");
		return ;
	}
	m_pcmWaveFormat = pcmWaveFormat;

	//根据输入的时间secLen(秒),计算需要载入的长度
	DWORD dwNum;
	dwNum = (DWORD)( pcmWaveFormat.nSamplesPerSec * secLen * pcmWaveFormat.nBlockAlign );
	m_buffer.SetNumSamples(dwNum / pcmWaveFormat.nBlockAlign, pcmWaveFormat.nBlockAlign);
	fread(m_buffer.GetBuffer(),1,dwNum,f);
}
//////////////////////////////////////////////////////////////////////
void CWave::SaveWaveHeader(FILE* f,WORD nChannels)
{
	assert(f);

	WORD nBlockAlign = nChannels * m_pcmWaveFormat.wBitsPerSample / 8;
	DWORD nAvgBytesPerSec = nBlockAlign * m_pcmWaveFormat.nSamplesPerSec;
	//f->Write("RIFF", 4) ;
	fwrite("RIFF",1,4,f);
	DWORD dwFileSize = m_buffer.GetNumSamples() * nBlockAlign + 36 ;
	//f->Write(&dwFileSize, sizeof(dwFileSize)) ;
	fwrite(&dwFileSize,1,4,f);
	//f->Write("WAVEfmt ", 8) ;
	fwrite("WAVEfmt ",1,8,f);
	DWORD dwFmtSize = 16L;
	//f->Write(&dwFmtSize, sizeof(dwFmtSize)) ;
	fwrite(&dwFmtSize,1,sizeof(dwFmtSize),f);
	//f->Write(&m_pcmWaveFormat.wFormatTag, sizeof(m_pcmWaveFormat.wFormatTag)) ;
	fwrite(&m_pcmWaveFormat.wFormatTag,1,sizeof(m_pcmWaveFormat.wFormatTag),f);
	//f->Write(&nChannels, sizeof(nChannels)); //单声道
	fwrite(&nChannels,1,sizeof(nChannels),f);
	//f->Write(&m_pcmWaveFormat.nSamplesPerSec, sizeof(m_pcmWaveFormat.nSamplesPerSec)) ;
	fwrite(&m_pcmWaveFormat.nSamplesPerSec,1,sizeof(m_pcmWaveFormat.nSamplesPerSec),f);
	//f->Write(&nAvgBytesPerSec, sizeof(nAvgBytesPerSec)) ;
	fwrite(&nAvgBytesPerSec,1,sizeof(nAvgBytesPerSec),f);
	//f->Write(&nBlockAlign, sizeof(nBlockAlign)) ; //数据块长度
	fwrite(&nBlockAlign,1,sizeof(nBlockAlign),f);
	//f->Write(&m_pcmWaveFormat.wBitsPerSample, sizeof(m_pcmWaveFormat.wBitsPerSample)) ;
	fwrite(&m_pcmWaveFormat.wBitsPerSample,1,sizeof(m_pcmWaveFormat.wBitsPerSample),f);
	//f->Write("data", 4);
	fwrite("data",1,4,f);
	DWORD dwLength = m_buffer.GetNumSamples() * nBlockAlign;
	//f->Write(&dwLength, sizeof(dwLength));
	fwrite(&dwLength,1,sizeof(dwLength),f);
}

void CWave::SaveRight(const char* strFile)
{
	FILE* fp = fopen(strFile,"wb");
	SaveRight(fp);
	if (fp)
	{
		fclose(fp);
	}
}

void CWave::SaveRight(FILE* f)
{
	assert(f);
	assert( m_buffer.GetNumSamples() > 0 );

	//WORD nChannels=1;//单声道
	SaveWaveHeader(f,1);
	//一个字节一个字节的读入，再保存
	BYTE* pBuffer;
	pBuffer = (BYTE *)m_buffer.GetBuffer();

	//区分8位和16位的情况
	DWORD dwOrigLength = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign;
	if (m_pcmWaveFormat.wBitsPerSample == 8)
	{
		DWORD i = 1; //右声道的数据
		std::vector<BYTE> vBuffer;
		while( i < (int)dwOrigLength )
		{
			//f->Write(pBuffer+i, 1);
			vBuffer.push_back( (BYTE) *(BYTE *)(pBuffer + i) );
			i += 2;
		}
		fwrite((BYTE *)vBuffer.begin().operator ->(),1,vBuffer.size(),f);
	}
	else if (m_pcmWaveFormat.wBitsPerSample == 16)
	{
		int i = 2; //右声道的数据
		std::vector<__int16> vBuffer;
		while( i < (int)dwOrigLength )
		{
			vBuffer.push_back( (__int16) *(WORD *)(pBuffer + i));
			i += 4;
		}
		fwrite((WORD *)vBuffer.begin().operator ->(),1,vBuffer.size()*sizeof(WORD),f);
		// #ifdef _RS_VC60
		// 		f->Write( (WORD *)vBuffer.begin(), vBuffer.size() * sizeof(WORD));
		// #else 
		// 		f->Write( (WORD *)vBuffer.begin().operator ->(), vBuffer.size() * sizeof(WORD));
		// #endif
	}
}

void CWave::SaveLeft(const char* strFile)
{
	FILE* fp = fopen(strFile,"wb");
	SaveLeft(fp);
	if (fp)
	{
		fclose(fp);
	}
}
void CWave::SaveLeft(FILE* f)
{
	assert(f);
	assert( m_buffer.GetNumSamples() > 0 );

	//WORD nChannels= 1; //单声道
	SaveWaveHeader(f,1);
	//一个字节一个字节的读入，再保存
	BYTE* pBuffer;
	pBuffer = (BYTE *)m_buffer.GetBuffer();

	//区分8位和16位的情况
	DWORD dwOrigLength = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign;
	if (m_pcmWaveFormat.wBitsPerSample == 8)
	{
		DWORD i = 0;  //左声道数据
		std::vector<BYTE> vBuffer;
		while( i < (int)dwOrigLength )
		{
			//f->Write(pBuffer+i, 1);
			vBuffer.push_back( (BYTE) *(BYTE *)(pBuffer + i) );
			i += 2;
		}
		fwrite((BYTE *)vBuffer.begin().operator ->(),1,vBuffer.size(),f);
	}
	else if (m_pcmWaveFormat.wBitsPerSample == 16)
	{
		int i = 0;  //左声道数据
		std::vector<__int16> vBuffer;
		while( i < (int)dwOrigLength )
		{
			vBuffer.push_back( (__int16) *(WORD *)(pBuffer + i));
			i += 4;
		}
		fwrite((WORD *)vBuffer.begin().operator ->(),1,vBuffer.size()*sizeof(WORD),f);
		// #ifdef _RS_VC60
		// 		f->Write( (WORD *)vBuffer.begin(), vBuffer.size() * sizeof(WORD));
		// #else 
		// 		f->Write( (WORD *)vBuffer.begin().operator ->(), vBuffer.size() * sizeof(WORD));
		// #endif
	}
}
void CWave::SaveMean(const char* strFile)
{
	FILE* fp = fopen(strFile,"wb");
	SaveMean(fp);
	if (fp)
	{
		fclose(fp);
	}
}
void CWave::SaveMean(FILE* f)
{
	assert(f);
	assert( m_buffer.GetNumSamples() > 0 );

	//WORD nChannels= 1; //单声道
	SaveWaveHeader(f,1);
	//一个字节一个字节的读入，再保存
	BYTE* pBuffer;
	pBuffer = (BYTE *)m_buffer.GetBuffer();

	//区分8位和16位的情况
	DWORD dwOrigLength = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign;
	if (m_pcmWaveFormat.wBitsPerSample == 8)
	{
		DWORD i = 0;  //左声道数据
		std::vector<BYTE> vBuffer;
		while( i < (int)dwOrigLength )
		{
			BYTE lvalue, rvalue, mean;
			lvalue = *(pBuffer + i);
			rvalue = *(pBuffer + i + 1);
			mean = (BYTE)((lvalue + rvalue) * 0.5f);
			//f->Write(&mean, sizeof(mean));
			vBuffer.push_back(mean);
			i += 2;
		}
		fwrite((BYTE *)vBuffer.begin().operator ->(),1,vBuffer.size(),f);
	}
	else if (m_pcmWaveFormat.wBitsPerSample == 16)
	{
		int i = 0;  //左声道数据
		std::vector<__int16> vBuffer;
		while( i < (int)dwOrigLength )
		{
			WORD *p16;
			__int16 lvalue, rvalue, mean; 
			//这里必须是__int16,不能是WORD,因为16位的wav data是有符号的

			p16 = (WORD *)(pBuffer + i);
			lvalue = *p16;
			p16 = (WORD *)(pBuffer + i + 2);
			rvalue = *p16;
			mean = (__int16)((lvalue + rvalue) * 0.5f);
			//f->Write(&mean, sizeof(mean));
			vBuffer.push_back(mean);
			i += 4;
		}
		fwrite((WORD *)vBuffer.begin().operator ->(),1,vBuffer.size()*sizeof(WORD),f);
		// #ifdef _RS_VC60
		// 		f->Write( (WORD *)vBuffer.begin(), vBuffer.size() * sizeof(WORD));
		// #else 
		// 		f->Write( (WORD *)vBuffer.begin().operator ->(), vBuffer.size() * sizeof(WORD));
		// #endif
	}
}

void CWave::SaveLeftRight(const char* strLeft, const char* strRight)
{
	FILE* fpl = fopen(strLeft,"wb");
	FILE* fpr = fopen(strRight,"wb");
	SaveLeftRight(fpl, fpr);
	if (fpl)
	{
		fclose(fpl);
	}
	if (fpr)
	{
		fclose(fpr);
	}
}

void CWave::SaveLeftRight(FILE *fl, FILE *fr)
{
	assert(fl);
	assert(fr);
	assert( m_buffer.GetNumSamples() > 0 );

	//WORD nChannels= 1; //单声道
	SaveWaveHeader(fl,1);
	SaveWaveHeader(fr,1);

	//一个字节一个字节的读入，再保存
	BYTE* pBuffer;
	pBuffer = (BYTE *)m_buffer.GetBuffer();

	//区分8位和16位的情况
	DWORD dwOrigLength = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign;
	if (m_pcmWaveFormat.wBitsPerSample == 8)
	{
		int i = 0; 
		std::vector<BYTE> vlBuffer;
		std::vector<BYTE> vrBuffer;
		while( i < (int)dwOrigLength )
		{
			vlBuffer.push_back( (BYTE) *(BYTE *)(pBuffer + i) );
			vrBuffer.push_back( (BYTE) *(BYTE *)(pBuffer + 1 + i) );
			i += 2;
		}

		fwrite((BYTE *)vlBuffer.begin().operator->(),1,vlBuffer.size(),fl);
		fwrite((BYTE *)vrBuffer.begin().operator->(),1,vrBuffer.size(),fr);
		// #ifdef _RS_VC60
		// 		fl->Write((BYTE *)vlBuffer.begin(), vlBuffer.size() );
		// 		fr->Write((BYTE *)vrBuffer.begin(), vrBuffer.size() );
		// #else
		// 		fl->Write((BYTE *)vlBuffer.begin().operator ->(), vlBuffer.size() );
		// 		fr->Write((BYTE *)vrBuffer.begin().operator ->(), vrBuffer.size() );		
		// #endif

	}
	else if (m_pcmWaveFormat.wBitsPerSample == 16)
	{
		int i = 0;
		std::vector<__int16> vlBuffer;
		std::vector<__int16> vrBuffer;
		while( i < (int)dwOrigLength )
		{
			vlBuffer.push_back( (__int16) *(WORD *)(pBuffer + i));
			vrBuffer.push_back( (__int16) *(WORD *)(pBuffer + 2 + i));
			i += 4;
		}
		fwrite((BYTE *)vlBuffer.begin().operator->(),1,vlBuffer.size()*sizeof(WORD),fl);
		fwrite((BYTE *)vrBuffer.begin().operator->(),1,vrBuffer.size()*sizeof(WORD),fr);
	}
}

void CWave::SaveLeftRightRevert(const char* strFile)
{
	FILE* fp = fopen(strFile,"wb");
	SaveLeftRightRevert(fp);
	if (fp)
	{
		fclose(fp);
	}
}
void CWave::SaveLeftRightRevert(FILE *f)
{
	assert(f);
	assert( m_buffer.GetNumSamples() > 0 );

	SaveWaveHeader(f,m_pcmWaveFormat.nChannels);

	//一个字节一个字节的读入，再保存
	BYTE* pBuffer;
	pBuffer = (BYTE *)m_buffer.GetBuffer();

	//区分8位和16位的情况
	DWORD dwOrigLength = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign;
	if (m_pcmWaveFormat.wBitsPerSample == 8)
	{
		int i = 0; 
		std::vector<BYTE> vBuffer;
		while( i < (int)dwOrigLength )
		{
			vBuffer.push_back( (BYTE) *(BYTE *)(pBuffer + 1 + i) );
			vBuffer.push_back( (BYTE) *(BYTE *)(pBuffer + i) );
			i += 2;
		}
		fwrite((BYTE *)vBuffer.begin().operator ->(),1,vBuffer.size(),f);
	}
	else if (m_pcmWaveFormat.wBitsPerSample == 16)
	{
		int i = 0;
		std::vector<__int16> vBuffer;
		while( i < (int)dwOrigLength )
		{
			vBuffer.push_back( (__int16) *(WORD *)(pBuffer + 2 + i));
			vBuffer.push_back( (__int16) *(WORD *)(pBuffer + i));
			i += 4;
		}
		fwrite((WORD *)vBuffer.begin().operator ->(),1,vBuffer.size()*sizeof(WORD),f);
	}
}

void CWave::LoadFormatFromFile(const std::string& fname)
{
	WORD nChannels;
	DWORD nFrequency;
	WORD nBits;
	std::ifstream ifile(fname.c_str());
	ifile >> nChannels >> nFrequency >> nBits;

	BuildFormat(nChannels, nFrequency, nBits);
}

//////////////////////////////////////////////////////////////////////
void* CWave::GetBuffer() const
{
	return m_buffer.GetBuffer();
}

//////////////////////////////////////////////////////////////////////
DWORD CWave::GetNumSamples() const
{
	return m_buffer.GetNumSamples();
}

//////////////////////////////////////////////////////////////////////
DWORD CWave::GetBufferLength() const
{
	return ( GetNumSamples() * m_pcmWaveFormat.nBlockAlign );
}

DWORD CWave::GetCurTotalBufferSize() const
{
	return m_buffer.GetCurTotalBufferSize();
}

void CWave::AddBuffer(void* pBuffer,DWORD dwNumSamples)
{
	m_buffer.AddBuffer(pBuffer,dwNumSamples,m_pcmWaveFormat.nBlockAlign);
}
//////////////////////////////////////////////////////////////////////
void CWave::CopyBuffer(void* pBuffer, DWORD dwNumSample)
{
	m_buffer.CopyBuffer(pBuffer, dwNumSample, m_pcmWaveFormat.nBlockAlign);
}

void CWave::ResetBuffer()
{
	m_buffer.ResetBuffer();
}

void CWave::Close()
{
	//SetBuffer(NULL, 0, FALSE);
	ResetBuffer();
}

CWave& CWave::operator+(const CWave& wave)
{
	//确认两个wave的格式相同
	WAVEFORMATEX wave_format;
	wave_format = wave.GetFormat();
//	if ( m_pcmWaveFormat == wave_format )
	{
		m_buffer.AddBuffer( wave.GetBuffer(), wave.GetNumSamples(), wave.GetFormat().nBlockAlign);		
	}
	return *this;
}

void CWave::GetBufferByNumSamples(const DWORD start_num_sample, 
								  const DWORD num_samples, 
								  void*& pBuffer,
								  DWORD& dwBufferLength) const
{
	dwBufferLength = num_samples * m_pcmWaveFormat.nBlockAlign;
	pBuffer = new char[ dwBufferLength ];
	
	DWORD restNumSamples;
	restNumSamples = GetNumSamples() - start_num_sample;
	
	if ( (dwBufferLength > 0) && (num_samples <= restNumSamples) )
		memcpy(pBuffer, (char *)m_buffer.GetBuffer() + start_num_sample * m_pcmWaveFormat.nBlockAlign, dwBufferLength);
}

void CWave::GetRestBuffer(const DWORD start_num_sample, 
						  void*& pBuffer, 
						  DWORD& dwBufferLength,
						  DWORD& dwRestNumSamples) const
{
	dwRestNumSamples = GetNumSamples() - start_num_sample;

	dwBufferLength = dwRestNumSamples * m_pcmWaveFormat.nBlockAlign;
	pBuffer = new char[ dwBufferLength ];
	
	if ( dwBufferLength > 0 )
		memcpy(pBuffer, (char *)m_buffer.GetBuffer() + start_num_sample * m_pcmWaveFormat.nBlockAlign, dwBufferLength);
}
