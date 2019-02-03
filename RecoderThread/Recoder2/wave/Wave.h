/*
	注释时间:2014-4-25
	author: glp
	wave 结构类 对CWaveBuffer进行管理，保存当前wave的格式
*/
#ifndef WAVE__H__
#define WAVE__H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "WaveBuffer.h"	

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
	
	//获取当前buffer的实际总大小
	DWORD GetCurTotalBufferSize() const;
	//获取具体数据的长度，单位字节
	DWORD GetBufferLength() const;
	//获取多少个采样点
	DWORD GetNumSamples() const;
	//获取具体数据的内存地址
	void* GetBuffer() const;
	//获取音频格式
	WAVEFORMATEX GetFormat() const;

	//从文件载入BuildFormat()所需要的参数
	void LoadFormatFromFile(const std::string& fname);
	void BuildFormat(WORD nChannels, DWORD nFrequency, WORD nBits);
	//根据bCopy设置来确定是CopyBuffer还是SetBuffer
	//void SetBuffer(void* pBuffer, DWORD dwNumSamples, bool bCopy = false);
	void CopyBuffer(void* pBuffer,DWORD dwNumSamples);
	void AddBuffer(void* pBuffer,DWORD dwNumSamples);

	void Load(const char* strFile);
	void Load(FILE* f);
	void Save(const char* strFile);
	void Save(FILE* f);
	void LoadSegment(FILE* f, const double secLen); //载入前secLen秒的数据
	void LoadSegment(const char* strFile, const double secLen); //载入前secLen秒的数据	void Close();
	
	void Close();
	void ResetBuffer();
///////////////////////////////////////

	//加上一个与本wav格式相同的wav,即把缓冲区拼接起来
	CWave& operator+(const CWave& wave);

	//得到指定起始位置和样本数的buffer数据
	void GetBufferByNumSamples(const DWORD start_num_sample, const DWORD num_samples,
								void*& pBuffer, DWORD& dwBufferLength) const;

	//得到从指定起始位置开始往后的所有buffer数据
	void GetRestBuffer(const DWORD start_num_sample, 
						void*& pBuffer, 
						DWORD& dwBufferLength,
						DWORD& dwRestNumSamples) const;


	CWave();
	//从构造拷贝函数实现代码中可以看到构造一个CWave对象的三个步骤
	CWave(const CWave& copy);
	CWave& operator=(const CWave& wave);
	virtual ~CWave();

private:
	CWaveBuffer m_buffer;
	WAVEFORMATEX m_pcmWaveFormat;
};

#endif//WAVE__H__