/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeDistortionMesh helper class header
*/


#ifndef HGEDISTORT_H
#define HGEDISTORT_H



#include "hgedef.h"
#include "../staff/ImgsetMgr.h"
#define HGEDISP_NODE		0
#define HGEDISP_TOPLEFT		1
#define HGEDISP_CENTER		2

/*
** HGE Distortion mesh class
*/
class hgeDistortionMesh : public Image
{
public:
     hgeDistortionMesh(int cols, int rows);
     hgeDistortionMesh(const hgeDistortionMesh &dm);
     virtual ~hgeDistortionMesh();

	 hgeDistortionMesh&	operator= (const hgeDistortionMesh &dm);

     void		drawMeshRender(float x, float y);
     void		Clear(DWORD col=0xFFFFFFFF, float z=0.5f);

     //void		SetTexture(HTEXTURE tex);
     void		SetTextureRect(float x, float y, int w, int h);
     void		SetBlendMode(int blend);
     void		SetZ(int col, int row, float z);
     void		SetColor(int col, int row, DWORD color);
     void		SetDisplacement(int col, int row, float dx, float dy, int ref);

     //HTEXTURE	GetTexture() const {return quad.tex;}
     void			GetTextureRect(float *x, float *y, float *w, float *h) const { *x=m_rect.origin.x; *y=m_rect.origin.y; *w=(float)m_rect.size.width; *h=(float)m_rect.size.height; }
     int				GetBlendMode() const { return m_blend; }
     float			GetZ(int col, int row) const;
     DWORD		GetColor(int col, int row) const;
     void			GetDisplacement(int col, int row, float *dx, float *dy, int ref) const;

	 int		GetRows() { return nRows; }
	 int		GetCols() { return nCols; }

private:
	hgeDistortionMesh();

	//static HGE	*hge;
	static ImgsetMgr *cls_imgsetMgr;

	CUSTOMVERTEX	*disp_array;
	int			nRows, nCols;
	float			cellw,cellh;
	//float			tx,ty,width,height;
	//hgeQuad		quad;
};


#endif
