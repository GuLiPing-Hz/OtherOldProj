 // Wave.h: interface for the CWave class.
#ifndef WAVE__H__
#define WAVE__H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WaveBuffer.h"	
#include <string>

class CWave
{
	void SaveWaveHeader(FILE* f,WORD nChannels);

public:
	void SaveRight(const char* strFile);
	void SaveRight(FILE* f);
	void SaveLeft(const char* strFile);
	void SaveLeft(FILE* f);
	void SaveMean(const char* strFile);
	void SaveMean(FILE* f);

	void SaveLeftRight(const char* strLeft, const char* strRight);
	void SaveLeftRight(FILE* fl, FILE* fr);

	void SaveLeftRightRevert(const char* strFile);
	void SaveLeftRightRevert(FILE *f);
	
	DWORD	GetCurTotalBufferSize() const;
	DWORD	GetBufferLength() const;
	DWORD	GetNumSamples() const;
	void*		GetBuffer() const;
	WAVEFORMATEX GetFormat() const;

	//���ļ�����BuildFormat()����Ҫ�Ĳ���
	void LoadFormatFromFile(const std::string& fname);
	void BuildFormat(WORD nChannels, DWORD nFrequency, WORD nBits);
	//����bCopy������ȷ����CopyBuffer����SetBuffer
	//void SetBuffer(void* pBuffer, DWORD dwNumSamples, bool bCopy = false);
	void CopyBuffer(void* pBuffer,DWORD dwNumSamples);
	void AddBuffer(void* pBuffer,DWORD dwNumSamples);

	void Load(const char* strFile);
	void Load(FILE* f);
	void Save(const char* strFile);
	void Save(FILE* f);
	void LoadSegment(FILE* f, const double secLen); //����ǰsecLen�������
	void LoadSegment(const char* strFile, const double secLen); //����ǰsecLen�������	void Close();
	
	void Close();
	void ResetBuffer();
///////////////////////////////////////

	//����һ���뱾wav��ʽ��ͬ��wav,���ѻ�����ƴ������
	CWave& operator+(const CWave& wave);

	//�õ�ָ����ʼλ�ú���������buffer����
	void GetBufferByNumSamples(const DWORD start_num_sample, const DWORD num_samples,
								void*& pBuffer, DWORD& dwBufferLength) const;

	//�õ���ָ����ʼλ�ÿ�ʼ���������buffer����
	void GetRestBuffer(const DWORD start_num_sample, 
						void*& pBuffer, 
						DWORD& dwBufferLength,
						DWORD& dwRestNumSamples) const;


	CWave();
	//�ӹ��쿽������ʵ�ִ����п��Կ�������һ��CWave�������������
	CWave(const CWave& copy);
	CWave& operator=(const CWave& wave);
	virtual ~CWave();

private:
	CWaveBuffer m_buffer;
	WAVEFORMATEX m_pcmWaveFormat;
};

#endif//WAVE__H__