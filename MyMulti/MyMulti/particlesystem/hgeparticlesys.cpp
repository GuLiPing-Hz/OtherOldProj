/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hg.relishgames.com
**
** hgeParticleSystem helper class implementation
*/


#include "hgeparticlesys.h"


//HGE	*hgeParticleSystem::cls_gl=0;
COpenGLWindow* hgeParticleSystem::cls_gl = NULL;

hgeParticleSystem::hgeParticleSystem(const char *filename, CGLSprite *sprite)
// :m_pointCreateRect(CGRect(-2.0f,-2.0f,4,4))
// ,m_nEmitter(nEmitter)
:m_bOneShotCreate(true)
// ,m_circleInsideA(.0f)
// ,m_circleOutsideA(.0f)
// ,m_circleInsideB(.0f)
// ,m_circleOutsideB(.0f)
,m_fInside_2_A(.0f)
,m_fInside_2_B(.0f)
,m_fOutside_2_A(.0f)
,m_fOutside_2_B(.0f)
{
	void *psi;

	//cls_gl=hgeCreate(HGE_VERSION);
	cls_gl = COpenGLWindow::getWindowSingleton();

	psi=cls_gl->getImgsetMgr()->Resource_Load(filename);
	if(!psi) return;
	memcpy(&m_info, psi, sizeof(structParticleSystemInfo));
	cls_gl->getImgsetMgr()->Resource_Free(psi);
	m_info.sprite=sprite;

	m_vecLocation.x=m_vecPrevLocation.x=0.0f;
	m_vecLocation.y=m_vecPrevLocation.y=0.0f;
	m_fTx=m_fTy=0;
	m_fScale = 1.0f;

	m_fEmissionResidue=0.0f;
	m_nParticlesAlive=0;
	m_fAge=-2.0;

	m_rectBoundingBox.Clear();
	m_bUpdateBoundingBox=false;
}


hgeParticleSystem::hgeParticleSystem(structParticleSystemInfo *psi/*,float a1,float b1,float a2,float b2,int nEmitter*/)
//:m_nEmitter(nEmitter)
:m_bOneShotCreate(true)
// ,m_pointCreateRect(CGRECTZERO)
// ,m_circleInsideA(a1)
//,m_circleOutsideA(a2)
// ,m_circleInsideB(b1)
// ,m_circleOutsideB(b2)
// ,m_fInsideA_2(a1*a1)
// ,m_fInsideB_2(b1*b1)
// ,m_fOutsideA_2(a2*a2)
// ,m_fOutsideB_2(b2*b2)
{
	cls_gl = COpenGLWindow::getWindowSingleton();
	memcpy(&m_info, psi, sizeof(structParticleSystemInfo));
	m_fInside_2_A = m_info.circleInsideA*m_info.circleInsideA;
	m_fInside_2_B = m_info.circleInsideB*m_info.circleInsideB;
	m_fOutside_2_A = m_info.circleOutsideA*m_info.circleOutsideA;
	m_fOutside_2_B = m_info.circleOutsideB*m_info.circleOutsideB;

	m_vecLocation.x=m_vecPrevLocation.x=0.0f;
	m_vecLocation.y=m_vecPrevLocation.y=0.0f;
	m_fTx=m_fTy=0;
	m_fScale = 1.0f;

	m_fEmissionResidue=0.0f;
	m_nParticlesAlive=0;
	m_fAge=-2.0;

	m_rectBoundingBox.Clear();
	m_bUpdateBoundingBox=false;
}

hgeParticleSystem::hgeParticleSystem(const hgeParticleSystem &ps)
{
	memcpy(this, &ps, sizeof(hgeParticleSystem));
	cls_gl=COpenGLWindow::getWindowSingleton();
}

void hgeParticleSystem::MoveTo(float x, float y, bool bMoveParticles)
{
	int i;
	float dx,dy;
	
	if(bMoveParticles)
	{
		dx=x-m_vecLocation.x;
		dy=y-m_vecLocation.y;

		for(i=0;i<m_nParticlesAlive;i++)
		{
			m_arrparticles[i].vecLocation.x += dx;
			m_arrparticles[i].vecLocation.y += dy;
		}

		m_vecPrevLocation.x=m_vecPrevLocation.x + dx;
		m_vecPrevLocation.y=m_vecPrevLocation.y + dy;
	}
	else
	{
		if(m_fAge==-2.0) { m_vecPrevLocation.x=x; m_vecPrevLocation.y=y; }
		else { m_vecPrevLocation.x=m_vecLocation.x;	m_vecPrevLocation.y=m_vecLocation.y; }
	}

	m_vecLocation.x=x;
	m_vecLocation.y=y;
}

void hgeParticleSystem::FireAt(float x, float y)
{
	Stop();
	MoveTo(x,y);
	Fire();
}

void hgeParticleSystem::Fire()
{
	if(m_info.fLifetime==-1.0f) m_fAge=-1.0f;
	else m_fAge=0.0f;
}

bool	hgeParticleSystem::isStop()
{
	return (m_fAge+2.0f < 0.001f);
}

void hgeParticleSystem::Stop(bool bKillParticles)
{
	m_fAge=-2.0f;
	if(bKillParticles) 
	{
		m_nParticlesAlive=0;
		m_rectBoundingBox.Clear();
	}
}

void hgeParticleSystem::drawRender(bool bAlphaBlend)
{
	int i;
	DWORD col;
	structParticle *par=m_arrparticles;

	ASSERT(m_info.sprite != NULL);
	
	col=m_info.sprite->GetColor().c;
	
	for(i=0; i<m_nParticlesAlive; i++)
	{
		if(m_info.colColorStart.r < 0)
			m_info.sprite->SetColor(SETA(m_info.sprite->GetColor().c,par->colColor.a*255));
		else
			m_info.sprite->SetColor(par->colColor.GetHWColor());
		m_info.sprite->drawRenderEx(par->vecLocation.x*m_fScale, par->vecLocation.y*m_fScale, par->fSpin/**par->fAge*/,bAlphaBlend, par->fSize*m_fScale);
		par++;
	}

	m_info.sprite->SetColor(col);
}


hgeRect *hgeParticleSystem::GetBoundingBox(hgeRect *rect) const
{
	*rect = m_rectBoundingBox;

	rect->x1 *= m_fScale;
	rect->y1 *= m_fScale;
	rect->x2 *= m_fScale;
	rect->y2 *= m_fScale;

	return rect;
}
