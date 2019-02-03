#ifndef AC_UTIL_DESTROYABLE_H
#define AC_UTIL_DESTROYABLE_H

#include <assert.h>
//#include "ac/log/log.h"

namespace ac {


class Destroyable;
struct DestroyCallback
{
	virtual ~DestroyCallback() {}
	virtual void call(Destroyable* p) = 0;
};


class Destroyable
{
public:

	virtual ~Destroyable() {}
	
	void Destroy()
	{
		//AC_INFO("try Destroy client=%x", this);
		if ( pCB_ ) {
			pCB_->call(this);
			//AC_INFO("success Destroy client=%x", this);
		}
	}

	inline void SetDestroyCallback(DestroyCallback * pCB)
	{
		assert(pCB);
		pCB_ = pCB;
	}

	inline DestroyCallback * GetDestroyCallback() const
	{
		return pCB_;
	}

private:
	DestroyCallback * pCB_;
};

class DestroyableGuard
{
public:
	DestroyableGuard(Destroyable *pDab) : m_pDab(pDab){}
	~DestroyableGuard(){
		m_pDab->Destroy();}
private:
	Destroyable *m_pDab;
};

} // namespace ac

#endif // AC_UTIL_DESTROYABLE_H

