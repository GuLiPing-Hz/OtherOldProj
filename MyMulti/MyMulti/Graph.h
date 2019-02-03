#pragma once

#include "Opengl.h"
#include "vmrutil.h"
#include <InitGuid.h>
#include "iaudioswitch.h"

enum eFileType
{
	avi,
	mp4,
	mpeg2,
};

class CRenderEngine;
class CGraph//用于管理KTV mv的播放包含视频，音频的graph
{
public:
	CGraph(HWND hwnd);
	virtual ~CGraph(void);

	static void releaseVMR9();
	HRESULT	gStartGraph(const char * fileName,bool notMV);
	//连接filter，需要再调用gStartGraph之后调用
	HRESULT gConnectGraph();
	HRESULT gCloseGraph();
	HRESULT getDefaultNotify(IVMRSurfaceAllocatorNotify9** ppIVMRSurfAllocNotify);
	void errorUninit();
	/////////////////////////-----------0,成功；-1失败------------//////////////////////////////////
	//播放控制
	int ktvStartPlayer(bool bFirstAudio);

	int restartPlayer();
	int startPlayer();
	int stopPlayer();
	int pausePlayer();
	int resumePlayer();
	//设置当前位置(毫秒)
	int setCurPosition(ulong nposition_ms,bool bAbsolute=true);
	//设置起始结束位置(毫秒)
	int setStartStopPosition(ulong nstart_ms,ulong nstop_ms);
	//获取当前位置(毫秒)
	int getPosition(ulong &nposition_ms);
	//获取媒体总长度:毫秒
	int getDuration(ulong &nduration_ms);
	//////////////////////////////////////////////////////////////////////////
	//切换音轨会顿一下
	void switchAudio();
	//改善音轨切换体验
	void switchAudioEx(bool bFirstAudio);
	//改变音调
	void changeCurPitch(const int nPitch);
	//播放消息
	HRESULT getGraphEvent(LONG& evCode,LONG& evParam1,LONG evParam2);
	HRESULT freeGraphEvent(LONG evCode,LONG evParam1,LONG evParam2);
	//获得音量
	int getVolume(long &lVolume);
	//设置音量 -10000到0之间
	int setVolume(const long lVolume);
private:
	//HRESULT setAllocatorPresenter( IBaseFilter* filter,CRenderEngine* pallocator );
	HRESULT initGraph(const char* filename);
	void			uinitGraph();

private:
	ICaptureGraphBuilder2*	m_pBuilder;
	IGraphBuilder*					m_pGraph;

	IBaseFilter* 						m_pSourceFilter;
	IBaseFilter* 						m_pDemultiplexer ;
	IBaseFilter*							m_pAvisplitter;
	IBaseFilter* 						m_pVDecoder ;
	IBaseFilter* 						m_pADecoder1 ;
	IBaseFilter* 						m_pADecoder2 ;
	IBaseFilter* 						m_pAudioSwitcher ;
	IBaseFilter* 						m_pAudioRender;
	IBaseFilter*							m_pVMR9;//Video Mixing Render

	IFileSourceFilter*						m_pFileSourceFilter;
	IBasicAudio	*								m_pBasicAudio;
	IMediaControl*							m_pMediaControl;
	IMediaSeeking*							m_pMediaSeeking;
	IMediaEventEx*							m_pMediaEventEx;
	IAMStreamSelect*						m_pStreamSelect;
	IOS_AudioSwitch*						m_pOSAudioSwitch;
	IOS_ChangePitch*						m_pOSChangePitch;
	//IVMRSurfaceAllocatorNotify9* m_lpIVMRSurfAllocNotify;

	bool												m_nStream;
	bool												m_bUseSameFilter;
	bool												m_binitGraph;
	HWND											m_hWnd;
	bool												m_bNotMv;
public:
	// 初始化音轨
	void initAudioTrack(long nIndex);
};