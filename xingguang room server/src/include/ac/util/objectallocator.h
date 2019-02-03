#ifndef AC_UTIL_OBJECT_ALLOCATOR_H
#define AC_UTIL_OBJECT_ALLOCATOR_H

#include <ac/util/allocator.h>


namespace ac {


template <class T>
class ObjectAllocator
{
public:
	virtual ~ObjectAllocator() {}
	virtual T * Create() = 0;
	virtual void Destroy(T * pObject) = 0;
};


template <class T>
class ObjectDefaultAllocator : public ObjectAllocator<T>
{
public:
	ObjectDefaultAllocator(Allocator * pAllocator) : pAllocator_(pAllocator) {}

	virtual T * Create()
	{
		void * p = pAllocator_->Allocate(sizeof(T));
		if ( p == NULL ) {
			return NULL;
		}
		return new (p) T;
	}

	virtual void Destroy(T * p)
	{
		p->~T();
		pAllocator_->Deallocate(p);		
	}

private:
	Allocator * pAllocator_;
};

} // namespace ac


#endif // AC_UTIL_OBJECT_ALLOCATOR_H

