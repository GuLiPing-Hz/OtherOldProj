#ifndef AC_COUNTER_H_
#define AC_COUNTER_H_
#include <ext/hash_map>
#include <utility>
#include "ac/util/seqmap.h"

using namespace std;
using namespace __gnu_cxx;
namespace ac
{
	class ClientSocketBase;
	class Counter
	{
		public:
			Counter(int min = 1,int max = 100000000)
				: m_count(min),m_min(min),m_max(max){}
			inline int Get(){
				if(m_count >= m_max) m_count = m_min;
				return ++m_count;
			}
			/*static Counter* GetCounter()
			{
				static Counter counter;
				return &counter;
			}*/
		private:
			int m_count;
			int m_min;
			int m_max;
	};
	typedef SeqMap<ClientSocketBase*> ClientMap;
	/*class ClientMap
	{
		public:
			void Put(int id,ClientSocketBase *pClient)
			{
				m_clientmap.insert(make_pair(id,pClient));
			}
			ClientSocketBase* Get(int id)
			{
				hash_map<int,ClientSocketBase*>::iterator it = m_clientmap.find(id);
				if(it == m_clientmap.end())
					return NULL;
				return it->second;
			}
			void Del(int id)
			{
				m_clientmap.erase(id);
			}
			size_t Size()
			{
				return m_clientmap.size();
			}
			hash_map<int,ClientSocketBase*>* GetMap(){
				return &m_clientmap;
			}
		private:
			hash_map<int,ClientSocketBase*> m_clientmap;	
	};*/
}

#endif

