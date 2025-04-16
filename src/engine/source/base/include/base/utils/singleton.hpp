#ifndef _SINGLETON_HPP
#define _SINGLETON_HPP

template <typename T>
class Singleton
{
public:
    static T& instance()
    {
        static T s_instance;
        return s_instance;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};

#endif // _SINGLETON_HPP
