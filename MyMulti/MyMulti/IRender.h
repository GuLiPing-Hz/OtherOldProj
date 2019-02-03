#pragma once
#include <set>
#include <string>

interface IDirect3DDevice9;
class COpenGLWindow;
typedef std::set<DWORD_PTR> SETDW;
class ID3DRender
{
public:
	virtual bool initialize(/*in*/const COpenGLWindow* pOwner)=0;
	virtual bool attacthFile(/*in*/const char* file,/*in*/bool notMv,/*in*/const char* name,/*out*/DWORD_PTR* dwId)=0;
	virtual bool detachFile(/*in*/const char* name)=0;
	virtual bool detachFileAll()=0;
	virtual bool getDevice(/*out*/IDirect3DDevice9** ppDev)=0;
	virtual bool beginLostDev()=0;
	virtual bool endLostDev()=0;
};
