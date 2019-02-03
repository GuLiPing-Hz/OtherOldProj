#ifndef AC_UTIL_HANDLE_H_
#define AC_UTIL_HANDLE_H_

namespace ac {

struct Handle
{
	int hdl;

	Handle(int hdl_ = -1)
	: hdl(hdl_)
	{
	}

	operator int() const
	{
		return hdl;
	}
};

} // namespace ac

#endif // AC_NETWORK_HANDLE_H_

