// $Id$

#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include <riegl/config.hpp>
#include <riegl/detail/binary.hpp>
#include <riegl/rxpmarker.hpp>
#include <riegl/scanexcept.hpp>

#include <vector>
#include <map>
#include <stdexcept>
#include <string>
#include <cassert>
#include <ostream>
#include <limits>

#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

namespace scanlib {

class packet_error
    : public scanlib_exception {
    unsigned id_main;
    unsigned id_sub;
public:
    explicit packet_error(
        unsigned id_main
        , unsigned id_sub
        , const std::string& what_arg
        , const std::string& file_arg = ""
        , unsigned line_arg = 0
    );
};

using namespace binary;

typedef base_type<unsigned,64, little> sc_uint64;
typedef base_type<unsigned,49, little> sc_uint49;
typedef base_type<unsigned,32, little> sc_uint32;
typedef base_type<unsigned,31, little> sc_uint31;
typedef base_type<unsigned,30, little> sc_uint30;
typedef base_type<unsigned,29, little> sc_uint29;
typedef base_type<unsigned,28, little> sc_uint28;
typedef base_type<unsigned,27, little> sc_uint27;
typedef base_type<unsigned,26, little> sc_uint26;
typedef base_type<unsigned,25, little> sc_uint25;
typedef base_type<unsigned,24, little> sc_uint24;
typedef base_type<unsigned,23, little> sc_uint23;
typedef base_type<unsigned,20, little> sc_uint22;
typedef base_type<unsigned,20, little> sc_uint21;
typedef base_type<unsigned,20, little> sc_uint20;
typedef base_type<unsigned,18, little> sc_uint18;
typedef base_type<unsigned,16, little> sc_uint16;
typedef base_type<unsigned,15, little> sc_uint15;
typedef base_type<unsigned,14, little> sc_uint14;
typedef base_type<unsigned,13, little> sc_uint13;
typedef base_type<unsigned,12, little> sc_uint12;
typedef base_type<unsigned,11, little> sc_uint11;
typedef base_type<unsigned,10, little> sc_uint10;
typedef base_type<unsigned, 9, little> sc_uint9;
typedef base_type<unsigned, 8, little> sc_uint8;
typedef base_type<unsigned, 7, little> sc_uint7;
typedef base_type<unsigned, 6, little> sc_uint6;
typedef base_type<unsigned, 5, little> sc_uint5;
typedef base_type<unsigned, 4, little> sc_uint4;
typedef base_type<unsigned, 3, little> sc_uint3;
typedef base_type<unsigned, 2, little> sc_uint2;

typedef base_type<unsigned, 1, little> sc_bit;

typedef base_type<signed,  64, little> sc_int64;
typedef base_type<signed,  32, little> sc_int32;
typedef base_type<signed,  30, little> sc_int30;
typedef base_type<signed,  28, little> sc_int28;
typedef base_type<signed,  26, little> sc_int26;
typedef base_type<signed,  25, little> sc_int25;
typedef base_type<signed,  24, little> sc_int24;
typedef base_type<signed,  23, little> sc_int23;
typedef base_type<signed,  22, little> sc_int22;
typedef base_type<signed,  21, little> sc_int21;
typedef base_type<signed,  20, little> sc_int20;
typedef base_type<signed,  19, little> sc_int19;
typedef base_type<signed,  18, little> sc_int18;
typedef base_type<signed,  17, little> sc_int17;
typedef base_type<signed,  16, little> sc_int16;
typedef base_type<signed,  15, little> sc_int15;
typedef base_type<signed,  14, little> sc_int14;
typedef base_type<signed,  13, little> sc_int13;
typedef base_type<signed,  12, little> sc_int12;
typedef base_type<signed,  11, little> sc_int11;
typedef base_type<signed,  10, little> sc_int10;
typedef base_type<signed,   9, little> sc_int9;
typedef base_type<signed,   8, little> sc_int8;
typedef base_type<signed,   7, little> sc_int7;
typedef base_type<signed,   6, little> sc_int6;
typedef base_type<signed,   5, little> sc_int5;
typedef base_type<signed,   4, little> sc_int4;
typedef base_type<signed,   3, little> sc_int3;
typedef base_type<signed,   2, little> sc_int2;

typedef base_type<unsigned, 8, little> sc_char;

typedef base_type<float,   32, little> sc_float32;
typedef base_type<float,   64, little> sc_float64;

struct lookup_table
{
    struct id
    {
        id(unsigned main=0, unsigned sub=0)
            : main(main), sub(sub)
        {}

        unsigned main;
        unsigned sub;

        bool operator<(const id& y) const
        {
            return main<y.main || (!(y.main<main) && sub<y.sub);
        }
    };

    id operator[](unsigned char esc) const
    {
        // hard coded volatile package
        if (254 == esc)
            return id(65535, 65535);

        // hard coded crc32_check
        if (253 == esc)
            return id(50,0);

        if (esc>(unsigned char)(id_table.size()))
            return id(0,unsigned(esc)); // debugging help, still an unknown id
        else
            return id_table[esc-1];
    }

    unsigned char operator[](id long_id) const
    {
        // hard coded volatile package
        if (65535 == long_id.main && 65535 == long_id.sub)
            return 254;

        // hard coded crc32_check package
        if (50 == long_id.main && 0 == long_id.sub)
            return 253;

        std::map<id, unsigned char>::const_iterator it;
        it = id_map.find(long_id);
        if (it != id_map.end())
            return it->second;
        else
            return 0;
    }

    unsigned char set(unsigned char esc, unsigned main, unsigned sub)
    {
        if (0 == id_map.count((id(main,sub)))) {
            if (0 == esc) { // allocate new entry
                if (id_table.size() >= 255)
                    esc = 255;
                else
                    esc = static_cast<unsigned char>(id_table.size()+1);
            }
        }
        else {
            if (0 == esc)
                esc = id_map[id(main,sub)];
        }

        if (esc < 255) {
            if (esc > static_cast<unsigned char>(id_table.size()))
                id_table.resize(esc);
            id_table[esc-1] = id(main,sub);
            id_map[id(main,sub)] = esc;
            return esc;
        }
        else
            return 0;
    }

    template<class P>
    unsigned char set (unsigned char esc = 0)
    {
        return set(esc, P::id_main, P::id_sub);
    }

    // load from sequence
    template<class S>
    void load(const S& s)
    {
        for (std::size_t n=0; n<s.size(); ++n)
        {
            set(static_cast<unsigned char>(n+1), s[n].main, s[n].sub);
        }
    }

    // load from sequence
    template<class S>
    void load(const S& s, std::size_t size)
    {
        for (std::size_t n=0; n<size; ++n)
        {
            set(static_cast<unsigned char>(n+1), s[n].main, s[n].sub);
        }
    }

    // store to sequence
    template<class S>
    void store(S& s)
    {
        for (std::size_t n=0; n<id_table.size(); ++n) {
            s[n].main = id_table[n].main;
            s[n].sub  = id_table[n].sub;
        }
    }

    // store to sequence
    template<class S>
    void store(S& s, std::size_t& size)
    {
        for (std::size_t n=0; n<id_table.size(); ++n) {
            s[n].main = id_table[n].main;
            s[n].sub  = id_table[n].sub;
        }
        size = id_table.size();
    }

    bool empty() const
    {
        return id_table.empty();
    }

private:
    std::vector<id> id_table;
    std::map<id, unsigned char> id_map; // reverse id lookup
};

template<class it>
struct write_package
{
    write_package(unsigned id_main, unsigned id_sub, it begin, it end)
        : esc(begin)
        , sub(begin)
        , main(begin)
        , begin_(begin)
        , end_(end)
    {
        if (65535 == id_main && 65535 == id_sub) {
            esc = 0xfffffffe; // hard coded volatile
            begin_ = esc.end();
        }
        else {
            esc = 0xffffffff;
            main = id_main;
            sub = id_sub;
            begin_ = main.end();
        }
    }

    write_package(unsigned char escape, it begin, it end)
        : esc(begin)
        , sub(begin)
        , main(begin)
        , begin_(begin)
        , end_(end)
    {
        esc = 0xffffff00|escape;
        begin_ = esc.end();
    }

    write_package(unsigned id_main, unsigned id_sub, const lookup_table& lu, it begin, it end)
        : esc(begin)
        , sub(begin)
        , main(begin)
        , begin_(begin)
        , end_(end)
    {
        unsigned char lu_esc = lu[lookup_table::id(id_main, id_sub)];
        if (lu_esc) {
            esc = 0xffffff00|lu_esc;
            begin_ = esc.end();
        }
        else {
            esc = 0xffffffff;
            main = id_main;
            sub = id_sub;
            begin_ = main.end();
        }
    }

    inline it begin() const
    { return begin_; }

    inline it end() const
    { return end_; }

    field<unsigned, sc_uint32,  0, it> esc;
    field<unsigned, sc_uint16, 32, it> sub;
    field<unsigned, sc_uint16, 48, it> main;

private:
    it begin_;
    it end_;
};

template<class it>
struct basic_package
{
public:
    basic_package(it begin, it end, const lookup_table& lu)
        : esc(begin)
        , sub(begin)
        , main(begin)
        , begin_(begin)
        , end_(end)
    {
        assert(0<(0xff&esc));

        if (0xff==(esc&0xff)) {
            id.main = main;
            id.sub  = sub;
            begin_  = main.end();
        }
        else if (0xfe==(esc&0xff)) {
            id.main = 65535; // hard coded "volatile" package
            id.sub  = 65535;
            begin_ = esc.end();
        }
        else {
            id = lu[(esc&0xff)];
            begin_  = esc.end();
        }
    }

    bool is_long_id() const
    { return 0xff==(esc&0xff); }

    inline it begin() const
    { return begin_; }

    inline it end() const
    { return end_; }

    lookup_table::id id;

    field<unsigned, sc_uint32,  0, it> esc;
    field<unsigned, sc_uint16, 32, it> sub;
    field<unsigned, sc_uint16, 48, it> main;

private:
    it begin_;
    it end_;
};

template<class pkg_type>
struct buffer_package
    : private write_package<typename pkg_type::iterator_type>
    , public pkg_type
{
    typedef typename pkg_type::iterator_type iterator_type;
    typedef scanlib::buffer buffer_type;
    typedef write_package<typename pkg_type::iterator_type> wpkg_type;

    // constructor using long package id
    explicit buffer_package(buffer_type& buf)
        : wpkg_type(pkg_type::id_main, pkg_type::id_sub, buf.begin(), buf.end())
        , pkg_type(wpkg_type::begin(), wpkg_type::end())
        , buf_(buf)
    {}

    // constructor using short escape
    explicit buffer_package(unsigned char escape, buffer_type& buf)
        : wpkg_type(escape, buf.begin(), buf.end())
        , pkg_type(wpkg_type::begin(), wpkg_type::end())
        , buf_(buf)
    {}

    // constructor using lookup table
    explicit buffer_package(const lookup_table& lu, buffer_type& buf)
        : wpkg_type(pkg_type::id_main, pkg_type::id_sub, lu, buf.begin(), buf.end())
        , pkg_type(wpkg_type::begin(), wpkg_type::end())
        , buf_(buf)
    {}

    iterator_type begin() const
    { return wpkg_type::begin(); }

    void resize()
    {
        buf_.resize( pkg_type::end() - buf_.begin());
    }

    template<class src_pkg_type>
    buffer_package& operator= (const src_pkg_type& y)
    {
        pkg_type& x(*this);
        x = y;
        return *this;
    }

private:
    buffer_type& buf_;
};

std::ios_base& set_rxp_braceless(std::ios_base& os);

std::ios_base& reset_rxp_braceless(std::ios_base& os);

bool is_rxp_braceless(std::ios_base& os);

class package_ostream_entry
    : public std::ostream::sentry
{
    std::ostream& os;
    std::ios_base::fmtflags original_flags;
    std::streamsize original_precision;
public:
    package_ostream_entry(std::ostream& s)
        : std::ostream::sentry(s)
        , os(s) {
        if (*this) {
            original_flags = os.flags();
            os.setf(std::ios_base::scientific, std::ios_base::floatfield);
            os.setf(std::ios_base::dec, std::ios_base::basefield);
            original_precision = os.precision(1+std::numeric_limits<float64_t>::digits10);
        }
    }

    ~package_ostream_entry() {
        if(*this) {
            os.flags(original_flags);
            os.precision(original_precision);
        }
    }
};

class package_istream_entry
    : public std::istream::sentry
{
    std::istream& is;
public:
    package_istream_entry(std::istream& s)
        : std::istream::sentry(s)
        , is(s) {
    }

    ~package_istream_entry() {
    }
};

template
<
    class charT
    , class traits
    , class T
    , unsigned size_
    , class BASE
    , unsigned offset
    , class it
>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& s, const binary::array<T, size_, BASE, offset, it>& x) {
    if (!is_unbraced(s)) s   << "[ ";
    s << x[0];
    for (std::size_t n=1; n<size_; ++n)
        s << ", " << x[n];
    if (!is_unbraced(s)) s   << " ]";
    return s;
}

template
<
    class charT
    , class traits
    , unsigned size_
    , class BASE
    , unsigned offset
    , class it
>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& s, const binary::array<char, size_, BASE, offset, it>& x) {
    s   << "\"";
    for (std::size_t n=0; n<size_; ++n) {
        if ('\0' == x[n]) break;
        s << x[n];
    }
    s   << "\"";
    return s;
}

template<
    class charT
    , class traits
    , class T
    , unsigned size_
    , class BASE
    , unsigned offset
    , class It
>
std::basic_istream<charT, traits>&
read_array(std::basic_istream<charT, traits>& in, unsigned size, binary::array<T,size_,BASE,offset,It>& arg)
{
    typename std::basic_istream<charT, traits>::sentry ok(in);
    if (ok) {
        if(!is_rxp_braceless(in)) in >> '[';
        if (size>0) in >> arg[0];
        for (unsigned n=1; n<size; ++n) {
            in >> ',' >> arg[n];
        }
        if(!is_rxp_braceless(in)) in >> ']';
    }
    return in;
}

template<
    class charT
    , class traits
    , unsigned size_
    , class BASE
    , unsigned offset
    , class It
>
std::basic_istream<charT, traits>&
read_array(std::basic_istream<charT, traits>& in, unsigned size, binary::array<char,size_,BASE,offset,It>& arg)
{
    typename std::basic_istream<charT, traits>::sentry ok(in);
    if (ok) {
        in >> '"';
        arg[0] = '\0';
        for (unsigned n=0; n<size; ++n) {
            arg[n] = in.get();
            if (arg[n] == '"') {
                in.putback('"');
                arg[n] = '\0';
            }
        }
        in >> '"';
    }
    return in;
}

template<class T, class charT, class traits>
std::basic_istream<charT, traits>&
read_array(std::basic_istream<charT, traits>& in, unsigned size, T* arg)
{
    typename std::basic_istream<charT, traits>::sentry ok(in);
    if (ok) {
        if (!is_rxp_braceless(in)) in >> '[';
        if (size>0) in >> arg[0];
        for (unsigned n=1; n<size; ++n)
            in >> ',' >> arg[n];
        if (!is_rxp_braceless(in)) in >> ']';
    }
    return in;
}

template<class charT, class traits>
std::basic_istream<charT, traits>&
read_array(std::basic_istream<charT, traits>& in, unsigned size, charT* arg)
{
    typename std::basic_istream<charT, traits>::sentry ok(in);
    if (ok) {
        in >> '"';
        if (size>0) arg[0] = in.get();
        for (unsigned n=1; n<size; ++n) {
            arg[n] = in.get();
            if (arg[n] == '"') {
                in.putback('"');
                arg[n] = '\0';
            }
        }
        in >> '"';
    }
    return in;
}

template<class T, class charT, class traits>
std::basic_ostream<charT, traits>&
write_array(std::basic_ostream<charT, traits>& out, unsigned size, const T* arg)
{
    typename std::basic_ostream<charT, traits>::sentry ok(out);
    if (ok) {
        if (!is_rxp_braceless(out)) out   << "[ ";
        out << arg[0];
        for (unsigned n=1; n<size; ++n)
            out << ", " << arg[n];
        if (!is_rxp_braceless(out)) out   << " ]";
    }
    return out;
}

template<class charT, class traits>
std::basic_ostream<charT, traits>&
write_array(std::basic_ostream<charT, traits>& out, unsigned size, const charT* arg)
{
    typename std::basic_ostream<charT, traits>::sentry ok(out);
    if (ok) {
        out   << "\"";
        for (unsigned n=0; n<size; ++n) {
            if ('\0' == arg[n]) break;
            out << arg[n];
        }
        out   << "\"";
    }
    return out;
}

template
<
    class T
    , unsigned size_
    , class BASE
    , unsigned offset
    , class it
>
std::ostream&
write_array(std::ostream& s, unsigned size, const binary::array<T, size_, BASE, offset, it>& x) {
    if (!is_rxp_braceless(s)) s   << "[ ";
    s << x[0];
    for (unsigned n=1; n<size_; ++n)
        s << ", " << x[n];
    if (!is_rxp_braceless(s)) s   << " ]";
    return s;
}

template
<
    unsigned size_
    , class BASE
    , unsigned offset
    , class it
>
std::ostream&
write_array(std::ostream& s, unsigned size, const binary::array<char, size_, BASE, offset, it>& x) {
    s   << "\"";
    for (unsigned n=0; n<size_; ++n) {
        if ('\0' == x[n]) break;
        s << x[n];
    }
    s   << "\"";
    return s;
}

extern const int stream_id_main_idx;
extern const int stream_id_sub_idx;
extern const int stream_unknown_idx;

} // namespace scanlib

#endif // PACKAGE_HPP
