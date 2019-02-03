/*
修改时间：2010.10.15
修改人：王鹏飞
修改原因：防止上层多次调用Destroy，导致list里面有多个同样的指针
修改方式：在每块内存的最后加上一个字节，用来控制这个指针是否已经在list中
*/


#ifndef AC_UTIL_OBJECT_POOL_H
#define AC_UTIL_OBJECT_POOL_H

#include <list>
#include <vector>
#include <sys/types.h>
#include <ac/util/allocator.h>
#include <ac/util/objectallocator.h>
#include <ac/util/mutex.h>


namespace ac {
template <class T>
class ObjectPoolAllocator : public ObjectAllocator<T>
{
public:
	ObjectPoolAllocator(size_t iInitNum, size_t iMaxNum=0, size_t iIncNum=0, Mutex * pMutex=NullMutex::Instance(), Allocator *pAllocator=Allocator::Instance())
		: pAllocator_(pAllocator), pMutex_(pMutex), iCurrNum_(0), iMaxNum_(iMaxNum), iIncNum_(iIncNum)
	{
		if ( iMaxNum_ < iInitNum ) {
			iMaxNum_ = iInitNum;
			iIncNum_ = 0;
		}
	
		IncreaseObjects(iInitNum);
	}

	virtual ~ObjectPoolAllocator()
	{
		for ( typename::std::list<T *>::iterator pos=Objects_.begin(); pos!=Objects_.end(); ++pos ) {
			(*pos)->~T();
		}
	
		for ( typename::std::vector<T *>::iterator pos=Chunks_.begin(); pos!=Chunks_.end(); ++pos ) {
			pAllocator_->Deallocate(*pos);
		}
	}

	inline bool IsInit() const { return iCurrNum_>0; }

	virtual T * Create()
	{
		LockGuard lock(pMutex_);
		if ( lock.Lock(true) != 0 ) {
			return NULL;
		}

		if ( Objects_.empty() ) {
			if ( IncreaseObjects(iIncNum_) != 0 ) {
				return NULL;
			}
		}
	
		T * pObject = *(Objects_.begin());
		Objects_.pop_front();

		*((char*)pObject+sizeof(T)) = 0;

		return pObject;
	}

	virtual void Destroy(T * pObject)
	{
		LockGuard lock(pMutex_);
		if ( lock.Lock(true) != 0 ) {
			return;
		}
	
		if ( pObject != NULL ) {
			if (*((char*)pObject+sizeof(T)) == 0)
			{
				Objects_.push_front(pObject);
				*((char*)pObject+sizeof(T)) = 1;
			}
		}
	}

private:

	int IncreaseObjects(size_t iIncNum)
	{
		if ( iCurrNum_ >= iMaxNum_ ) {
			return -1;
		}

		if ( iCurrNum_ + iIncNum > iMaxNum_ ) {
			iIncNum = iMaxNum_ - iCurrNum_;
		}

		T * pChunk = (T *)pAllocator_->Allocate((sizeof(T)+1)*iIncNum);
		if ( pChunk == NULL ) {
			return -1;
		}

		for ( size_t i=0; i<iIncNum; ++i ) {
			T* p = new((void *)((char*)pChunk+(sizeof(T)+1)*i)) T;
			Objects_.push_back(p);
		}

		Chunks_.push_back(pChunk);
		iCurrNum_ += iIncNum;		
		return 0;		
	}

private:

	Allocator *	pAllocator_;
	Mutex *		pMutex_;

	size_t		iCurrNum_;
	size_t		iMaxNum_;
	size_t		iIncNum_;
	
	std::list<T *>	Objects_;
	std::vector<T *>	Chunks_;
};

//原来的代码
//template <class T>
//class ObjectPoolAllocator : public ObjectAllocator<T>
//{
//public:
//	ObjectPoolAllocator(size_t iInitNum, size_t iMaxNum=0, size_t iIncNum=0, Mutex * pMutex=NullMutex::Instance(), Allocator *pAllocator=Allocator::Instance())
//		: pAllocator_(pAllocator), pMutex_(pMutex), iCurrNum_(0), iMaxNum_(iMaxNum), iIncNum_(iIncNum)
//	{
//		if ( iMaxNum_ < iInitNum ) {
//			iMaxNum_ = iInitNum;
//			iIncNum_ = 0;
//		}
//
//		IncreaseObjects(iInitNum);
//	}
//
//	virtual ~ObjectPoolAllocator()
//	{
//		for ( typename::std::list<T *>::iterator pos=Objects_.begin(); pos!=Objects_.end(); ++pos ) {
//			(*pos)->~T();
//		}
//
//		for ( typename::std::vector<T *>::iterator pos=Chunks_.begin(); pos!=Chunks_.end(); ++pos ) {
//			pAllocator_->Deallocate(*pos);
//		}
//	}
//
//	inline bool IsInit() const { return iCurrNum_>0; }
//
//	virtual T * Create()
//	{
//		LockGuard lock(pMutex_);
//		if ( lock.Lock(true) != 0 ) {
//			return NULL;
//		}
//
//		if ( Objects_.empty() ) {
//			if ( IncreaseObjects(iIncNum_) != 0 ) {
//				return NULL;
//			}
//		}
//
//		T * pObject = *(Objects_.begin());
//		Objects_.pop_front();
//
//		return pObject;
//	}
//
//	virtual void Destroy(T * pObject)
//	{
//		LockGuard lock(pMutex_);
//		if ( lock.Lock(true) != 0 ) {
//			return;
//		}
//
//		if ( pObject != NULL ) {
//			Objects_.push_front(pObject);
//		}
//	}
//
//private:
//
//	int IncreaseObjects(size_t iIncNum)
//	{
//		if ( iCurrNum_ >= iMaxNum_ ) {
//			return -1;
//		}
//
//		if ( iCurrNum_ + iIncNum > iMaxNum_ ) {
//			iIncNum = iMaxNum_ - iCurrNum_;
//		}
//
//		T * pChunk = (T *)pAllocator_->Allocate(sizeof(T)*iIncNum);
//		if ( pChunk == NULL ) {
//			return -1;
//		}
//
//		for ( size_t i=0; i<iIncNum; ++i ) {
//			T* p = new((void *)(pChunk+i)) T;
//			Objects_.push_back(p);
//		}
//
//		Chunks_.push_back(pChunk);
//		iCurrNum_ += iIncNum;		
//		return 0;		
//	}
//
//private:
//
//	Allocator *	pAllocator_;
//	Mutex *		pMutex_;
//
//	size_t		iCurrNum_;
//	size_t		iMaxNum_;
//	size_t		iIncNum_;
//
//	std::list<T *>	Objects_;
//	std::vector<T *>	Chunks_;
//};

} // namespace ac


#endif // AC_UTIL_OBJECT_POOL_H

