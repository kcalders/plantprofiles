/* $Id$ */

#ifndef CTRLIFC_H
#define CTRLIFC_H

/*!\file
 * The Riegl Scanner Control (DLL) interface definitions.
 */

#ifdef major
#   undef major
#endif
#ifdef minor
#   undef minor
#endif

#ifdef _WIN32
#   ifdef CTRLIFC_BUILD_DLL
#       define CTRLIFC_API __declspec(dllexport)
#   else
#       define CTRLIFC_API __declspec(dllimport)
#   endif
#elif defined(DOXYGEN)
    /*!\brief Public interface tag. */
#   define CTRLIFC_API
#else
#   define CTRLIFC_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*!\file
 * Basic data types used in the DLL interface.
 */
#ifdef DOXYGEN
    /*!\brief unsigned 16 bit integer */
    typedef IMPLEMENTATION_DEFINED  ctrlifc_uint16_t;
    typedef IMPLEMENTATION_DEFINED  ctrlifc_int32_t;
#else
/* The following typedefs might need configuration... */
#   if defined(__MINGW32__) || defined(__GNUC__)
#   include <stdint.h>
    typedef uint16_t         ctrlifc_uint16_t;
    typedef int32_t          ctrlifc_int32_t;
#   elif defined(_MSC_VER)
    typedef unsigned __int16 ctrlifc_uint16_t;
    typedef unsigned __int32 ctrlifc_int32_t;
#   endif
#endif

/*---------------------------------------------------------------------------*/
#ifdef DOXYGEN
/*!\brief a handle to a stream of 3d pointcloud data */
typedef IMPLEMENTATION_DEFINED ctrlifc_session_handle;
#else
struct ctrlifc_session_impl;
typedef struct ctrlifc_session_impl* ctrlifc_session;
#endif

/*!\brief A zero delimited string */
typedef char* ctrlifc_sz;

/*!\brief A list of zero delimited strings */
typedef char** ctrlifc_szlist;

/*!\brief A zero delimited constant string */
typedef const char* ctrlifc_csz;

/*!\brief A list of zero delimited string constants */
typedef const char** ctrlifc_cszlist;

/**************************************************************************//**
 *  Get version number from library.
 *
 *  This version number usually is different from the version number of the
 *  distribution set of the library. This version number is about the API
 *  of the shared object (DLL).
 *  A change in major version number generally indicates a breaking change
 *  in the API or semantics. A change in minor version usually does not
 *  require a change of user code.
 * \param major major version number
 * \param minor minor version number
 * \param build build number (revision control)
 * \return 0 for succes, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_get_library_version(
    ctrlifc_uint16_t* major,
    ctrlifc_uint16_t* minor,
    ctrlifc_uint16_t* build
);

/**************************************************************************//**
 *  Get extended version information.
 *
 *  The build_version allows traceability to the SCM system.
 * The build tag contains additional information about the build.
 * \param build_version [out] SCM build version
 * \param build_tag [out] additional build information
 * \return 0 for succes, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_get_library_info(
    ctrlifc_csz *build_version,
    ctrlifc_csz *build_tag
);

/**************************************************************************//**
 * Create a scanner control session.
 *
 * The create_session function returns a handle that references the session.
 * \param s [out] a handle identifying this particular session.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_construct_session(ctrlifc_session* s);

/**************************************************************************//**
 * Close a session.
 *
 * This function must be called when done with the ssession to release the
 * resources associated with the handle.
 * \param s [in] the ssession heandle that has been returned from construct.
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_destruct_session(ctrlifc_session s);

/**************************************************************************//**
 * Get last error message.
 *
 * A failing function (indicated by !=0 return) usually will set an error
 * description string, that can be retrieved with this function.
 * \param s [in] the session handle
 * \param message [ const out] The error message
 * \return 0 for succes, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_get_last_error(ctrlifc_session s, ctrlifc_csz *message_buffer);

/**************************************************************************//**
 * Establish a session to a scanner.
 *
 * The connect function will locate the scanner by means of the authority
 * information, which may be given as "host:port", where host and port
 * either are in text or numeric forms.
 * \param s [in] the session handle
 * \param authority [in] the address of the scanner
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_connect(ctrlifc_session s, ctrlifc_csz authority);

/**************************************************************************//**
 * Close connection to scanner.
 *
 * The disconnect function will close the connection to the scanner.
 * \param s [in] the session handle
  * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_disconnect(ctrlifc_session s);

/**************************************************************************//**
 * INTERNAL ONLY Start logging of client/server communication to file.
 *
 * Information is indicated with "i " and errors are reported using "! ".
 * \param s [in] the session handle
 * \param logfile [in] the callback function
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_start_logging(ctrlifc_session s, ctrlifc_csz logfile);

/**************************************************************************//**
 * INTERNAL ONLY Stop logging of client/server communication to file.
 *
 * \param s [in] the session handle
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_stop_logging(ctrlifc_session s);

/**************************************************************************//**
 * Set a property value.
 *
 * The set_property function sets a new value for a property.
 * The new value is given as string value:
 * - string properties are to be enclosed in quotation marks "..."
 * - integer and floating point values must be converted into a string
 * - boolean properties are 1 (TRUE) and 0 (FALSE)
 * - vectors elements are separated using , (comma)
 * \param s [in] the session handle
 * \param name [in] the name of the property to set
 * \param value [in] the new property value
 * \param remarks [out] received comment lines from instrument (optional).
 * \param remarks_count [out] number of comment lines from instrument.
 * \param status [out] control port status (optional). Format is "<...>".
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_set_property(
    ctrlifc_session s,
    ctrlifc_csz property,
    ctrlifc_csz value,
    ctrlifc_cszlist* remarks,
    ctrlifc_int32_t* remarks_count,
    ctrlifc_csz* status
);

/**************************************************************************//**
 * Get a property value.
 *
 * The get_property function returns the value of a property.
 * The value is returned as string value:
 * - string properties are enclosed in quotation marks "..."
 * - integer and floating point values must be converted
 * - boolean properties are strings 1 (TRUE) and 0 (FALSE)
 * - vectors elements are separated using , (comma)
 * \param s [in] the session handle
 * \param name [in] the name of the property to retrieve
 * \param value [out] the property value as string
 * \param remarks [out] received comment lines from instrument (optional).
 * \param remarks_count [out] number of comment lines from instrument.
 * \param status [out] control port status (optional). Format is "<...>".
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_get_property(
    ctrlifc_session s,
    ctrlifc_csz property,
    ctrlifc_csz* value,
    ctrlifc_cszlist* remarks,
    ctrlifc_int32_t* remarks_count,
    ctrlifc_csz* status
);

/**************************************************************************//**
 * Execute a command (function call).
 *
 * The execute_command function calls a function of the scanner interface.
 * The value is returned as string value:
 * - string properties are enclosed in quotation marks "..."
 * - integer and floating point values must be converted
 * - boolean properties are strings 1 (TRUE) and 0 (FALSE)
 * - vectors elements are separated using , (comma)
 * \param s [in] the session handle
 * \param command [in] the name of the command/function to execute/call
 * \param arguments [in] the arguments for the function.
 *   Specify "" for no arguments to be passed to command.
 * \param result [out] result value as string (optional).
 *   Vectors are returned as value list separated by , (comma).
 *   In case of failure, this parameter contains the error message.
 * \param remarks [out] received comment lines from instrument (optional).
 * \param remarks_count [out] number of comment lines from instrument.
 * \param status [out] control port status (optional). Format is "<...>".
 * \return 0 for success, !=0 for failure
 *****************************************************************************/
CTRLIFC_API int
ctrlifc_execute_command(
    ctrlifc_session s,
    ctrlifc_csz command,
    ctrlifc_csz arguments,
    ctrlifc_csz* result,
    ctrlifc_cszlist* remarks,
    ctrlifc_int32_t* remarks_count,
    ctrlifc_csz* prompt
);

#ifdef __cplusplus
}
#endif

#endif /* CTRLIFC_H */
