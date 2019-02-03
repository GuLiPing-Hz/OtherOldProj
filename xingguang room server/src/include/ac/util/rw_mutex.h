#ifndef AC_UTIL_RW_MUTEX_H_
#define AC_UTIL_RW_MUTEX_H_

#include <pthread.h>

namespace ac
{

class RWMutex
{
public:
	virtual ~RWMutex() {}

	virtual int AcquireRead(bool block=true) const = 0;
	virtual int AcquireWrite(bool block=true) const = 0;
	virtual int Release() const = 0;
};


//--------------------------------------------------------------------


class RLockGuard
{
public:

	RLockGuard(const RWMutex * mutex) : mutex_(mutex), acquired_(false) {}

	~RLockGuard()
	{
		if (acquired_) {
			mutex_->Release();
		}
	}
    
	int Lock(bool block=true) const
	{
		if ( acquired_ ) {
			return -1;
		}

		if ( mutex_->AcquireRead(block) != 0 ) {
			return -1;
		}

		acquired_ = true;
		return 0;
	}

	int Unlock() const
	{
		if ( !acquired_ ) {
			return -1;
		}
		if ( mutex_->Release() != 0 ) {
			return -1;
		}
		acquired_ = false;
		return 0;
	}
	
	bool IsLocked() const
	{
		return acquired_;
	}
   
private:
    const RWMutex * mutex_;
    mutable bool acquired_;
};


//--------------------------------------------------------------------


class WLockGuard
{
public:

	WLockGuard(const RWMutex * mutex) : mutex_(mutex), acquired_(false) {}

	~WLockGuard()
	{
		if (acquired_) {
			mutex_->Release();
		}
	}
    
	int Lock(bool block=true) const
	{
		if ( acquired_ ) {
			return -1;
		}

		if ( mutex_->AcquireWrite(block) != 0 ) {
			return -1;
		}

		acquired_ = true;
		return 0;
	}

	int Unlock() const
	{
		if ( !acquired_ ) {
			return -1;
		}
		if ( mutex_->Release() != 0 ) {
			return -1;
		}
		acquired_ = false;
		return 0;
	}
	
	bool IsLocked() const
	{
		return acquired_;
	}
   
private:
    const RWMutex * mutex_;
    mutable bool acquired_;
};


//--------------------------------------------------------------------


class ThreadRWMutex : public RWMutex
{
public:
	ThreadRWMutex()
	{
		pthread_rwlock_init(&rwmutex_, NULL);
	}
		
	~ThreadRWMutex()
	{
		pthread_rwlock_destroy(&rwmutex_);
	}
		
	int AcquireRead(bool block) const
	{
		if ( block ) {
			return pthread_rwlock_rdlock(&rwmutex_);
		}
		else {
			return pthread_rwlock_tryrdlock(&rwmutex_);
		}
	}

	int AcquireWrite(bool block) const
	{
		if ( block ) {
			return pthread_rwlock_wrlock(&rwmutex_);
		}
		else {
			return pthread_rwlock_trywrlock(&rwmutex_);
		}
	}	
		
	int Release() const
	{
		return pthread_rwlock_unlock(&rwmutex_);
	}

private:
	mutable pthread_rwlock_t rwmutex_;
};

}

#endif // AC_UTIL_RW_MUTEX_H_

