/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeDistortionMesh helper class implementation
*/


#include "hgedistort.h"

// HGE *hgeDistortionMesh::hge=0;
ImgsetMgr *hgeDistortionMesh::cls_imgsetMgr = NULL;

hgeDistortionMesh::hgeDistortionMesh(int cols, int rows)
{
	int i;
	
	//hge=hgeCreate(HGE_VERSION);
	cls_imgsetMgr = ImgsetMgr::getSingleton();

	nRows=rows;
	nCols=cols;
	cellw=cellh=0;
	//m_imgset=0;
	m_blend=BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_ZWRITE;
	disp_array=new CUSTOMVERTEX[rows*cols];

	for(i=0;i<rows*cols;i++)
	{
		disp_array[i].point.x=0.0f;
		disp_array[i].point.y=0.0f;
		disp_array[i].u=0.0f;
		disp_array[i].v=0.0f;
		
		disp_array[i].point.z=0.5f;
		disp_array[i].colour=0xFFFFFFFF;
	}
}

hgeDistortionMesh::hgeDistortionMesh(const hgeDistortionMesh &dm)
{
	//hge=hgeCreate(HGE_VERSION);
	cls_imgsetMgr = ImgsetMgr::getSingleton();

	nRows=dm.nRows;
	nCols=dm.nCols;
	cellw=dm.cellw;
	cellh=dm.cellh;
	m_rect.origin.x=dm.m_rect.origin.x;
	m_rect.origin.y=dm.m_rect.origin.y;
	m_rect.size.width=dm.m_rect.size.width;
	m_rect.size.height=dm.m_rect.size.height;
	m_imgset=dm.m_imgset;

	disp_array=new CUSTOMVERTEX[nRows*nCols];
	memcpy(disp_array, dm.disp_array, sizeof(CUSTOMVERTEX)*nRows*nCols);
}

hgeDistortionMesh::~hgeDistortionMesh()
{
	delete[] disp_array;
	//hge->Release();
}

hgeDistortionMesh& hgeDistortionMesh::operator= (const hgeDistortionMesh &dm)
{
	if(this!=&dm)
	{
		nRows=dm.nRows;
		nCols=dm.nCols;
		cellw=dm.cellw;
		cellh=dm.cellh;
		m_rect.origin.x=dm.m_rect.origin.x;
		m_rect.origin.y=dm.m_rect.origin.y;
		m_rect.size.width=dm.m_rect.size.width;
		m_rect.size.height=dm.m_rect.size.height;
		m_imgset=dm.m_imgset;

		if (disp_array)
		{
			delete[] disp_array;
		}
		
		disp_array=new CUSTOMVERTEX[nRows*nCols];
		memcpy(disp_array, dm.disp_array, sizeof(CUSTOMVERTEX)*nRows*nCols);
	}

	return *this;
	
}

// void hgeDistortionMesh::SetTexture(HTEXTURE tex)
// {
// 	quad.tex=tex;
// }

void hgeDistortionMesh::SetTextureRect(float x, float y, int w, int h)
{
	int i,j;
	int tw,th;

	m_rect.origin.x=x; 
	m_rect.origin.y=y; 
	m_rect.size.width=w; 
	m_rect.size.height=h;

	if (m_imgset)
	{
		tw=m_imgset->getTextureSize().width;//(float)hge->Texture_GetWidth(quad.tex);
		th=m_imgset->getTextureSize().height;//(float)hge->Texture_GetHeight(quad.tex);
	}
	else
	{
		tw = w;
		th = h;
	}

	cellw=w/(nCols-1.0f);
	cellh=h/(nRows-1.0f);

	for(j=0; j<nRows; j++)
		for(i=0; i<nCols; i++)
		{
			disp_array[j*nCols+i].u=(x+i*cellw)/tw;
			disp_array[j*nCols+i].v=(y+j*cellh)/th;

			disp_array[j*nCols+i].point.x=i*cellw;
			disp_array[j*nCols+i].point.y=j*cellh;
		}
}

void hgeDistortionMesh::SetBlendMode(int blend)
{
	m_blend=blend;
}

void hgeDistortionMesh::Clear(DWORD col, float z)
{
	int i,j;

	for(j=0; j<nRows; j++)
		for(i=0; i<nCols; i++)
		{
			disp_array[j*nCols+i].point.x=i*cellw;
			disp_array[j*nCols+i].point.y=j*cellh;
			disp_array[j*nCols+i].colour=col;
			disp_array[j*nCols+i].point.z=z;
		}
}

void hgeDistortionMesh::drawMeshRender(float x, float y)
{
	int i,j,idx;

	for(j=0; j<nRows-1; j++)
		for(i=0; i<nCols-1; i++)
		{
			idx=j*nCols+i;
			
			draw(disp_array[idx],disp_array[idx+1],disp_array[idx+nCols+1],disp_array[idx+nCols]);
// 			quad.v[0].u=disp_array[idx].tx;
// 			quad.v[0].v=disp_array[idx].ty;
// 			quad.v[0].point.x=x+disp_array[idx].x;
// 			quad.v[0].point.y=y+disp_array[idx].y;
// 			quad.v[0].point.z=disp_array[idx].z;
// 			quad.v[0].colour=disp_array[idx].col;
// 
// 			quad.v[1].tx=disp_array[idx+1].tx;
// 			quad.v[1].ty=disp_array[idx+1].ty;
// 			quad.v[1].x=x+disp_array[idx+1].x;
// 			quad.v[1].y=y+disp_array[idx+1].y;
// 			quad.v[1].z=disp_array[idx+1].z;
// 			quad.v[1].col=disp_array[idx+1].col;
// 
// 			quad.v[2].tx=disp_array[idx+nCols+1].tx;
// 			quad.v[2].ty=disp_array[idx+nCols+1].ty;
// 			quad.v[2].x=x+disp_array[idx+nCols+1].x;
// 			quad.v[2].y=y+disp_array[idx+nCols+1].y;
// 			quad.v[2].z=disp_array[idx+nCols+1].z;
// 			quad.v[2].col=disp_array[idx+nCols+1].col;
// 
// 			quad.v[3].tx=disp_array[idx+nCols].tx;
// 			quad.v[3].ty=disp_array[idx+nCols].ty;
// 			quad.v[3].x=x+disp_array[idx+nCols].x;
// 			quad.v[3].y=y+disp_array[idx+nCols].y;
// 			quad.v[3].z=disp_array[idx+nCols].z;
// 			quad.v[3].col=disp_array[idx+nCols].col;

			//hge->Gfx_RenderQuad(&quad);
		}
}

void hgeDistortionMesh::SetZ(int col, int row, float z)
{
	if(row<nRows && col<nCols) disp_array[row*nCols+col].point.z=z;
}

void hgeDistortionMesh::SetColor(int col, int row, DWORD color)
{
	if(row<nRows && col<nCols) disp_array[row*nCols+col].colour=color;
}

void hgeDistortionMesh::SetDisplacement(int col, int row, float dx, float dy, int ref)
{
	if(row<nRows && col<nCols)
	{
		switch(ref)
		{
			case HGEDISP_NODE:		dx+=col*cellw; dy+=row*cellh; break;
			case HGEDISP_CENTER:		dx+=cellw*(nCols-1)/2;dy+=cellh*(nRows-1)/2; break;
			case HGEDISP_TOPLEFT:	break;
		}

		disp_array[row*nCols+col].point.x=dx;
		disp_array[row*nCols+col].point.y=dy;
	}
}

float hgeDistortionMesh::GetZ(int col, int row) const
{
	if(row<nRows && col<nCols) return disp_array[row*nCols+col].point.z;
	else return 0.0f;
}

DWORD hgeDistortionMesh::GetColor(int col, int row) const
{
	if(row<nRows && col<nCols) return disp_array[row*nCols+col].colour;
	else return 0;
}

void hgeDistortionMesh::GetDisplacement(int col, int row, float *dx, float *dy, int ref) const
{
	if(row<nRows && col<nCols)
	{
		switch(ref)
		{
			case HGEDISP_NODE:		*dx=disp_array[row*nCols+col].point.x-col*cellw;
									*dy=disp_array[row*nCols+col].point.y-row*cellh;
									break;

			case HGEDISP_CENTER:	*dx=disp_array[row*nCols+col].point.x-cellw*(nCols-1)/2;
									*dy=disp_array[row*nCols+col].point.x-cellh*(nRows-1)/2;
									break;

			case HGEDISP_TOPLEFT:	*dx=disp_array[row*nCols+col].point.x;
									*dy=disp_array[row*nCols+col].point.y;
									break;
		}
	}
}

