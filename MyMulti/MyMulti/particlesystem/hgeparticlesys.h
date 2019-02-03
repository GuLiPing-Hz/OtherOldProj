/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hg.relishgames.com
**
** hgeParticleSystem helper class header
*/


#ifndef HGEPARTICLE_H
#define HGEPARTICLE_H

#include "ParticleInfo.h"
#include "hgerect.h"

#include "../OpenGLWindow.h"

class hgeParticleSystem
{
public:
	
	hgeParticleSystem(const char *filename, CGLSprite *sprite);
	
	hgeParticleSystem(structParticleSystemInfo *psi/*,int nEmitter=EMITTER_POINT*/);

	hgeParticleSystem(const hgeParticleSystem &ps);
	~hgeParticleSystem() {  }

	hgeParticleSystem&	operator= (const hgeParticleSystem &ps);


	void				drawRender(bool bAlphaBlend=true);
	void				FireAt(float x, float y);
	void				Fire();
	void				Stop(bool bKillParticles=false);
	void				Update(float fDeltaTime);
	void				MoveTo(float x, float y, bool bMoveParticles=false);
	void				Transpose(float x, float y) { m_fTx=x; m_fTy=y; }
	void				SetScale(float scale) { m_fScale = scale; }
	void				TrackBoundingBox(bool bTrack) { m_bUpdateBoundingBox=bTrack; }

	int				GetParticlesAlive() const { return m_nParticlesAlive; }
	float				GetAge() const { return m_fAge; }
	void				SetPosition(float x,float y) {m_vecLocation.x=x;m_vecLocation.y=y;m_vecPrevLocation.x=x;m_vecPrevLocation.y=y;}
	void				GetPosition(float *x, float *y) const { *x=m_vecLocation.x; *y=m_vecLocation.y; }
	void				GetTransposition(float *x, float *y) const { *x=m_fTx; *y=m_fTy; }
	float				GetScale() { return m_fScale; }
	hgeRect*		GetBoundingBox(hgeRect *rect) const;
	void				SetPointCreateRect(const CGRect& rect){m_info.pointCreateRect=rect;}
	bool				isStop();
private:
	hgeParticleSystem();

	//static HGE			*hge;
	static COpenGLWindow* cls_gl;
protected:
	float					m_fAge;
	float					m_fEmissionResidue;
	bool					m_bOneShotCreate;

	hgeVector2		m_vecPrevLocation;
	hgeVector2		m_vecLocation;
	float					m_fTx, m_fTy;
	float					m_fScale;

	int					m_nParticlesAlive;
	hgeRect			m_rectBoundingBox;
	bool					m_bUpdateBoundingBox;

	structParticle			m_arrparticles[MAX_PARTICLES];
	

	float 				m_fInside_2_A;
	float 				m_fInside_2_B;
	float					m_fOutside_2_A;
	float					m_fOutside_2_B;
public:
	structParticleSystemInfo m_info;
};


#endif
