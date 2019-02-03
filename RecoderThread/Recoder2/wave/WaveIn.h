/*
	注释时间:2014-4-25
	author: glp
	声音录入
*/

#ifndef WAVEIN__H__
#define WAVEIN__H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
#include "WaveDevice.h"

#ifndef __WXUTIL__
class CCritSec 
{
	// make copy constructor and assignment operator inaccessible

	CCritSec(const CCritSec &refCritSec);
	CCritSec &operator=(const CCritSec &refCritSec);

	CRITICAL_SECTION m_CritSec;

public:
	CCritSec() {
		InitializeCriticalSection(&m_CritSec);
	};

	~CCritSec() {
		DeleteCriticalSection(&m_CritSec);
	};

	void Lock() {
		EnterCriticalSection(&m_CritSec);
	};

	void Unlock() {
		LeaveCriticalSection(&m_CritSec);
	};
};

class CAutoLock 
{
	// make copy constructor and assignment operator inaccessible

	CAutoLock(const CAutoLock &refAutoLock);
	CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
	CCritSec * m_pLock;

public:
	CAutoLock(CCritSec * plock)
	{
		m_pLock = plock;
		m_pLock->Lock();
	};

	~CAutoLock() {
		m_pLock->Unlock();
	};
};
#endif//__WXUTIL__

//////////////////////////////////////////////////////////////////////
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

#define WAVEIN_BUFFER_SIZE 548

#define NUMWAVEINHDR 2

typedef enum _eRecordState
{
	RS_Running,
	RS_Pause,
	RS_Stop
}eRecordState;

//////////////////////////////////////////////////////////////////////
class CWaveIn  
{
	friend void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
public:

	CWaveIn();
	CWaveIn(WAVEFORMATEX tagFormat, const CWaveDevice& aDevice);
	virtual	~CWaveIn();

	static DWORD CALLBACK WaveInThread(LPVOID lparam);
public:
	void				zeroEndData();
	UINT			getNumDevs(){return m_nDevIds;}
	std::string	getError() const;
	DWORD		getPosition();
	bool				isRecording();
	CWave*		makeWave();

	//addedy by forrest
	DWORD getCurrentTime();

	void	setRecordFlag(bool bAddBuffer);
	bool resume();
	bool open(UINT dev_id,UINT bufferSize = 4096);
	bool open(const char* strDevName=NULL,UINT bufferSize = 4096);
	bool pause();
	bool record(UINT nTaille = WAVEIN_BUFFER_SIZE);
	bool stop();	
	void	close();

	DWORD	getNumSamples();
	void			setDevice(const CWaveDevice& aDevice);
	void			setWaveFormat(WAVEFORMATEX tagFormat);

private:
	bool			createThread(DWORD& threadID);
	bool			addNewBuffer(WAVEHDR* pWaveHdr);
	bool			addNewHeader(HWAVEIN hwi,unsigned int i,bool bClose=false);
	void			freeListOfHeader();
	void			initListOfHeader();
	bool			isError(MMRESULT nResult);

public:
	DWORD					m_dwThreadID;
	HANDLE					m_hThread;
	bool							m_bRunning;
	DWORD					m_dwLastBufferPos;
	DWORD					m_dwLastBufferStart;
	eRecordState			m_eRecordState;
	unsigned int			m_uQueue;
	CCritSec					m_csRecordState;
private:
	HWAVEIN				m_hWaveIn;
	bool							m_bAddBuffer;
	UINT						m_nError;
	int							m_nIndexWaveHdr;
	UINT						m_nBufferSize;
	WAVEHDR				m_tagWaveHdr[NUMWAVEINHDR];
	CWaveDevice			m_waveDevice;
	UINT						m_nDevIds;

public:
	CWave						m_wave;
};

#endif // WAVEIN__H__