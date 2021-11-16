// $Id$

#ifndef BINARY_HPP
#define BINARY_HPP

#include <riegl/config.hpp>

#include <cstddef>
#include <limits>
#include <iterator>
#include <cassert>
#include <string>
#include <stdexcept>
#include <istream>
#include <ostream>

#ifdef __GNUC__
#   ifdef NDEBUG
#       define inline inline __attribute((always_inline))
#   endif
#endif

namespace binary {
using scanlib::uintmax_t;
using scanlib::uint8_t;
using scanlib::int8_t;

//*****************************************************************************
// Low level - little end - binary read
//*****************************************************************************
#define READ_LITTLE_END                                                       \
enum { B_digits = std::numeric_limits<                                        \
        typename std::iterator_traits<It>::value_type >::digits };            \
enum { T_digits = static_cast<unsigned>(std::numeric_limits<T>::is_signed)    \
                + std::numeric_limits<T>::digits };                           \
uintmax_t u = 0;                                                          \
unsigned n,s;                                                                 \
signed char sh;                                                               \
                                                                              \
for (n=offset/B_digits,s=0; n<1+(offset+digits-1)/B_digits; ++n,++s)          \
{                                                                             \
    sh = T_digits-digits-offset%B_digits+s*B_digits;                          \
    if (sh>0)                                                                 \
        u |= uintmax_t(data[n])<<sh;                                          \
    else                                                                      \
        u |= uintmax_t(data[n])>>(-sh);                                       \
}                                                                             \
                                                                              \
return (static_cast<T>(u))>>(T_digits-digits);

//-----------------------------------------------------------------------------
// compile time offset version
template
<
    class      T
    , unsigned digits
    , unsigned offset
    , class    It
>
inline T
read_little_end
(
    It data
)
{
    /*READ_LITTLE_END*/
    enum { B_digits = std::numeric_limits<
            typename std::iterator_traits<It>::value_type >::digits };
    enum { T_digits = static_cast<unsigned>(std::numeric_limits<T>::is_signed)
                    + std::numeric_limits<T>::digits };
    uintmax_t u = 0;
    unsigned n,s;
    signed char sh;

    for (n=offset/B_digits,s=0; n<1+(offset+digits-1)/B_digits; ++n,++s)
    {
        sh = T_digits-digits-offset%B_digits+s*B_digits;
        if (sh>0)
            u |= uintmax_t(data[n])<<sh;
        else
            u |= uintmax_t(data[n])>>(-sh);
    }

    return (static_cast<T>(u))>>(T_digits-digits);
}
//-----------------------------------------------------------------------------
// runtime offset version
template
<
    class      T
    , unsigned digits
    , class    It
>
T
read_little_end
(
    It         data
    , unsigned offset
)
{
    /*READ_LITTLE_END*/
    enum { B_digits = std::numeric_limits<
            typename std::iterator_traits<It>::value_type >::digits };
    enum { T_digits = static_cast<unsigned>(std::numeric_limits<T>::is_signed)
                    + std::numeric_limits<T>::digits };
    uintmax_t u = 0;
    unsigned n,s;
    signed char sh;// = T_digits-digits-offset%B_digits;

    for (n=offset/B_digits,s=0; n<1+(offset+digits-1)/B_digits; ++n,++s)
    //for (n=offset/B_digits; n<1+(offset+digits-1)/B_digits; ++n)
    {
        sh = T_digits-digits-offset%B_digits+s*B_digits;
        if (sh>0)
            u |= uintmax_t(data[n])<<sh;
        else
            u |= uintmax_t(data[n])>>(-sh);
        //sh += B_digits;
    }

    return (static_cast<T>(u))>>(T_digits-digits);
}
#undef READ_LITTLE_END

//*****************************************************************************
// Low level - little end - binary write
//*****************************************************************************
#define WRITE_LITTLE_END                                                      \
typedef typename std::iterator_traits<It>::value_type BYTE;                   \
enum { B_digits = std::numeric_limits<BYTE>::digits };                        \
const uintmax_t B_mask = (((uintmax_t(1)<<(B_digits-1))-1)<<1)|1 ;            \
const uintmax_t T_mask = (((uintmax_t(1)<<(digits  -1))-1)<<1)|1 ;            \
unsigned n = offset/B_digits;                                                 \
unsigned s;                                                                   \
                                                                              \
BYTE b = static_cast<BYTE>(B_mask&((t&T_mask)<<(offset%B_digits)));           \
                                                                              \
if (offset%B_digits || digits<B_digits) {                                     \
    b |= data[n] & ~((T_mask)<<(offset%B_digits));                            \
}                                                                             \
                                                                              \
data[n] = b;                                                                  \
                                                                              \
if ((offset+digits-1)/B_digits > offset/B_digits) {                           \
                                                                              \
    for ( n=1+offset/B_digits,s=1; n<(offset+digits-1)/B_digits; ++n,++s)     \
        data[n] = static_cast<BYTE>(t >> (s*B_digits - offset%B_digits));     \
                                                                              \
    b = static_cast<BYTE>(t >> (s*B_digits - offset%B_digits));               \
    if ((offset+digits)%B_digits)                                             \
        b = (      b &  ((1U<<(offset+digits)%B_digits)-1) )                  \
          | (data[n] & ~((1U<<(offset+digits)%B_digits)-1) )                  \
          ;                                                                   \
                                                                              \
    data[n] = b;                                                              \
}
//-----------------------------------------------------------------------------
// compile time offset version
template
<
    class      T
    , unsigned digits
    , unsigned offset
    , class    It
>
inline void
write_little_end
(
    const T& t
    , It data
)
{
    WRITE_LITTLE_END
}
//-----------------------------------------------------------------------------
// runtime offset version
template
<
    class T
    , unsigned digits
    , class It
>
inline void
write_little_end
(
    const T& t
    , It data
    , unsigned offset
)
{
    WRITE_LITTLE_END
}
#undef WRITE_LITTLE_END

//*****************************************************************************
// The base type
//*****************************************************************************
struct little {};
struct big {};

template<class S, unsigned digits_, class E = little>
struct base_type
{
    typedef S sign_type;
    typedef E endianess;
    enum { digits = digits_ };
};

//*****************************************************************************
// The field type
//*****************************************************************************
template
<
    class      T        // host type to represent the value
    , class    BASE     // size and other props. (see base_type)
    , unsigned offset   // bit offset within the enclosing sequence
    , class    It       // iterator type of sequence
        = unsigned char*
    , class    ST
        = typename BASE::sign_type
    , class    E
        = typename BASE::endianess
    , class    Cat
        = typename std::iterator_traits<It>::iterator_category
>
struct field; // the general case is left undefined

//*****************************************************************************
// Unsigned little end specialization
//*****************************************************************************
template
<
    class      T
    , class    BASE
    , unsigned offset
    , class    It
>
struct field
<
    T
    , BASE
    , offset
    , It
    , unsigned
    , little
    , std::random_access_iterator_tag
>
{
    enum { BYTE_digits = std::numeric_limits<
        typename std::iterator_traits<It>::value_type >::digits };

    field(It begin, unsigned begin_bit = 0)
        : data(begin), data_bit(begin_bit)
    {}

    inline operator T() const
    {
        if (data_bit)
            return read_little_end<T, BASE::digits>
                (data, data_bit+offset);
        else
            return read_little_end<T, BASE::digits, offset>
                (data);
    }

    void operator=(const T& t)
    {
        if (data_bit)
            write_little_end<T, BASE::digits>
                (t, data, data_bit+offset);
        else
            write_little_end<T, BASE::digits, offset>
                (t, data);
    }

    inline It begin(void) const
    {
        return data;
    }

    inline It end(void) const
    {
        return data+1+(data_bit+offset+BASE::digits-1)/BYTE_digits;
    }

     ~field() {}

protected:
    field() {}

private:
    //field(const field& );
    const field& operator=(const field& );

    // start byte, start bit within byte
    It const data;
    const unsigned data_bit;
};

//*****************************************************************************
// Signed little end specialization
//*****************************************************************************
template
<
    class      T
    , class    BASE
    , unsigned offset
    , class    It
>
struct field
<
    T
    , BASE
    , offset
    , It
    , signed
    , little
    , std::random_access_iterator_tag
>
{
    enum { BYTE_digits = std::numeric_limits<
        typename std::iterator_traits<It>::value_type >::digits };

    field(It begin, unsigned begin_bit = 0)
        : data(begin), data_bit(begin_bit)
    {}

    inline operator T() const
    {
        if (data_bit)
            return read_little_end<T, BASE::digits>
                (data, data_bit+offset);
        else
            return read_little_end<T, BASE::digits, offset>
                (data);
    }

    void operator=(const T& t)
    {
        if (data_bit)
            write_little_end<T, BASE::digits>
                (t, data, data_bit+offset);
        else
            write_little_end<T, BASE::digits, offset>
                (t, data);
    }

    inline It begin(void) const
    {
        return data;
    }

    inline It end(void) const
    {
        return data+1+(data_bit+offset+BASE::digits-1)/BYTE_digits;
    }

     ~field() {}

protected:
    field() {}

private:
    //field(const field& );
    const field& operator=(const field& );

    // start byte, start bit within byte
    It const data;
    const unsigned data_bit;
};

//*****************************************************************************
// Float little end specialization
//*****************************************************************************
template
<
    class      T
    , class    BASE
    , unsigned offset
    , class    It
>
struct field
<
    T
    , BASE
    , offset
    , It
    , float
    , little
    , std::random_access_iterator_tag
>
{
    enum { BYTE_digits = std::numeric_limits<
        typename std::iterator_traits<It>::value_type >::digits };

    field(It begin, unsigned begin_bit = 0)
        : data(begin), data_bit(begin_bit)
    {}

    inline operator T() const
    {
        if (std::numeric_limits<T>::is_iec559
            && sizeof(T)*std::numeric_limits<unsigned char>::digits
                == BASE::digits
            && sizeof(uintmax_t) >= sizeof(T) ) {
                // todo: a union works for the currently used compilers,
                // but is not a true portable way
                if (sizeof(T) == sizeof(double)) {
                    union {
                        uintmax_t ul;
                        T t;
                    } u;
                    if (data_bit)
                        u.ul = read_little_end<uintmax_t, BASE::digits>
                            (data, data_bit+offset);
                    else
                        u.ul = read_little_end<uintmax_t, BASE::digits
                            , offset>(data);
                    return u.t;
                }
                else if (sizeof(T) == sizeof(float)) {
                    union {
                        unsigned long ul;
                        T t;
                    } u;
                    uintmax_t temp;
                    if (data_bit)
                        temp = read_little_end<uintmax_t, BASE::digits>
                            (data, data_bit+offset);
                    else
                        temp = read_little_end<uintmax_t, BASE::digits
                            , offset>(data);
                    u.ul = static_cast<unsigned long>(temp);
                    return u.t;
                }
        }
        else
            return T();
    }

    void operator=(const T& t)
    {
        if (std::numeric_limits<T>::is_iec559
            && sizeof(T)*std::numeric_limits<unsigned char>::digits
                == BASE::digits
            && sizeof(uintmax_t) >= sizeof(T) ) {
                if (sizeof(T) == sizeof(double)) {
                    union {
                        uintmax_t ul;
                        T t;
                    } u;
                    u.t = t;
                    if (data_bit)
                        write_little_end<uintmax_t, BASE::digits>
                            (u.ul, data, data_bit+offset);
                    else
                        write_little_end<uintmax_t, BASE::digits, offset>
                            (u.ul, data);
                }
                else if (sizeof(T) == sizeof(float)) {
                    union {
                        unsigned long ul;
                        T t;
                    } u;
                    u.t = t;
                    if (data_bit)
                        write_little_end<unsigned long, BASE::digits>
                            (u.ul, data, data_bit+offset);
                    else
                        write_little_end<unsigned long, BASE::digits, offset>
                            (u.ul, data);
                }
        }
    }

    inline It begin(void) const
    {
        return data;
    }

    inline It end(void) const
    {
        return data+1+(data_bit+offset+BASE::digits-1)/BYTE_digits;
    }

     ~field() {}

protected:
    field() {}

private:
    //field(const field& );
    const field& operator=(const field& );

    // start byte, start bit within byte
    It const data;
    const unsigned data_bit;
};

template<class T> struct stream_type { typedef T type; };
template<> struct stream_type<uint8_t> { typedef unsigned type; };
template<> struct stream_type<int8_t> { typedef int type; };

template<class charT, class traits, class T, class BASE, unsigned offset, class It>
std::basic_istream<charT, traits>&
operator>>(std::basic_istream<charT, traits>& in, field<T,BASE,offset,It> arg)
{ typename stream_type<T>::type t; in >> t; arg = t; return in; }

template<class charT, class traits, class BASE, unsigned offset, class It, class T>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& out, field<T,BASE,offset,It> arg)
{ typename stream_type<T>::type t =arg ; out << t; return out; }

//*****************************************************************************
// The array type
//*****************************************************************************
template
<
    class T
    , unsigned size_
    , class BASE
    , unsigned offset
    , class It
        = unsigned char*
>
struct array
{
    enum { BYTE_digits = std::numeric_limits<
        typename std::iterator_traits<It>::value_type >::digits };

    typedef field<T, BASE, offset, It> field_type;

    array(It begin)
        : data(begin)
    {}

    ~array()
    {}

    inline field_type operator[](std::size_t index) const
    {
        assert(index<size_);
        std::size_t index_offset = (index*BASE::digits)/BYTE_digits;
        unsigned data_bit = (index*BASE::digits)%BYTE_digits;
        return field_type(data+index_offset, data_bit);
    }

    inline std::size_t size() const
    { return size_; }

    inline It begin(void) const
    {
        return data;
    }

    inline It end(void) const
    { return (operator[](size_-1)).end(); }

protected:
    array() {}
    It const data;
};
//*****************************************************************************
// The array type, specialized for unsigned char
//*****************************************************************************
template
<
    unsigned size_
    , class BASE
    , unsigned offset
    , class It
>
struct array
<
    char
    , size_
    , BASE
    , offset
    , It
>
{
    enum { BYTE_digits = std::numeric_limits<
        typename std::iterator_traits<It>::value_type >::digits };

    typedef field<char, BASE, offset, It> field_type;

    array(It begin)
        : data(begin)
    {}

    ~array()
    {}

    inline field_type operator[](std::size_t index) const
    {
        assert(index<size_);
        std::size_t index_offset = (index*BASE::digits)/BYTE_digits;
        unsigned data_bit = static_cast<unsigned>((index*BASE::digits)%BYTE_digits);
        return field_type(data+index_offset, data_bit);
    }

    void operator=(const char* arg)
    {
        std::size_t n;
        for (n=0; *arg && n<size_; ++n,++arg) (*this)[n]
            = (const unsigned char)*arg;
        for ( ; n<size_; ++n) (*this)[n] = 0;
    }

    // copy to std::string
    operator std::string () const
    {
        std::string s;
        char c;
        for (std::size_t n=0; n<size_; ++n) {
            c = (*this)[n];
            if ('\0' == c) break;
            s += c;
        }
        return s;
    }

    inline std::size_t size() const
    { return size_; }

    inline It begin(void) const
    {
        return data;
    }

    inline It end(void) const
    { return (operator[](size_-1)).end(); }

protected:
    array() {}
    It const data;
};

//*****************************************************************************
// The sequence type
//*****************************************************************************
template
<
    class C
    , unsigned nbits
    , unsigned offset
    , class It
>
struct sequence
{
    // the use of an explicit name "sequence_definition"
    // prevents specification of more than one sequence
    // per struct
    typedef typename C::sequence_definition seq_type;
    enum { BYTE_digits = std::numeric_limits<
        typename std::iterator_traits<It>::value_type >::digits };

    sequence(It begin, It end, std::size_t& size, bool dirty = false)
        : data(begin)
        , max_size_(((end-begin)*BYTE_digits-offset)/nbits)
        , size_(size)
        , max_access(0)
        , dirty_(dirty)
    { if (!((end-begin)*BYTE_digits >= signed(offset)))
        throw(std::range_error("Missing data for sequence."));
      size_ = max_size_;
      if (dirty_) max_access = max_size_-1;
    }

    inline seq_type operator[](std::size_t index) const
    {
#       ifndef NDEBUG
        if (!(index<size()))
            throw(std::range_error("Sequence index is out of bounds."));
#       endif
        dirty_ = true;
        if (index>max_access) max_access = index;
        std::size_t index_offset = (offset+index*nbits)/BYTE_digits;
        unsigned begin_bit = unsigned((offset+index*nbits)%BYTE_digits);
        return seq_type(data+index_offset, begin_bit);
    }

    inline std::size_t max_size() const
    {
        return max_size_;
    }

    inline std::size_t size() const
    {
#       ifndef NDEBUG
        if (size_>max_size_)
            throw(std::range_error("Sequence size is larger than maximum."));
#       endif
        return size_;
    }

    // this will rearm the automatic fill sensing
    inline void clean()
    {
        max_access = 0;
        dirty_ = false;
    }

    inline It begin(void) const
    {
        return data;
    }

    inline It end(std::size_t index) const
    {
        return data+1+((offset+(index+1)*nbits-1)/BYTE_digits);
    }

    inline It end(void) const
    {
        if (dirty_)
            return end(max_access);
        else
            return data+1+((offset-1)/BYTE_digits);
    }

protected:
    It const data;
    const std::size_t max_size_;
    std::size_t& size_;
    mutable std::size_t max_access;
    mutable bool dirty_;
};

//*****************************************************************************
// Some handy abbreviations
//*****************************************************************************

typedef base_type<unsigned,64, little> bt_uint64_le;
typedef base_type<unsigned,32, little> bt_uint32_le;
typedef base_type<unsigned,26, little> bt_uint26_le;
typedef base_type<unsigned,24, little> bt_uint24_le;
typedef base_type<unsigned,16, little> bt_uint16_le;
typedef base_type<unsigned, 8, little> bt_uint8_le;
typedef base_type<unsigned, 7, little> bt_uint7_le;
typedef base_type<unsigned, 6, little> bt_uint6_le;
typedef base_type<unsigned, 5, little> bt_uint5_le;
typedef base_type<unsigned, 4, little> bt_uint4_le;
typedef base_type<unsigned, 3, little> bt_uint3_le;
typedef base_type<unsigned, 2, little> bt_uint2_le;
typedef base_type<unsigned, 1, little> bt_bit;

typedef base_type<signed,  64, little> bt_int64_le;
typedef base_type<signed,  32, little> bt_int32_le;
typedef base_type<signed,  24, little> bt_int24_le;
typedef base_type<signed,  20, little> bt_int20_le;
typedef base_type<signed,  16, little> bt_int16_le;
typedef base_type<signed,  12, little> bt_int12_le;
typedef base_type<signed,   8, little> bt_int8_le;

typedef base_type<unsigned, 8, little> bt_char_le;

typedef base_type<float,   32, little> bt_float32_le;
typedef base_type<float,   64, little> bt_float64_le;

} // namespace binary

#ifdef __GNUC__
#   ifdef NDEBUG
#       undef inline
#   endif
#endif

#endif // BINARY_HPP
