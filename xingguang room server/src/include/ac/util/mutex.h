#ifndef AC_UTIL_MUTEX_H_
#define AC_UTIL_MUTEX_H_

#include <pthread.h>
#include <sys/sem.h>

namespace ac
{

class Mutex
{
public:
	virtual ~Mutex() {}
	virtual int Acquire(bool block=true) const = 0;
	virtual int Release() const = 0;
};


//--------------------------------------------------------------------


class LockGuard
{
public:

	LockGuard(const Mutex * mutex) : mutex_(mutex), acquired_(false) {}

	~LockGuard()
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

		if ( mutex_->Acquire(block) != 0 ) {
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
    const Mutex * mutex_;
    mutable bool acquired_;
};


//--------------------------------------------------------------------


class NullMutex : public Mutex
{
public:
	virtual int Acquire(bool) const { return 0; }
	virtual int Release() const { return 0; }
public:
	static NullMutex * Instance();
};


//--------------------------------------------------------------------


class ThreadMutex : public Mutex
{
public:
	ThreadMutex()
	{
		pthread_mutex_init(&mutex_, NULL);
	}
	
	virtual ~ThreadMutex()
	{
		pthread_mutex_destroy(&mutex_);
	}
	
	virtual int Acquire(bool block=true) const
	{
		if ( block ) {
			return pthread_mutex_lock(&mutex_);
		}
		else {
			return pthread_mutex_trylock(&mutex_);
		}
	}
	
	virtual int Release() const
	{
		return pthread_mutex_unlock(&mutex_);
	}

private:
	mutable pthread_mutex_t mutex_;
};


//--------------------------------------------------------------------

class SemMutex : public Mutex
{
public:
	SemMutex(key_t key, int oflag=0644);
	virtual int Acquire(bool block=true) const;
	virtual int Release() const;

private:
	int semid_;
};

}

#endif // AC_MUTEX_H_

