#pragma once

#include "RenderEngine.h"
#include <math.h>

class CDrawMonitor : public CRenderEngine
{
public:
	void adjustVertex(CUSTOMVERTEX* pVertex,const CGPoint& lefttop,const CGPoint& leftbottom,const CGPoint& rightbottom
		,const CGPoint& righttop,unsigned long ltc,unsigned long lbc,unsigned long rbc,unsigned long rtc)
	{
		//逆时针
		pVertex[0].point = lefttop; // 左上
		pVertex[1].point = leftbottom; // 左下
		pVertex[2].point = rightbottom; // 右下
		pVertex[3].point = lefttop; // 左上
		pVertex[4].point = rightbottom; // 右下
		pVertex[5].point = righttop; // 右上

		pVertex[0].colour = ltc;//argb
		pVertex[1].colour = lbc;
		pVertex[2].colour = rbc;
		pVertex[3].colour = ltc;//argb
		pVertex[4].colour = rbc;
		pVertex[5].colour = rtc;
	}
	void algorithm(float& y,float& z,float change)
	{
		y-=change;
		z=10.0f+3*fabsf(y-1.0f);
		if(z>100.0f)
			z = 1.0f;
		if(y<=-7.5f)
		{
			float littleChange=y+7.5f;
			y = 7.5f+littleChange;
		}
	}
	void renderGraphic()
	{
		static float s_adjust;
		static float s_rate = 1.0f;
		static int nControl = 0;
		static float s_z = 10.0f;
		static float s_y1 = 7.5f;
		static float s_z2 = 10.0f;
		static float s_y2 = s_y1+3.0f;
		static float s_z3 = 10.0f;
		static float s_y3 = s_y2+3.0f;
		static float s_z4 = 10.0f;
		static float s_y4 = s_y3+3.0f;
		static float s_z5 = 10.0f;
		static float s_y5 = s_y4+3.0f;
		algorithm(s_y1,s_z,s_rate);
		algorithm(s_y2,s_z2,s_rate);
		algorithm(s_y3,s_z3,s_rate);
		algorithm(s_y4,s_z4,s_rate);
		algorithm(s_y5,s_z5,s_rate);
		char buf[260] = {0};
		s_rate -= 0.001f;
		if(s_rate <= 0.0f)
		{
			s_rate = 0.0f;
			float minydistance;
			float ydistance1 = fabsf(s_y1-1.0f);
			minydistance = ydistance1;
			float ydistance2 = fabsf(s_y2-1.0f);
			minydistance = min(minydistance,ydistance2);
			float ydistance3 = fabsf(s_y3-1.0f);
			minydistance = min(minydistance,ydistance3);
			float ydistance4 = fabsf(s_y4-1.0f);
			minydistance = min(minydistance,ydistance4);
			float ydistance5 = fabsf(s_y5-1.0f);
			minydistance = min(minydistance,ydistance5);
			if(minydistance > 0.1f)
			{
				sprintf_s(buf,"distance %f\n",minydistance);
				OutputDebugStringA(buf);
				s_adjust = minydistance;
				s_rate = 0.01f;
			}
			nControl ++;
		}
		CUSTOMVERTEX structVertex[30];
		adjustVertex(structVertex,CGPoint(-1.0f,  s_y1, s_z),CGPoint(-1.0f, s_y1-2.0f, s_z),CGPoint( 1.0f, s_y1-2.0f, s_z),CGPoint( 1.0f,  s_y1, s_z),
			0xffff0000,0xffff0000,0xffff0000,0xffff0000);
		adjustVertex(structVertex+6,CGPoint(-1.0f,  s_y2, s_z2),CGPoint(-1.0f, s_y2-2.0f, s_z2),CGPoint( 1.0f, s_y2-2.0f, s_z2),CGPoint( 1.0f,  s_y2, s_z2),
			0xff0000ff,0xff0000ff,0xff0000ff,0xff0000ff);
		adjustVertex(structVertex+12,CGPoint(-1.0f,  s_y3, s_z3),CGPoint(-1.0f, s_y3-2.0f, s_z3),CGPoint( 1.0f, s_y3-2.0f, s_z3),CGPoint( 1.0f,  s_y3, s_z3),
			0xff00ff00,0xff00ff00,0xff00ff00,0xff00ff00);
		adjustVertex(structVertex+18,CGPoint(-1.0f,  s_y4, s_z4),CGPoint(-1.0f, s_y4-2.0f, s_z4),CGPoint( 1.0f, s_y4-2.0f, s_z4),CGPoint( 1.0f,  s_y4, s_z4),
			0xffffff00,0xffffff00,0xffffff00,0xffffff00);
		adjustVertex(structVertex+24,CGPoint(-1.0f,  s_y5, s_z5),CGPoint(-1.0f, s_y5-2.0f, s_z5),CGPoint( 1.0f, s_y5-2.0f, s_z5),CGPoint( 1.0f,  s_y5, s_z5),
			0xffff00ff,0xffff00ff,0xffff00ff,0xffff00ff);
		m_pD3ddev->SetFVF(D3DFVF_CUSTOMVERTEX);
		m_pD3ddev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,10,(void*)structVertex,sizeof(CUSTOMVERTEX));  //draw quad

		if(nControl >= 400)
		{
			s_rate = 1.0f;
			nControl = 0;
		}
	}
	void renderVideo()
	{
		CAutoLock lock(&m_cs);
		MAPSTRINGID::iterator it = m_mapStrId.begin();
		for(it;it!=m_mapStrId.end();it++)
		{
			CVideo* pVideo;
			if (SUCCEEDED(getVideo(it->second,&pVideo)))
			{
				pVideo->renderVideo(m_pD3ddev);
			}
		}
	}
	int clearBuffer()
	{ 
		return m_pD3ddev->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER , D3DCOLOR_XRGB(0,0,0), 1.0f, 0L ); 
	}
	//Begin drawing
	int beginDrawing ()
	{ 
		return m_pD3ddev->BeginScene (); 
	}
	//End drawing
	int endDrawing ()
	{
		return m_pD3ddev->EndScene (); 
	}
	int present ()
	{
		return m_pD3ddev->Present (NULL, NULL, NULL, NULL); 
	}
	void setRenderTarget()
	{
		m_pD3ddev->SetRenderTarget( 0, m_pRenderTarget );
	}
};
