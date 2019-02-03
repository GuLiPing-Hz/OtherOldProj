/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeSprite helper class header
*/


#ifndef HGESPRITE_H
#define HGESPRITE_H

#include "../OpenGLWindow.h"
#include "hgerect.h"
#include "../staff/Image.h"
#include <string>

float Texture_GetWidth(std::string imgsetName);
float Texture_GetHeight(std::string imgsetName);
/*
** HGE Sprite class
*/
class CGLSprite : public Image
{
public:
	CGLSprite(Imageset* imgset/*HTEXTURE tex*/, float x, float y, int w, int h);
	CGLSprite(const CGLSprite &spr);
	virtual ~CGLSprite() { /*hge->Release();*/ }
	
	void		drawRender(float x, float y);
	void		drawRenderEx(float x, float y, float rot,bool bFirstVB=true, float hscale=1.0f, float vscale=0.0f);
	void		drawRenderStretch(float x1, float y1, float x2, float y2);
	void		drawRender4V(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);

	//void		SetTexture(CGRect tex_rect/*HTEXTURE tex*/);
	void		SetTextureRect(float x, float y, int w, int h, bool adjSize = true);
	void		SetColor(KKColor col, int i=-1);//设置4个顶点的颜色，从0代表左上角，1代表右上角，2代表右下角，3代表左下角
	void		SetZ(float z, int i=-1);
	void		SetBlendMode(int blend) { m_blend=blend; }

	//HTEXTURE	GetTexture() const { return quad.tex; }
	void				GetTextureRect(float *x, float *y, float *w, float *h) const { *x=m_rect.origin.x; *y=m_rect.origin.y; *w=(float)m_rect.size.width; *h=(float)m_rect.size.height; }
	KKColor		GetColor(int i=0) const;
	float				GetZ(int i=0) const { return m_z[i]; }
	int				GetBlendMode() const { return m_blend; }

	hgeRect*	GetBoundingBox(float x, float y, hgeRect *rect) const { rect->Set(x-m_hotX, y-m_hotY, x-m_hotX+m_rect.size.width, y-m_hotY+m_rect.size.height); return rect; }
	hgeRect*	GetBoundingBoxEx(float x, float y, float rot, float hscale, float vscale,  hgeRect *rect) const;

protected:
	CGLSprite();
	//static HGE	*hge;
	//static ImgsetMgr *cls_imgsetMgr;

	//hgeQuad		quad;
	//float		tx, ty, width, height;
	//float		tex_width, tex_height;
	KKColorRect	m_color_rect;

	float		m_z[4];
};


#endif
