/* $Id$ */

#ifndef SCANIFC_H
#define SCANIFC_H

#include <riegl/detail/baseifc_t.h>
#include <riegl/detail/pointsifc_t.h>

/*!\file
 * The shared object (DLL) interface definitions.
 */

#ifdef major
#   undef major
#endif
#ifdef minor
#   undef minor
#endif

#ifdef _WIN32
#   ifdef SCANIFC_BUILD_DLL
#       define SCANIFC_API __declspec(dllexport)
#   else
#       define SCANIFC_API __declspec(dllimport)
#   endif
#elif defined(DOXYGEN)
    /*!\brief Public interface tag. */
#   define SCANIFC_API
#else
#   define SCANIFC_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
#ifdef DOXYGEN
/*!\brief a handle to a stream of 3d pointcloud data */
typedef IMPLEMENTATION_DEFINED point3dstream_handle;
/*!\brief a handle to a multiplexed stream of 3d pointcloud data */
typedef IMPLEMENTATION_DEFINED rmsstream_handle;
#else
struct point3dstream;
typedef struct point3dstream* point3dstream_handle;
struct rmsstream;
typedef struct rmsstream* rmsstream_handle;
#endif

/**************************************************************************//**
 *  Get version number from library.
 *  This version number usually is different from the version number of the
 *  distribution set of the library. This version number is about the API
 *  of the shared object (DLL).
 *  A change in major version number generally indicates a breaking change
 *  in the API or semantics. A change in minor version usually does not
 *  require a change of user code.
 * \param major major version number
 * \param minor minor version number
 * \param build build number (revision control)
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_get_library_version
(
    scanifc_uint16_t    *major
    , scanifc_uint16_t  *minor
    , scanifc_uint16_t  *build
);

/**************************************************************************//**
 *  Get extended version information.
 *
 *  The build_version allows traceability to the SCM system.
 * The build tag contains additional information about the build.
 * \param build_version [out] SCM build version
 * \param build_tag [out] additional build information
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_get_library_info(
    scanifc_csz     *build_version
    , scanifc_csz   *build_tag
);

/**************************************************************************//**
 * Get last error message.
 * A failing function (indicated by !=0 return) usually will set an error
 * description string, that can be retrieved with this function.
 * \param message_buffer [in,out] user supplied buffer for the zero delimited
 *        string.
 * \param message_buffer_size [in] size of the user supplied buffer
 * \param message_size [out] size of the message, if larger than
 *        message_buffer_size the message is truncated to fit within the
 *        buffer.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_get_last_error
(
    scanifc_sz          message_buffer
    , scanifc_uint32_t  message_buffer_size
    , scanifc_uint32_t  *message_size
);

/**************************************************************************//**
 * Open 3D pointcloud data stream.
 * The open function returns a handle that references the stream. The stream
 * is read sequentially from start to end.
 * \param uri [in] unified resource identifier of the 'rxp' stream containing
 *        the pointcloud data.
 * \param sync_to_pps [in] if 0 does not use pps timestamps embedded in the
 *        rxp stream. If set to 1 requires pps timestamps.
 * \param h3ds [out] a handle identifying this particular stream, for use
 *        in the read function.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_open
(
    scanifc_csz             uri
    , scanifc_bool          sync_to_pps
    , point3dstream_handle  *h3ds
);
/**************************************************************************//**
 * Open 3D pointcloud data stream with logging.
 * The open function returns a handle that references the stream. The stream
 * is read sequentially from start to end. A log of the data is written to
 * the file system.
 * \param uri [in] unified resource identifier of the 'rxp' stream containing
 *        the pointcloud data.
 * \param sync_to_pps [in] if 0 does not use pps timestamps embedded in the
 *        rxp stream. If set to 1 requires pps timestamps.
 * \param log [in] a filename, where binary data will be logged.
 * \param h3ds [out] a handle identifying this particular stream, for use
 *        in the read function.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_open_with_logging
(
    scanifc_csz             uri
    , scanifc_bool          sync_to_pps
    , scanifc_csz           log
    , point3dstream_handle  *h3ds
);
/**************************************************************************//**
 * Open 3D pointcloud data stream from rms.
 * The open function returns a handle that references the stream. The stream
 * is read sequentially from start to end. A log of the data is written to
 * the file system.
 * \param uri [in] unified resource identifier of the 'rms' stream containing
 *        the pointcloud data.
 * \param sync_to_pps [in] if 0 does not use pps timestamps embedded in the
 *        rxp stream. If set to 1 requires pps timestamps.
 * \param log [in] a filename, where binary data will be logged.
 *        Optional parameter, zero if not required.
 * \param jobname [in] a name of the job to select from rms file.
 *        Optional parameter, zero if not required (the first will be selected).
 * \param h3ds [out] a handle identifying this particular stream, for use
 *        in the read function.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_open_rms
(
    scanifc_csz             uri
    , scanifc_bool          sync_to_pps
    , scanifc_csz           log
    , scanifc_csz           jobname
    , point3dstream_handle  *h3ds
);
/**************************************************************************//**
 * Open 3D pointcloud data stream with processing parameters.
 * The open function returns a handle that references the stream. The stream
 * is read sequentially from start to end.
 * \param uri [in] unified resource identifier of the 'rxp' stream containing
 *        the pointcloud data.
 * \param sync_to_pps [in] if 0 does not use pps timestamps embedded in the
 *        rxp stream. If set to 1 requires pps timestamps.
 * \param arg [in] processing arguments.
 * \param h3ds [out] a handle identifying this particular stream, for use
 *        in the read function.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_open_with_arg
(
    scanifc_csz             uri
    , scanifc_bool          sync_to_pps
    , scanifc_csz           arg
    , point3dstream_handle  *h3ds
);

/**************************************************************************//**
 * Append a demultiplexer.
 * (Note: don't confuse with rms, which is a different concept.)
 * A demultiplexer will write selected packages from the rxp stream to the
 * file with name 'filename' in ASCII format.
 * The demultiplexer will run while 'read' is executed, so it is necessary
 * to call into read until no more data is available, even if the result
 * values from read are of no interest. The 'selections' or 'classes'
 * may be 0 if not used.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \param filename [in] the name of the target file for this de-multiplexer.
 * \param selections [in] a space delim. list of package names to be used.
 * \param classes [in] a space delim. list of (package) class names.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_add_demultiplexer
(
    point3dstream_handle    h3ds
    , scanifc_csz           filename
    , scanifc_csz           selections
    , scanifc_csz           classes
);

/**************************************************************************//**
 * Override the default rangegate of the sensor.
 * This function is only meaningful for sensor types that support "multiple
 * turn around" zones, i.e. the laser clock is faster than the ambiguity
 * range of the rangegate would allow. The ambiguity can be resolved by
 * operator asstitence in specifying the intended range.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \param zone [in] a value specifying the MTA zone (0 is special, see below)
 * \param near [in] the closer range of the range gate (valid if zone=0)
 * \param far [in] the far end of the range gate (valid if zone=0)
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_set_rangegate
(
    point3dstream_handle    h3ds
    , scanifc_uint16_t      zone
    , scanifc_float32_t     near
    , scanifc_float32_t     far
);

/*! INTERNAL ONLY */
SCANIFC_API int
scanifc_point3dstream_set_notch_filter
(
    point3dstream_handle h3ds
    , scanifc_float32_t range_min
    , scanifc_float32_t range_max
    , scanifc_float32_t amplitude_max
);
/**************************************************************************//**
 * Read the meta information from the stream.
 * The function will read just enough from the stream as to make sure the
 * meta information has been discovered.
 * \param meta [out] number of points returned (may be smaller than want).
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_get_meta
(
    point3dstream_handle    h3ds
    , scanifc_csz          *meta
);
/**************************************************************************//**
 * Read some points from the stream.
 * The read function will fill point data into user supplied buffers. The
 * buffer pointers may be zero, in which case this particular buffer will
 * not be filled. After the end of a frame it is possible to call into read
 * again to obtain the next frame. The end of all available data is reached
 * when both: "got" and "end_of_frame" are zero at the same time.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \param want [in] the size of the result buffers (count of points).
 * \param pxyz32 [out] pointer to xyz buffer
 * \param pattributes [out] pointer to amplitude and quality buffer
 * \param ptime [out] pointer to timestamp buffer
 * \param got [out] number of points returned (may be smaller than want).
 * \param end_of_frame [out] != 0 if end of frame detected
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_read
(
    point3dstream_handle    h3ds
    , scanifc_uint32_t      want
    , scanifc_xyz32         *pxyz32
    , scanifc_attributes    *pattributes
    , scanifc_time_ns       *ptime
    , scanifc_uint32_t      *got
    , scanifc_bool          *end_of_frame
);
/**************************************************************************//**
 * Total number of octets already read. This function may be used for
 * debugging or as a progress indicator if reading from files for which the
 * size is known in advance.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \param pos [out] number of octets
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_tellg
(
    point3dstream_handle    h3ds
    , scanifc_uint64_t      *pos
);
/**************************************************************************//**
 * Cancel a blocking read.
 * This function needs to be called from a different thread.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_cancel
(
    point3dstream_handle h3ds
);
/**************************************************************************//**
 * Close a pointcloud stream.
 * This function must be called when done with the stream to release the
 * resources associated with the handle.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_point3dstream_close
(
    point3dstream_handle h3ds
);

/**************************************************************************//**
 * Open 3D pointcloud data stream from an rms stream.
 * The open function returns a handle that references the stream. The stream
 * is read sequentially from start to end.
 * \param uri [in] unified resource identifier of the 'rms' stream containing
 *        the pointcloud data.
 * \param sync_to_pps [in] if 0 does not use pps timestamps embedded in the
 *        rxp stream. If set to 1 requires pps timestamps.
 * \param jobs_size [in] number of jobs descriptors. Zero selects all jobs.
 * \param jobs [in] array of size jobs_size with job names.
 * \param hrms [out] a handle identifying this particular stream, for use
 *        in the read function.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_rmsstream_open
(
    scanifc_csz uri
    , scanifc_bool      sync_to_pps
    , scanifc_uint16_t  jobs_size
    , scanifc_csz*      jobs
    , rmsstream_handle  *hrms
);

/**************************************************************************//**
 * Read some points from the stream.
 * The read function will fill point data into user supplied buffers. The
 * buffer pointers may be zero, in which case this particular buffer will
 * not be filled.
 * \param hrms [in] the stream heandle that has been returned from open.
 * \param want [in] the size of the result buffers (count of points).
 * \param pxyz32 [out] pointer to xyz buffer
 * \param pattributes [out] pointer to amplitude and quality buffer
 * \param ptime [out] pointer to timestamp buffer
 * \param got [out] number of points returned (may smaller than want).
 * \param end_of_frame [out] != 0 if end of frame detected
 * \param jobnr [out] job number
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_rmsstream_read
(
    rmsstream_handle        hrms
    , scanifc_uint32_t      want
    , scanifc_xyz32         *pxyz32
    , scanifc_attributes    *pattributes
    , scanifc_time_ns       *ptime
    , scanifc_uint32_t      *got
    , scanifc_bool          *end_of_frame
    , scanifc_uint16_t      *jobnr
);
/**************************************************************************//**
 * Total number of octets already read. This function may be used for
 * debugging or as a progress indicator if reading from files for which the
 * size is known in advance.
 * \param hrms [in] the stream heandle that has been returned from open.
 * \param pos [out] number of octets
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_rmsstream_tellg
(
    rmsstream_handle        hrms
    , scanifc_uint64_t      *pos
);
/**************************************************************************//**
 *  Get job name.
 *  This function may be called after scanifc_rmsstream_read has dicovered
 *  a new jobnr.
 * \param hrms [in] the stream heandle that has been returned from open.
 * \param jobnr [in] index of desired jobname
 * \param jobname [out] jobname
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_rmsstream_get_jobname(
    rmsstream_handle    hrms
    , scanifc_uint16_t  jobnr
    ,scanifc_csz        *jobname
);

/**************************************************************************//**
 * Cancel a blocking read.
 * This function needs to be called from a different thread.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_rmsstream_cancel
(
    rmsstream_handle hrms
);

/**************************************************************************//**
 * Close a rms stream.
 * This function must be called when done with the stream to release the
 * resources associated with the handle.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
SCANIFC_API int
scanifc_rmsstream_close
(
    rmsstream_handle hrms
);

#ifdef __cplusplus
}
#endif

#endif /* SCANIFC_H */
