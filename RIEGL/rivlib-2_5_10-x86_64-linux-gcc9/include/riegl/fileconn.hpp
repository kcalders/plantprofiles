// $Id$

//!\file fileconn.hpp
//! The file connection classes

#ifndef FILECONN_HPP
#define FILECONN_HPP

#include <riegl/connection.hpp>

#include <cstdio>
#include <string>
#include <csignal>
#include <memory>
#include <locale>

namespace scanlib {

//!\brief the file connection class
class RIVLIB_API file_rconnection
    : public basic_rconnection
{
    class impl; std::unique_ptr<impl> pimpl;
    friend class basic_rconnection; // for create

public:

    ~file_rconnection();

    //! Seek to a possition that has been retrieved with tellg
    file_rconnection&
    seekg(
        pos_type pos
    );

    //! The cancel request
    //! note: this function is not really useful for files
    void cancel();
    //! The shutdown request
    //! note: this function is not really useful for files
    void request_shutdown();

protected:
    //! constructor for file connection
    //!\param file_uri a file specifier e.g. file:filename.rxp
    //!\param continuation not applicable to file connections
    file_rconnection(
        const std::string& file_uri
        , const std::string& /*continuation = std::string()*/
        , const std::string& parameters = std::string()
    );

    virtual size_type more_input(
        void* buf
        , size_type count
    );

private:
    std::string filename;
    std::sig_atomic_t is_cancelled;
};

//!\brief INTERNAL ONLY
class RIVLIB_API file_wconnection
    : public basic_wconnection
{
    class impl; std::unique_ptr<impl> pimpl;
    friend class basic_wconnection;

public:

    ~file_wconnection();

    virtual size_type write(
        const void* buf
        , size_type count
    );

protected:
    file_wconnection(
        const std::string& file_uri
    );

private:
    FILE* file;
};

//! Set the locale to use for path names
std::locale file_connection_imbue(const std::locale& loc);

} // namespace scanlib

#endif // FILECONN_HPP
