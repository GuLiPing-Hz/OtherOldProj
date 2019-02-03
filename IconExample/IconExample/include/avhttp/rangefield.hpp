//
// rangefield.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_RANGEFIELD_HPP
#define AVHTTP_RANGEFIELD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <map>
#include <vector>
#include <algorithm>    // for std::min/std::max

#include <boost/cstdint.hpp>
#include <boost/assert.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "avhttp/bitfield.hpp"



namespace avhttp {

// �����������ݷ�Χ����.
struct range
{
	inline range(boost::int64_t l, boost::int64_t r)
		: left(l)
		, right(r)
	{}
	inline range()
		: left(0)
		, right(0)
	{}

	inline boost::int64_t size()
	{
		return right - left + 1;	// ע��: ����http������ұ߽�, ���Ա�����ұ߽�����ȥ!!!
	}

	inline bool operator ==(const range& r) const
	{
		if (left == r.left && right == r.right)
			return true;
		return false;
	}

	inline bool operator!=(const range& r) const
	{
		if (left == r.left && right == r.right)
			return false;
		return true;
	}

	boost::int64_t left;
	boost::int64_t right;
};

// һ�������������ֵ�λͼʵ��(����Range tree).
class rangefield
{
	typedef std::map<boost::int64_t, boost::int64_t> range_map;

public:
	// @param size��ʾ������ܴ�С.
	inline rangefield(boost::int64_t size = 0)
		: m_need_merge(false)
		, m_size(size)
#ifndef AVHTTP_DISABLE_THREAD
		, m_mutex(boost::make_shared<boost::mutex>())
#endif
	{}

	inline ~rangefield()
	{}

	rangefield(const rangefield& rhs)
	{
		m_need_merge = rhs.m_need_merge;
		m_ranges = rhs.m_ranges;
		m_size = rhs.m_size;
#ifndef AVHTTP_DISABLE_THREAD
		m_mutex = boost::make_shared<boost::mutex>();
#endif
	}

	const rangefield& operator=(const rangefield& rhs)
	{
		m_need_merge = rhs.m_need_merge;
		m_ranges = rhs.m_ranges;
		m_size = rhs.m_size;
#ifndef AVHTTP_DISABLE_THREAD
		m_mutex = boost::make_shared<boost::mutex>();
#endif
		return *this;
	}

public:

	///����rangefield.
	// @param size��ʾ������ܴ�С.
	void reset(boost::int64_t size = 0)
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif
		m_size = size;
		m_need_merge = false;
		m_ranges.clear();
	}

	///�������Ĵ�С.
	// @�������������ܴ�С.
	boost::int64_t size() const
	{
		return m_size;
	}

	///��ӻ����range, ����Ϊ[r.left, r.right)
	// @param r����, �������ұ߽紦.
	// @��ע: ���һ�����䵽range��, �����ص����, �ɹ�����true.
	inline bool update(const range& r)
	{
		return update(r.left, r.right);
	}

	///��ӻ����range, ����Ϊ[left, right)
	// @param left��߽߱�.
	// @param right�ұ߽߱�, �������߽紦.
	// @��ע: ���һ�����䵽range��, �����ص����, �ɹ�����true.
	inline bool update(const boost::int64_t& left, const boost::int64_t& right)
	{
		BOOST_ASSERT((left >= 0 && left < right) && right <= m_size);

#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif

		if ((left < 0 || right > m_size) || (right <= left))
			return false;
		m_ranges[left] = right;
		m_need_merge = true;
		return true;
	}

	///����Ƿ���������.
	// @param r����, �������ұ߽紦.
	// @�������range��������Ƿ������ı�������range��.
	// @��ע: ����������һ���뿪����[left, right), ���������ұ߽�.
	inline bool check_range(const range& r)
	{
		return check_range(r.left, r.right);
	}

	///����Ƿ���������.
	// @param left��߽߱�.
	// @param right�ұ߽߱�, �������߽紦.
	// @�������[left, right)��������Ƿ������ı�������range��.
	// @��ע: ����������һ���뿪����[left, right), ���������ұ߽�.
	inline bool check_range(const boost::int64_t& left, const boost::int64_t& right)
	{
		BOOST_ASSERT((left >= 0 && left < right) && right <= m_size);

		// ������.
		if (m_need_merge)
			merge();

#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif

		for (range_map::iterator i = m_ranges.begin();
			i != m_ranges.end(); i++)
		{
			if (left >= i->first && right <= i->second)
				return true;
		}

		return false;
	}

	///��ȡ��[left, right)���������.
	// @param left��߽߱�, Ҳ���������.
	// @param right�ұ߽߱�, �������߽紦, ͬΪ�������.
	// @����false��ʾ������Ŀռ��ǿյ�.
	// @��ע: �����������һ���뿪����[left, right), ���������ұ߽�.
	inline bool get_range(boost::int64_t& left, boost::int64_t& right)
	{
		BOOST_ASSERT((left >= 0 && left < right) && right <= m_size);

		// ������.
		if (m_need_merge)
			merge();

#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif

		for (range_map::iterator i = m_ranges.begin();
			i != m_ranges.end(); i++)
		{
			if (left >= i->first)
			{
				if (right <= i->second)
					return true;
				if (right > i->first && i->second > left)
				{
					right = i->second;
					return true;
				}
			}
		}

		return false;
	}

	///�����϶�ռ�.
	// @param r����, �������ұ߽紦.
	// @����false��ʾû�пռ��ʧ��.
	// @��ע: �����������һ���뿪����[left, right), ���������ұ߽�.
	inline bool out_space(range& r)
	{
		return out_space(r.left, r.right);
	}

	///�����϶�ռ�.
	// @param left ��ʾ����ʼ��ʼ�߽�.
	// @param right ��ʾ�ұߵı߽�, �������߽紦.
	// @����false��ʾû�пռ��ʧ��.
	// @��ע: �����������һ���뿪����[left, right), ���������ұ߽�.
	inline bool out_space(boost::int64_t& left, boost::int64_t& right)
	{
		// ������.
		if (m_need_merge)
			merge();
		return out_space(0, left, right);
	}

	///��ָ��λ�ÿ�ʼ, �����϶�ռ�.
	// @param offset ��ʾ����ռ���offset���濪ʼ.
	// @param left ��ʾ����ʼ��ʼ�߽�.
	// @param right ��ʾ�ұߵı߽�, �������߽紦.
	// @����false��ʾû�пռ��ʧ��.
	// @��ע: �����������һ���뿪����[left, right), ���������ұ߽�.
	inline bool out_space(boost::int64_t offset, boost::int64_t& left, boost::int64_t& right)
	{
		// ������.
		if (m_need_merge)
			merge();

#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif

		// inv���ÿһ��Ԫ�ض��ǿռ�, ��Ԫ���з�������.
		range_map inv = inverse_impl();

		// û�пռ�.
		if (inv.size() == 0)
			return false;
		range_map::iterator i;
		for (i = inv.begin(); i != inv.end(); i++)
		{
			// ƫ��λ���ڿռ���, �ͷ�����οռ�.
			if (offset >= i->first)
			{
				if (offset < i->second)
				{
					left = offset;
					right = i->second;
					return true;
				}
			}
			else // offset����������, ֱ�ӷ��ص�һ���ռ�.
			{
				left = i->first;
				right = i->second;
				return true;
			}
		}

		// �ߵ���, ��û�ҵ���Ч�Ŀռ�, ��ֱ�ӷ��ص�һ��ռ�.
		i = inv.begin();
		left = i->first;
		right = i->second;

		return true;
	}

	///���λͼ�Ƿ��Ѿ�����.
	inline bool is_full()
	{
		// ������.
		if (m_need_merge)
			merge();

#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif

		// ֱ���ж�����߽�.
		if (m_ranges.size() == 1)
		{
			range_map::iterator i = m_ranges.begin();
			if (i->first == 0 && i->second == m_size)
				return true;
		}
		return false;
	}

	///�õ��������д�С.
	inline boost::int64_t range_size() const
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif
		boost::int64_t size = 0;
		for (range_map::const_iterator i = m_ranges.begin();
			i != m_ranges.end(); i++)
		{
			size += i->second - i->first;
		}

		return size;
	}

	///��ָ����С���Ϊλͼ��.
	// @param bitfield��intΪ��λ��λͼ����, ÿ��Ԫ�ر�ʾ1��piece, Ϊ0��ʾ��, Ϊ1��ʾ��.
	// @param piece_sizeָ����piece��С.
	inline void range_to_bitfield(bitfield& bf, int piece_size)
	{
		// �����������Ŀռ�, ������������ϲ�Ϊһ������.
		if (m_need_merge)
			merge();

#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif

		// �������Ƭ����.
		int piece_num = (m_size / piece_size) + (m_size % piece_size == 0 ? 0 : 1);

		// ����λͼ�ռ�, Ĭ��Ϊ0.
		bf.resize(piece_num, 0);

		boost::int64_t l = 0;
		boost::int64_t r = 0;
		range_map::iterator it = m_ranges.begin();
		range_map::iterator end = m_ranges.end();

		// ��0λ�ÿ�ʼ, ÿ�μ��piece_size��С�Ŀռ��Ƿ���������, ����
		// ���������õ�λͼΪ1, ��ʾ������.
		for (int i = 0; i < piece_num; i++)
		{
			l = i * piece_size;
			r = (i + 1) * piece_size;
			r = r > m_size ? m_size : r;

			bool is_complete = false;
			if (it != end)
			{
				// ���Ѿ����ص�������.
				if (l >= it->first && r <= it->second)
				{
					is_complete = true;
				}
				else
				{
					// �����ұ߽���ڵ��ڵ�ǰ������ұ߽�, ����һֱʹ�õ�ǰ�����ж�.
					// �����ͱ�����ÿ�δ�ͷѭ������.
					if (r >= it->second)
					{
						if (++it != end)
						{
							if (l >= it->first && r <= it->second)
							{
								is_complete = true;
							}
						}
					}
				}
			}

			if (is_complete)
			{
				bf.set_bit(i);
			}
		}
	}

	///��ָ���ķ�Ƭ��Сbitfield����rangefield.
	// @param bfΪָ����bitfield.
	// @param piece_size��ָ���ķ�Ƭ��С.
	inline void bitfield_to_range(const bitfield& bf, int piece_size)
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif
		m_ranges.clear();

		boost::int64_t left = 0;
		boost::int64_t right = 0;
		bool left_record = false;
		int index = 0;

		for (bitfield::const_iterator i = bf.begin(); i != bf.end(); i++, index++)
		{
			BOOST_ASSERT(index * piece_size < m_size);

			if (*i && !left_record)
			{
				left = index * piece_size;
				right = left + piece_size;

				left_record = true;
				continue;
			}

			if (*i && left_record)
			{
				right += piece_size;
			}

			if (left_record && !(*i))
			{
				// �õ�����.
				right = (std::min)(right, m_size);
				m_ranges[left] = right;
				left_record = false;
			}
		}

		if (left_record)
		{
			right = (std::min)(right, m_size);
			m_ranges[left] = right;
			left_record = false;
		}

		m_need_merge = true;
	}

	///���range����, ����ʹ��.
	inline void print()
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif
		for (range_map::iterator i = m_ranges.begin();
			i != m_ranges.end(); i++)
		{
			std::cout << i->first << "   ---    " << i->second << "\n";
		}
	}

	///��תrangefield.
	inline rangefield inverse()
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif

		range_map m = inverse_impl();
		rangefield f(m_size);

		f.m_need_merge = m_need_merge;
		f.m_ranges = m;

		return f;
	}

protected:

	///���ط�ת��range_item.
	inline range_map inverse_impl()
	{
		// ������.
		if (m_need_merge)
			merge();

		range_map reverse_map;
		boost::int64_t point = 0;

		// ����϶��ӵ�reverse_map��.
		for (range_map::iterator i = m_ranges.begin();
			i != m_ranges.end(); i++)
		{
			if (i == m_ranges.begin())
			{
				if (i->first != 0)
				{
					point = 0;
					reverse_map.insert(std::make_pair(point, i->first));
					point = i->second;
					continue;
				}
				else
				{
					point = i->second;
					continue;
				}
			}

			reverse_map.insert(std::make_pair(point, i->first));
			point = i->second;
		}

		// β���ж�.
		if (point != m_size || m_ranges.size() == 0)
		{
			reverse_map.insert(std::make_pair(point, m_size));
		}

		return reverse_map;
	}

	///�������range�е��ص�����.
	inline void merge()
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(*m_mutex);
#endif
		std::map<boost::int64_t, boost::int64_t> result;
		std::pair<boost::int64_t, boost::int64_t> max_value;
		max_value.first = 0;
		max_value.second = 0;
		for (range_map::iterator i = m_ranges.begin();
			i != m_ranges.end(); i++)
		{
			// i ��max_value ֮��, ����֮.
			// [        ]
			//   [    ]
			// --------------
			// [         ]
			if (i->first >= max_value.first && i->second <= max_value.second)
				continue;
			// �������:
			//      [   ]
			// [       ]
			// --------------
			// [        ]
			if (i->first <= max_value.first && (i->second >= max_value.first && i->second <= max_value.second))
			{
				max_value.first = i->first;	// �����ײ�λ��.
				continue;
			}
			// �������:
			// [   ]
			//     [     ]
			// --------------
			// [         ]
			if ((i->first > max_value.first && i->first <= max_value.second))
			{
				max_value.second = i->second;
				continue;
			}
			// �������:
			// [   ]
			//       [     ]
			// --------------
			// [   ] [     ]
			if ((i->first > max_value.second) || (max_value.first == 0 && max_value.second == 0))
			{
				if (max_value.first != 0 || max_value.second != 0)
					result.insert(max_value);
				max_value = *i;
			}
		}
		result.insert(max_value);
		m_ranges = result;
		m_need_merge = false;
	}

private:
	bool m_need_merge;
	boost::int64_t m_size;
	range_map m_ranges;
#ifndef AVHTTP_DISABLE_THREAD
	typedef boost::shared_ptr<boost::mutex> mutex_ptr;
	mutex_ptr m_mutex;
#endif
};

} // namespace avhttp

#endif // AVHTTP_RANGEFIELD_HPP
