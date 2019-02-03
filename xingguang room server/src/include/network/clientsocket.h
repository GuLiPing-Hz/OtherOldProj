#ifndef AC_CLIENTSOCKET_H_
#define AC_CLIENTSOCKET_H_

#include "network/eventhandler.h"
#include "network/counter.h"
#include "network/package.h"
#include "ac/util/objectmanager.h"
#include "ac/util/objectpool.h"

#define BUFSIZE_RECV  8*1024

namespace ac
{
	//class FDEventHandler;
	class DataBlock;
	class DataDecoderBase;

	class ClientSocketBase : public FDEventHandler/*,public Destroyable*/
	{
		public:
			virtual ~ClientSocketBase(){}
			ClientSocketBase() : m_timeout(3){}
			ClientSocketBase(Reactor *pReactor) : FDEventHandler(pReactor){m_timeout = 3;}
			//void SetID(int id){m_clientid = id;}
			//int GetID(){return m_clientid;}
			void SetTimeout(int timeout){m_timeout = timeout;}
			int GetTimeout() const {return m_timeout;}
			void SetDecoder(DataDecoderBase* pDecoder){m_pDecoder = pDecoder;}
			DataDecoderBase* GetDecoder(){return m_pDecoder;}
			virtual void OnFDRead();
			virtual void OnFDWrite();
			virtual void OnFDReadTimeOut();
			virtual void OnFDWriteTimeOut();
			virtual void Close()
			{
				m_recvdata.InitPos();
				m_senddata.InitPos();
				FDEventHandler::Close();
			}
			DataBlock* GetRB(){return &m_recvdata;}
			DataBlock* GetWB(){return &m_senddata;}
			virtual int AddBuf(const char* buf,size_t buflen);
			void GetPeerIp(char *ip);
			//add by jinguanfu 2011/4/11
			virtual void SetErrorMsg(char * msg)
			{
				strcpy(m_ErrMsg,msg);

			};
		private:
			DataBlock m_recvdata;
			DataBlock m_senddata;
			//int m_clientid;
			int m_timeout;
			DataDecoderBase *m_pDecoder;
		public:
			char m_ErrMsg[200];
	};

	class ClientSocket : public ClientSocketBase , public Destroyable
	{
		public:
			virtual ~ClientSocket(){}
			ClientSocket(){}
			ClientSocket(Reactor *pReactor) : ClientSocketBase(pReactor){}
			void SetClientMap(ClientMap *pClientMap){m_pClientMap = pClientMap;}
			virtual void Close()
			{
				m_pClientMap->Del(GetClientID());
				ClientSocketBase::Close();
				Destroy();
			};

		private:
			ClientMap *m_pClientMap;
	};

	class BackClientSocketBase : public ClientSocketBase , public TMEventHandler
	{
		public:
			BackClientSocketBase(Reactor *pReactor,const char* ip,int port) : ClientSocketBase(pReactor),m_port(port)
			{
				strncpy(m_ip,ip,sizeof(m_ip));
				m_isconnect = false;
				m_isreconnect = true;
			}
			virtual ~BackClientSocketBase(){}
			int Connect();
			virtual void OnFDWrite();
			virtual void Close();
			void RealClose();
			virtual void OnTimeOut();
			inline void SetReconnect(bool isreconnect = false){
				m_isreconnect = isreconnect;}
			inline char* Getip(){
				return m_ip;
			}
			inline int Getport() const{
				return m_port;
			}
			inline bool IsConnect(){
				return m_isconnect;
			}
			virtual int AddBuf(const char* buf,size_t buflen)
			{
				if(m_isconnect == false)
					return -1;
				return ClientSocketBase::AddBuf(buf,buflen);
			}
			virtual void OnConnect(){}
		private:
			char m_ip[20];
			int m_port;
			bool m_isconnect;
			bool m_isreconnect;
	};
}

#endif
