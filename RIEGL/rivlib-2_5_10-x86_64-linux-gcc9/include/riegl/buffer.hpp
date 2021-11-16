// $Id$

#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <riegl/config.hpp>

#include <iterator>

//! The basic data type for RXP streams. Need to be 32 bit.
#ifndef SCANLIB_RXPMARKER_ELEMENT_TYPE
#   define SCANLIB_RXPMARKER_ELEMENT_TYPE scanlib::uint32_t
#endif

namespace scanlib {

//!\brief buffer class
//!\details The buffer class, despite its name is only a reference to a memory range
//! within the decoder buffer. The purpose of this is to avoid unnecessary copies of the
//! data.
template<class it>
class RIVLIB_API basic_buffer
{
public:
    typedef typename std::iterator_traits<it>::value_type value_type;
    typedef std::size_t size_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef typename std::iterator_traits<iterator>::difference_type difference_type;

    basic_buffer()
        : begin_(0)
        , end_(0)
        , max_end_(0)
    {}
    ~basic_buffer()
    {}

    //!\return iterator to begin of buffer
    inline iterator begin()
        { return begin_; }
    //!\return iterator to end of buffer
    inline iterator end()
        { return end_;   }

    //!\return const iterator to begin of buffer for reading
    inline const_iterator begin() const
        { return begin_; }
    //!\return const iterator to end of buffer space for reading
    inline const_iterator end() const
        { return end_;   }

    //!\return size of buffer space
    inline size_type size() const
        { return end_ - begin_;     }
    //!\return  capacity of buffer space
    inline size_type max_size() const
        { return max_end_ - begin_; }
    //!\return buffer is empty indicator
    inline bool empty() const
        { return (begin_ == end_); }

    //! resize the buffer, adjust begin and end
    //!\param s new size
    //!\param z optional initialization value
    inline void resize(
        size_type s
        , value_type z = value_type()
    )
    {
        if (begin_+s < end_) {
            end_ = begin_+s;
        }
        else if (begin_+s <= max_end_) {
            for ( ; end_ != begin_+s; ++end_)
                *end_ = z;
        }
        else
             throw(std::out_of_range("basic_buffer::resize"));
    }

    // implementation interface

    //! INTERNAL ONLY
    iterator begin_;
    //! INTERNAL ONLY
    iterator end_;
    //! INTERNAL ONLY
    iterator max_end_;

private:
    //! INTERNAL ONLY
    basic_buffer(const basic_buffer&);

    //! INTERNAL ONLY
    const basic_buffer& operator=(const basic_buffer&);
};

//! specialisation of basic_buffer for most common use
typedef basic_buffer<SCANLIB_RXPMARKER_ELEMENT_TYPE*> buffer;

} // namespace scanlib

#endif //BUFFER_HPP
