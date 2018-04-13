#ifndef VFS_H
#define VFS_H

#include <type_traits>
#include <memory>
#include <iostream>
#include <string>

using istream_ptr = std::unique_ptr<std::istream>;

class Vfs {
    static Vfs sInstance;

public:
    Vfs() = default;
    Vfs(Vfs&&) = delete;
    Vfs(const Vfs&) = delete;
    Vfs& operator=(Vfs&&) = delete;
    Vfs& operator=(const Vfs&) = delete;

    static Vfs &get() noexcept { return sInstance; }

    istream_ptr openInput(const char *fname);
    istream_ptr openInput(const std::string &fname) { return openInput(fname.c_str()); }
};


template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline T read_le(std::istream &file)
{
    T ret = 0;
    char bytes[sizeof(T)];
    file.read(bytes, sizeof(T));
    if(file.gcount() == (std::streamsize)sizeof(T))
    {
        for(size_t i = sizeof(T);i > 0;)
            ret = (ret<<8) | (bytes[--i]&0xff);
    }
    return ret;
}

template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline T read_be(std::istream &file)
{
    T ret = 0;
    char bytes[sizeof(T)];
    file.read(bytes, sizeof(T));
    if(file.gcount() == sizeof(T))
    {
        for(size_t i = 0;i < sizeof(T);)
            ret = (ret<<8) | (bytes[i++]&0xff);
    }
    return ret;
}

#endif /* VFS_H */
