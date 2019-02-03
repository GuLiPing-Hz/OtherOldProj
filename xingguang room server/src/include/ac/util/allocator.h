/* -*- C++ -*- */
//=============================================================================
/**
 *  @file       allocator.h
 *  @brief      ÄÚ´æ·ÖÅäÆ÷
 *  @version    1.0
 *  @author     Sunbird
 *  @date       2007/08/13
 */
//=============================================================================

#ifndef AC_UTIL_ALLOCATOR_H_
#define AC_UTIL_ALLOCATOR_H_

#include <sys/types.h>
#include <stdlib.h>

namespace ac
{
	

class Allocator
{
public:
	virtual ~Allocator() {}
	virtual void * Allocate(size_t iSize, size_t * pRealSize=NULL) = 0;
	virtual void Deallocate(void * pData) = 0;
public:
	static Allocator * Instance();
};


class MallocAllocator : public Allocator
{
public:
	virtual void * Allocate(size_t iSize, size_t * pRealSize=NULL); 
	virtual void Deallocate(void * pData);
public:
	static MallocAllocator * Instance();
};


class NewAllocator : public Allocator
{
public:
	virtual void * Allocate(size_t iSize, size_t * pRealSize=NULL);	
	virtual void Deallocate(void * pData);
public:
	static NewAllocator * Instance();	
};


}

#endif // AC_ALLOCATOR_H_

