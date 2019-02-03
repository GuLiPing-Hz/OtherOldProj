#ifndef AC_UTIL_OBJECT_MANAGER_H
#define AC_UTIL_OBJECT_MANAGER_H

#include <assert.h>
#include <ac/util/objectallocator.h>
#include <ac/util/destroyable.h>


namespace ac {


template <class T>
class ObjectManager : public DestroyCallback
{
public:
	ObjectManager(ObjectAllocator<T> * pObjectAllocator) 
		: pObjectAllocator_(pObjectAllocator)
	{
		assert(pObjectAllocator_);
	}

	T * Create()
	{
		T * p = pObjectAllocator_->Create();
		if ( p != NULL ) {
			p->SetDestroyCallback(this);
		}
		return p;
	}

	virtual void call(Destroyable* pObject)
	{
		//printf("[[[[[[[[[[]]]]]]]]]]");
		return pObjectAllocator_->Destroy(static_cast<T*>(pObject));
	}

private:
	ObjectAllocator<T> * pObjectAllocator_;
};


} // namespace ac


#endif // AC_UTIL_OBJECT_MANAGER_H
