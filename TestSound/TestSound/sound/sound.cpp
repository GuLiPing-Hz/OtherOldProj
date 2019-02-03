
#include "stdafx.h"
#include "sound.h"

#define WIDTH_TM 600

QWORD g_loop[2];//循环起始点，结束点

void CALLBACK LoopSyncProc(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	if (!BASS_ChannelSetPosition(channel,g_loop[0],BASS_POS_BYTE)) // try seeking to loop start
		BASS_ChannelSetPosition(channel,0,BASS_POS_BYTE); // failed, go to start of file instead
}


CGLSound::CGLSound(HWND hWnd)
:m_bInit(false)
,m_hsam(NULL)
,m_nchan(0)
{
	SoundInit(hWnd);
}

CGLSound::~CGLSound()
{
	SoundUninit();
}

bool CGLSound::SoundInit(HWND hWnd)
{

	if (HIWORD(BASS_GetVersion())!=BASSVERSION) {
		MessageBox(0,"An incorrect version of BASS.DLL was loaded",0,MB_ICONERROR);
		return 0;
	}

	if (!BASS_Init(-1,44100,0,hWnd,NULL)) 
	{
		//writeLog("Can't initialize device");
		return false;
	}

	m_bInit = true;
	return m_bInit;
	// check the correct BASS was loaded
	
}

void	CGLSound::SoundUninit()
{
	if (m_bInit)
	{
		if (m_hsam)
		{
			BASS_StreamFree(m_hsam);
		}
		BASS_Free();
	}
}

bool	CGLSound::SoundLoad(const char* file,bool bloop)
{
	if (bloop)
	{
		if (!m_bInit)
		{
			return false;
		}
		if (!(m_nchan=BASS_StreamCreateFile(FALSE,file,0,0,0))
			&& !(m_nchan=BASS_MusicLoad(FALSE,file,0,0,BASS_MUSIC_RAMPS|BASS_MUSIC_POSRESET|BASS_MUSIC_PRESCAN,1)))
		{
			//writeLog("music file open error");
			return false;
		}
		m_bpp = BASS_ChannelGetLength(m_nchan,BASS_POS_BYTE)/WIDTH_TM;
		if (m_bpp<BASS_ChannelSeconds2Bytes(m_nchan,0.02))
		{
			m_bpp = BASS_ChannelSeconds2Bytes(m_nchan,0.02);
		}
		BASS_ChannelSetSync(m_nchan,BASS_SYNC_END|BASS_SYNC_MIXTIME,0,LoopSyncProc,0);
	}
	else
	{
		m_hsam = BASS_StreamCreateFile(FALSE,file,0,0,0);
	}

	return true;
}

bool	CGLSound::SoundPlay()
{
	if (!m_bInit)
	{
		return false;
	}
	bool b = true;
	if (m_nchan)
	{
		b = BASS_ChannelPlay(m_nchan,TRUE);
	} 
	if (m_hsam)
	{
		b = BASS_ChannelPlay(m_hsam,TRUE);
	}
	return b;
}
bool	CGLSound::SoundPause()
{
	if (!m_bInit)
	{
		return false;
	}
	bool b = true;
	if (m_nchan)
	{
		b = BASS_ChannelPause(m_nchan);
	}
	if (m_hsam)
	{
		b = BASS_ChannelPause(m_hsam);
	}
	return b;
}

bool	CGLSound::SoundStop()
{
	if (!m_bInit)
	{
		return false;
	}
	bool b = true;
	if (m_nchan)
	{
		b = BASS_ChannelStop(m_nchan);
	}
	if (m_hsam)
	{
		b = BASS_ChannelStop(m_hsam);
	}
	return b;
}

bool	CGLSound::SoundSetLoopStart(QWORD pos)
{
	g_loop[0] = pos;
	return true;
}

bool	CGLSound::SoundSetLoopEnd(QWORD pos)
{
	if (!m_bInit)
	{
		return false;
	}
	g_loop[1] = pos;
	BASS_ChannelRemoveSync(m_nchan,m_lsync);
	m_lsync = BASS_ChannelSetSync(m_nchan,BASS_SYNC_POS|BASS_SYNC_MIXTIME,g_loop[1],LoopSyncProc,0);
	return true;
}

bool	CGLSound::SoundSetGStreamVol(float fVol)
{
	if(!m_bInit)
	{
		return false;
	}

	fVol = fVol>1.0f?1.0f:fVol<0.0f?0.0f:fVol;

	DWORD dwV = DWORD(fVol*10000);
	char buf[260] = {0};
	sprintf_s(buf,259,"cur volume %f\n",fVol);
	OutputDebugStringA(buf);
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM,dwV);

	bool b = true;
// 	if (m_nchan)
// 	{
// 		float oldV;
// 		b = BASS_ChannelGetAttribute ( m_nchan, BASS_ATTRIB_VOL, &oldV );
// 		b = BASS_ChannelSetAttribute(m_nchan,BASS_ATTRIB_VOL,fVol);
// 	}
// 	if (m_hsam)
// 	{
// 		b = BASS_ChannelSetAttribute(m_hsam,BASS_ATTRIB_VOL,fVol);
// 	}
	return b;
}
