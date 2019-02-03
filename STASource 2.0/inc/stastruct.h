#pragma once
#include "wmreadercb.h"
#include "Stream.h"
#include "ringex.h"
/**
	@file stastruct.h
	@brief stasource 中使用的结构体定义.
*/

struct tag_complete
{
	bool		_complete;	///< 现在的文件长度
	ULONGLONG	_size;		///< 文件长度
	ULONGLONG	_readed;	///< 文件长度
	void clear()
	{
		_size = _readed = 0;
		_complete = false;
	}
};

struct tag_asf
{
	bool	_audio;
	bool	_video;
	bool	_image;
	bool	_script;
	tag_asf()
	{
		clear();
	}
	void clear()
	{
		_audio = _video = _image = _script = false;
	}
};

struct tag_file
{
	tstring		_file;		///< 文件完全路径
	tstring		_version;	///< 加密版本号
	tstring		_suffix;	///< 媒体类型(后缀)
	bool		_changestart;
	bool		_ista;		///< 标记是否是 sta 文件
	Burning::CFile*	_cfile;	///< 文件指针对象
	CStream*	_stream;	///< 数据流对象
	tag_complete	_complete;
	bool		_init;

	tstring		_language;		///< 音轨语言
	bool		_isMultiStream;	///< 多音轨标记
	QWORD		_duration;		///< 总时长
	bool		_canseek;		///< 是否可以定位
	bool		_canbroadcast;	///< 是否支持广播
	tag_asf		_asftype;
	QWORD		_qwTimeElapsed;	///< 已经逝去的时间
	bool		_iseof;			///< 是否已经结束
	bool		_bQuit;			///< 退出控制

	DWORD		_datalensd;		///< one second data length;
	DWORD		_dataAlign;		///< 数据对齐

	tag_file()
	{
		_cfile	= NULL;
		_stream = NULL;
	}
	~tag_file()
	{
		clear();
	}

	void StreamClose()
	{
		if(_stream != NULL)
		{
			_stream->Close();
		}
	}

	void StreamClear()
	{
		if(_stream != NULL)
		{
			_stream->Clear();
		}
	}

	void clear()
	{
		_complete.clear();
		_file = _version = _suffix = TEXT("");
		_changestart = _ista = _init = false;
		SafeDelete(_cfile);
		if ( _stream != NULL )
		{
//			_stream->Release();
			SafeRelease(_stream);
		}
		_language		= TEXT("");
		_isMultiStream	= false;
		_canseek		= false;
		_canbroadcast	= false;
		_asftype.clear();
		_qwTimeElapsed	= 0;
		_iseof			= false;
		_bQuit			= false;
		_datalensd		= _dataAlign = 0;
		//SafeDeleteArray(_buffer);
	}
};

struct tag_thread
{
	HANDLE	_thread;	///< 线程句柄
	bool	_quit;		///< 是否安全退出
	tag_thread()
	{
		_thread = INVALID_HANDLE_VALUE;
		_quit = false;
	
	}
	~tag_thread()
	{
		_quit = true;
		if(_thread != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(_thread,2000);			//使用时间超时以让线程安全退出
			CloseHandle(_thread);
			_thread = INVALID_HANDLE_VALUE;
		}
	}
	void Clear()
	{
		_quit = false;
		if(_thread != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(_thread,2000);			//使用时间超时以让线程安全退出
			CloseHandle(_thread);
			_thread = INVALID_HANDLE_VALUE;
		}
	}
};

struct tag_wmrender
{
	IWMReader*			_lpReader;
	CWMReaderCallBack*	_lpReaderCB;
	CRingEx				_ringex;
	HANDLE				_asynevent;
	tag_wmrender()
	{
		_lpReader	= NULL;
		_lpReaderCB = NULL;
		_asynevent  = NULL;
	}

	~tag_wmrender()
	{
		if(_lpReader != NULL) _lpReader->Close();
		SafeCloseHandle(_asynevent);
		SafeRelease(_lpReaderCB);
		SafeRelease(_lpReader);
	}

	bool IsOk()
	{
		return (_lpReader != NULL && _lpReaderCB != NULL);
	}

	void AllStop()
	{
		_ringex.Close();
	}

	void Clear()
	{
		_ringex.Reset();
	}

	void WMReaderWaitEvent(DWORD time = INFINITE)
	{
		WaitForSingleObject(_asynevent,time);
		ResetEvent(_asynevent);
	}

	void WMReaderResetEvent()
	{
		ResetEvent(_asynevent);
	}

	void WMReaderSetEvent()
	{
		SetEvent(_asynevent);
	}

	bool Initialize( WMCBFUNC eof )
	{
		if(_lpReader == NULL)
		{
			if( WMCreateReader( NULL,WMT_RIGHT_PLAYBACK,&_lpReader ) != S_OK)
			{
				OutputDebugStr(TEXT("Create wmreader faield!\n"));
				return false;
			}
		}

		if(_asynevent == NULL)
		{
			_asynevent = CreateEvent(NULL,FALSE,FALSE,NULL);
			if(_asynevent == NULL)
			{
				OutputDebugStr(TEXT("Create wmreader closeevent failed!\n"));
				return false;
			}
		}

		if( _lpReaderCB == NULL )
		{
			_lpReaderCB = new CWMReaderCallBack();
			if(_lpReaderCB == NULL)
			{
				OutputDebugStr(TEXT("Create wmreadercb failed!\n"));
				return false;
			}
			if(!_lpReaderCB->Initialize(eof,_asynevent,&_ringex,_lpReader)) 
				return false;
		}

		return true;
	}

	HRESULT Open(LPCTSTR pwszUrl)
	{
		if(IsNull(pwszUrl)) 
			return S_FALSE;
		HRESULT hr = S_OK;
		return _lpReader->Open(pwszUrl,_lpReaderCB, NULL);
	}

	HRESULT Open(IStream* stream)
	{
		HRESULT hr = S_OK;
		IWMReaderAdvanced2* lpAdvance = NULL;
		hr = _lpReader->QueryInterface(IID_IWMReaderAdvanced2,(void**)&lpAdvance);
		if(hr != S_OK)
		{
			OutputDebugStr(TEXT("WMReader:QueryInterface IID_IWMReaderAdvanced2 failed! \n"));
			return hr;
		}
		hr = lpAdvance->OpenStream(stream,_lpReaderCB,NULL);
		SafeRelease(lpAdvance);
		return hr;
	}
};

struct tag_global
{
	DWORD	_delay;
	HANDLE	_event;
	tag_global()
	{
		_event = CreateEvent(NULL,FALSE,FALSE,NULL);
		_delay = 76;
	}
	~tag_global()
	{
		CloseHandle(_event);
		_event = NULL;
	}
	void GlobalWaitEvent(DWORD time)
	{
		WaitForSingleObject(_event,time);
	}

	void GlobalResetEvent()
	{
		ResetEvent(_event);
	}

	void GlobalSetEvent()
	{
		SetEvent(_event);
	}
};

struct tag_asynctime
{
	REFERENCE_TIME	_start;			///< 启动时间
	REFERENCE_TIME	_changestart;	///< 选择后的播放时间
	DWORD			_timestamp;
	double			_lastDuration;	///< 最后一个 sample 的持续时间
	bool			_first;			///< 第一次运行不需要进行延迟.true 为开始延迟计算.
	tag_asynctime()
	{
		clear();
	}

	void clear()
	{
		_start			= 0;
		_timestamp		= 0;
		_lastDuration	= 0;
		_first			= false;
		_changestart	= 0;
	}

	DWORD Passage()
	{
		if(_timestamp == 0)
		{
			_timestamp = GetTickCount();
			return 0;
		}

		DWORD passage = GetTickCount() - _timestamp;
		_timestamp = GetTickCount();
		return passage;
	}
};