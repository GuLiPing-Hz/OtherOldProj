#ifndef AC_UTIL_COMPOSITESTREAM_H_
#define AC_UTIL_COMPOSITESTREAM_H_

#include <ac/util/protocolstream.h>

namespace ac {

template<class Stream>
class CompositeWriteStream : public WriteStreamImpl {
	DataBlock datablock;
	Stream stream;

public:
	CompositeWriteStream(char* buffer, size_t len)
	: stream(buffer, len)
	{
	}

    virtual bool Write(const char* str, size_t len)
	{
		return stream.Write(str, len);
	}

    virtual bool Write(int i)
	{
		return stream.Write(i);
	}

    virtual bool Write(short i)
	{
		return stream.Write(i);
	}

    virtual bool Write(char c)
	{
		return stream.Write(c);
	}

    virtual void Clear()
	{
		return stream.Clear();
	}

	void Flush()
	{
		if (stream.IsValid())
		{
			stream.Flush();
			datablock.Append(stream.GetData(), stream.GetSize());
		}
		stream.Clear();
	}

	virtual const char* GetData() const
	{
		return datablock.GetBegin();
	}

	virtual size_t GetSize() const
	{
		return datablock.GetSize();
	}

	virtual bool IsValid() const
	{
		return datablock.GetSize() > 0 || stream.IsValid();
	}

	virtual char* GetCurrent() const
		{ return stream.GetCurrent(); }
};

typedef CompositeWriteStream<TextWriteStream> CompositeTextWriteStream;
typedef CompositeWriteStream<BinaryWriteStream> CompositeBinaryWriteStream;

} // namespace ac

#endif // AC_UTIL_COMPOSITESTREAM_H_
