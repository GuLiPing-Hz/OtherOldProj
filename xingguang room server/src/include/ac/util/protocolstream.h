#ifndef AC_UTIL_PROTOCOLSTREAM_H_
#define AC_UTIL_PROTOCOLSTREAM_H_

#include <ac/util/stream.h>
#include <ac/util/data_block.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string>
#include <map>

using namespace std;

namespace ac
{

enum {
	TEXT_PACKLEN_LEN		= 4,
	TEXT_PACKAGE_MAXLEN		= 0xffff,
	BINARY_PACKLEN_LEN		= 2,
	BINARY_PACKAGE_MAXLEN	= 0xffff,

	TEXT_PACKLEN_LEN_2		= 6,
	TEXT_PACKAGE_MAXLEN_2	= 0xffffff,
};

struct ReadStreamImpl : public ReadStream
{
	virtual const char* GetData() const = 0;

	virtual size_t GetSize() const = 0;
};

struct WriteStreamImpl : public WriteStream
{
	virtual const char* GetData() const = 0;

	virtual size_t GetSize() const = 0;

	virtual bool IsValid() const = 0;
};


class TextReadStream : public ReadStreamImpl
{
private:
	const char* const ptr;
	const size_t len;
	const char* cur;

	TextReadStream(const TextReadStream&);
	TextReadStream& operator=(const TextReadStream&);

public:
	TextReadStream(const char* ptr, size_t len);

	virtual size_t GetSize() const;

	virtual bool IsEmpty() const;
	
	virtual bool Read(char* str, size_t strlen, /* out */ size_t & len);

	virtual bool Read(/* out */ int & i);

	virtual bool Read(/* out */ short & i);

	virtual bool Read(/* out */ char & c);

	virtual const char* GetData() const;

	virtual size_t ReadAll(char * szBuffer, size_t iLen) const;

	virtual bool IsEnd() const;

public:

	bool ReadLength(size_t & len);

	bool ReadContent(char* buf, size_t buflen);

	bool ReadLengthWithoutOffset(size_t & len);	
};


class TextWriteStream : public WriteStreamImpl
{
private:
	char* const ptr;
	const size_t len;
	char*  cur;

	TextWriteStream(const TextWriteStream&);
	TextWriteStream& operator=(const TextWriteStream&);

public:
	TextWriteStream(char* ptr, size_t len);

	virtual bool Write(const char* str, size_t len);

	virtual bool Write(int i);

	virtual bool Write(short i);

	virtual bool Write(char c);	

	virtual size_t GetSize() const;

	virtual bool IsValid() const;

	virtual void Flush();

	virtual void Clear();

	virtual const char* GetData() const;

	virtual char* GetCurrent() const
		{ return cur; }

public:
	bool WriteLength(size_t len);

	bool WriteContent(const char* buf, size_t buflen);
};


class BinaryReadStream : public ReadStreamImpl
{
private:
	const char* const ptr;
	const size_t len;
	const char* cur;

	BinaryReadStream(const BinaryReadStream&);
	BinaryReadStream& operator=(const BinaryReadStream&);

public:
	BinaryReadStream(const char* ptr, size_t len);

	virtual size_t GetSize() const;

	virtual bool IsEmpty() const;
 
	virtual bool Read(string *str, /* out */ size_t& outlen);

	virtual bool Read(char* str, size_t strlen, /* out */ size_t& len);

	virtual bool Read(/* out */ int & i);

	virtual bool Read(/* out */ short & i);

	virtual bool Read(/* out */ char & c);

	virtual const char* GetData() const;

	virtual size_t ReadAll(char * szBuffer, size_t iLen) const;

	virtual bool IsEnd() const;
	
public:

	bool ReadLength(size_t & len);

	bool ReadLengthWithoutOffset(size_t & outlen);	
};

class BinaryWriteStream : public WriteStreamImpl
{
private:
	char* const ptr;
	const size_t len;
	char*  cur;

	BinaryWriteStream(const BinaryWriteStream&);
	BinaryWriteStream& operator=(const BinaryWriteStream&);

public:
	BinaryWriteStream(char* ptr, size_t len);

	virtual bool Write(const char* str, size_t len);

	virtual bool Write(int i);

	virtual bool Write(short i);

	virtual bool Write(char c);	

	virtual size_t GetSize() const;

	virtual bool IsValid() const;

	virtual void Flush();

	virtual void Clear();

	virtual const char* GetData() const;

	virtual char* GetCurrent() const
		{ return cur; }

public:
	bool WriteLength(size_t len);
};

class TextReadStream2 : public ReadStreamImpl
{
private:
	const char* const ptr;
	const size_t      len;
	const char* cur;

	TextReadStream2(const TextReadStream2&);
	TextReadStream2& operator=(const TextReadStream2&);

public:
	TextReadStream2(const char* ptr, size_t len);

	virtual size_t GetSize() const;

	virtual bool IsEmpty() const;
	
	virtual bool Read(char* str, size_t strlen, /* out */ size_t & len);

	virtual bool Read(/* out */ int & i);

	virtual bool Read(/* out */ short & i);

	virtual bool Read(/* out */ char & c);

	virtual const char* GetData() const;

	virtual size_t ReadAll(char * szBuffer, size_t iLen) const;

	virtual bool IsEnd() const;

public:

	bool ReadLength(size_t & len);

	bool ReadContent(char* buf, size_t buflen);

	bool ReadLengthWithoutOffset(size_t & len);	
};


class TextWriteStream2 : public WriteStreamImpl
{
private:
	char* const ptr;
	const size_t len;
	char*  cur;

	TextWriteStream2(const TextWriteStream2&);
	TextWriteStream2& operator=(const TextWriteStream2&);

public:
	TextWriteStream2(char* ptr, size_t len);

	virtual bool Write(const char* str, size_t len);

	virtual bool Write(int i);

	virtual bool Write(short i);

	virtual bool Write(char c);	

	virtual size_t GetSize() const;

	virtual bool IsValid() const;

	virtual void Flush();

	virtual void Clear();

	virtual const char* GetData() const;

	virtual char* GetCurrent() const
		{ return cur; }

public:
	bool WriteLength(size_t len);

	bool WriteContent(const char* buf, size_t buflen);
};

class ResultSet : public map<string,string>
{
public:
	string* operator()(int i,int j);
};

int GetResult(BinaryReadStream *stream,int &col,int &field,ResultSet* resultset);//key string "0,4"代表1行5列
} // namespace ac

#endif // AC_UTIL_PROTOCOLSTREAM_H_

