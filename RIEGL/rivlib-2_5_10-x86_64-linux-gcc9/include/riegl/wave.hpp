// $Id$
#ifndef WAVE_HPP
#define WAVE_HPP

#include <riegl/pointcloud.hpp>
#include <riegl/detail/clockcomp.hpp>

#include <vector>
#include <deque>
#include <ostream>
#include <complex>
#include <bitset>

namespace scanlib {

class wave
    : public scanlib::pointcloud
{
public:

    wave(bool sync_to_pps = false, bool decompress = false);
    ~wave();

    struct channel_t {
        std::string name;
        double sampling_time;
        unsigned adc_resolution_bits;
        double wavelength;
        double delta_st;
        uint64_t sysmod;
        double a_lin;
        double delay;
        double scale;
        std::vector<std::complex<double>> expsum_a;
        std::vector<std::complex<double>> expsum_b;
    };
    
    std::bitset<16> channel_filter;
    std::vector<channel_t> channel_properties;
    std::size_t reference_channel_index;
    double t_null_pdae;

    struct lookup_table {
        double abscissa_scale;
        double abscissa_offset;
        double ordinate_scale;
        double ordinate_offset;
        std::vector<int16_t> abscissa;
        std::vector<int16_t> ordinate;
        double operator()(double x);
    };

    lookup_table amplitude_low_power;
    lookup_table amplitude_high_power;
    lookup_table position_diff_low_power;
    lookup_table position_diff_high_power;

protected:

    struct wavelet{
        wavelet() : time(0), channel(), compressed(false) {}
        double time;
        std::size_t channel;
        bool compressed;
        std::vector<uint16_t> data;
    };
    struct waveform {
        double time_sorg;
        double time;
        double origin[3];
        double direction[3];
        unsigned facet;
        double line_angle;
        double frame_angle;
        bool pps;

        std::vector<wavelet> data;
        struct statistic_t { float mean; float stdev; };
        std::vector<statistic_t> channel_statistic;
    };
    virtual void on_wave(
        waveform& wfm
    ) = 0;

    void write_meta_component(std::ostream& out);

    void on_check_parameters_before_measurement_starting();
    void on_measurement_starting();

    void on_rxp_parameters(
        const rxp_parameters<iterator_type>& arg
    );
    void on_units(
        const units<iterator_type>& arg
    );
    void on_units_2(
        const units_2<iterator_type>& arg
    );
    void on_units_3(
        const units_3<iterator_type>& arg
    );
    void on_pps_sync(
        const pps_sync<iterator_type>& arg
    );
    void on_pps_sync_hr(
        const pps_sync_hr<iterator_type>& arg
    );
    void on_sbl_dg_parameters(
        const sbl_dg_parameters<iterator_type>& arg
    );
    void on_sbl_dg_header(
        const sbl_dg_header<iterator_type>& arg
    );
    void on_sbl_dg_data(
        const sbl_dg_data<iterator_type>& arg
    );
    void on_sbl_dg_data_compressed(
        const sbl_dg_data_compressed<iterator_type>& arg
    );
    void on_sbl_dg_header_hr(
        const sbl_dg_header_hr<iterator_type>& arg
    );
    void on_sbl_dg_data_hr(
        const sbl_dg_data_hr<iterator_type>& arg
    );
    void on_sbl_dg_data_compressed_hr(
        const sbl_dg_data_compressed_hr<iterator_type>& arg
    );
    void on_sbl_dg_channel(
        const sbl_dg_channel<iterator_type>& arg
    );
    void on_sbl_dg_channel_header(
        const sbl_dg_channel_header<iterator_type>& arg
    );
    void on_sbl_dg_channel_header_1(
        const sbl_dg_channel_header_1<iterator_type>& arg
    );
    void on_sbl_dg_channel_expsum(
        const sbl_dg_channel_expsum<iterator_type>& arg
    );
    void on_sbl_dg_channel_expsum_1(
        const sbl_dg_channel_expsum_1<iterator_type>& arg
    );
    void on_sbl_dg_channel_data(
        const sbl_dg_channel_data<iterator_type>& arg
    );
    void on_sbl_dg_channel_data_compressed(
        const sbl_dg_channel_data_compressed<iterator_type>& arg
    );
    void on_shot_end(
    );
    void on_calib_table(
        const calib_table<iterator_type>& arg
    );
    void on_pulse_model_expsum(
        const pulse_model_expsum<iterator_type>& arg
    );

private:
    struct implementation;
    implementation* const priv;
    friend struct implementation;

    std::pair<double,double> times;
    double systime_hr_unit;
    unsigned systime_hr_bits;
    double systime_unit;

    std::vector<uint16_t> data_compressed;
    detail::clock_compensator wave_clock;
    std::deque<waveform*> fifo;
    bool have_wave;
    bool decompress;
    bool sbl_dg_parameters_seen; // can possibly removed
    
    void fill_lookup_table(lookup_table& table, const calib_table<iterator_type>& arg);

    uint64_t sysmod;

};

} // end namespace wavelib

#endif // WAVE_HPP
