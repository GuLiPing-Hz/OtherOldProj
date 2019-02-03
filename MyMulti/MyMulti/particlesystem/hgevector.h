/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeVector helper class
*/


#ifndef HGEVECTOR_H
#define HGEVECTOR_H


#include "../staff/ImgsetMgr.h"
#include <math.h>


/*
** Fast 1.0/sqrtf(float) routine
*/
float InvSqrt(float x);

class hgeVector2
{
public:
	float	x,y;

	hgeVector2(float _x, float _y)	{ x=_x; y=_y; }
	hgeVector2()						{ x=0; y=0; }

	hgeVector2	operator-  () const { return hgeVector2(-x, -y);		}
	hgeVector2	operator-  (const hgeVector2 &v) const { return hgeVector2(x-v.x, y-v.y); }
	hgeVector2	operator+  (const hgeVector2 &v) const { return hgeVector2(x+v.x, y+v.y); }
	hgeVector2&	operator-= (const hgeVector2 &v)		  { x-=v.x; y-=v.y; return *this;	}
	hgeVector2&	operator+= (const hgeVector2 &v)		  { x+=v.x; y+=v.y; return *this;	}
	bool		operator== (const hgeVector2 &v)	const { return (x==v.x && y==v.y);		}
	bool		operator!= (const hgeVector2 &v)	const { return (x!=v.x || y!=v.y);		}

	hgeVector2	operator/  (const float scalar)	const { return hgeVector2(x/scalar, y/scalar); }
	hgeVector2	operator*  (const float scalar) const { return hgeVector2(x*scalar, y*scalar); }
	hgeVector2&	operator*= (const float scalar)		  { x*=scalar; y*=scalar; return *this;   }

	float		Dot(const hgeVector2 *v) const { return x*v->x + y*v->y; }
	float		Length() const { return sqrtf(Dot(this)); }
	float		Angle(const hgeVector2 *v = 0) const;
	
	void		Clamp(const float max) { if(Length() > max)	{ Normalize(); x *= max; y *= max; } }
	hgeVector2*	Normalize() { float rc=InvSqrt(Dot(this)); x*=rc; y*=rc; return this; }
	hgeVector2*	Rotate(float a);
};

inline hgeVector2 operator* (const float s, const hgeVector2 &v)		{ return v*s; }
inline float	 operator^ (const hgeVector2 &v, const hgeVector2 &u) { return v.Angle(&u); }
inline float	 operator% (const hgeVector2 &v, const hgeVector2 &u) { return v.Dot(&u); }


#endif
