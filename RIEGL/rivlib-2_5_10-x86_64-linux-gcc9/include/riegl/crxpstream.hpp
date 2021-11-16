// $Id$

#ifndef CRXPSTREAM_HPP
#define CRXPSTREAM_HPP

#include <string>
#include <riegl/rxpstream.hpp>

namespace scanlib {

class crxp_istream
{
public:

    typedef rxp_istream::pos_type pos_type;
    
    crxp_istream(const char* uri);
    crxp_istream(const std::string& uri);
    ~crxp_istream();

    operator void*();
    bool good() const;

    std::shared_ptr<basic_rconnection> rdcon() const;

    crxp_istream& operator>>(package_id& x);
    crxp_istream& operator>>(rxp_packet& p);

    template<class P>
    crxp_istream& operator>>(P& p) {
        rxp_packet rxp;
        operator>>(rxp);
        if (P::id_main == rxp.id_main && P::id_sub == rxp.id_sub) {
            p = rxp_cast<P>(rxp);
        } else
            throw std::runtime_error("crxp_istream: id mismatch");
        return *this;
    }

    pos_type tellg() const;
    
private:
    struct impl;
    impl* pimpl;

    // not copyable
    crxp_istream(const crxp_istream& rhs);
    crxp_istream& operator=(const crxp_istream& rhs);
};

class crxp_ostream
{
public:
    crxp_ostream(const char* uri);
    crxp_ostream(const std::string& uri);
    ~crxp_ostream();
    
    operator void*();
    bool good() const;

    crxp_ostream& operator<<(const package_id& x);
    crxp_ostream& operator<<(const rxp_packet& p);

    template<class P>
    crxp_ostream& operator<<(const P& p) {
        operator<<(rxp_packet(p));
        return *this;
    }

private:
    struct impl;
    impl* pimpl;

    // not copyable
    crxp_ostream(const crxp_ostream& rhs);
    crxp_ostream& operator=(const crxp_ostream& rhs);
};

} // namespace scanlib

#endif // CRXPSTREAM_HPP
