/*
	注释时间:2014-4-25
	author: glp
	wave 设备类
*/
#ifndef WAVEDEVICE__H__
#define WAVEDEVICE__H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Wave.h"

class CWaveDevice  
{
public:
	CWaveDevice(const CWaveDevice& copy);
	UINT GetDevice() const;
	bool IsOutputFormat(const CWave& wave);
	bool IsInputFormat(const CWave& wave);
	CWaveDevice(UINT nDevice = WAVE_MAPPER);
	virtual ~CWaveDevice();

private:
	UINT m_nDevice;
};

#endif // WAVEDEVICE__H__
