#ifndef AC_NONCOPYABLE_H_
#define AC_NONCOPYABLE_H_

namespace ac
{

/*! \brief ����Ҫ�󲻿ɸ�ֵ(��������)���඼���Լ̳��ڴ�
 *  \note ժ��IceUtil::::noncopyable
 */
class NonCopyable
{

protected:
    NonCopyable() {}
    ~NonCopyable() {}

private:
    NonCopyable(const NonCopyable &);
    const NonCopyable & operator=(const NonCopyable &);
};

}

#endif // AC_NONCOPYABLE_H_
