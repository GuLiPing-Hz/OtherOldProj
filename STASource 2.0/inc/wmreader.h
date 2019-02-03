#pragma once
#include "wmreadercb.h"
#include "ringex.h"
#include "Stream.h"
/**
	@file wmreader.h
	@brief wmreader ���ܷ�װ����
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
	tstring			_language;		///< ��������
	bool			_isMultiStream;	///< ��������
	QWORD			_duration;		///< ��ʱ��
	bool			_canseek;		///< �Ƿ���Զ�λ
	bool			_canbroadcast;	///< �Ƿ�֧�ֹ㲥
	DWORD			_number;		///< �������ı��
	tag_asf			_asftype;
	QWORD			_qwTimeElapsed;	///< �Ѿ���ȥ��ʱ��
	bool			_iseof;			///< �Ƿ��Ѿ�����
	bool			_bQuit;			///< �˳�����
	AM_MEDIA_TYPE*	_pAMMediaType;	///< ý������
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

	HRESULT			Initialize();				///< ��ʼ������
	void			UnInitialize();
	HRESULT			Open(LPCTSTR pwszUrl);		///< ���ļ�
	HRESULT			Open(IStream* stream);		///< ��������

	HRESULT			FillHeader();				///< �ļ�ͷ��亯��

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
	IWMReaderAdvanced*  m_lpReaderAdvanced;		///< �ṩ��ѡ������Ľӿ�
	HANDLE				m_syncevent;

	CStream*			m_stream;
	CRingEx				m_ringex;

	READERATTRIBUTE		m_fileattribute;
};