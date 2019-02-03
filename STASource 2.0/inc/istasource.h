#pragma once

#if defined __cplusplus
extern "C"{
#endif

#if !defined(DEVELOP_DLL)
	// {538AD679-1E72-4db7-B046-422055281698}
	static const GUID CLSID_STASOURCE = {0x538ad679, 0x1e72, 0x4db7, {0xb0, 0x46, 0x42, 0x20, 0x55, 0x28, 0x16, 0x98}};
	// {E5F6502F-280A-4083-BE57-883E33F0DEF1}
	static const GUID IID_ISTASOURCE = {0xe5f6502f, 0x280a, 0x4083, {0xbe, 0x57, 0x88, 0x3e, 0x33, 0xf0, 0xde, 0xf1}};
#endif

	DECLARE_INTERFACE_(ISTASource,IUnknown){
		STDMETHOD(GetLanguageLen)(THIS_ DWORD& dwlen) PURE;
		STDMETHOD(GetLanguage)(THIS_ LPWSTR language) PURE;
		STDMETHOD(IsMultiStream)(THIS_ bool& multistream) PURE;
		STDMETHOD(SetStream)(THIS_ DWORD number) PURE;	//hongjin 2013-5-30，改为两路输出，取消音轨切换
		STDMETHOD(SetDelayTime)(THIS_ DWORD time) PURE;	///< 设置声音缓冲时间,单位:毫秒
		STDMETHOD(SetPause)() PURE;	//设置暂停
		STDMETHOD(SetPlay)() PURE;	//设置播放
	};

#if defined __cplusplus
};
#endif