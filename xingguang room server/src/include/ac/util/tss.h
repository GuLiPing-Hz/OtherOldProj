#ifndef AC_UTIL_TSS_H_
#define AC_UTIL_TSS_H_

#include <pthread.h>

typedef void(*CLEANUP_FUNC)(void* p);

namespace ac {

class TSS
{
private:
	pthread_key_t key;

public:
	~TSS(){pthread_key_delete(key);}
	TSS(CLEANUP_FUNC cleanfunc);
	void set(void* p);
	void* get();
};

} // namespace ac

#endif // AC_UTIL_TSS_H_

