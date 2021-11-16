// $Id$

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

//!\file connection.hpp
//! The abstract base classes for read and write connection protocols.
 
#include <riegl/config.hpp>

#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <ctime>
#include <memory>
#include <vector>

namespace scanlib {

//!\brief exception, thrown when cancelled
class RIVLIB_API cancelled
    : public std::runtime_error
{
public:
    //! \param[in] message A description of the cancellation request.
    explicit cancelled(const std::string& message)
        : std::runtime_error(message)
    {}
    virtual ~cancelled() throw()
    {}
};

//!\brief base class for connections
//!\details Basic connection class for read and write protocols.
class RIVLIB_API basic_connection
{
public:
    typedef std::size_t     size_type;
    typedef std::uintmax_t  pos_type;

    //! The open and close may be implemented by a derived object
    //! where the constructor needs to block to establish
    //! or shut down the connection.
    virtual void
    open(
    ) {
    }
    virtual void
    close(
    ) {
    }

    //! Cause a blocked read or write to throw an exception of
    //! type "cancelled".
    virtual void
    cancel(
    ) {
    } //!<cancel outstanding operation

    virtual ~basic_connection() {}

    bool
    eoi(
    ) const {
        return is_eoi;
    }

    operator void*(
    ) {
        return is_eoi?0:this;
    }

public:
    std::string id; //!< An id describing the connection, e.g. the file name from the remote side.

protected:
    bool is_eoi;

    //! The default constructor is protected
    //! to enforce derivation from this class.
    //!\post id is initialized to a default value of "UNNAMED_<year><month><day>_<hour><minute><second>"
    basic_connection(
    )   : is_eoi(false)
    {
        // set a default id
        char t[8+13+1];
        time_t now = time(0);
        strftime(t, 8+13+1, "UNNAMED_%y%m%d_%H%M%S", gmtime(&now));
        id = t;
    }

private:
    // Prohibit copying of connection object.
    basic_connection(const basic_connection&);
    const basic_connection& operator=(const basic_connection&);
};


//!\brief abstract protocol class for read connections.
//!\details This is the abstract base class for read connections
class RIVLIB_API basic_rconnection
    :  public std::enable_shared_from_this<basic_rconnection>, virtual public basic_connection
{
    class impl; std::unique_ptr<impl> pimpl;

public:

    //! virtual constructor
    //!\param uri connection uri
    //!\param continuation resume information for aborted transfers
    //!\return connection class matching the protocol specified in uri
    static std::shared_ptr<basic_rconnection>
    create(
        const std::string& uri
        , const std::string& continuation = std::string()
        , const std::string& parameters = std::string()
    )
    { return std::shared_ptr<basic_rconnection>(create_impl(uri, continuation, parameters)); }

    //! Read at maximum count bytes into user provided buffer.
    //! Function may block if no bytes available.
    //!\throw scanlib::cancelled
    //!\param buf user provided buffer
    //!\param count size of buffer in bytes
    //!\return number of actually read bytes or zero if sending
    //!        side has finished sending.
    size_type
    readsome(
        void* buf
        , size_type count
    );

    //! Read count bytes into user provided buffer.
    //! Function will block if no bytes available.
    //!\throw scanlib::cancelled
    //!\param buf user provided buffer
    //!\param count size of buffer in bytes
    //!\return a reference to the read object.
    basic_rconnection&
    read(
        void* buf
        , size_type count
    );

    //!\return return the number of bytes read during last succesful read
    size_type
    gcount(
    ) const {
        return read_count;
    }

    //!\return total number of octets already read
    virtual pos_type
    tellg(
    ) const {
        return read_pos;
    }

    //! The number of octetts of the stream if its file based, else zero.
    pos_type
    size() const {
        return max_read_pos;
    }

    //! Seek to a possition that has been retrieved with tellg if the
    //! stream is file based, else throws
    virtual basic_rconnection&
    seekg(
        pos_type pos
    );
    
    //! Set a filname for data logging. The rxp stream will
    //! be logged in binary to this file.
    //!\param name filname of logfile
    void
    open_logfile(
        const std::string& name
        , std::intmax_t split = INTMAX_MAX
    );

    //! Flush the logfile
    void
    flush_logfile(
    );

    //! Close the logfile
    void
    close_logfile(
    );

    //! Get context to continue an aborted transfer.
    //! The returned string can be feed into create to resume a transfer.
    //!\return the continuation string
    virtual std::string continuation() const
    { return std::string(); }

    //! Signal a request to stop sending to the remote peer.
    //! After requesting a shutdown, the application is expected
    //! to read data until end of file is detected.
    virtual void request_shutdown() {}

    ~basic_rconnection(
    );

protected:
    basic_rconnection(
        const std::string& parameters = std::string()
    );

    virtual size_type
    more_input(
        void* buf
        , size_type count
    ) = 0;

    void
    initialize(
        const std::string& parameters
    );

    size_type read_count;
    pos_type  read_pos;
    pos_type max_read_pos;
    //std::vector<char> rxp_parameters_buffer;
    std::string parameters;

private:
    //! INTERNAL ONLY
    static basic_rconnection*
    create_impl(
        const std::string& uri
        , const std::string& continuation = std::string()
        , const std::string& parameters = std::string()
    );

    bool is_preamble_checked;
};

//!\brief INTERNAL ONLY
class RIVLIB_API basic_wconnection
    : virtual public basic_connection
{
public:
    // virtual constructor
    static std::shared_ptr<basic_wconnection>
    create(
        const std::string& uri
    )
    { return std::shared_ptr<basic_wconnection>(create_impl(uri)); }

    // Write at maximum count bytes from user provided buffer.
    // Return number of actually written bytes.
    // Function may block if it cannot send at least one byte.
    virtual size_type write(
        const void* buf
        , size_type count
    ) = 0;

    // Signal sending has finished, i.e. no more calls to write
    // will follow.
    virtual void shutdown() {}

    // Callback function that can be called by an implementation
    // of connection to signal the user a request to initiate
    // a graceful shutdown.
    // Function must not block and may be invoked from another
    // thread than read or write.
    //std::tr1::function<void (void)> on_shutdown_request;

protected:
    basic_wconnection() {};

private:
    static basic_wconnection*
    create_impl(
        const std::string& uri
    );
};

//!\brief INTERNAL ONLY
class RIVLIB_API basic_rwconnection
    : public basic_rconnection
    , public basic_wconnection
{
    // virtual constructor
    static std::shared_ptr<basic_rwconnection>
    create(
        const std::string& uri
    )
    { return std::shared_ptr<basic_rwconnection>(create_impl(uri)); }

protected:
    basic_rwconnection() {};

private:
    static basic_rwconnection*
    create_impl(
        const std::string& uri
    );
};

} // namespace scanlib



#endif // CONNECTION_HPP
