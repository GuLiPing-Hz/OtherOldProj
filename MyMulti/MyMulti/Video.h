#pragma once

#include "def.h"
#include "Graph.h"

interface IDirect3DDevice9;
interface IDirect3DTexture9;
interface IDirect3DVertexBuffer9;
interface IFilterGraph;
interface IVMRSurfaceAllocatorNotify9;
interface IBaseFilter;

class CVideo
{
public:
	CVideo(const char* name);
	virtual ~CVideo(void);

	void getCurPos(CGPoint& lt,CGPoint& lb,CGPoint& rt,CGPoint& rb);
	void updatePos(const CGPoint& lt,const CGPoint& lb,const CGPoint& rt,const CGPoint& rb);
	void updateColor(const int lt,const int lb,const int rt,const int rb);
	bool setGraph(CGraph* pGraph);
	void setSrcRect( float fTU, float fTV );
	void renderVideo(IDirect3DDevice9* pDev);

	void deleteSurfaces();
	HRESULT allocateSurfaceBuffer(DWORD dwN );
	HRESULT setVideoSize(   LONG lImageW,LONG lImageH );

public:
	int m_nTag;
	DWORD_PTR m_dwID;
	char m_strName[260];//±êÊ¶Ãû×Ö

	CGraph* m_pGraph;
	IVMRSurfaceAllocatorNotify9 *m_pDefaultNotify;

	IDirect3DTexture9* m_pTex;
	IDirect3DSurface9** m_ppSur;

	DWORD m_dwNumBufActuallyAllocated;
private:
	IDirect3DVertexBuffer9*	m_pBuffer;
	DWORD m_dwNumBuf;

	LONG m_lImageWidth;
	LONG m_lImageHeight;

	CUSTOMVERTEX		m_structVertex[4];
};
