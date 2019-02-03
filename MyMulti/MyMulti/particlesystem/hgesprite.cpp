/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeSprite helper class implementation
*/


#include "hgesprite.h"
#include <math.h>
#include "../staff/ImgsetMgr.h"
#include "../Opengl.h"

float Texture_GetWidth(std::string imgsetName)
{
	if (imgsetName.size())
	{
		Imageset *imgset = ImgsetMgr::getSingleton()->getImageset(imgsetName);
		if (imgset)
		{
			return (float)imgset->getTextureSize().width;
		}
		LOG_ERN0("Texture_GetWidth getImgset failed");
	}
	LOG_ERN0("Texture_GetWidth imgsetName failed");
	return 0.0f;
}
float Texture_GetHeight(std::string imgsetName)
{
	if (imgsetName.size())
	{
		Imageset *imgset = ImgsetMgr::getSingleton()->getImageset(imgsetName);
		if (imgset)
		{
			return (float)imgset->getTextureSize().height;
		}
		LOG_ERN0("Texture_GetHeight getImgset failed");
	}
	LOG_ERN0("Texture_GetHeight imgsetName failed");
	return 0.0f;
}
//HGE *hgeSprite::hge=0;
//ImgsetMgr *hgeSprite::cls_imgsetMgr = NULL;

CGLSprite::CGLSprite(Imageset* imgset/*HTEXTURE tex*/, float texx, float texy, int w, int h)
:Image(CGRect(CGPoint(texx,texy),CGSize(w,h)),imgset)
,m_color_rect(WHITE_RECT)
{
	//float texx1, texy1, texx2, texy2;

	//hge=hgeCreate(HGE_VERSION);
	//cls_imgsetMgr = ImgsetMgr::getSingleton();

	//tx=texx; ty=texy;
	//width=w; height=h;
	//if(texture)

	m_hotX=0;
	m_hotY=0;
	m_z[0] = m_z[1] = m_z[2] = m_z[3] = 0.0f;
// 	quad.v[0].z = 
// 	quad.v[1].z = 
// 	quad.v[2].z = 
// 	quad.v[3].z = 0.5f;
// 	
// 	quad.v[0].col = 
// 	quad.v[1].col = 
// 	quad.v[2].col = 
// 	quad.v[3].col = 0xffffffff;

//		quad.blend=BLEND_DEFAULT;
}

CGLSprite::CGLSprite(const CGLSprite &spr)
:m_color_rect(WHITE_RECT)
{
	memcpy(this, &spr, sizeof(CGLSprite));
	//hge=hgeCreate(HGE_VERSION);
}

void CGLSprite::drawRender(float x, float y)
{
	float tempx1, tempy1;//, tempx2, tempy2;

	tempx1 = x-m_hotX;
	tempy1 = y-m_hotY;
	//tempx2 = x+m_rect.size.width-hotX;
	//tempy2 = y+m_rect.size.height-hotY;
	draw(CGPoint(tempx1,tempy1),NULL,m_color_rect);
// 	quad.v[0].x = tempx1; quad.v[0].y = tempy1;
// 	quad.v[1].x = tempx2; quad.v[1].y = tempy1;
// 	quad.v[2].x = tempx2; quad.v[2].y = tempy2;
// 	quad.v[3].x = tempx1; quad.v[3].y = tempy2;

	//hge->Gfx_RenderQuad(&quad);
}


void CGLSprite::drawRenderEx(float x, float y, float rot,bool bFirstVB, float hscale, float vscale)
{//////////////////////////////////////////////////////////////////////////
	drawPSEx(CGPoint(m_hotX,m_hotY),CGPoint(x,y),rot,hscale,vscale,m_color_rect,bFirstVB);
	//hge->Gfx_RenderQuad(&quad);
}


void CGLSprite::drawRenderStretch(float x1, float y1, float x2, float y2)
{
// 	quad.v[0].x = x1; quad.v[0].y = y1;
// 	quad.v[1].x = x2; quad.v[1].y = y1;
// 	quad.v[2].x = x2; quad.v[2].y = y2;
// 	quad.v[3].x = x1; quad.v[3].y = y2;
	int w = (int)(x2-x1);
	int h = (int)(y2-y1);
	draw(CGRect(x1,y1,w,h),NULL,m_color_rect);
	//hge->Gfx_RenderQuad(&quad);
}


void CGLSprite::drawRender4V(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)
{
// 	quad.v[0].x = x0; quad.v[0].y = y0;
// 	quad.v[1].x = x1; quad.v[1].y = y1;
// 	quad.v[2].x = x2; quad.v[2].y = y2;
// 	quad.v[3].x = x3; quad.v[3].y = y3;
	draw(CGPoint(x0,y0,m_z[0]),CGPoint(x1,y1,m_z[1]),CGPoint(x2,y2,m_z[2]),CGPoint(x3,y3,m_z[3]),m_color_rect);
	//hge->Gfx_RenderQuad(&quad);
}


hgeRect* CGLSprite::GetBoundingBoxEx(float x, float y, float rot, float hscale, float vscale, hgeRect *rect) const
{
	float tx1, ty1, tx2, ty2;
	float sint, cost;

	rect->Clear();
	
	tx1 = -m_hotX*hscale;
	ty1 = -m_hotY*vscale;
	tx2 = (m_rect.size.width-m_hotX)*hscale;
	ty2 = (m_rect.size.height-m_hotY)*vscale;

	if (rot != 0.0f)
	{
		cost = cosf(rot);
		sint = sinf(rot);
			
		rect->Encapsulate(tx1*cost - ty1*sint + x, tx1*sint + ty1*cost + y);	
		rect->Encapsulate(tx2*cost - ty1*sint + x, tx2*sint + ty1*cost + y);	
		rect->Encapsulate(tx2*cost - ty2*sint + x, tx2*sint + ty2*cost + y);	
		rect->Encapsulate(tx1*cost - ty2*sint + x, tx1*sint + ty2*cost + y);	
	}
	else
	{
		rect->Encapsulate(tx1 + x, ty1 + y);
		rect->Encapsulate(tx2 + x, ty1 + y);
		rect->Encapsulate(tx2 + x, ty2 + y);
		rect->Encapsulate(tx1 + x, ty2 + y);
	}

	return rect;
}

// void hgeSprite::SetTexture(std::string imgsetName/*HTEXTURE tex*/)
// {
// 	float tx1,ty1,tx2,ty2;
// 	float tw,th;
// 	Imageset *img = ImgsetMgr::getSingleton()->getImageset(imgsetName);
// 	if (img)
// 	{
// 		//quad.tex=(HTEXTURE)img->getTexture();//tex;
// 
// 		//if(tex)
// 		{
// 			tw = (float)img->getTextureSize().width;//(float)hge->Texture_GetWidth(tex);
// 			th = (float)img->getTextureSize().height;//(float)hge->Texture_GetHeight(tex);
// 		}
// 	}
// 	else
// 	{
// 		tw = 1.0f;
// 		th = 1.0f;
// 	}
// 
// 	if(tw!=tex_width || th!=tex_height)
// 	{
// 		tx1=quad.v[0].tx*tex_width;
// 		ty1=quad.v[0].ty*tex_height;
// 		tx2=quad.v[2].tx*tex_width;
// 		ty2=quad.v[2].ty*tex_height;
// 
// 		tex_width=tw;
// 		tex_height=th;
// 
// 		tx1/=tw; ty1/=th;
// 		tx2/=tw; ty2/=th;
// 
// 		quad.v[0].tx=tx1; quad.v[0].ty=ty1; 
// 		quad.v[1].tx=tx2; quad.v[1].ty=ty1; 
// 		quad.v[2].tx=tx2; quad.v[2].ty=ty2; 
// 		quad.v[3].tx=tx1; quad.v[3].ty=ty2; 
// 	}
// 	
// }


void CGLSprite::SetTextureRect(float x, float y, int w, int h, bool adjSize)
{
	//float tx1, ty1, tx2, ty2;
	bool bX,bY,bHS;

	if (adjSize)
	{
		m_rect = CGRectMake(x,y,w,h);
	}

	bX=m_bXFlip; bY=m_bYFlip; bHS=m_bHSFlip;
	m_bXFlip=false; m_bYFlip=false;
	setFlip(bX,bY,bHS);
}

KKColor CGLSprite::GetColor(int i) const
{
	if (i == 0)
	{
		return m_color_rect.left_top_color;
	}
	else if(i == 1)
	{
		return m_color_rect.right_top_color;
	}
	else if (i == 2)
	{
		return m_color_rect.right_bottom_color;
	}
	else if (i == 3)
	{
		return m_color_rect.left_bottom_color;
	}
	return KKColor(1.0f,1.0f,1.0f,1.0f);
}

void CGLSprite::SetColor(KKColor col, int i)
{
	if (i == 0)
	{
		m_color_rect.left_top_color = col;
	}
	else if(i == 1)
	{
		m_color_rect.right_top_color = col;
	}
	else if (i == 2)
	{
		m_color_rect.right_bottom_color = col;
	}
	else if (i == 3)
	{
		m_color_rect.left_bottom_color = col;
	}
	else
	{
		m_color_rect.left_top_color = col;
		m_color_rect.right_top_color = col;
		m_color_rect.right_bottom_color = col;
		m_color_rect.left_bottom_color = col;
	}
}

void CGLSprite::SetZ(float z, int i)
{
	if (i == -1)
	{
		m_z[0] = z;
		m_z[1] = z;
		m_z[2] = z;
		m_z[3] = z;
	}
	else
	{
		m_z[i] = z;
	}
}
