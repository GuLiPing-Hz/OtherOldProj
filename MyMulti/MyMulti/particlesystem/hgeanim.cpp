/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hg.relishgames.com
**
** hgeAnimation helper class implementation
*/


#include "hgeanim.h"
#include "../staff/ImgsetMgr.h"
#include <string>


CGLAnimation::CGLAnimation(Imageset *imgset/*HTEXTURE tex*/, int nframes, float FPS, float x, float y, int w, int h)
	: CGLSprite(imgset/*tex*/, x, y, w, h)
	//,m_bInit(false)
{
	//orig_width = hg->Texture_GetWidth(tex, true);
	orig_width = 0;
	if (imgset)
	{
		orig_width = imgset->getTextureSize().width;
	}
	
	fSinceLastFrame=-1.0f;
	fSpeed=1.0f/FPS;
	bPlaying=false;
	nFrames=nframes;

	Mode=HGEANIM_FWD | HGEANIM_LOOP;
	nDelta=1;
	SetFrame(0);

	//m_bInit = true;
}

CGLAnimation::CGLAnimation(const CGLAnimation & anim)
: CGLSprite(anim)
{ 
	// Copy hgeAnimation parameters: 
	this->orig_width	  = anim.orig_width;
	this->bPlaying        = anim.bPlaying; 
	this->fSpeed          = anim.fSpeed; 
	this->fSinceLastFrame = anim.fSinceLastFrame;
	this->Mode            = anim.Mode;
	this->nDelta          = anim.nDelta;
	this->nFrames         = anim.nFrames;
	this->nCurFrame       = anim.nCurFrame;

}

void CGLAnimation::SetMode(int mode)
{
	Mode=mode;

	if(mode & HGEANIM_REV)
	{
		nDelta = -1;
		SetFrame(nFrames-1);
	}
	else
	{
		nDelta = 1;
		SetFrame(0);
	}
}


void CGLAnimation::Play()
{
	bPlaying=true;
	fSinceLastFrame=-1.0f;
	if(Mode & HGEANIM_REV)
	{
		nDelta = -1;
		SetFrame(nFrames-1);
	}
	else
	{
		nDelta = 1;
		SetFrame(0);
	}
}


void CGLAnimation::Update(float fDeltaTime)
{
	if(!bPlaying) return;

	if(fSinceLastFrame == -1.0f)
		fSinceLastFrame=0.0f;
	else
		fSinceLastFrame += fDeltaTime;

	while(fSinceLastFrame >= fSpeed)
	{
		fSinceLastFrame -= fSpeed;

		if(nCurFrame + nDelta == nFrames)
		{
			switch(Mode)
			{
				case HGEANIM_FWD:
				case HGEANIM_REV | HGEANIM_PINGPONG:
					bPlaying = false;
					break;

				case HGEANIM_FWD | HGEANIM_PINGPONG:
				case HGEANIM_FWD | HGEANIM_PINGPONG | HGEANIM_LOOP:
				case HGEANIM_REV | HGEANIM_PINGPONG | HGEANIM_LOOP:
					nDelta = -nDelta;
					break;
			}
		}
		else if(nCurFrame + nDelta < 0)
		{
			switch(Mode)
			{
				case HGEANIM_REV:
				case HGEANIM_FWD | HGEANIM_PINGPONG:
					bPlaying = false;
					break;

				case HGEANIM_REV | HGEANIM_PINGPONG:
				case HGEANIM_REV | HGEANIM_PINGPONG | HGEANIM_LOOP:
				case HGEANIM_FWD | HGEANIM_PINGPONG | HGEANIM_LOOP:
					nDelta = -nDelta;
					break;
			}
		}

		if(bPlaying) SetFrame(nCurFrame+nDelta);
	}
}

void CGLAnimation::SetFrame(int n)
{
	float tx1, ty1;//, tx2, ty2;
	bool bX, bY, bHS;
	int ncols = int(orig_width) / int(m_rect.size.width);


	n = n % nFrames;
	if(n < 0) n = nFrames + n;
	nCurFrame = n;

	// calculate texture coords for frame n
	ty1 = m_rect.origin.y;
	tx1 = m_rect.origin.x + n*m_rect.size.width;

	if(tx1 > orig_width-m_rect.size.width)
	{
		n -= int(orig_width-m_rect.origin.x) / int(m_rect.size.width);
		tx1 = float(m_rect.size.width * (n%ncols));
		ty1 += float(m_rect.size.height * (1 + n/ncols));
	}

	m_rect.origin.x = tx1;//¸Ä±äÎÆÀíÍ¼Æ¬£¬
	m_rect.origin.y = ty1;
// 	tx2 = tx1 + m_rect.size.width;
// 	ty2 = ty1 + m_rect.size.height;
// 
// 	tx1 /= tex_width;
// 	ty1 /= tex_height;
// 	tx2 /= tex_width;
// 	ty2 /= tex_height;
// 
// 	quad.v[0].tx=tx1; quad.v[0].ty=ty1;
// 	quad.v[1].tx=tx2; quad.v[1].ty=ty1;
// 	quad.v[2].tx=tx2; quad.v[2].ty=ty2;
// 	quad.v[3].tx=tx1; quad.v[3].ty=ty2;

	bX=m_bXFlip; bY=m_bYFlip; bHS=m_bHSFlip;
	m_bXFlip=false; m_bYFlip=false;
	setFlip(bX,bY,bHS);
}

