#ifndef PARTICLEINFO__H__
#define PARTICLEINFO__H__

#include "hgesprite.h"
#include "hgevector.h"
#include "hgecolor.h"
#include "../GLDef.h"

#define MAX_PARTICLES	400
#define MAX_PSYSTEMS	100

struct structParticle
{
	hgeVector2	vecLocation;
	hgeVector2	vecVelocity;

	float		fGravity;
	float		fRadialAccel;
	float		fTangentialAccel;

	float		fSpin;
	float		fSpinDelta;

	float		fSize;
	float		fSizeDelta;

	hgeColor	colColor;		// + alpha
	hgeColor	colColorDelta;

	float		fAge;
	float		fTerminalAge;
};

struct structParticleSystemInfo
{
	CGLSprite*		sprite;    // texture + blend mode
	bool					bOneShot;
	bool					bSpread;
	//////////////////////////////////////////////////////////////////////////glp
	int					nEmitter;
	CGRect			pointCreateRect;//粒子生成框
	float					circleInsideA;
	float					circleOutsideA;
	float					circleInsideB;
	float					circleOutsideB;

	float					fOriginX;//原点坐标
	float					fOriginY;
	//////////////////////////////////////////////////////////////////////////
	int		nEmission; // particles per sec
	float		fLifetime;

	float		fParticleLifeMin;
	float		fParticleLifeMax;

	float		fDirection;
	float		fSpread;
	bool		bRelative;

	bool		btwoEnds;
	float		fSpeedMin;
	float		fSpeedMax;

	float		fGravityMin;
	float		fGravityMax;

	float		fRadialAccelMin;
	float		fRadialAccelMax;

	float		fTangentialAccelMin;
	float		fTangentialAccelMax;

	float		fSizeStart;
	float		fSizeEnd;
	float		fSizeVar;

	float		fSpinStart;
	float		fSpinEnd;
	float		fSpinVar;

	hgeColor	colColorStart; // + alpha
	hgeColor	colColorEnd;
	float			fColorVar;
	float			fAlphaVar;
};

#endif//PARTICLEINFO__H__