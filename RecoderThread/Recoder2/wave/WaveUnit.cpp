// WZ_WaveUnit.cpp: implementation of the CWZ_WaveUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveUnit.h"
#include <fstream>
#include <math.h>
//#include <loghelp.h>

//#define LOG_ERN0(x) MessageBoxA(NULL,x,"info",MB_OK)

static CWave wave_t;
static CWave wave_pre;

static byte Pitch[256]={-128,-127-126,-125,-124,-123,-122,-121,-120,-119,-118,-117,-116,-115,-114,-113,-112,-111,-110,-109,-108,-107,-106
										,-105,-104,-103,-102,-101,-100,-99,-98,-97,-96,-95,-94,-93,-92,-91,-90,-89,-88,-87,-86,-85,-84,-83,-82,-81,-80,-79,-78
										,-77,-76,-75,-74,-73,-72,-71,-70,-69,-68,-67,-66,-65,-64,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,-52,-51,-50,-49,-48
										,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,-37,-36,-35,-34,-33,-32,-31,-30,-29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18
										,-17,-16,-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24
										,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60
										,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94
										,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119
										,120,121,122,123,124,125,126,127
};
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaveUnit::CWaveUnit()
{

}

CWaveUnit::~CWaveUnit()
{

}

void CWaveUnit::SetWaveInConfig(const SWAVEIN_CONFIG& wavein_config)
{
	m_wavein_config = wavein_config;
}

void CWaveUnit::SetWaveInConfig(const WORD& nChannels, 
								   const DWORD& nFrequency, 
								   const WORD& nBits)
{
	m_wavein_config.nChannels = nChannels;
	m_wavein_config.nFrequency = nFrequency;
	m_wavein_config.nBits = nBits;
	m_wavein_config.strChannelSing = "";
	m_wavein_config.strOrigFileName = "";
}

BOOL CWaveUnit::RecordOpen()
{
	//	m_waveObj.BuildFormat(1, 8000, 16);
	//	m_waveObj.BuildFormat(1, 11025, 8);
	//从配置文件读入的格式设置
	m_waveObjIn.BuildFormat(m_wavein_config.nChannels, 
						  m_wavein_config.nFrequency, 
						  m_wavein_config.nBits);
	
	if ( !m_waveDeviceIn.IsInputFormat(m_waveObjIn) ) 
	{
		MessageBoxA(NULL,m_waveIn.getError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0("waveIn Format non't support");
		return FALSE;
	}
	
	m_waveIn.setWaveFormat( m_waveObjIn.GetFormat() );
	m_waveIn.setDevice(m_waveDeviceIn);
	
	if ( !m_waveIn.open() ) 
	{
		MessageBoxA(NULL,"WAVE ERROR",m_waveIn.getError().c_str(),MB_OK);
		//LOG_ERN0(m_waveIn.getError().c_str());
		return FALSE;
	}
	
	return TRUE;
}

BOOL CWaveUnit::RecordOpen(HWND hWnd,DWORD nRecordFrom1,DWORD nRecordFrom2,UINT device ,const char* strWaveDev)
{
	if (!initMixerDev(hWnd,nRecordFrom1,nRecordFrom2,device))
	{
		return FALSE;
	}
	//	m_waveObj.BuildFormat(1, 8000, 16);
	//	m_waveObj.BuildFormat(1, 11025, 8);
	//从配置文件读入的格式设置
	m_waveObjIn.BuildFormat(m_wavein_config.nChannels, 
		m_wavein_config.nFrequency, 
		m_wavein_config.nBits);

	if ( !m_waveDeviceIn.IsInputFormat(m_waveObjIn) ) 
	{
		MessageBoxA(NULL,"waveIn Format non support","WAVE ERROR",MB_OK);
		//LOG_ERN0("Format non support");
		return FALSE;
	}

	m_waveIn.setWaveFormat( m_waveObjIn.GetFormat() );
	m_waveIn.setDevice(m_waveDeviceIn);

	if ( !m_waveIn.open(strWaveDev) ) 
	{
		MessageBoxA(NULL,m_waveIn.getError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveIn.getError().c_str());
		return FALSE;
	}

	return TRUE;
}

BOOL CWaveUnit::RecordOpen(HWND hWnd,DWORD nRecordFrom,UINT device,const char* strWaveDev)
{
	if (!initMixerDev(hWnd,nRecordFrom,device))
	{
		return FALSE;
	}
	//	m_waveObj.BuildFormat(1, 8000, 16);
	//	m_waveObj.BuildFormat(1, 11025, 8);
	//从配置文件读入的格式设置
	m_waveObjIn.BuildFormat(m_wavein_config.nChannels, 
		m_wavein_config.nFrequency, 
		m_wavein_config.nBits);

	if ( !m_waveDeviceIn.IsInputFormat(m_waveObjIn) ) 
	{
		MessageBoxA(NULL,"waveIn Format non support","WAVE ERROR",MB_OK);
		//LOG_ERN0("waveIn Format non support");
		return FALSE;
	}

	m_waveIn.setWaveFormat( m_waveObjIn.GetFormat() );
	m_waveIn.setDevice(m_waveDeviceIn);

	if ( !m_waveIn.open(strWaveDev) ) 
	{
		MessageBoxA(NULL,m_waveIn.getError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveIn.getError().c_str());
		return FALSE;
	}

	return TRUE;
}


void CWaveUnit::RecordStart()
{
	if ( !m_waveIn.record() ) 
	{
		MessageBoxA(NULL,m_waveIn.getError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveIn.getError().c_str());
		return;
	}
}

void CWaveUnit::RecordContinue() 
{
	if ( !m_waveIn.resume() ) 
	{
		MessageBoxA(NULL,m_waveIn.getError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveIn.getError().c_str());
		return;
	}
}

void CWaveUnit::RecordPause() 
{
	if ( !m_waveIn.pause() ) 
	{
		MessageBoxA(NULL,m_waveIn.getError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveIn.getError().c_str());
		return;
	}
}

void CWaveUnit::RecordStop()
{
	if ( !m_waveIn.stop() ) 
	{
		MessageBoxA(NULL,m_waveIn.getError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveIn.getError().c_str());
		return;
	}
}

void CWaveUnit::RecordClose()
{
	m_waveIn.close();
}

void	CWaveUnit::setRecordFlag(bool bAddBuffer)
{
	m_waveIn.setRecordFlag(bAddBuffer);
}


void CWaveUnit::GetRecordTime(int& ms)
{
	//ms = m_waveIn.GetCurrentTime();
	DWORD pos;
	pos = m_waveIn.getPosition();

	ms = (int)((pos * 1000.0) / m_wavein_config.nFrequency);
	return;
}

//zyg [2005-02-17] /////////////
DWORD CWaveUnit::GetRecordPos()
{
	//ms = m_waveIn.GetCurrentTime();
	return m_waveIn.getPosition();
}

void CWaveUnit::SaveLeftRigthWavFileFormWaveIn(const std::string str1,const std::string str2)
{
	CWave* wave = m_waveIn.makeWave();
	wave->SaveLeftRight( str1.c_str(), str2.c_str());
}

void CWaveUnit::SaveWavFileFromWaveIn(const std::string& fname)
{
	CWave* wave = m_waveIn.makeWave();
	wave->Save( fname.c_str() );
}

CWave* CWaveUnit::GetWaveFromWaveIn()
{
	return  m_waveIn.makeWave();
}

void CWaveUnit::SaveStereoWavFileFromWaveIn(const std::string& left_fname, 
											   const std::string& right_fname)
{
	CWave* wave = m_waveIn.makeWave();
	wave->SaveLeft( left_fname.c_str() );
	wave->SaveRight( right_fname.c_str() );
}


void CWaveUnit::SetWaveObjOutFromWavFile(const std::string& fname)
{
	m_waveObjOut.Load( fname.c_str() );
}

void CWaveUnit::PlayStart()
{
	if ( m_waveOut.IsPlaying() ) 
	{
		if ( !m_waveOut.Close() ) 
		{
			MessageBoxA(NULL,m_waveOut.GetError().c_str(),"WAVE ERROR",MB_OK);
			//LOG_ERN0(m_waveOut.GetError().c_str());
			return;
		}
	}
	if ( !m_waveDeviceOut.IsOutputFormat(m_waveObjOut) ) 
	{
		MessageBoxA(NULL,m_waveOut.GetError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0("Format non support");
		return;
	}

	m_waveOut.SetWave(m_waveObjOut);
	m_waveOut.SetDevice(m_waveDeviceOut);
	
	if ( !m_waveOut.Open() ) 
	{
		MessageBoxA(NULL,m_waveOut.GetError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveOut.GetError().c_str());
		return;
	}
	if ( !m_waveOut.Play() ) 
	{
		MessageBoxA(NULL,m_waveOut.GetError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveOut.GetError().c_str());
		return;
	}
}

void CWaveUnit::PlayContinue() 
{
	if ( !m_waveOut.Continue() ) {
		MessageBoxA(NULL,m_waveOut.GetError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveOut.GetError().c_str());
	}
}

void CWaveUnit::PlayPause() 
{
	if ( !m_waveOut.Pause() ) {
		MessageBoxA(NULL,m_waveOut.GetError().c_str(),"WAVE ERROR",MB_OK);
	}
}

void CWaveUnit::PlayStop() 
{
	if ( !m_waveOut.Close() ) {
		MessageBoxA(NULL,m_waveOut.GetError().c_str(),"WAVE ERROR",MB_OK);
		//LOG_ERN0(m_waveOut.GetError().c_str());
	}
	
}

void CWaveUnit::GetWaveFromWavFile(const std::string& fname, CWave& wave)
{
	wave.Load( fname.c_str() );
}

void CWaveUnit::GetWaveData(const CWave& wave, 
							   std::vector<float>& vBuffer)
{
	//如果wave是mono，则直接可得
	//如果wave是stereo, 则加权平均两个声道数据作为结果
	if (wave.GetFormat().nChannels == 1) //mono
	{
		GetMonoWaveData(wave, vBuffer);
	}
	else
	{
		//GetMonoEightFromStereoSixteenWaveData2(wave,vBuffer);
		std::vector<float> vLeftBuffer;
		std::vector<float> vRightBuffer;
		GetOneChannelOfStereoWaveData(wave, 'l', vLeftBuffer);
		GetOneChannelOfStereoWaveData(wave, 'r', vRightBuffer);
		for(unsigned int i=0; i < vLeftBuffer.size(); i++)
		{
			float datum;
			datum = (vLeftBuffer[i] + vRightBuffer[i]) * 0.5f;
			vBuffer.push_back(datum);
		}
	}
}

void CWaveUnit::GetMonoWaveData(const CWave& wave, 
								   std::vector<float>& vBuffer)
{
	//BYTE* waveBuffer;
	unsigned char* waveBuffer;
	waveBuffer = (unsigned char *)wave.GetBuffer();
	DWORD dwBufferLength;
	dwBufferLength = wave.GetBufferLength();

	//dwBufferLength意义是所有样本 * 每个样本所占的字节数
	
	//分别处理8bit和16bit的情况
	//8bit mono: 每个样本一个字节
	if (wave.GetFormat().wBitsPerSample == 8)
    {
		for(unsigned int i = 0; i < dwBufferLength; i++) 
		{
			// make signed from unsigned
			vBuffer.push_back( (float)(waveBuffer[i]- 128  ));//- (unsigned char)128
		}
	}
	else if (wave.GetFormat().wBitsPerSample == 16)
	{
		//16bit mono: 每个样本两个字节
		//16bit的wave本身是从32767 ~ -32768
		//而负数的表示是由最高位为1来表示的
		//因此当用BYTE取得数据后拼装转换成WORD是无符号数据
		for(unsigned int i=0, j=0; i < dwBufferLength; i=i+2, j++) 
		{
			BYTE low, high;
			low = waveBuffer[i];
			high = waveBuffer[i+1];
			//MAKEWORD(low,high)把两个字节组合成16位的无符号数
			//(__int16)把16位的无符号数强制转换成有符号数, 注:如果用(int)则不行
			float datum;
			datum = (__int16)MAKEWORD(low, high);
			vBuffer.push_back( datum );
		}
	}
}

void CWaveUnit::GetMonoEightFromStereoSixteenWaveData(const CWave& wave, std::vector<float>& vBuffer)
{
	BYTE* waveBuffer = (BYTE *)wave.GetBuffer();
	DWORD dwBufferLength = wave.GetBufferLength();
	DWORD dwNumSamples = wave.GetNumSamples();


	float WeightStereo16ToMono8 = 0.5f;
	PSHORT pStereoLeft  = reinterpret_cast< short *>(waveBuffer);
	PSHORT pStereoRight = pStereoLeft + 1;

	LONG         lResult   = 0;

	const  float fcStep    = 1.0/16;

	DWORD dwNewSampleSize = dwBufferLength/4;
	//dwNewSampleSize = dwSrcSampleSize /4;
	char * pResample = new char[dwNewSampleSize];
	for (DWORD i=0; i< dwNewSampleSize; i++)
	{
		//---------------- Mixing data firstly.--------------------------------
		lResult =(LONG)((*pStereoLeft + *pStereoRight)* WeightStereo16ToMono8) ;
		if (lResult > 32767)
		{
			lResult = 32767;
			if (WeightStereo16ToMono8 > fcStep)
				WeightStereo16ToMono8 -= fcStep;
		}
		else if (lResult < -32768)
		{
			lResult = -32768;
			if (WeightStereo16ToMono8 > fcStep)
				WeightStereo16ToMono8 -= fcStep;
		}
		else
		{
			if ( WeightStereo16ToMono8 !=10.0/16)
				WeightStereo16ToMono8 += fcStep;
		}

		// Normalize the data value from 16 bits to 8 bits:Translate the data to positive ones.
		// The correct value is 1/257 = 255/65535, but it's wise to use 256 =2^8 for computer's
		// computation.65535/256 is 255, not overflow for BYTE.	      
		lResult /= 256;   // the value is between -128~127.
		//lResult += 128;		        // the value is between 0~255.

		//*(pDes + i)= (BYTE)lResult;
		vBuffer.push_back((CHAR)lResult);
		*pResample = (CHAR)lResult;
		pStereoLeft  += 2;
		pStereoRight += 2;
	}

	static bool b = false;
	if (!b)
	{
		wave_pre.BuildFormat(2,11025,16);
		wave_t.BuildFormat(1,11025,8);
		b= true;
	}
	wave_pre.AddBuffer(waveBuffer,dwNumSamples);
	wave_t.AddBuffer(pResample,dwNewSampleSize);
	static int n = 0;
	n++;
	if (n%1000 == 0)
	{
		wave_pre.Save("F:\\resamplepre.wav");
		wave_t.Save("F:\\resample.wav");
	}
	delete pResample;
}

inline float Saturate(float input, float fMax)
{
	static const float fGrdDiv = 0.5f;
	float x1 = fabsf(input + fMax);
	float x2 = fabsf(input - fMax);
	return fGrdDiv * (x1 - x2);
}

void CWaveUnit::GetMonoEightFromStereoSixteenWaveData2(const CWave& wave, std::vector<float>& vBuffer)
{
	BYTE* waveBuffer = (BYTE *)wave.GetBuffer();
	DWORD dwBufferLength = wave.GetBufferLength();
	DWORD dwNumSamples = wave.GetNumSamples();


	float WeightStereo16ToMono8 = 0.5f;
	PSHORT pData  = reinterpret_cast< short *>(waveBuffer);
	//PSHORT pStereoRight = pStereoLeft + 1;

	/*LONG         lResult   = 0;*/

	/*const  float fcStep    = 1.0/16;*/

	DWORD dwNewSampleSize = dwBufferLength/4;
	//dwNewSampleSize = dwSrcSampleSize /4;
	char * pResample = new char[dwNewSampleSize];
	for (DWORD i=0; i< dwNewSampleSize; i++)
	{
		float tmp_float = (*pData /*+ i*2)*/    + .5f) * (1.0f /  32767.503f);
		CHAR byteEnd  = (CHAR)((short)((Saturate(tmp_float, 1.0f) * 32767.505f) - .5f) / 256 /*+ 128*/);

		vBuffer.push_back((CHAR)byteEnd);
		*pResample = byteEnd;
		pData  += 2;
	}
	static bool b = false;
	if (!b)
	{
		wave_pre.BuildFormat(2,11025,16);
		wave_t.BuildFormat(1,11025,8);
		b= true;
	}
	wave_pre.AddBuffer(waveBuffer,dwNumSamples);
	wave_t.AddBuffer(pResample,dwNewSampleSize);
	static int n = 0;
	n++;
	if (n%1000 == 0)
	{
		wave_pre.Save("F:\\resamplepre.wav");
		wave_t.Save("F:\\resample.wav");
	}
	delete pResample;

}

void CWaveUnit::GetMonoEightFromStereoSixteenWaveData3(const CWave& wave, std::vector<float>& vBuffer)
{
	static bool b = false;
	if (!b)
	{
		wave_pre.BuildFormat(2,11025,16);
		wave_t.BuildFormat(1,11025,8);
		b= true;
	}

	BYTE* waveBuffer = (BYTE *)wave.GetBuffer();
	DWORD dwBufferLength = wave.GetBufferLength();
	DWORD dwNumSamples = wave.GetNumSamples();


	float WeightStereo16ToMono8 = 0.5f;
	PSHORT pData  = reinterpret_cast< short *>(waveBuffer);

	DWORD dwNewSampleSize = dwBufferLength/4;
	//dwNewSampleSize = dwSrcSampleSize /4;
	char * pResample2 = new char[dwNewSampleSize*1];
	for (DWORD i=0; i< dwNewSampleSize; i++)
	{
 		char buf[260] = {0};
// 		sprintf(buf,"ushort %d\n",(USHORT(*pData))>>8);
// 		OutputDebugStringA(buf);
		UCHAR byteEnd  = (UCHAR(((USHORT(*pData))>>8) -128) );
 		//sprintf(buf,"byte End %d\n",(int)byteEnd);
 		//OutputDebugStringA(buf);
		vBuffer.push_back(byteEnd);
		*pResample2 = byteEnd;
		pData  += 2;
	}
	
	wave_pre.AddBuffer(waveBuffer,dwNumSamples);
	wave_t.AddBuffer(pResample2,dwNewSampleSize);
	static int n = 0;
	n++;
	if (n%1000 == 0)
	{
		wave_pre.Save("F:\\resamplepre.wav");
		wave_t.Save("F:\\resample.wav");
	}
	delete pResample2;
}

void CWaveUnit::GetMonoEightFromStereoSixteenWaveData4(const CWave& wave, std::vector<float>& vBuffer)
{
	BYTE* waveBuffer = (BYTE *)wave.GetBuffer();
	DWORD dwBufferLength = wave.GetBufferLength();
	DWORD dwNumSamples = wave.GetNumSamples();


	float WeightStereo16ToMono8 = 0.5f;
	PSHORT pData  = reinterpret_cast< short *>(waveBuffer);

	DWORD dwNewSampleSize = dwBufferLength/4;
	//dwNewSampleSize = dwSrcSampleSize /4;
	char * pResample = new char[dwNewSampleSize];
	for (DWORD i=0; i< dwNewSampleSize; i++)
	{
		CHAR byteEnd  = (CHAR)(Pitch[(UCHAR((USHORT(*pData))>>8))]);
		vBuffer.push_back(byteEnd);
		*pResample = byteEnd;
		pData  += 2;
	}

	static bool b = false;
	if (!b)
	{
		wave_pre.BuildFormat(2,11025,16);
		wave_t.BuildFormat(1,11025,8);
		b= true;
	}
	wave_pre.AddBuffer(waveBuffer,dwNumSamples);
	wave_t.AddBuffer(pResample,dwNewSampleSize);
	static int n = 0;
	n++;
	if (n%1000 == 0)
	{
		wave_pre.Save("F:\\resamplepre.wav");
		wave_t.Save("F:\\resample.wav");
	}
	delete pResample;
}

void CWaveUnit::GetOneChannelOfStereoWaveData(const CWave& wave, 
												 const char channel,
												 std::vector<float>& vBuffer)
{
	if(wave.GetFormat().nChannels != 2)
	{
		vBuffer.clear();
		return;
	}

	BYTE* waveBuffer;
	DWORD dwBufferLength;
	waveBuffer = (BYTE *)wave.GetBuffer();
	dwBufferLength = wave.GetBufferLength();
	DWORD dwNumSamples;
	dwNumSamples = wave.GetNumSamples();
	
	
	//分别处理8bit和16bit的情况
	//8bit stereo: 每个样本两个字节
	if (wave.GetFormat().wBitsPerSample == 8)
    {
		for(unsigned int i = 0, j=0; i < dwBufferLength; i = i + 2, j++) 
		{
			BYTE a_sample = 0;
			if (channel == 'l') //取左声道数据
				a_sample = waveBuffer[i];
			else
				a_sample = waveBuffer[i + 1];
			//将无符号数变成有符号的数据
			vBuffer.push_back( (float)(a_sample - 128) );
		}
	}
	else if (wave.GetFormat().wBitsPerSample == 16)
	{
		//16bit stereo: 每个样本4个字节
		//16bit的wave本身是从32767 ~ -32768
		//而负数的表示是由最高位为1来表示的
		//因此当用BYTE取得数据后拼装转换成WORD是无符号数据
		for(unsigned int i=0, j=0; i < dwBufferLength; i=i+4, j++) 
		{
			BYTE low, high;
			if (channel == 'l')
			{
				low = waveBuffer[i];
				high = waveBuffer[i+1];
			}
			else
			{
				low = waveBuffer[i+2];
				high = waveBuffer[i+3];
			}
			//MAKEWORD(low,high)把两个字节组合成16位的无符号数
			//(__int16)把16位的无符号数强制转换成有符号数, 注:如果用(int)则不行
			float datum = 0.0f;
			//datum = (__int16)(MAKEWORD(low, high));
			datum = (short)(MAKEWORD(low, high)-32768);
			vBuffer.push_back( datum );
		}
	}
	
}

void CWaveUnit::GetMonoWaveData(const CWave& wave, 
								   float** pfbuffer, int& bfLength)
{

	if(wave.GetFormat().nChannels != 1)
	{
		*pfbuffer = NULL;
		bfLength = 0;
		return;
	}

	BYTE* waveBuffer;
	waveBuffer = (BYTE *)wave.GetBuffer();
	DWORD dwBufferLength;
	dwBufferLength = wave.GetBufferLength();
	DWORD dwNumSamples;
	dwNumSamples = wave.GetNumSamples();
	

	//分别处理8bit和16bit的情况
	//8bit:
	if (wave.GetFormat().wBitsPerSample == 8)
    {
		(*pfbuffer) = new float[dwBufferLength];
		bfLength = dwBufferLength;
		
		for(unsigned int i = 0; i < dwBufferLength; i++) 
		//for(int i = 0; i < dwNumSamples; i++) 
		{
			// make signed from unsigned
			(*pfbuffer)[i] = (float)(waveBuffer[i] - 128);
		}
	}
	else if (wave.GetFormat().wBitsPerSample == 16)
	{
		//16bit: 每个样本两个字节
		(*pfbuffer) = new float[dwBufferLength/2];
		bfLength = dwBufferLength / 2;

		//16bit的wave本身是从32767 ~ -32768
		//而负数的表示是由最高位为1来表示的
		//因此当用BYTE取得数据后拼装转换成WORD是无符号数据
		for(unsigned int i=0, j=0; i < dwBufferLength; i=i+2, j++) 
		{
			BYTE low, high;
			low = waveBuffer[i];
			high = waveBuffer[i+1];

			//MAKEWORD(low,high)把两个字节组合成16位的无符号数
			//(__int16)把16位的无符号数强制转换成有符号数, 注:如果用(int)则不行
			float datum;
			datum = (__int16)MAKEWORD(low, high);
			(*pfbuffer[j]) = datum; 
		}
	}
}

void CWaveUnit::GetOneChannelOfStereoWaveData(const CWave& wave, 
												 const char channel,
												 float** pfbuffer, 
												 int& bfLength)
{
	if(!pfbuffer)
		return ;

	if(wave.GetFormat().nChannels != 2)
	{
		*pfbuffer = NULL;
		bfLength = 0;
		return;
	}

	BYTE* waveBuffer;
	waveBuffer = (BYTE *)wave.GetBuffer();
	DWORD dwBufferLength;
	dwBufferLength = wave.GetBufferLength();
	DWORD dwNumSamples;
	dwNumSamples = wave.GetNumSamples();
	
	
	//分别处理8bit和16bit的情况
	//8bit: 每个样本两个字节
	if (wave.GetFormat().wBitsPerSample == 8)
    {
		(*pfbuffer) = new float[dwBufferLength / 2];
		bfLength = dwBufferLength / 2;
		
		for(unsigned int i = 0, j=0; i < dwBufferLength; i = i + 2, j++) 
		{
			BYTE a_sample;
			if (channel == 'l') //取左声道数据
				a_sample = waveBuffer[i];
			else
				a_sample = waveBuffer[i + 1];
			//将无符号数变成有符号的数据
			(*pfbuffer)[j] = (float)(a_sample - 128);
		}
	}
	else if (wave.GetFormat().wBitsPerSample == 16)
	{
		//16bit stereo: 每个样本4个字节
		(*pfbuffer) = new float[dwBufferLength/4];
		bfLength = dwBufferLength / 4;
		
		//16bit的wave本身是从32767 ~ -32768
		//而负数的表示是由最高位为1来表示的
		//因此当用BYTE取得数据后拼装转换成WORD是无符号数据
		for(unsigned int i=0, j=0; i < dwBufferLength; i=i+4, j++) 
		{
			BYTE low, high;
			if (channel == 'l')
			{
				low = waveBuffer[i];
				high = waveBuffer[i+1];
			}
			else
			{
				low = waveBuffer[i+2];
				high = waveBuffer[i+3];
			}
			//MAKEWORD(low,high)把两个字节组合成16位的无符号数
			//(__int16)把16位的无符号数强制转换成有符号数, 注:如果用(int)则不行
			float datum = 0.0f;
			//datum = (__int16)( 127.5*((unsigned int)MAKEWORD(low, high))/32767.5 );
			datum = (short)(MAKEWORD(low, high)-32768);
			(*pfbuffer[j]) = datum; 
		}
	}
}

void CWaveUnit::GetOneChannelOfSteroWaveData(const CWave& wave,const char channel,BYTE* &pByte,int& length,double& db)
{
	static double s_peak = 0.0;

	if(wave.GetFormat().nChannels != 2)
	{
		pByte = NULL;
		length = 0;
		return;
	}

	BYTE* waveBuffer;
	waveBuffer = (BYTE *)wave.GetBuffer();
	DWORD dwBufferLength;
	dwBufferLength = wave.GetBufferLength();
	DWORD dwNumSamples;
	dwNumSamples = wave.GetNumSamples();

	assert(dwBufferLength != 0);

	pByte = new BYTE[dwBufferLength/2];
	length = dwBufferLength / 2;

	//分别处理8bit和16bit的情况
	//8bit: 每个样本两个字节
	if (wave.GetFormat().wBitsPerSample == 8)
	{
		for(unsigned int i = 0, j=0; i < dwBufferLength; i = i + 2) 
		{
			BYTE a_sample;
			if (channel == 'l') //取左声道数据
				a_sample = waveBuffer[i];
			else
				a_sample = waveBuffer[i + 1];
			//将无符号数变成有符号的数据
			pByte[j++] = a_sample;

			float s = (float)(a_sample-128);
			float p = s*s;
			if (p>s_peak)
				s_peak = p;
			else
				s_peak *= 0.999;
		}
	}
	else if (wave.GetFormat().wBitsPerSample == 16)
	{
		//16bit的wave本身是从32767 ~ -32768
		//而负数的表示是由最高位为1来表示的
		//因此当用BYTE取得数据后拼装转换成WORD是无符号数据
		for(unsigned int i=0, j=0; i < dwBufferLength; i=i+4) 
		{
			BYTE low, high;
			if (channel == 'l')
			{
				low = pByte[j++] = waveBuffer[i];
				high = pByte[j++] = waveBuffer[i+1];
			}
			else
			{
				low = pByte[j++] = waveBuffer[i+2];
				high = pByte[j++] = waveBuffer[i+3];
			}

			short s_1 = MAKEWORD(low, high);
			double s = s_1 /*/ 32768.0*/;
			s = s*s;
			if (s>s_peak)
				s_peak = s;
			else
				s_peak *= 0.999;
		}
	}

	db = 10.0*log10(s_peak);
	char buf[260] = {0};
	sprintf(buf,"db %.2lf\n",db);
	OutputDebugStringA(buf);
}

int  CWaveUnit::GetWaveInTailByNumSamples( const unsigned int num_samples, CWave& wave )
{
	if (m_waveIn.getNumSamples() >= num_samples )
	{
		wave.BuildFormat(m_waveIn.m_wave.GetFormat().nChannels,m_waveIn.m_wave.GetFormat().nSamplesPerSec,m_waveIn.m_wave.GetFormat().wBitsPerSample);
		wave.CopyBuffer(((char*)m_waveIn.m_wave.GetBuffer())+(m_waveIn.m_wave.GetNumSamples()-num_samples)*m_waveIn.m_wave.GetFormat().nBlockAlign
			,num_samples);
		return num_samples;
	}
	return m_waveIn.getNumSamples();
}

BOOL CWaveUnit::IsPlaying()
{
	return m_waveOut.IsPlaying();
}

BOOL CWaveUnit::IsRecording()
{
	return m_waveIn.isRecording();
}

void CWaveUnit::GetWaveFromWaveByNumSamples(const CWave& waveSrc, 
											   CWave& waveDst, 
											   const DWORD start_num_sample, 
											   const DWORD num_samples)
{
	void *pBuffer = NULL;
	DWORD dwBufferLength;

	waveSrc.GetBufferByNumSamples(start_num_sample, num_samples, pBuffer, dwBufferLength);
	waveDst.BuildFormat(waveSrc.GetFormat().nChannels, waveSrc.GetFormat().nSamplesPerSec, waveSrc.GetFormat().wBitsPerSample);
	waveDst.CopyBuffer(pBuffer, num_samples);
}

CWaveIn * CWaveUnit::GetWaveIn()
{
	return &m_waveIn;
}

void CWaveUnit::GetWaveInConfig(SWAVEIN_CONFIG &wavein_config)
{
	wavein_config = m_wavein_config;
}

BOOL CWaveUnit::initMixerDev(HWND hWnd,DWORD nRecordFrom,UINT device)
{
	if (!m_mixerDev.Open(hWnd,device))
	{
		return FALSE;
	}

	MIXERLINE mxl;
	MIXERLINE mxlSub;

	if (!m_mixerDev.getRecordLineInfoSource(&mxl,&mxlSub,nRecordFrom))
	{
		MessageBoxA(NULL,"getRecordLineInfoSource failed","WAVE ERROR",MB_OK);
		//LOG_ERN0("getRecordLineInfoSource failed");
		return FALSE;
	}
	if (!m_mixerDev.setRecordMixerLinebyType(mxl,mxlSub))
	{
		//w riteLog("[DXLOG CWZ_WaveUnit::initMixerDev]:setRecordMixerLinebyType failed Don't worry!!");
		//return FALSE;
	}
	return TRUE;
}

BOOL CWaveUnit::initMixerDev(HWND hWnd,DWORD nRecordFrom1,DWORD nRecordFrom2,UINT device)
{
	if (!m_mixerDev.Open(hWnd,device))
	{
		return FALSE;
	}

	MIXERLINE mxl;
	MIXERLINE mxlSub1;
	MIXERLINE mxlSub2;
	if (!m_mixerDev.getRecordLineInfoSource(&mxl,&mxlSub1,nRecordFrom1))
	{
		MessageBoxA(NULL,"mxlSub1 getRecordLineInfoSource failed","WAVE ERROR",MB_OK);
		//LOG_ERN0("mxlSub1 getRecordLineInfoSource failed");
		return FALSE;
	}
	if (!m_mixerDev.setRecordMixerLinebyType(mxl,mxlSub1))
	{
		//w riteLog("[DXLOG CWZ_WaveUnit::initMixerDev]:setRecordMixerLinebyType failed Don't worry!!");
		//return FALSE;
	}

	if (!m_mixerDev.getRecordLineInfoSource(&mxl,&mxlSub2,nRecordFrom2))
	{
		MessageBoxA(NULL,"mxlSub2 getRecordLineInfoSource failed","WAVE ERROR",MB_OK);
		//LOG_ERN0("mxlSub2 getRecordLineInfoSource failed");
		return FALSE;
	}
	if (!m_mixerDev.setRecordMixerLinebyType(mxl,mxlSub2))
	{
		//w riteLog("[DXLOG CWZ_WaveUnit::initMixerDev]:setRecordMixerLinebyType failed Don't worry!!");
		//return FALSE;
	}
	return TRUE;
}
