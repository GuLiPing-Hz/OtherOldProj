#pragma once

#include <commctrl.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <devicetopology.h>
#include <Audioclient.h>

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
{ (punk)->Release(); (punk) = NULL; }




typedef enum WindowsType //枚举操作系统类型
{
	UnknownOS = 0,
	WindowsXP = 1,
	WindowsVista = 2,
	Windows7 = 3,
};


#define MAX_VOL 100 //定义最大音量为100

class CMyVolumeCtrl
{

public:
	CMyVolumeCtrl(void);
	~CMyVolumeCtrl(void);
	int GetMaxVol(); //获得最大音量
	int GetMinVol(); //获得最小音量
	int GetVolume(int nType); //获得当前音量
	BOOL SetVolume(int nType,int nVol); //设置音量
	BOOL SetMute(int nType,BOOL bMute); //设置静音状态
	BOOL GetMute(int nType); //获取静音状态

protected: 
	BOOL vInitEndPoint(int nType);
	BOOL vMuteSet(int nType,BOOL bMute);
	BOOL vMuteGet(int nType);
	BOOL vVolumeSet(int nType,DWORD dwVolume);
	BOOL vVolumeGet(int nType,DWORD* pdwVolume);
	BOOL vUninitEndPoint();
	
protected:
	WindowsType vGetWindowsType();//获得当前操作系统类型

protected:
	WindowsType m_eWinType;
	IDeviceTopology *pDeviceTopology;
	IAudioEndpointVolume *m_pEndptVolCtrl;
	IMMDeviceEnumerator *m_pEnumerator;
	IAudioVolumeLevel *m_pVolumeLevel;
	GUID m_guidMyContext;

public:
	float VMin,VMax,VStep;
	IMMDevice *m_pDevice;
	BOOL SelectCaptureDevice(IMMDevice *pEndptDev);
	BOOL getMicrophoneBoostVolumeLevel(IMMDevice *pEndptDev);
	BOOL GetMicLevelRange();
	BOOL SetMicLevel(float fValue);
};
