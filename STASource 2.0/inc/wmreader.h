#pragma once
#include "wmreadercb.h"
#include "ringex.h"
#include "Stream.h"
/**
	@file wmreader.h
	@brief wmreader 功能封装对象
*/

struct tag_asf{
	bool	_audio;
	bool	_video;
	bool	_image;
	bool	_script;
	tag_asf(){
		clear();
	}
	void clear(){
		_audio = _video = _image = _script = false;
	}
};

struct READERATTRIBUTE{
	tstring			_language;		///< 音轨语言
	bool			_isMultiStream;	///< 多音轨标记
	QWORD			_duration;		///< 总时长
	bool			_canseek;		///< 是否可以定位
	bool			_canbroadcast;	///< 是否支持广播
	DWORD			_number;		///< 数据流的编号
	tag_asf			_asftype;
	QWORD			_qwTimeElapsed;	///< 已经逝去的时间
	bool			_iseof;			///< 是否已经结束
	bool			_bQuit;			///< 退出控制
	AM_MEDIA_TYPE*	_pAMMediaType;	///< 媒体类型
	READERATTRIBUTE():_pAMMediaType(NULL){
		clear();
	}

	~READERATTRIBUTE(){
		clear();
	}

	void clear(){
		_language		= TEXT("");
		_isMultiStream	= false;
		_canseek		= false;
		_canbroadcast	= false;
		_number			= 0;
		_asftype.clear();
		_qwTimeElapsed	= 0;
		_iseof			= false;
		_bQuit			= false;
		SafeComMemFree(_pAMMediaType);
	}
};

class CWMReader:public IWMReaderCallback,public IWMReaderCallbackAdvanced{
public:
	CWMReader();
	~CWMReader();

	HRESULT STDMETHODCALLTYPE		QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject ); 
	ULONG STDMETHODCALLTYPE			AddRef();
	ULONG STDMETHODCALLTYPE			Release();

	// --- IWMReaderCallback methods --- 
	HRESULT STDMETHODCALLTYPE		OnStatus(WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext);
	HRESULT STDMETHODCALLTYPE		OnSample(DWORD dwOutputNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR *pSample,void __RPC_FAR *pvContext );

	// --- IWMReaderCallbackAdvanced methods --- 
	virtual HRESULT STDMETHODCALLTYPE OnStreamSample(WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext); ///< 1
	virtual HRESULT STDMETHODCALLTYPE OnTime(QWORD qwCurrentTime,void __RPC_FAR * pvContext);	///< 1
	virtual HRESULT STDMETHODCALLTYPE OnStreamSelection(WORD wStreamCount,WORD __RPC_FAR * pStreamNumbers,WMT_STREAM_SELECTION __RPC_FAR * pSelections,void __RPC_FAR * pvContext); ///< 1
	virtual HRESULT STDMETHODCALLTYPE OnOutputPropsChanged(DWORD dwOutputNum,WM_MEDIA_TYPE __RPC_FAR * pMediaType,void __RPC_FAR * pvContext );	///< 1
	virtual HRESULT STDMETHODCALLTYPE AllocateForOutput(DWORD dwOutputNum,DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext);
	virtual HRESULT STDMETHODCALLTYPE AllocateForStream(WORD wStreamNum,DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext);

	HRESULT			Initialize();				///< 初始化函数
	void			UnInitialize();
	HRESULT			Open(LPCTSTR pwszUrl);		///< 打开文件
	HRESULT			Open(IStream* stream);		///< 打开数据流

	HRESULT			FillHeader();				///< 文件头填充函数

	HRESULT			Close(void);
	HRESULT			Start(QWORD inStartTime = 0);
	HRESULT			Stop(void);
	HRESULT			Pause(void);
	HRESULT			Resume(void);

	bool			IsOk(){
		return (m_lpReader != NULL);
	}

private:
	IWMReader*			m_lpReader;
	IWMReaderAdvanced*  m_lpReaderAdvanced;		///< 提供了选择音轨的接口
	HANDLE				m_syncevent;

	CStream*			m_stream;
	CRingEx				m_ringex;

	READERATTRIBUTE		m_fileattribute;
};