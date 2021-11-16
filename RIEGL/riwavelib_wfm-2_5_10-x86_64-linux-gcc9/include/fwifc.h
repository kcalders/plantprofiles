// $Id$

#ifndef FWIFC_H
#define FWIFC_H

#ifdef _WIN32
#   ifdef FWIFC_DYN_LINK
#       ifdef FWIFC_SOURCE
#           define FWIFC_API __declspec(dllexport)
#       else
#           define FWIFC_API __declspec(dllimport)
#       endif
#   else
#       define FWIFC_API
#   endif
#else
#   ifdef FWIFC_DYN_LINK
#       define FWIFC_API __attribute__ ((visibility ("default")))
#   else
#       define FWIFC_API
#   endif
#endif

/* Error codes */
#define FWIFC_END_OF_FILE            -1
#define FWIFC_NO_ERROR                0
#define FWIFC_ERR_BAD_ARG             1
#define FWIFC_ERR_UNSUPPORTED_FORMAT  2
#define FWIFC_ERR_MISSING_INDEX       3
#define FWIFC_ERR_UNKNOWN_EXCEPTION   4
#define FWIFC_ERR_NOT_IMPLEMENTED     5
#define FWIFC_ERR_RUNTIME             6
#define FWIFC_ERR_NOT_LICENSED        7

/* Index to calibration tables (get_calib) */
#define FWIFC_CALIB_AMPL_CH0          0
#define FWIFC_CALIB_AMPL_CH1          1
#define FWIFC_CALIB_RNG_CH0           2
#define FWIFC_CALIB_RNG_CH1           3

/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#   if defined(_MSC_VER) && (_MSC_VER <= 1500)
    typedef unsigned char    uint8_t;
    typedef signed   char     int8_t;
    typedef unsigned short  uint16_t;
    typedef          short   int16_t;
    typedef unsigned int    uint32_t;
    typedef          int     int32_t;
    typedef unsigned __int64  uint64_t;
    typedef          __int64   int64_t;
    typedef unsigned __int64 uintmax_t;
    typedef          __int64  intmax_t;
    typedef float            float32_t;
    typedef double           float64_t;
#   else
#       include <cstdint>
#   endif
#else
#   include <stdint.h>
#endif

typedef int32_t          fwifc_int32_t;
typedef uint16_t         fwifc_uint16_t;
typedef uint32_t         fwifc_uint32_t;
typedef int32_t          fwifc_bool;
typedef const char*      fwifc_csz;
typedef uint8_t          fwifc_byte;
typedef double           fwifc_float64_t;

typedef fwifc_uint16_t   fwifc_sample_t;

typedef struct fwifc_sbl_struct {
    fwifc_float64_t   time_sosbl;  /* start of sample block in s */
    fwifc_uint32_t    channel;      /* 0:high, 1:low, 2:shp, 3:reference */
    fwifc_uint32_t    sample_count;
    fwifc_uint32_t    sample_size;
    fwifc_sample_t    *sample;
} fwifc_sbl_t;

struct fwifc_file_t;
typedef struct fwifc_file_t* fwifc_file;

/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_get_library_version
(
    fwifc_uint16_t    *api_major
    , fwifc_uint16_t  *api_minor
    , fwifc_csz       *build_version
    , fwifc_csz       *build_tag
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_get_last_error
(
    fwifc_csz       *message
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_open
(
    fwifc_csz path
    , fwifc_file *file
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_close
(
    fwifc_file file
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_reindex
(
    fwifc_file file
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_seek_time
(
    fwifc_file file
    , fwifc_float64_t time
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_seek_time_external
(
    fwifc_file file
    , fwifc_float64_t time
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_seek
(
    fwifc_file file
    , fwifc_uint32_t record
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_tell
(
    fwifc_file file
    , fwifc_uint32_t *record
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_get_info
(
    fwifc_file file
    , fwifc_csz       *instrument
    , fwifc_csz       *serial
    , fwifc_csz       *epoch     /* of time_external can be a datetime */
                                 /* "2010-11-16T00:00:00" if known     */
                                 /* or "DAYSEC" or "WEEKSEC"           */
                                 /* "UNKNOWN" if unknown               */
    , fwifc_float64_t *v_group /* group velocity */
    , fwifc_float64_t *sampling_time /* sampling interval in seconds */
    , fwifc_uint16_t  *flags   /*GPS_avail, ...*/
    , fwifc_uint16_t  *num_facets /* number of mirror facets*/
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_get_calib
(
    fwifc_file file
    , fwifc_uint16_t  table_kind  /* one of the FWIFC_CALIB_xxx constants */
    , fwifc_uint32_t   *count     /* length of returned table */
    , fwifc_float64_t* *abscissa  /* table values, valid until next call into*/
    , fwifc_float64_t* *ordinate  /* library. */
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_read
(
    fwifc_file file
    , fwifc_float64_t *time_sorg     /* start of range gate in s(hi res) */
    , fwifc_float64_t *time_external /* external time in s relative to epoch*/
    , fwifc_float64_t *origin        /* origin vector in m */
    , fwifc_float64_t *direction     /* direction vector dimensionless */
    , fwifc_uint16_t  *flags  /* GPS, ...*/
    , fwifc_uint16_t  *facet  /* facet number */
    , fwifc_uint32_t  *sbl_count
    , fwifc_uint32_t  *sbl_size
    , fwifc_sbl_t*    *sbl
);
/*---------------------------------------------------------------------------*/
FWIFC_API fwifc_int32_t
fwifc_set_sosbl_relative
(
    fwifc_file file
    , fwifc_int32_t value    /* 1=relative to time_sorg, 0=absolute (default)*/
);
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* FWIFC_H */
