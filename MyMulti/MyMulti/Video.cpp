#include "StdAfx.h"
#include "Video.h"

CVideo::CVideo(const char* name)
:m_nTag(TAG_VIDEO_MY)
,m_dwID(0L)
,m_ppSur(NULL)
,m_pTex(NULL)
,m_pDefaultNotify(NULL)
,m_pGraph(NULL)
,m_pBuffer(NULL)
,m_dwNumBuf(0L)
,m_dwNumBufActuallyAllocated(0L)
,m_lImageHeight(0L)
,m_lImageWidth(0L)
{
	if(name)
		strcpy_s(m_strName,name);
	else
		m_strName[0] = 0;

	m_structVertex[0].point = CGPoint(-1.0f,  1.0f, 0.0f); // 左上
	m_structVertex[1].point = CGPoint(-1.0f, -1.0f, 0.0f); // 左下
	m_structVertex[2].point = CGPoint( 1.0f,  1.0f, 0.0f); // 右上
	m_structVertex[3].point = CGPoint( 1.0f, -1.0f, 0.0f); // 右下

	// set up diffusion:
	m_structVertex[0].colour = 0xffffffff;//argb
	m_structVertex[1].colour = 0xffffffff;
	m_structVertex[2].colour = 0xffffffff;
	m_structVertex[3].colour = 0xffffffff;


	// set up texture coordinates
	m_structVertex[0].u = 0.0f; m_structVertex[0].v = 0.0f; // 左下
	m_structVertex[1].u = 0.0f; m_structVertex[1].v = 1.0f; // 左上
	m_structVertex[2].u = 1.0f; m_structVertex[2].v = 0.0f; // 右下
	m_structVertex[3].u = 1.0f; m_structVertex[3].v = 1.0f; // 右上
}

CVideo::~CVideo(void)
{
	SAFE_RELEASE(m_pTex);
// 	if (m_pGraph)
// 		m_pGraph->stopPlayer();
	if(m_pDefaultNotify)
		m_pDefaultNotify->AdviseSurfaceAllocator(m_dwID,NULL);
	SAFE_RELEASE(m_pDefaultNotify);
	SAFE_DELETE(m_pGraph);
	deleteSurfaces();
}

void CVideo::getCurPos(CGPoint& lt,CGPoint& lb,CGPoint& rt,CGPoint& rb)
{
	lt=m_structVertex[0].point; // 左上
	lb=m_structVertex[1].point; // 左下
	rt=m_structVertex[2].point; // 右上
	rb=m_structVertex[3].point; // 右下
}

void CVideo::updatePos(const CGPoint& lt,const CGPoint& lb,const CGPoint& rt,const CGPoint& rb)
{
	m_structVertex[0].point = lt; // 左上
	m_structVertex[1].point = lb; // 左下
	m_structVertex[2].point = rt; // 右上
	m_structVertex[3].point = rb; // 右下
}

void CVideo::updateColor(const int lt,const int lb,const int rt,const int rb)
{
	m_structVertex[0].colour = lt;//argb
	m_structVertex[1].colour = lb;
	m_structVertex[2].colour = rt;
	m_structVertex[3].colour = rb;
}

bool CVideo::setGraph(CGraph* pGraph)
{
	SAFE_DELETE(m_pGraph);
	if(!pGraph)
		return false;
	m_pGraph = pGraph;
	HRESULT hr = m_pGraph->getDefaultNotify(&m_pDefaultNotify);
	return SUCCEEDED(hr);
}

void CVideo::setSrcRect( float fTU, float fTV )
{
	m_structVertex[0].u = 0.0f; m_structVertex[0].v = 0.0f; // low left
	m_structVertex[1].u = 0.0f; m_structVertex[1].v = fTV;  // high left
	m_structVertex[2].u = fTU;  m_structVertex[2].v = 0.0f; // low right
	m_structVertex[3].u = fTU;  m_structVertex[3].v = fTV;  // high right
}

void CVideo::renderVideo(IDirect3DDevice9* pDev)
{
	if (NULL/*m_pTex*/)
	{
		HRESULT hr;
		FAIL_RET3( pDev->SetTexture( 0, m_pTex));
		//FAIL_RET3( d3dDevice->SetStreamSource(0, m_vertexBuffer, 0, sizeof(CUSTOMVERTEX)  ) );            //set next source ( NEW )
		FAIL_RET3( pDev->SetFVF( D3DFVF_CUSTOMVERTEX ) );
		//FAIL_RET3( d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2 ));  //draw quad 
		FAIL_RET3( pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,(void*)m_structVertex,sizeof(CUSTOMVERTEX)) );  //draw quad 
		FAIL_RET3( pDev->SetTexture( 0, NULL));
	}
}

void CVideo::deleteSurfaces()
{
	if( m_ppSur )
	{
		for( DWORD dwS = 0; dwS<m_dwNumBuf; dwS++)
		{
			if( m_ppSur[dwS] )
			{
				(m_ppSur[dwS])->Release();
				m_ppSur[dwS] = NULL;
			}
		}
		delete[] m_ppSur;
		m_ppSur = NULL;
	}
	m_dwNumBuf = 0L;
	m_dwNumBufActuallyAllocated = 0L;
}

HRESULT CVideo::allocateSurfaceBuffer(DWORD dwN )
{
	if( dwN < 1)
	{
		return E_INVALIDARG;
	}

	deleteSurfaces();
	m_dwNumBuf = dwN;
	m_ppSur = new IDirect3DSurface9*[m_dwNumBuf];

	if( !m_ppSur )
	{
		m_dwNumBuf = 0L;
		return E_OUTOFMEMORY;
	}

	ZeroMemory( m_ppSur, m_dwNumBuf * sizeof(IDirect3DSurface9*));
	return S_OK;
}

HRESULT CVideo::setVideoSize(   LONG lImageW,LONG lImageH )
{
	if( lImageW < 1 ||
		lImageH < 1 )
	{
		return E_INVALIDARG;
	}

	m_lImageWidth = lImageW;
	m_lImageHeight = lImageH;
	return S_OK;
}
