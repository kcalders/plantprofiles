// $Id$

#ifndef RDDPCONN_HPP
#define RDDPCONN_HPP

#include <riegl/connection.hpp>

#include <string>

namespace scanlib {
//*****************************************************************************
//  rddp_rconnection
//*****************************************************************************
class RIVLIB_API rddp_rconnection
    : public basic_rconnection
{
    friend class basic_rconnection;

public:

    ~rddp_rconnection();

    void open();
    void close();
    void cancel();
    void request_shutdown();

protected:

    rddp_rconnection(
        const std::string& rddp_uri
        , const std::string& continuation = std::string()
        , const std::string& parameters = std::string()
    );

    size_type more_input(
        void* buf
        , size_type count
    );

private:
    class impl;
    impl* pimpl;
};

} // namespace scanlib


#endif // RDDPCONN_HPP
