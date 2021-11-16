/* $Id$ */

#ifndef BASEIFC_T_H
#define BASEIFC_T_H

/*!\file
 * Basic data types used in the DLL interface.
 */
#ifdef DOXYGEN
    /*!\brief unsigned 8 bit integer */
    typedef IMPLEMENTATION_DEFINED   scanifc_uint8_t;
    /*!\brief signed 8 bit integer */
    typedef IMPLEMENTATION_DEFINED    scanifc_int8_t;
    /*!\brief unsigned 16 bit integer */
    typedef IMPLEMENTATION_DEFINED  scanifc_uint16_t;
    /*!\brief signed 16 bit integer */
    typedef IMPLEMENTATION_DEFINED   scanifc_int16_t;
    /*!\brief unsigned 32 bit integer */
    typedef IMPLEMENTATION_DEFINED  scanifc_uint32_t;
    /*!\brief signed 32 bit integer */
    typedef IMPLEMENTATION_DEFINED   scanifc_int32_t;
    /*!\brief unsigned 64 bit integer */
    typedef IMPLEMENTATION_DEFINED  scanifc_uint64_t;
    /*!\brief signed 64 bit integer */
    typedef IMPLEMENTATION_DEFINED   scanifc_int64_t;
    /*!\brief unsigned largest available bitsize integer */
    typedef IMPLEMENTATION_DEFINED scanifc_uintmax_t;
    /*!\brief signed largest available bitsize integer */
    typedef IMPLEMENTATION_DEFINED  scanifc_intmax_t;
    /*!\brief 32 bit floating point */
    typedef IMPLEMENTATION_DEFINED scanifc_float32_t;
    /*!\brief 64 bit floating point */
    typedef IMPLEMENTATION_DEFINED scanifc_float64_t;
#elif defined(__cplusplus)
#   include <riegl/config.hpp>
    typedef scanlib::uint8_t   scanifc_uint8_t;
    typedef scanlib::int8_t    scanifc_int8_t;
    typedef scanlib::uint16_t  scanifc_uint16_t;
    typedef scanlib::int16_t   scanifc_int16_t;
    typedef scanlib::uint32_t  scanifc_uint32_t;
    typedef scanlib::int32_t   scanifc_int32_t;
    typedef scanlib::uint64_t  scanifc_uint64_t;
    typedef scanlib::int64_t   scanifc_int64_t;
    typedef scanlib::uintmax_t scanifc_uintmax_t;
    typedef scanlib::intmax_t  scanifc_intmax_t;
    typedef scanlib::float32_t scanifc_float32_t;
    typedef scanlib::float64_t scanifc_float64_t;
#else /* plain C version */
/* The following typedefs might need configuration... */
#if defined(__MINGW32__) || defined(__GNUC__)
#   include <stdint.h>
    typedef uint8_t   scanifc_uint8_t;
    typedef int8_t    scanifc_int8_t;
    typedef uint16_t  scanifc_uint16_t;
    typedef int16_t   scanifc_int16_t;
    typedef uint32_t  scanifc_uint32_t;
    typedef int32_t   scanifc_int32_t;
    typedef uint64_t  scanifc_uint64_t;
    typedef int64_t   scanifc_int64_t;
    typedef uintmax_t scanifc_uintmax_t;
    typedef intmax_t  scanifc_intmax_t;
    typedef float scanifc_float32_t;
    typedef double scanifc_float64_t;
#   endif
#   ifdef _MSC_VER
    typedef unsigned __int8  scanifc_uint8_t;
    typedef unsigned __int16 scanifc_uint16_t;
    typedef unsigned __int32 scanifc_uint32_t;
    typedef unsigned __int64 scanifc_uint64_t;
    typedef __int8           scanifc_int8_t;
    typedef __int16          scanifc_int16_t;
    typedef __int32          scanifc_int32_t;
    typedef __int64          scanifc_int64_t;
    typedef float            scanifc_float32_t;
    typedef double            scanifc_float64_t;
#   endif
#endif

typedef char*           scanifc_sz;
/*!\brief pointer to zero delimited string */
typedef const char*     scanifc_csz;
/*!\brief pointer to constant zero delimited string */
typedef scanifc_int32_t scanifc_bool;
/*!\brief boolean type */

#endif /* BASEIFC_T_H */
