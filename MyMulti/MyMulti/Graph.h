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
class CGraph//���ڹ���KTV mv�Ĳ��Ű�����Ƶ����Ƶ��graph
{
public:
	CGraph(HWND hwnd);
	virtual ~CGraph(void);

	static void releaseVMR9();
	HRESULT	gStartGraph(const char * fileName,bool notMV);
	//����filter����Ҫ�ٵ���gStartGraph֮�����
	HRESULT gConnectGraph();
	HRESULT gCloseGraph();
	HRESULT getDefaultNotify(IVMRSurfaceAllocatorNotify9** ppIVMRSurfAllocNotify);
	void errorUninit();
	/////////////////////////-----------0,�ɹ���-1ʧ��------------//////////////////////////////////
	//���ſ���
	int ktvStartPlayer(bool bFirstAudio);

	int restartPlayer();
	int startPlayer();
	int stopPlayer();
	int pausePlayer();
	int resumePlayer();
	//���õ�ǰλ��(����)
	int setCurPosition(ulong nposition_ms,bool bAbsolute=true);
	//������ʼ����λ��(����)
	int setStartStopPosition(ulong nstart_ms,ulong nstop_ms);
	//��ȡ��ǰλ��(����)
	int getPosition(ulong &nposition_ms);
	//��ȡý���ܳ���:����
	int getDuration(ulong &nduration_ms);
	//////////////////////////////////////////////////////////////////////////
	//�л�������һ��
	void switchAudio();
	//���������л�����
	void switchAudioEx(bool bFirstAudio);
	//�ı�����
	void changeCurPitch(const int nPitch);
	//������Ϣ
	HRESULT getGraphEvent(LONG& evCode,LONG& evParam1,LONG evParam2);
	HRESULT freeGraphEvent(LONG evCode,LONG evParam1,LONG evParam2);
	//�������
	int getVolume(long &lVolume);
	//�������� -10000��0֮��
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
	// ��ʼ������
	void initAudioTrack(long nIndex);
};