// $Id$

#ifndef RIVLIB_CONFIG_HPP
#define RIVLIB_CONFIG_HPP

#include <string>
#include <cctype>
#include <istream>
#include <string>
#include <istream>
#include <ostream>

#include <iostream> //DEBUG:

#if !defined(SCANLIB_BIGEND_HOST)
#   include <stdio.h>
#   if defined(__GNU_LIBRARY__)
#       include <endian.h>
#       if (__BYTE_ORDER == __BIG_ENDIAN)
#           define SCANLIB_BIGEND_HOST
#       endif
#   endif
#endif

#ifdef _WIN32
#   ifdef RIVLIB_SHARED
#       ifdef RIVLIB_SOURCE
#           define RIVLIB_API __declspec(dllexport)
#           ifdef _MSC_VER
#               define RTL_API __declspec(dllimport)
#           else
#               define RTL_API
#           endif
#       else
#           define RIVLIB_API __declspec(dllimport)
#           define RTL_API __declspec(dllimport)
#       endif
#   else
#       define RIVLIB_API
#       define RTL_API
#   endif
#else
#   define RTL_API
#   define RIVLIB_API
#endif


#include <stdint.h>
namespace scanlib {
    using ::uint8_t;
    using ::int8_t;
    using ::uint16_t;
    using ::int16_t;
    using ::uint32_t;
    using ::int32_t;
    using ::uint64_t;
    using ::int64_t;
    using ::uintmax_t;
    using ::intmax_t;
    typedef float float32_t;
    typedef double float64_t;
}

#if defined(_MSC_VER)
# if (_MSC_VER <= 1500) // pull tr1 into namespace std
	namespace std{ namespace tr1 {} using namespace tr1; }
# endif
#endif

namespace scanlib {

template<class charT, class traits>
inline std::basic_istream<charT, traits>&
operator>>(std::basic_istream<charT, traits>& in, uint8_t& arg)
{ unsigned int t; in >> t; arg = t; return in; }

template<class charT, class traits>
inline std::basic_istream<charT, traits>&
operator>>(std::basic_istream<charT, traits>& in, int8_t& arg)
{ signed int t; in >> t; arg = t; return in; }

} // scanlib

namespace std {

// the unsigned/signed char inserters should be defined in a standard lib, but
// seem not to be...

template<class charT, class traits>
basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits>& out, unsigned char arg)
{ out << static_cast<unsigned int>(arg); return out; }

template<class charT, class traits>
basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits>& out, signed char arg)
{ out << static_cast<signed int>(arg); return out; }

template<class charT, class traits>
basic_istream<charT, traits>&
operator>>(basic_istream<charT, traits>& in, const charT& arg)
{
    typename basic_istream<charT, traits>::sentry ok(in);
    if (ok) {
        charT t;
        in >> t;
        if (t != arg)
            in.setstate(ios::failbit);
    }
    return in;
}

template<class charT, class traits>
basic_istream<charT, traits>&
operator>>(basic_istream<charT, traits>& in, const basic_string<charT>& arg)
{
    typename basic_istream<charT, traits>::sentry ok(in);
    if(ok){
        charT c;
        typename basic_string<charT>::size_type n=0;
        while(n<arg.size() && in.get(c)) {
            if (arg[n++] != c) {
                in.putback(c);
                break;
            }
        }
        if (n<arg.size())
            in.setstate(std::ios::failbit);
    }
    return in;
}

template<class charT, class traits>
basic_istream<charT, traits>&
operator>>(basic_istream<charT, traits>& in, const charT* arg)
{
    return operator>>(in,basic_string<charT>(arg));
}

template<class charT, class traits>
basic_istream<charT, traits>&
endl(basic_istream<charT, traits>& in)
{
    typename basic_istream<charT, traits>::sentry ok(in, true);
    if (ok) {
        charT c;
        while(in.get(c)) {
            if (!isspace(c)) {
                in.putback(c);
                in.setstate(ios::failbit);
                break;
            }
            else if (c == '\n')
                break;
        }
    }
    return in;
}
} // namespace std

#define RIVLIB_STRING_HELPER2(a) #a
#define RIVLIB_STRING_HELPER1(a) RIVLIB_STRING_HELPER2(a)

#ifndef SVN_VERSION
#   define SVN_VERSION UNKNOWN
#endif
#define RIVLIB_BUILD_VERSION RIVLIB_STRING_HELPER1(SVN_VERSION)

#ifndef PRJ_VERSION
#   define PRJ_VERSION UNKNOWN
#endif
#define RIVLIB_VERSION RIVLIB_STRING_HELPER1(PRJ_VERSION)

#ifndef BUILD_TAG
#   define BUILD_TAG UNKNOWN
#endif
#define RIVLIB_BUILD_TAG RIVLIB_STRING_HELPER1(BUILD_TAG)

namespace scanlib {
    extern const std::string version;
    extern const std::string build_version;
    extern const std::string build_tag;
    extern const std::string ridataspec;
}

namespace scanlib {
namespace detail {
inline void
current_function_helper() { // __FUNCSIC__ only defined inside functions
#   if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
#       define RIVLIB_FUNCTION __PRETTY_FUNCTION__
#   elif defined(__DMC__) && (__DMC__ >= 0x810)
#       define RIVLIB_FUNCTION __PRETTY_FUNCTION__
#   elif defined(__FUNCSIG__)
#       define RIVLIB_FUNCTION __FUNCSIG__
#   elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#       define RIVLIB_FUNCTION __FUNCTION__
#   elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#       define RIVLIB_FUNCTION __FUNC__
#   elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#       define RIVLIB_FUNCTION __func__
#   else
#       define RIVLIB_FUNCTION "(unknown)"
#   endif
}
} } // namespace scanlib

#define SCLOC "rivlib : " RIVLIB_BUILD_VERSION " : " RIVLIB_BUILD_TAG " : " __FILE__ " : " RIVLIB_STRING_HELPER1(__LINE__)

#endif // RIVLIB_CONFIG_HPP
