/*
	注释时间:2014-4-25
	author: glp
	混音设备管理类
*/
#pragma once
#include "WaveBuffer.h"
#include <vector>
#include <map>
#include <string>

typedef std::vector<MIXERCAPS>	VECTMIXERCAPS;
typedef std::vector<MIXERLINE>		VECTMIXERLINE;
typedef std::map<std::string,UINT>	MAPSTRDEVID;//一个可用的id名字，与对应的设备id，从0开始

class CMixerDev
{
public:
	CMixerDev(void);
	~CMixerDev(void);

	//默认枚举录音设备
	static void EnumMixerDevIdUnit(OUT MAPSTRDEVID& mapMixerInfo
														,OUT VECTMIXERCAPS& vectMixerCaps
														,IN const DWORD dwDestType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN);
	static void EnumMixerDev(/*out*/ VECTMIXERCAPS& vectMixerCaps);
	static bool EnumMixerDevUnit(/*out*/VECTMIXERLINE& vectMixerLine,
				/*in*/UINT uMxId,
				/*in*/DWORD dwDestType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN);

	BOOL Open(HWND hWnd,UINT defaultMxID = 1);//0是"主音量"，1是"录音控制"
	UINT getMixerID();

	//可以通过多种条件来检索：
	//方式一：根据目标单元编号MIXERLINE mxl
	//mxl.dwDestination=0;等同用mxl.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
	//mxl.dwDestination=1;等同用mxl.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_WAVEIN
	//对应flag: MIXER_GETLINEINFOF_DESTINATION
	//mxl.dwDestination=1;

	//方式二：根据线路类型
	//mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	//对应flag: MIXER_GETLINEINFOF_COMPONENTTYPE
	BOOL setRecordMixerLinebyType(MIXERLINE& mxl,MIXERLINE& mxlSub1,LPMIXERLINE pmxlSub2 = NULL,
					DWORD dwControlType=MIXERCONTROL_CONTROLTYPE_MUX,bool bClean=true);
	//MIXERCONTROL_CONTROLTYPE_VOLUME音量控制
	//MIXERCONTROL_CONTROLTYPE_MIXER 多个音频线路可以选多个
	//MIXERCONTROL_CONTROLTYPE_MUX 多个音频线路只能选一个
	//MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT,多个选项可以选择多个
	//MIXERCONTROL_CONTROLTYPE_SINGLESELECT 多个选项只能选一个
	//MIXERCONTROL_CONTROLTYPE_MUTE 静音控制

	//得到要求的输入的MIXERLINE，默认麦克风
	//pmxl代表主LINE，pmxlSub代表LINE子线
	//举例：pmxl代表"录音控制"的线路，pmxlSub可以代表"麦克风音量"的线路
	BOOL getRecordLineInfoSource(LPMIXERLINE pmxl,LPMIXERLINE pmxlSub, DWORD nSrcType);
	//MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT Stereo
	//MIXERLINE_COMPONENTTYPE_SRC_ANALOG Mono

	//BOOL getLineInfoDestination(LPMIXERLINE pmxl, DWORD dwSrcType);//得到要求的输出 的MIXERLINE
	BOOL GetLineInfo(LPMIXERLINE pmxl,LPMIXERLINE pmxlSub, DWORD dwDstType, DWORD dwSrcType);//得到要求的输出和输入 的MIXERLINE
	

	BOOL GetLineControl(LPMIXERCONTROL pmxc, LPMIXERLINE pmxl, DWORD dwType,DWORD fdwControls);
private:
	UINT							m_uMxId;
	HMIXER						m_hMixer;
	UINT							m_nMixerDevs;
};
