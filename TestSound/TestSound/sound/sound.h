#pragma  once

#include "bass.h"
//#include "../Opengl.h"

void CALLBACK LoopSyncProc(HSYNC handle, DWORD channel, DWORD data, void *user);

class CGLSound
{
public:
	CGLSound(HWND hWnd);
	virtual ~CGLSound();

public:
	bool				IsSoundInit(){return m_bInit;}
	bool				SoundLoad(const char* file,bool bloop=true);
	bool				SoundPlay();
	bool				SoundPause();
	bool				SoundStop();
	bool				SoundSetLoopStart(QWORD pos);
	bool				SoundSetLoopEnd(QWORD pos);
	bool				SoundSetGStreamVol(float fVol);
private:
	bool				SoundInit(HWND hWnd);
	void				SoundUninit();
private:
	bool					m_bInit;
	HSTREAM		m_hsam;
	DWORD			m_nchan;//ÒôÆµchannel¾ä±ú
	HSYNC			m_lsync;		// looping sync
	DWORD			m_bpp;
};
