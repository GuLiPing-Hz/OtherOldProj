/*
	注释时间:2014-4-25
	author: glp
	wave 管理类，包含录音保存，声音播放，设备选择等等
*/

#ifndef WAVEUNIT__H__
#define WAVEUNIT__H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataStructure.h"

#include "WaveIn.h"
#include "WaveOut.h"
#include "MixerDev.h"

#include <string>
#include <vector>

struct SWAVEIN_CONFIG
{
	WORD nChannels;
	DWORD nFrequency;
	WORD nBits;
	std::string strChannelSing;
	std::string strOrigFileName;
};

inline float Saturate(float input, float fMax);


class CWaveUnit  
{
public:
	BOOL initMixerDev(HWND hWnd,DWORD nRecordFrom,UINT device);
	BOOL initMixerDev(HWND hWnd,DWORD nRecordFrom1,DWORD nRecordFrom2,UINT device);

	void GetWaveInConfig(SWAVEIN_CONFIG & wavein_config);
	CWaveIn * GetWaveIn();

	CWaveUnit();
	virtual ~CWaveUnit();

	void SetWaveInConfig(const SWAVEIN_CONFIG& wavein_config);
	void SetWaveInConfig(const WORD& nChannels, const DWORD& nFrequency, const WORD& nBits);

	//单路录制
	BOOL		RecordOpen(HWND hWnd,DWORD nRecordFrom,UINT device = (UINT)-1,const char* strWaveDev=NULL);
	//多路录制（二路）
	BOOL		RecordOpen(HWND hWnd,DWORD nRecordFrom1,DWORD nRecordFrom2,UINT device = (UINT)-1,const char* strWaveDev=NULL);

	BOOL		RecordOpen();
	void			RecordStart();
	//根据测试，可以RecordPause后，再RecordPause；
	void			RecordPause();
	void			RecordContinue();
	void			RecordStop();
	void			RecordClose();
	void			setRecordFlag(bool bAddBuffer);
	void			GetRecordTime(int& ms);//ms
	DWORD	GetRecordPos();//sample


	void SetWaveObjOutFromWavFile(const std::string& fname);
	void PlayStart();
	void PlayPause();
	void PlayContinue();
	void PlayStop();

	//将waveIn得到声音以wave的格式存放到文件
	void SaveLeftRigthWavFileFormWaveIn(const std::string str1,const std::string str2);
	void SaveWavFileFromWaveIn(const std::string& fname);
	void SaveStereoWavFileFromWaveIn(const std::string& left_fname,const std::string& right_fname);

	//得到由waveIn输入的声音所生成的wave
	CWave* GetWaveFromWaveIn();


	//从wav文件读入得到wave
	static void GetWaveFromWavFile(const std::string& fname, CWave& wave);

	//从指定的wave得到其数据
	//如果wave是mono，则直接可得
	//如果wave是stereo, 则加权平均两个声道数据作为结果
	static void GetWaveData(const CWave& wave, std::vector<float>& vBuffer);

	//从指定的monowave得到其数据(vector)
	static void GetMonoWaveData(const CWave& wave, std::vector<float>& vBuffer);
	//stereo 16 转 mono 8 并获取其中的数据
	static void GetMonoEightFromStereoSixteenWaveData(const CWave& wave, std::vector<float>& vBuffer);
	static void GetMonoEightFromStereoSixteenWaveData2(const CWave& wave, std::vector<float>& vBuffer);
	static void GetMonoEightFromStereoSixteenWaveData3(const CWave& wave, std::vector<float>& vBuffer);
	static void GetMonoEightFromStereoSixteenWaveData4(const CWave& wave, std::vector<float>& vBuffer);

	//从指定的stereo wave得到其中一个指定声道的数据(vector)
	static void GetOneChannelOfStereoWaveData( const CWave& wave, const char channel, 
										std::vector<float>& vBuffer);

	//从指定的monowave得到其数据(pbuffer)
	static void GetMonoWaveData(const CWave& wave,
						 float** pfbuffer, int& bfLength);
	//从指定的stereo wave得到其中一个指定声道的数据(pbuffer)
	static void GetOneChannelOfStereoWaveData(const CWave& wave, const char channel,
									   float** pfbuffer, int& bfLength);
	/*
	从给定的wave对象中，获取某个声道的源数据
	@param wave[in]：指定的wave对象，需要是双声道格式的
	@param channel[in]：指定哪一个声道 
		'l' - 左声道；
		'r' - 右声道；
		@注：非法数据，默认右声道
	@param pByte[out]：声道数据存放的位置
	@param length[in out]：pByte的大小，返回实际存放的大小

	@注：pByte需要自己判断然后释放数据[delete]
	*/ 
	static void GetOneChannelOfSteroWaveData(const CWave& wave,const char channel,BYTE* &pByte,int& length,double& db);
	
	//分步得到waveIn缓冲区中的数据，
	//通过指定样本数目来控制每一步得到的数据量
	//waveIn的缓冲区是一个list Of waveBuffer，因此需要记录两个位置信息
	//start_pos: 标志是list中的哪一个buffer;
	//start_dwPosInWaveBuffer: 标志某一个waveBuffer中的位置


	BOOL IsPlaying();
	BOOL IsRecording();
	////////////////////////////////
	
	int  GetWaveInTailByNumSamples( const unsigned int num_samples, CWave& wave );
	////////////////////////////////
	
	//从源wave中生成指定样本的目标wave
	//即：waveDst是从waveSrc的第start_num_sample个样本开始，获取num_samples个样本数据所生成的CWave
	void GetWaveFromWaveByNumSamples(const CWave& waveSrc,
									 CWave& waveDst,
									 const DWORD start_num_sample, 
									 const DWORD num_samples);
	
private:
	SWAVEIN_CONFIG m_wavein_config;
	
	CWave					m_waveObjIn;
	CWave					m_waveObjOut;
	CWaveIn				m_waveIn;
	CWaveOut			m_waveOut;
	CWaveDevice		m_waveDeviceIn;
	CWaveDevice		m_waveDeviceOut;

	CMixerDev			m_mixerDev;
public:
};

#endif // WAVEUNIT__H__
