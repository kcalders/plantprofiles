// $Id$

#ifndef COMPRESSED_HPP
#define COMPRESSED_HPP

#include <riegl/ridataspec.hpp>

namespace scanlib {

class RIVLIB_API compressed_packets
    :  public virtual basic_packets
{
public:
    compressed_packets(bool suppress_waveforms = false);
    virtual ~compressed_packets();

    unsigned decompressed_sequel;

protected:
    void on_packed_shot_echos_hr(const packed_shot_echos_hr<iterator_type>& p);
    void on_packed_shot_echos_sbl_hr(const packed_shot_echos_sbl_hr<iterator_type>& p);
    void on_packed_key_laser_shot_2angles(const packed_key_laser_shot_2angles<iterator_type>& p);
    void on_packed_key_laser_shot_2angles_rad(const packed_key_laser_shot_2angles_rad<iterator_type>& p);
    void on_packed_key_laser_shot_2angles_hr(const packed_key_laser_shot_2angles_hr<iterator_type>& p);
    void on_packed_frame_laser_shot_2angles(const packed_frame_laser_shot_2angles<iterator_type>& p);
    void on_packed_frame_laser_shot_2angles_rad(const packed_frame_laser_shot_2angles_rad<iterator_type>& p);
    void on_packed_frame_laser_shot_2angles_hr(const packed_frame_laser_shot_2angles_hr<iterator_type>& p);
    void on_packed_key_echo(const packed_key_echo<iterator_type>& p);
    void on_packed_key_echo_hr(const packed_key_echo_hr<iterator_type>& p);
    void on_packed_key_echo_hr_1(const packed_key_echo_hr_1<iterator_type>& p);
    void on_packed_frame_echo(const packed_frame_echo<iterator_type>& p);
    void on_packed_frame_echo_hr(const packed_frame_echo_hr<iterator_type>& p);
    void on_packed_frame_echo_hr_1(const packed_frame_echo_hr_1<iterator_type>& p);
    bool on_dispatch(const iterator_type& begin, const iterator_type& end);

private:
    struct impl;
    impl* pimpl;

    // not copyable
    compressed_packets(const compressed_packets& rhs);
    compressed_packets& operator=(const compressed_packets& rhs);
};

}

#endif // COMPRESSED_HPP
