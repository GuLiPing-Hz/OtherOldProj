// WaveIn.cpp: implementation of the CWaveIn class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveIn.h"

extern HANDLE g_hEventEval;


//////////////////////////////////////////////////////////////////////
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	WAVEHDR* pWaveHdr = ( (WAVEHDR*)(DWORD_PTR)dwParam1 );
	CWaveIn* pWaveIn= NULL;
	if (pWaveHdr)
	{
		pWaveIn = (CWaveIn*)(pWaveHdr->dwUser);
		if (!pWaveIn)
		{
			return ;
		}
	}

	switch(uMsg) {
	case WIM_DATA:
		{
			do
			{
				if (pWaveHdr->dwBytesRecorded>0) 
				{
					pWaveIn->addNewBuffer(pWaveHdr);
					//SetEvent(g_hEventEval);
				}
				{//由于在调用waveInReset 还在回调函数里面执行以下代码，会导致死锁。所以增加了临界区的保护
					if (pWaveIn->m_eRecordState == RS_Running)
					{
						CAutoLock lock(&(pWaveIn->m_csRecordState));
						if ( pWaveIn->isError(waveInUnprepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR))) ) 
						{
							return ;
						}

						//if (pWaveHdr->dwBytesRecorded==WAVEIN_BUFFER_SIZE) /*pWaveHdr->dwFlags & WHDR_DONE == WHDR_DONE*/
						{
							////加入新的内存
							//if (pWaveHdr->dwBytesRecorded==WAVEIN_BUFFER_SIZE)//结束的时候，没填充满，跳过。否则程序会卡住。
							pWaveHdr->dwBytesRecorded=0;
							//将缓冲区信息和输入设备相关联
							if(pWaveIn->isError(waveInPrepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR))))
							{
								return ;
							}
							//将缓冲区地址和输入设备相关联
							if(pWaveIn->isError(waveInAddBuffer(hwi, pWaveHdr, sizeof(WAVEHDR))))
							{
								return ;
							}
							return ;
						}
					}
				}
			}while(false);
			pWaveIn->m_uQueue--;
			break;
		}		
	case WIM_CLOSE:
		{
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
CWaveIn::CWaveIn(WAVEFORMATEX tagFormat, const CWaveDevice& aDevice) : m_waveDevice(aDevice), \
m_hWaveIn(0), m_nIndexWaveHdr(-1),m_nBufferSize(0)
,m_eRecordState(RS_Stop),m_uQueue(0)
,m_bAddBuffer(true)
,m_bRunning(true)
,m_hThread(NULL)
,m_dwThreadID(0)
{
	m_nDevIds = waveInGetNumDevs();
	setWaveFormat(tagFormat);
	initListOfHeader();
}

//////////////////////////////////////////////////////////////////////
CWaveIn::CWaveIn() : m_hWaveIn(0), m_nIndexWaveHdr(-1), m_nBufferSize(0),m_eRecordState(RS_Stop),m_uQueue(0)
,m_bAddBuffer(true)
,m_bRunning(true)
,m_hThread(NULL)
,m_dwThreadID(0)
{
	m_nDevIds = waveInGetNumDevs();
	initListOfHeader();
	m_dwLastBufferPos=0;
	m_dwLastBufferStart=0;
}

//////////////////////////////////////////////////////////////////////
CWaveIn::~CWaveIn()
{
	m_bRunning = false;
	stop();
	WaitForSingleObject(m_hThread,INFINITE);
	if (m_hThread)
		CloseHandle(m_hThread);
}

void CWaveIn::setDevice(const CWaveDevice &aDevice)
{
	m_waveDevice = aDevice;
}

//////////////////////////////////////////////////////////////////////
void CWaveIn::setWaveFormat(WAVEFORMATEX tagFormat)
{
	m_wave.BuildFormat(tagFormat.nChannels, tagFormat.nSamplesPerSec, tagFormat.wBitsPerSample);
}

//////////////////////////////////////////////////////////////////////
void CWaveIn::initListOfHeader()
{
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		m_tagWaveHdr[i].lpData = NULL;
	}
}

void CWaveIn::close()//释放
{
	m_wave.Close();
}

void	CWaveIn::setRecordFlag(bool bAddBuffer)
{
	m_bAddBuffer = bAddBuffer;
}
//////////////////////////////////////////////////////////////////////
bool CWaveIn::resume()
{
	if (m_hWaveIn) {
		return !isError( waveInStart(m_hWaveIn) );
	}
	m_eRecordState = RS_Running;
	return true;
}

DWORD CALLBACK CWaveIn::WaveInThread(LPVOID lparam)
{
	CWaveIn* pWaveIn = (CWaveIn*)lparam;
	if(!pWaveIn)
	{
		return -1;
	}
	while (pWaveIn->m_bRunning)
	{
		MSG msg = {0};
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			LPWAVEHDR pWaveHdr = (LPWAVEHDR) (msg.lParam);
			if (msg.message == WIM_DATA)
			{
				do
				{
					if (pWaveHdr->dwBytesRecorded>0) 
					{
						pWaveIn->addNewBuffer(pWaveHdr);
						//SetEvent(g_hEventEval);
					}
					{//由于在调用waveInReset 还在回调函数里面执行以下代码，会导致死锁。所以增加了临界区的保护
						if (pWaveIn->m_eRecordState == RS_Running)
						{
							CAutoLock lock(&(pWaveIn->m_csRecordState));
							if ( pWaveIn->isError(waveInUnprepareHeader(pWaveIn->m_hWaveIn, pWaveHdr, sizeof(WAVEHDR))) ) 
							{
								break ;
							}

							//if (pWaveHdr->dwBytesRecorded==WAVEIN_BUFFER_SIZE) /*pWaveHdr->dwFlags & WHDR_DONE == WHDR_DONE*/
							{
								////加入新的内存
								//if (pWaveHdr->dwBytesRecorded==WAVEIN_BUFFER_SIZE)//结束的时候，没填充满，跳过。否则程序会卡住。
								pWaveHdr->dwBytesRecorded=0;
								//将缓冲区信息和输入设备相关联
								if(pWaveIn->isError(waveInPrepareHeader(pWaveIn->m_hWaveIn, pWaveHdr, sizeof(WAVEHDR))))
								{
									break ;
								}
								//将缓冲区地址和输入设备相关联
								if(pWaveIn->isError(waveInAddBuffer(pWaveIn->m_hWaveIn, pWaveHdr, sizeof(WAVEHDR))))
								{
									break ;
								}
								break ;
							}
						}
					}
					pWaveIn->m_uQueue--;
				}while(false);
			}
		}

		Sleep(1);
	}

	return 0;
}

bool CWaveIn::createThread(DWORD& threadID)
{	
	if (m_hThread)
	{
		threadID = m_dwThreadID;
		return true;
	}
	m_hThread = CreateThread(NULL,0,WaveInThread,(LPVOID)this,0,&m_dwThreadID);
	threadID = m_dwThreadID;
	return !!m_hThread;
}

bool CWaveIn::open(UINT dev_id,UINT bufferSize)
{
	MMRESULT mr;
	DWORD threadId;
	if(!createThread(threadId))
		return false;
	if (mr = waveInOpen(&m_hWaveIn, dev_id, &m_wave.GetFormat(), threadId, NULL, CALLBACK_THREAD) )
	{
		return false;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////
bool CWaveIn::open(const char* strDevName,UINT bufferSize )
{
	if (m_hWaveIn)
	{
		waveInClose(m_hWaveIn);
		m_hWaveIn = NULL;
	}

	MMRESULT mr;
	DWORD threadId;
	if(!createThread(threadId))
		return false;
	if (strDevName == NULL)
	{
		if (mr = waveInOpen(&m_hWaveIn, m_waveDevice.GetDevice(), &m_wave.GetFormat(), threadId, NULL, CALLBACK_THREAD) )
		{
			return false;
		}
	}
	else
	{
		WAVEINCAPS waveInCaps;
		UINT i;
		for (i=0;i<m_nDevIds;i++)
		{
			waveInGetDevCaps(i,&waveInCaps,sizeof(WAVEINCAPS));
			if (strcmp(strDevName,waveInCaps.szPname) == 0)
			{
				break;
			}
		}

		if (mr =waveInOpen(&m_hWaveIn, i, &m_wave.GetFormat(), threadId, NULL, CALLBACK_THREAD) )
		{
			return false;
		}
	}
	return true;

}

//////////////////////////////////////////////////////////////////////////

bool CWaveIn::pause()
{
	if (m_eRecordState != RS_Pause)
	{
		if (m_hWaveIn) {
			return !isError( waveInStop(m_hWaveIn) );
		}
		m_eRecordState = RS_Pause;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CWaveIn::record(UINT nTaille/* = 4096*/)
{
	assert(nTaille > 0);
	assert(m_hWaveIn);

	if (m_eRecordState == RS_Running)
	{
		return true;
	}

	setWaveFormat( m_wave.GetFormat() );
	m_nBufferSize = nTaille;
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		if ( !addNewHeader(m_hWaveIn,i) ) {
			return false;
		}
	}
	if ( isError(waveInStart(m_hWaveIn)) ) {
		return false;
	}
	m_eRecordState = RS_Running;
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CWaveIn::stop()
{
	if (m_hWaveIn)
	{
		if (m_eRecordState != RS_Stop)
		{
			m_eRecordState = RS_Stop;
			CAutoLock lock(&m_csRecordState);
			if ( isError(waveInReset(m_hWaveIn)) ) {
				return false;
			}
			Sleep(500);
			MMRESULT mr;
			for (int i=0;i<NUMWAVEINHDR;i++)/*MMSYSERR_NOERROR*/
			{
				mr = waveInUnprepareHeader (m_hWaveIn, &m_tagWaveHdr[i], sizeof (WAVEHDR));
				if(m_tagWaveHdr[i].lpData)
				{
					delete m_tagWaveHdr[i].lpData;
					m_tagWaveHdr[i].lpData = NULL;
				}
			}
			if ( isError( waveInClose(m_hWaveIn)) ) {
				return false;
			}
			assert(m_uQueue==0);
			m_hWaveIn = 0;
		}
	}
	return true;
}

bool CWaveIn::addNewBuffer(WAVEHDR *pWaveHdr)
{
	if(!m_bAddBuffer)
		return false;
	assert(pWaveHdr);
	m_wave.AddBuffer(pWaveHdr->lpData,pWaveHdr->dwBytesRecorded/m_wave.GetFormat().nBlockAlign);
	return true;
}

bool CWaveIn::addNewHeader(HWAVEIN hwi,unsigned int i,bool bClose)
{
	assert(m_nBufferSize > 0);

	m_nIndexWaveHdr = i;

	if (m_tagWaveHdr[m_nIndexWaveHdr].lpData == NULL) {
		m_tagWaveHdr[m_nIndexWaveHdr].lpData = new char[m_nBufferSize];//缓冲区地址  
	}

	ZeroMemory(m_tagWaveHdr[m_nIndexWaveHdr].lpData, m_nBufferSize);
	m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = m_nBufferSize;//缓冲区长度
	m_tagWaveHdr[m_nIndexWaveHdr].dwBytesRecorded = 0;//add by glp
	m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = 0;
	m_tagWaveHdr[m_nIndexWaveHdr].dwLoops = 1;
	m_tagWaveHdr[m_nIndexWaveHdr].dwUser = (DWORD_PTR)(void*)this;
	m_tagWaveHdr[m_nIndexWaveHdr].lpNext = NULL;
	m_tagWaveHdr[m_nIndexWaveHdr].reserved = 0;
	//将缓冲区信息和输入设备相关联
	if ( isError(waveInPrepareHeader(hwi, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	//将缓冲区地址和输入设备相关联
	if ( isError(waveInAddBuffer(hwi, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	m_uQueue++;
	return true;
}

//////////////////////////////////////////////////////////////////////
void CWaveIn::freeListOfHeader()
{
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		if (m_tagWaveHdr[i].lpData)
		{
			delete[] m_tagWaveHdr[i].lpData;
			m_tagWaveHdr[i].lpData = NULL;
		}
	}
}

DWORD CWaveIn::getNumSamples()
{
	return m_wave.GetNumSamples();
}

//////////////////////////////////////////////////////////////////////
std::string CWaveIn::getError() const
{
	if (m_nError != MMSYSERR_NOERROR) {
		TCHAR szText[MAXERRORLENGTH + 1];
		if ( waveInGetErrorText(m_nError, szText, MAXERRORLENGTH) == MMSYSERR_NOERROR ) {
			return szText;
		}
	}
	return "";
}

//////////////////////////////////////////////////////////////////////
DWORD CWaveIn::getPosition()
{
	if (m_hWaveIn) {
		MMTIME mmt;
		mmt.wType = TIME_SAMPLES;
		if ( isError(waveInGetPosition(m_hWaveIn, &mmt, sizeof(MMTIME))) ) {
			return -1;
		}
		else {
			return mmt.u.sample;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////
//addedy by forrest
DWORD CWaveIn::getCurrentTime()
{
	if (m_hWaveIn) {
		MMTIME mmt;
		mmt.wType = TIME_MS;
		if ( isError(waveInGetPosition(m_hWaveIn, &mmt, sizeof(MMTIME))) ) {
			return -1;
		}
		else {
			return mmt.u.ms;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////
bool CWaveIn::isError(MMRESULT nResult)
{
	m_nError = nResult;
	return (m_nError != MMSYSERR_NOERROR);
}

//////////////////////////////////////////////////////////////////////
bool CWaveIn::isRecording()
{
	return (m_eRecordState == RS_Running);
}

//////////////////////////////////////////////////////////////////////
CWave* CWaveIn::makeWave()
{
	return &m_wave;
}

void CWaveIn::zeroEndData()
{
	memset(((char*)m_wave.GetBuffer())+m_wave.GetBufferLength(),128,m_wave.GetCurTotalBufferSize()-m_wave.GetBufferLength());
}

