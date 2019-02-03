#ifndef SEQMAP_H_
#define SEQMAP_H_

#include <ext/hash_map>
#include <utility>

using namespace std;
using namespace __gnu_cxx;

namespace ac
{
template <class T>
class SeqMap
{
	public:
		void Put(int id,T t)
		{
			m_clientmap.insert(make_pair(id,t));
		}
		T* Get(int id)
		{
			typename hash_map<int,T>::iterator it;
			it = m_clientmap.find(id);
			if(it == m_clientmap.end())
				return NULL;
			return &(it->second);
		}
		inline void Del(int id)
		{
			m_clientmap.erase(id);
		}
		inline size_t Size()
		{
			return m_clientmap.size();
		}
		inline void Clear()
		{
			return m_clientmap.clear();
		}
		hash_map<int,T>* GetMap(){
			return &m_clientmap;
		}
	private:
		hash_map<int,T> m_clientmap;
};
}
#endif
