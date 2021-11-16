// $Id$

//!\file rmsmarker.hpp
//! The decoder marker classes for multiplexed streams.

#ifndef RMSMARKER_HPP
#define RMSMARKER_HPP

#include <riegl/config.hpp>
#include <riegl/connection.hpp>
#include <riegl/buffer.hpp>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

//#ifndef RIVLIB_TR1_DIR
//#   if defined(_MSC_VER)
//#       include <memory>
//#   else
//#       include <tr1/memory>
//#   endif
//#else
//#   include <tr1/memory>
//#endif


namespace scanlib {

//! The RMS decoder class
/*!
    This class demultiplexes several rxp streams from an rms stream.
    Then each rxp stream is processed as an escaped raw binary
    input stream and binary packets that are properly
    segemented by the package markers are provided on the output.
    The class also undoes the  escaping that was inserted on the
    transmitting side.
    Each package is retuned together with a number denoting the
    rxp stream it is belonging to.
 */
class RIVLIB_API decoder_rmsmarker
{
public:
    typedef uint64_t pos_type;//!< INTERNAL ONLY
    typedef uint32_t value_type; //!< INTERNAL ONLY
    typedef value_type* iterator; //!< INTERNAL ONLY

    //! array of job names
    std::map<uint16_t, std::string>         jobs;
    //! array of joob identifier names as fetched from the original device
    std::map<uint16_t, std::string>         ids;

    //! This constructor accepts an rconnection as a shared pointer
    //! Use this constructor to delegate management of object lifetime
    //! to the decoder.
    //! \param rconnection_ the source data connection
    //! \param max_jobs_ the maximum number of different jobs that will be
    //!        be returned. If set to zero all jobs will be returned.
    //! \param jobs_ a list of jobnames to filter the returned jobs.
    //!        The order of names given will correspond with the jobnr
    //!        returned in the get function.
    //! \param with_preamble do not skip preample (leave default)
    decoder_rmsmarker(
        std::shared_ptr<basic_rconnection> rconnection_
        , unsigned max_jobs_ = 0
        , std::vector<std::string> jobs_ = std::vector<std::string>()
    )   :   rxp_mode(false)
            , end_of_data(false)
            , end_of_input(false)
            , preamble_once(true)
            , read_pending(0)
            , max_jobs(max_jobs_)
            , read_pos(0) {
        rconnection = rconnection_;
        impl_rconnection = rconnection.get();

        for (uint16_t n=0; n<jobs_.size(); ++n) {
            delivered_jobs[jobs_[n]] = n;
            jobs[n] = jobs_[n];
        }
        read_buffer.reserve(8192);  // must be smaller than 64k and larger than
                                    // preamble size !!
        read_ptr = read_buffer.end();
        current = processed_contexts.end();
    }

    //! alternative form for end of input testing
    //!\param none
    operator const void* (
    ) const {
        return end_of_input?0:this;
    }

    //! return true if end of input has been reached
    //!\param none
    bool
    eoi(
    ) const {
        return end_of_input;
    }

    //! get the next available binary data packet
    //!\param b a buffer proxy
    //!\return jobnr of packet
    uint16_t
    get(
        buffer& b
    );

    //! INTERNAL ONLY
    uint16_t
    drain(
        void* buf
        , uint16_t count
    );

    //! INTERNAL ONLY
    pos_type tellg() const;
    //    { return read_pos-4; } //fixme: the -4 is not strictly correct while in preamble

private:
    //! INTERNAL ONLY
    struct context {
        context(
            uint16_t nr_ = 0
        )   : nr(nr_)
            , pos(0)
            , need_start(true)
            , first_iteration(false)
            , unescape(false)
            , eoi(false)
            , preamble_once(true) {
            packet_buffer.reserve(1024);
        }

        uint16_t                nr;
        unsigned char           value[4];
        unsigned char           pos;
        bool                    need_start;
        bool                    first_iteration;
        bool                    unescape;
        bool                    eoi;
        bool                    preamble_once;
        std::vector<value_type> packet_buffer;
        std::vector<char>       preamble_buffer;
    };

    basic_rconnection*                          impl_rconnection;
    std::shared_ptr<basic_rconnection>     rconnection;
    bool                                        rxp_mode;
    bool                                        end_of_data;
    bool                                        end_of_input;
    bool                                        preamble_once;
    std::vector<unsigned char>                  read_buffer;
    uint16_t                                    read_pending;
    std::vector<unsigned char>::const_iterator  read_ptr;
    std::map<uint16_t, context>                 processed_contexts;
    std::map<uint16_t, context>::iterator       current;
    std::map<std::string, uint16_t>             delivered_jobs;
    std::map<uint16_t, std::string>             discovered_jobs;
    unsigned                                    max_jobs;
    std::set<uint16_t>                          skipped_nr;
    pos_type                                    read_pos;
    //! INTERNAL ONLY
    void
    underflow(
    );
};

} // namespace rivlib

#endif //RMSMARKER_HPP
