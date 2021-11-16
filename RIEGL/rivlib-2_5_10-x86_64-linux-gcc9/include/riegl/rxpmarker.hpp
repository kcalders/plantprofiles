// $Id$

//!\file rxpmarker.hpp
//! The decoder (and encoder) marker buffer classes.

#ifndef RXPMARKER_HPP
#define RXPMARKER_HPP

#include <riegl/config.hpp>
#include <riegl/connection.hpp>
#include <riegl/buffer.hpp>

#ifndef SCANLIB_RXPMARKER_ELEMENT_TYPE
#   define SCANLIB_RXPMARKER_ELEMENT_TYPE scanlib::uint32_t
#endif

#include <string>
#include <cstddef>
#include <cstdio>
#include <stdexcept>
#include <memory>

namespace scanlib {
//! INTERNAL ONLY
class RIVLIB_API basic_rxpmarker
{
public:
    typedef SCANLIB_RXPMARKER_ELEMENT_TYPE value_type;
    typedef std::size_t size_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef std::iterator_traits<iterator>::difference_type difference_type;

    virtual ~basic_rxpmarker();

    virtual uint16_t get(buffer& b) = 0;
    virtual void put(buffer& b) = 0;

protected:
    basic_rxpmarker(
        size_type size
        , size_type headroom
        , bool with_preamble);

    void attempt_to_resize(
        size_type new_size
        , size_type new_headroom
    );

    inline iterator next_rxpmarker_iterator(iterator it)
    {
        iterator r = ++it;
        if (it >= buf+size-headroom)
            r = buf;
        return r;
    }

    size_type size;        // total fifo size
    size_type headroom;    // headroom >= max expected packet size
    iterator buf;

    bool put_blocked;
    bool get_blocked;

    // fifo is a ringbuffer from "buf" to "buf + size - headroom"
    // fifo_in, fifo_out and fifo_eoi are managed with atomic access
    // -> no expensive mutex used.
    iterator fifo_in;   // fifo in
    iterator fifo_out;  // fifo out; in==out -> empty; in+1==out -> full
    long fifo_eoi;      // end of input, the fifo will not get new data

    bool process_preamble;
public:
    static value_type preamble[];
    static size_t preamble_size;

};

//!\brief INTERNAL ONLY
class RIVLIB_API encoder_rxpmarker
    : public basic_rxpmarker
{
public:

    encoder_rxpmarker(
        basic_wconnection& wconnection
        , size_type size = 131072
        , size_type headroom = 65536
        , bool with_preamble = true
    );

    encoder_rxpmarker(
        std::shared_ptr<basic_wconnection> wconnection
        , size_type size = 131072
        , size_type headroom = 65536
        , bool with_preamble = true
    )
        : basic_rxpmarker(size, headroom, with_preamble)
        , wconnection_(0)
        , encoder_pos(0)
    {
        encoder_pos      = buf;
        owned_wconnection = wconnection;
        wconnection_ = owned_wconnection.get();
    }

    ~encoder_rxpmarker();

    void eot();

    void flush();

    uint16_t get(buffer& b);
    void put(buffer& b);

private:
    basic_wconnection* wconnection_;
    buffer auto_buffer;
    std::shared_ptr<basic_wconnection> owned_wconnection;

    size_type write(
        const_iterator begin
        , size_type count
    );

    iterator encoder_pos;
};

//! The RXP decoder class
/*!
    This class consumes an escaped raw binary input stream and provides
    binary packets that are properly segemented by the package markers
    that designate the package boundaries. The class also undoes the
    escaping that was inserted on the transmitting side.
 */
class RIVLIB_API decoder_rxpmarker
    : public basic_rxpmarker
{
public:
    typedef uint64_t pos_type;//!< INTERNAL ONLY

    //! This constructor accepts a rconnection by reference.
    //! \param rconnection the source data connection
    //! \param size internal buffer space for packets (leave default)
    //! \param headroom internal headroom for packet unescaping (leave default)
    //! \param with_preamble do not skip preample (leave default)
    decoder_rxpmarker(
        basic_rconnection& rconnection
        , size_type size = 1024
        , size_type headroom = 340
        , bool with_preamble = true
    )
        : basic_rxpmarker(size, headroom, with_preamble)
        , rconnection_(&rconnection)
        , end_of_input(false)
        , beyond_end_of_input(false)
        , decoder_pos(0)
        , read_pos(0)
    {
        decoder_pos = buf;
    }

    //! This constructor accepts an rconnection as a shared pointer
    //! Use this constructor to delegate management of object lifetime
    //! to the decoder.
    //! \param rconnection the source data connection
    //! \param size internal buffer space for packets (leave default)
    //! \param headroom internal headroom for packet unescaping (leave default)
    //! \param with_preamble do not skip preample (leave default)
    decoder_rxpmarker(
        std::shared_ptr<basic_rconnection> rconnection
        , size_type size = 1024
        , size_type headroom = 340
        , bool with_preamble = true
    )
        : basic_rxpmarker(size, headroom, with_preamble)
        , rconnection_(0)
        , end_of_input(false)
        , beyond_end_of_input(false)
        , decoder_pos(0)
        , read_pos(0)
    {
        decoder_pos = buf;
        owned_rconnection = rconnection;
        rconnection_ = owned_rconnection.get();
    }

    //! alternative form for end of input testing
    //!\param none
    operator const void*() const
        { return end_of_input?0:this; }

    //! return true if end of input has been reached
    //!\param none
    bool eoi() const
        { return end_of_input; }

    //! get the next available binary data packet
    //!\param b a buffer proxy
    uint16_t get(buffer& b);
    void put(buffer& b); //!< INTERNAL ONLY

    //! INTERNAL ONLY
    uint16_t drain(void* buffer, uint16_t count);

    //! INTERNAL ONLY
    pos_type tellg() const
        { return read_pos; }

    //! INTERNAL ONLY
    void seekg(pos_type pos);
        
private:
    basic_rconnection* rconnection_;
    buffer auto_buffer;
    std::shared_ptr<basic_rconnection> owned_rconnection;

    size_type read(
        iterator begin
        , size_type count
    );//!< INTERNAL ONLY

    void fill(); //!< INTERNAL ONLY

    bool end_of_input;//!< INTERNAL ONLY
    bool beyond_end_of_input;
    iterator decoder_pos;
    pos_type read_pos;
};

} // namespace scanlib

#endif // RXPMARKER_HPP
