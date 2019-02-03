/*
	注释时间:2014-4-25
	author: glp
	声音输出
*/
//////////////////////////////////////////////////////////////////////
#ifndef WAVEOUT__H__
#define WAVEOUT__H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
#include "WaveDevice.h"

//////////////////////////////////////////////////////////////////////
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

//////////////////////////////////////////////////////////////////////
#ifdef WAVE_OUT_BUFFER_SIZE
#undef WAVE_OUT_BUFFER_SIZE
#endif
#define WAVEOUT_BUFFER_SIZE 4096

#define NUMWAVEOUTHDR 3
#define INFINITE_LOOP INT_MAX

//////////////////////////////////////////////////////////////////////
class CWaveOut
{
	friend void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
public:
	std::string GetError() const;
	DWORD GetPosition();
	bool IsPlaying();
	
	bool Close();
	bool Continue();
	bool FullPlay(int nLoop = -1, DWORD dwStart = -1, DWORD dwEnd = -1);
	bool Open();
	bool Pause();
	bool Play(DWORD dwStart = -1, DWORD dwEnd = -1);
	bool Stop();

	void ModifyWaveOutBufferLength(DWORD dwLength);
	void SetDevice(const CWaveDevice& aDevice);
	void SetWave(const CWave& aWave);

	CWaveOut();
	CWaveOut(const CWave& aWave, const CWaveDevice& aDevice);
	virtual ~CWaveOut();
private:
	bool AddFullHeader(HWAVEOUT hwo, int nLoop);
	bool AddNewHeader(HWAVEOUT hwo);
	DWORD GetBufferLength();
	bool IsError(MMRESULT nResult);
	bool ResetRequired(CWaveOut* pWaveOut);
private:

	DWORD m_dwEndPos;
	DWORD m_dwStartPos;
	DWORD m_dwWaveOutBufferLength;
	HWAVEOUT m_hWaveOut;
	UINT m_nError;
	int m_nIndexWaveHdr;
	WAVEHDR m_tagWaveHdr[NUMWAVEOUTHDR];
	CWave m_wave;
	CWaveDevice m_waveDevice;
};

#endif // WAVEOUT__H__