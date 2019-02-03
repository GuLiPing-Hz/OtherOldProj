#ifndef AC_PACKAGE_H_
#define AC_PACKAGE_H_

#include <ac/util/data_block.h>
#include <ac/util/non_copyable.h>
#include <ac/util/destroyable.h>

namespace ac
{
	class PackageBase2 : public NonCopyable
	{
		public: 
			void SetClientID(int id){m_clientid = id;}
			int GetClientID() const {return m_clientid;}
		private:
			int m_clientid;
	};
	class PackageBase : public PackageBase2
	{
		public:
			int AddBuf(const char* buf,size_t buflen)
			{
				m_db.InitPos();
				return m_db.Append(buf,buflen);
			}
			size_t GetBuflen() const {return m_db.GetPos();}
			char *GetBuf() const {return m_db.GetBuf();}
		private:
			DataBlock m_db;
	};
	class Package : public PackageBase , public Destroyable
	{
		public:
			timeval m_pushqueuetime;
			timeval m_popqueuetime;
	};
	class ResultPackage : public Package
	{
		public:
			ResultPackage() : m_result(0){}
			int GetResult() const {return m_result;}
			void SetResult(int result) {m_result = result;}
		private:
			int m_result;
	};
}

#endif
