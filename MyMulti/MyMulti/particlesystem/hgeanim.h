/*
** 
** 
** 
**
** hgeAnimation helper class header
*/


#ifndef HGEANIM_H
#define HGEANIM_H


#include "hgesprite.h"


#define HGEANIM_FWD			0
#define HGEANIM_REV			1
#define HGEANIM_PINGPONG	2
#define HGEANIM_NOPINGPONG	0
#define HGEANIM_LOOP		4
#define HGEANIM_NOLOOP		0


/*
** Animation class
*/
class CGLAnimation : public CGLSprite
{//要求是正方形的纹理,而且每个正方形大小都一样
public:
	CGLAnimation(Imageset *imgset/*HTEXTURE tex*/,int nframes, float FPS, float x, float y, int w, int h);
	CGLAnimation(const CGLAnimation &anim);
	
	void		Play();
	void		Stop() { bPlaying=false; }
	void		Resume() { bPlaying=true; }
	void		Update(float fDeltaTime);
	bool		IsPlaying() const { return bPlaying; }

	//void		SetTexture(std::string imgsetName/*HTEXTURE tex*/) { hgeSprite::SetTexture(imgsetName); orig_width = Texture_GetWidth(imgsetName); }
	void		SetTextureRect(float x1, float y1, float x2, float y2) { CGLSprite::SetTextureRect(x1,y1,(int)(x2-x1),(int)(y2-y1)); SetFrame(nCurFrame); }
	void		SetMode(int mode);
	void		SetSpeed(float FPS) { fSpeed=1.0f/FPS; }
	void		SetFrame(int n);
	void		SetFrames(int n) { nFrames=n; }

	int		GetMode() const { return Mode; }
	float		GetSpeed() const { return 1.0f/fSpeed; }
	int		GetFrame() const { return nCurFrame; }
	int		GetFrames() const { return nFrames; }

private:
	CGLAnimation();

	int			orig_width;

	bool		bPlaying;

	float		fSpeed;
	float		fSinceLastFrame;

	int			Mode;
	int			nDelta;
	int			nFrames;
	int			nCurFrame;

	//bool			m_bInit;
};


#endif
