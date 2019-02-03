#include "hgeparticlesys.h"

void hgeParticleSystem::Update(float fDeltaTime)
{
	int i;
	float ang;
	structParticle *par;
	hgeVector2 vecAccel, vecAccel2;

	if(m_fAge >= 0)
	{
		m_fAge += fDeltaTime;
		if(m_fAge >= m_info.fLifetime) m_fAge = -2.0f;
	}

	// update all alive particles

	if(m_bUpdateBoundingBox) m_rectBoundingBox.Clear();
	par=m_arrparticles;

	for(i=0; i<m_nParticlesAlive; i++)
	{
		par->fAge += fDeltaTime;
		if(par->fAge >= par->fTerminalAge)
		{
			m_nParticlesAlive--;
			if (m_info.bOneShot&&m_nParticlesAlive==0)
			{
				m_bOneShotCreate = true;
			}
			memcpy(par, &m_arrparticles[m_nParticlesAlive], sizeof(structParticle));
			i--;
			continue;
		}

		vecAccel = par->vecLocation-m_vecLocation;
		vecAccel.Normalize();
		vecAccel2 = vecAccel;
		vecAccel *= par->fRadialAccel;

		// vecAccel2.Rotate(M_PI_2);
		// the following is faster
		ang = vecAccel2.x;
		vecAccel2.x = -vecAccel2.y;
		vecAccel2.y = ang;

		vecAccel2 *= par->fTangentialAccel;
		par->vecVelocity += (vecAccel+vecAccel2)*fDeltaTime;
		par->vecVelocity.y += par->fGravity*fDeltaTime;

		par->vecLocation += par->vecVelocity*fDeltaTime;

		par->fSpin += par->fSpinDelta*fDeltaTime;
		par->fSize += par->fSizeDelta*fDeltaTime;
		par->colColor += par->colColorDelta*fDeltaTime;

		if(m_bUpdateBoundingBox) m_rectBoundingBox.Encapsulate(par->vecLocation.x, par->vecLocation.y);

		par++;
	}

	// generate new particles
	if(m_fAge != -2.0f)
	{
		if ((m_info.bOneShot&&m_bOneShotCreate) || !m_info.bOneShot)
		{
			int nParticlesCreated;
			if (m_info.bOneShot)
			{
				nParticlesCreated = m_info.nEmission;
				m_bOneShotCreate = false;
			}
			else
			{
				float fParticlesNeeded = m_info.nEmission*fDeltaTime + m_fEmissionResidue;
				nParticlesCreated = (unsigned int)fParticlesNeeded;
				m_fEmissionResidue=fParticlesNeeded-nParticlesCreated;
			}
// 		float fParticlesNeeded = info.nEmission*fDeltaTime + m_fEmissionResidue;
// 		int nParticlesCreated = (unsigned int)fParticlesNeeded;
// 		m_fEmissionResidue=fParticlesNeeded-nParticlesCreated;

			par=&m_arrparticles[m_nParticlesAlive];

			for(i=0; i<nParticlesCreated; i++)
			{
				if(m_nParticlesAlive>=MAX_PARTICLES) break;

				par->fAge = 0.0f;
				par->fTerminalAge = cls_gl->getTimerListener()->Random_Float(m_info.fParticleLifeMin, m_info.fParticleLifeMax);

				par->vecLocation = m_vecPrevLocation+(m_vecLocation-m_vecPrevLocation)*cls_gl->getTimerListener()->Random_Float(0.0f, 1.0f);
				if (m_info.nEmitter & EMITTER_RECT)
				{	
					par->vecLocation.x = cls_gl->getTimerListener()->Random_Float(m_info.pointCreateRect.origin.x,m_info.pointCreateRect.origin.x+m_info.pointCreateRect.size.width);
					par->vecLocation.y = cls_gl->getTimerListener()->Random_Float(m_info.pointCreateRect.origin.y,m_info.pointCreateRect.origin.y+m_info.pointCreateRect.size.height);
				}
				else if (m_info.nEmitter & EMITTER_POINT)
				{
					if (m_info.bSpread)
					{
						par->vecLocation.x += cls_gl->getTimerListener()->Random_Float(-2.0f, 2.0f);
						par->vecLocation.y += cls_gl->getTimerListener()->Random_Float(-2.0f, 2.0f);
					}
				}
				else if (m_info.nEmitter & EMITTER_CIRCLE)
				{
					m_fInside_2_A = m_info.circleInsideA*m_info.circleInsideA;
					m_fInside_2_B = m_info.circleInsideB*m_info.circleInsideB;

					float tmpX = cls_gl->getTimerListener()->Random_Float(-m_info.circleInsideA,m_info.circleInsideA);
					float tmpY = (float) sqrt(m_fInside_2_B-m_fInside_2_B*tmpX*tmpX/m_fInside_2_A);
					tmpY = cls_gl->getTimerListener()->Random_Float(-tmpY,tmpY);
					par->vecLocation.x = tmpX+m_info.fOriginX;
					par->vecLocation.y = tmpY+m_info.fOriginY;
				}
				else if (m_info.nEmitter & EMITTER_RING)
				{
					if (m_info.circleInsideA<=m_info.circleOutsideA || m_info.circleInsideB<=m_info.circleOutsideB)//不在要求内
					{
						return ;
					}

					m_fInside_2_A = m_info.circleInsideA*m_info.circleInsideA;
					m_fInside_2_B = m_info.circleInsideB*m_info.circleInsideB;

					float tmpX = cls_gl->getTimerListener()->Random_Float(-m_info.circleInsideA,m_info.circleInsideA);
					float tmpY1,tmpY2;
					tmpY1 = (float) sqrt(m_fInside_2_B - m_fInside_2_B*tmpX*tmpX/m_fInside_2_A);
					if ((tmpX>-m_info.circleInsideA&&tmpX<-m_info.circleOutsideA) || (tmpX>m_info.circleOutsideA&&tmpX<m_info.circleInsideA))
					{
						tmpY2 = -tmpY1;
					}
					else
					{
						static bool bControl = true;

						m_fOutside_2_A = m_info.circleOutsideA*m_info.circleOutsideA;
						m_fOutside_2_B = m_info.circleOutsideB*m_info.circleOutsideB;

						tmpY2 = (float) sqrt(m_fOutside_2_B - m_fOutside_2_B*tmpX*tmpX/m_fOutside_2_A);

						if (bControl)
						{
							bControl = false;
						}
						else
						{
							float tmp = tmpY1;
							tmpY1 = -tmpY2;
							tmpY2 = -tmp;
							bControl =true;
						}

					}
					float tmpY = cls_gl->getTimerListener()->Random_Float(tmpY2,tmpY1);
					par->vecLocation.x = tmpX+m_info.fOriginX;
					par->vecLocation.y = tmpY+m_info.fOriginY;
				}
				else 
				{
					return ;
				}

				ang=m_info.fDirection-M_PI_2+cls_gl->getTimerListener()->Random_Float(0,m_info.fSpread)-m_info.fSpread/2.0f;
				if(m_info.bRelative) ang += (m_vecPrevLocation-m_vecLocation).Angle()+M_PI_2;
				par->vecVelocity.x = cosf(ang);
				par->vecVelocity.y = sinf(ang);
				//par->vecVelocity *= cls_gl->GetTimerListener()->Random_Float(m_info.fSpeedMin, m_info.fSpeedMax);

				if (m_info.btwoEnds)
				{
					static bool bControl = true;
					if (bControl)
					{
						par->vecVelocity *=m_info.fSpeedMin;
						bControl = false;
					}
					else
					{
						par->vecVelocity *=m_info.fSpeedMax;
						bControl = true;
					}
				}
				else
				{
					par->vecVelocity *= cls_gl->getTimerListener()->Random_Float(m_info.fSpeedMin, m_info.fSpeedMax);
				}

				par->fGravity = cls_gl->getTimerListener()->Random_Float(m_info.fGravityMin, m_info.fGravityMax);
				par->fRadialAccel = cls_gl->getTimerListener()->Random_Float(m_info.fRadialAccelMin, m_info.fRadialAccelMax);
				par->fTangentialAccel = cls_gl->getTimerListener()->Random_Float(m_info.fTangentialAccelMin, m_info.fTangentialAccelMax);

				par->fSize = cls_gl->getTimerListener()->Random_Float(m_info.fSizeStart, m_info.fSizeStart+(m_info.fSizeEnd-m_info.fSizeStart)*m_info.fSizeVar);
				par->fSizeDelta = (m_info.fSizeEnd-par->fSize) / par->fTerminalAge;

				par->fSpin = cls_gl->getTimerListener()->Random_Float(m_info.fSpinStart, m_info.fSpinStart+(m_info.fSpinEnd-m_info.fSpinStart)*m_info.fSpinVar);
				par->fSpinDelta = (m_info.fSpinEnd-par->fSpin) / par->fTerminalAge;

				par->colColor.r = cls_gl->getTimerListener()->Random_Float(m_info.colColorStart.r, m_info.colColorStart.r+(m_info.colColorEnd.r-m_info.colColorStart.r)*m_info.fColorVar);
				par->colColor.g = cls_gl->getTimerListener()->Random_Float(m_info.colColorStart.g, m_info.colColorStart.g+(m_info.colColorEnd.g-m_info.colColorStart.g)*m_info.fColorVar);
				par->colColor.b = cls_gl->getTimerListener()->Random_Float(m_info.colColorStart.b, m_info.colColorStart.b+(m_info.colColorEnd.b-m_info.colColorStart.b)*m_info.fColorVar);
				par->colColor.a = cls_gl->getTimerListener()->Random_Float(m_info.colColorStart.a, m_info.colColorStart.a+(m_info.colColorEnd.a-m_info.colColorStart.a)*m_info.fAlphaVar);

				par->colColorDelta.r = (m_info.colColorEnd.r-par->colColor.r) / par->fTerminalAge;
				par->colColorDelta.g = (m_info.colColorEnd.g-par->colColor.g) / par->fTerminalAge;
				par->colColorDelta.b = (m_info.colColorEnd.b-par->colColor.b) / par->fTerminalAge;
				par->colColorDelta.a = (m_info.colColorEnd.a-par->colColor.a) / par->fTerminalAge;

				if(m_bUpdateBoundingBox) m_rectBoundingBox.Encapsulate(par->vecLocation.x, par->vecLocation.y);

				m_nParticlesAlive++;
				par++;
			}
		}
	}

	m_vecPrevLocation=m_vecLocation;
}
