// $Id$

#ifndef POINTCLOUD_HPP
#define POINTCLOUD_HPP

//! \file pointcloud.hpp
//! The pointcloud extractor and helper classes.


#include <riegl/config.hpp>
#include <riegl/ridataspec.hpp>
#include <riegl/detail/clockcomp.hpp>
#include <riegl/compressed.hpp>
#include <riegl/scanmech.hpp>

#include <string>
#include <ostream>
#include <vector>
#include <limits>
#include <map>
#include <utility>

namespace scanlib {

//! geometric data and attributes of a point
/*! This class holds the geometric x,y,z coordinates of a measured
    point and its attributes such as amplitude and reflectance.
 */
class RIVLIB_API target
{
public:
    target() :
        echo_range(0)
        , time(0)
        , time_sorg(0)
        , amplitude(0)
        , reflectance(0)
        , deviation(0)
        , background_radiation(std::numeric_limits<float>::quiet_NaN())
        , is_sw(false)
        , is_high_power(false)
        , is_rising_edge(false)
        , is_pps_locked(false)
        , facet(0)
        , segment(0)
    {
        vertex[0] = vertex[1] = vertex[2] = 0.0;
    }

    //! x-y-z coordinates
    /*! Cartesian coordinates in scanners own coordinate system.
        vertex[i] = beam_origin[i] + echo_range * beam_direction[i]
     */
    float vertex[3];

    //! echo range in units of meter
    /*! The distance from the laser origin to the target. Note that this
        is not the same as sqrt(vertex[0]^2 + vertex[1]^2 + vertex[2]^2).
     */
    double echo_range;

    //! zone index, zero offset
    /*! The zone this target belongs to. Note: zone == zone_index + 1
     */
    unsigned short zone_index;
    
    //! time stamp in [s]
    /*! The epoch (i.e. where time equals zero) depends on a flag
        that has to be supplied in the constructor of the pointcloud object.
     */
    double time;

    //! time of start of rangegate in [s]
    /*! The timestamp of the start of the rangegate (internal time).
     */
    double time_sorg;

    //! relative amplitude in [dB]
    /*! This is the ratio of the received power to the power received at
       the detection limit expressed in dB.
     */
    float amplitude;

    //! relative reflectance in [dB]
    /*! This is the ratio of the received power to the power that would
        be received from a white diffuse target at the same distance
        expressed in dB. The reflectance represents a range independent
        property of the target.  The surface normal of this target is assumed
        to be in parallel to the laser beam direction.
     */
    float reflectance;

    //! a measure of pulse shape distortion
    /*! A larger value for deviation indicates larger distortion.
      */
    float deviation;

    //! a measure of background radiation
    /*! A measure of background radiation. This information is optional.
        For instruments that do not deliver this value, the field is set
        to NaN.
     */
    float background_radiation;

    //! INTERNAL ONLY
    bool is_sw;             // generated by software full wave analysis

    //! INTERNAL ONLY
    bool is_high_power;     // high power channel flag

    //! INTERNAL ONLY
    bool is_rising_edge;    // edge flag

    //! time is a valid pps time
    /*! The external PPS signal was found to be synchronized at the time of
        the current laser shot.
     */
    bool is_pps_locked;

    //! facet number
    /*! The current facet of the polygonal mirror
     */
    unsigned facet;   // facet number

    //! segment number
    /*! The current segment, usually some partitioning of the angle range
     *
     */
    unsigned segment; // segment number
};

//!\brief pointcloud class
//!\details The pointcloud class combines several (undocumented) raw basic
//! packets into meaningful XYZ data with calibration already applied.
//! This makes the pointcloud class the main access point to data for client
//! applications. The class provides several virtual functions that the
//! client application is expected to override.
//! For every laser echo discovered in the raw data stream from the scanning
//! device an invocation of the on_echo_transformed is done. The function
//! provides as a parameter the type of echo. Each echo unambiguously belongs
//! to one of the possible classes of type echo_type. Please note that for
//! specifying all echoes that are farthest away you need to look for both
//! single _and_ last echoes. Similar reasoning is for closest echoes. I.e.
//! specifying only last will give you the echo points that are farthest away
//! but only if they are part of multi-return.
//! The time reference for the time stamps are user selectable. PPS time stamps
//! from a GPS receiver or internal time of the scanner device can be
//! choosen.
//! The point data is accesible through the targets member variable. This
//! variable is an array that always holds all echoes that belong to the
//! same laser shot. The echo that belongs to the current call to
//! on_echo_transformed is always accesible by means of the variable
//! target_count. I.e to access the top of the array specify
//! targets[target_count-1]. This design makes it possible for the client
//! application to either act on each echo separately or evaluate all
//! echoes that belong to a laser shot at once when the last echo for this
//! shot is seen. The targets array is reset to empty with each new laser shot.
class RIVLIB_API pointcloud
    //:  public virtual basic_packets
    : public compressed_packets
{
public:
    //! the echo type classifications, each echo belongs to exactly one class
    typedef enum { single, first, interior, last, none } echo_type;

    //! the value of PI
    static const double pi;

    //! constructor
    //!\param sync_to_pps_ use external time reference for time
    pointcloud(bool sync_to_pps_ = false);

    //! constructor
    //!\param out write pointcloud data in ASCII to a stream
    //!\param sync_to_pps_ use external time reference for time
    pointcloud(std::ostream& out, bool sync_to_pps_ = false);
    virtual ~pointcloud();

    //! override the default rangegate of the sensor
    //!\param zone select the mta zone. (if all parameters are zero, select sensor default)
    //!\param near if zone=0 specifies the closer end of the rangegate
    //!\param far if zone=0 specifies the farthmost end of the rangegate
    void set_rangegate(unsigned zone, double near = 0, double far = 0);

    //! override the default rangegate of the sensor and retrieve echoes as if
    //! in the selected zones.
    //!\param zones select amount of zones and zone indices
    //!\param standard_zone select the mta zone. (same as for set_rangegate)
    //!\param near if standard_zone=0 specifies the closer end of the rangegate
    //!\param far if standard_zone=0 specifies the farthmost end of the rangegate
    void set_multi_mta_zones(const std::vector<unsigned>& multi_zones,
                        unsigned standard_zone, double near = 0, double far = 0);

    //! retrieve meta information about the scan
    //!\param out stream receiving the meta information
    void write_meta(std::ostream& out, bool pretty = false);
protected:
    //! INTERNAL ONLY
    virtual void write_meta_component(std::ostream& out);

public:
    //! INTERNAL ONLY
    void set_notch_filter(float range_min, float range_max, float amplitude_max);

    //! number of facets (instrument constant)
    unsigned num_facets;
    //! group velocity of laser beam
    double group_velocity;
    //! maximum unambiguoas range (mta zone width)
    double unambiguous_range;
    //! flag indicating the time reference to use
    bool sync_to_pps;
    //! effective digitizer sampling time
    double sampling_time;

    //! the timestamp of the current shot in sec (ref. depends on sync_to_pps)
    double time;
    //! time of start of range gate of last shot
    double time_sorg;
    //! the timestamp of the current event (counter_sync, pwm_sync) in sec (ref. depends on sync_to_pps)
    double time_event;
    //! beam direction of current shot (unity direction vector)
    double beam_direction[3];
    //! (virtual) beam origin of current shot (unit is meter)
    double beam_origin[3];
    //! mirror  facet of current shot
    unsigned facet;

    //! all targets belonging to the last laser shot
    std::vector<target> targets;
    typedef std::vector<target>::size_type target_count_type;
    //! the number of targets belonging to the last laser shot
    target_count_type target_count;

    //! retrieve targets as if not in the standard zone
    //! multi_multi_mta_zones[multi_mta_zone_index][target_index]
    std::vector<std::vector<target>> multi_mta_targets;

    // instrument descriptors
    std::string serial;  //!< scanner serial number
    std::string type_id; //!< scanner type
    std::string build;   //!< scanner build id
    std::string ident;   //!< scanner identification (e.g. in an array)

protected:

    //! callback when the measurement is about to start, i.e. metadata is ready
    //! this packet is similar to on_meas_start but not the same, since the
    //! latter is generated by the device while this one considers more cases
    virtual void on_measurement_starting();
    
    //! callback when a laser shot is available, will be invoked
    //! even if no echoes are available for this shot
    virtual void on_shot();

    //! callback after all data belonging to a shot is available
    //! i.e. all echo packets (same as last echo)
    virtual void on_shot_end();

    //! callback when a complete laser point is available
    //! the point information is accesible from the targets member
    //!\param echo the classification of the last echo
    virtual void on_echo_transformed(echo_type echo);

    //! callback to indicate that at least two regular pps pulses have been seen
    //!\param none
    virtual void on_pps_synchronized();

    //! callback to indicate that pps pulses have stopped
    //!\param none
    virtual void on_pps_sync_lost();

    virtual void on_check_parameters_before_measurement_starting(); //!< INTERNAL_ONLY
    // overridden
    void on_header(const header<iterator_type>& h); //!< INTERNAL ONLY
    void on_header_device(const header_device<iterator_type>& h); //!< INTERNAL ONLY
    void on_rxp_parameters(const rxp_parameters<iterator_type>& arg); //!< INTERNAL ONLY
    void on_units(const units<iterator_type>& arg); //!< INTERNAL ONLY
    void on_units_1(const units_1<iterator_type>& arg); //!< INTERNAL ONLY
    void on_units_2(const units_2<iterator_type>& arg); //!< INTERNAL ONLY
    void on_units_3(const units_3<iterator_type>& arg); //!< INTERNAL ONLY
    void on_units_4(const units_4<iterator_type>& arg); //!< INTERNAL ONLY
    void on_device_geometry(const device_geometry<iterator_type>& arg); //!< INTERNAL ONLY
    void on_device_geometry_1(const device_geometry_1<iterator_type>& arg); //!< INTERNAL ONLY
    void on_device_geometry_2(const device_geometry_2<iterator_type>& arg); //!< INTERNAL ONLY
    void on_device_geometry_3(const device_geometry_3<iterator_type>& arg); //!< INTERNAL ONLY
    void on_device_geometry_4(const device_geometry_4<iterator_type>& arg); //!< INTERNAL ONLY
    void on_device_geometry_5(const device_geometry_5<iterator_type>& arg); //!< INTERNAL ONLY
    void on_device_geometry_6(const device_geometry_6<iterator_type>& arg); //!< INTERNAL ONLY
    void on_device_geometry_7(const device_geometry_7<iterator_type>& arg); //!< INTERNAL ONLY
    void on_scan_segments_fov(const scan_segments_fov<iterator_type>& arg); //!< INTERNAL ONLY
    void on_pps_sync(const pps_sync<iterator_type>& arg); //!< INTERNAL ONLY
    void on_pps_sync_hr(const pps_sync_hr<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_shot(const laser_shot<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_shot_1angle(const laser_shot_1angle<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_shot_2angles(const laser_shot_2angles<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_shot_2angles_rad(const laser_shot_2angles_rad<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_shot_6angles(const laser_shot_6angles<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_shot_2angles_hr(const laser_shot_2angles_hr<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_shot_utctime_origin_direction(const laser_shot_utctime_origin_direction<iterator_type>& arg); //!< INTERNAL ONLY
    void on_counter_sync(const counter_sync<iterator_type>& arg); //!< INTERNAL ONLY
    void on_counter_sync_2angles_hr(const counter_sync_2angles_hr<iterator_type>& arg); //!< INTERNAL ONLY
    void on_pwm_sync(const pwm_sync<iterator_type>& arg); //!< INTERNAL ONLY
    void on_echo(const echo<iterator_type>& arg); //!< INTERNAL ONLY
    void on_echo_1(const echo_1<iterator_type>& arg); //!< INTERNAL ONLY
    void on_echo(int32_t range, uint16_t ampl, int16_t refl, uint16_t flags, int16_t dev); //!< INTERNAL ONLY
    void on_laser_echo(const laser_echo<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_echo_qual(const laser_echo_qual<iterator_type>& arg); //!< INTERNAL ONLY
    void on_laser_echo_sw(const laser_echo_sw<iterator_type>& arg); //!< INTERNAL ONLY
    void on_line_start_segment_1(const line_start_segment_1<iterator_type>& arg); //!< INTERNAL ONLY
    void on_line_start_segment_2(const line_start_segment_2<iterator_type>& arg); //!< INTERNAL ONLY
    void on_line_start_segment_3(const line_start_segment_3<iterator_type>& arg); //!< INTERNAL ONLY
    void on_line_stop(const line_stop<iterator_type>& arg); //!< INTERNAL ONLY
    void on_frame_stop(const frame_stop<iterator_type>& arg); //!< INTERNAL ONLY
    void on_meas_start(const meas_start<iterator_type>& arg); //!< INTERNAL ONLY
    void on_meas_stop(const meas_stop<iterator_type>& arg); //!< INTERNAL ONLY
    void on_nrange_table(const nrange_table<iterator_type>& arg); //!< INTERNAL ONLY
    void on_reftab_table(const reftab_table<iterator_type>& arg); //!< INTERNAL ONLY
    void on_arange_table(const arange_table<iterator_type>& arg); //!< INTERNAL ONLY
    void on_atmosphere_2(const atmosphere_2<iterator_type>& arg); //!< INTERNAL ONLY
    void on_atmosphere_3(const atmosphere_3<iterator_type>& arg); //!< INTERNAL ONLY
    void on_atmosphere_4(const atmosphere_4<iterator_type>& arg); //!< INTERNAL ONLY
    void on_mta_settings(const mta_settings<iterator_type>& arg); //!< INTERNAL ONLY
    void on_mta_settings_1(const mta_settings_1<iterator_type>& arg); //!< INTERNAL ONLY
    void on_m_sequence_mta(const m_sequence_mta<iterator_type>& arg); //!< INTERNAL ONLY
    void on_pulse_position_modulation(const pulse_position_modulation<iterator_type>& arg); //!< INTERNAL ONLY
    void on_pulse_position_modulation_1(const pulse_position_modulation_1<iterator_type>& arg); //!< INTERNAL ONLY
    void on_dyntrig(const dyntrig<iterator_type>& arg); //!< INTERNAL ONLY
    void on_biaxial_geometry(const biaxial_geometry<iterator_type>& arg); //!< INTERNAL ONLY
    void on_notch_filter(const notch_filter<iterator_type>& arg); //!< INTERNAL ONLY
    void on_calib_table(const calib_table<iterator_type>& arg); //!< INTERNAL ONLY
    void on_notch_filter_modification_parameters(const notch_filter_modification_parameters<iterator_type>& arg); //!< INTERNAL ONLY
    void on_range_finder_debug_laser(const range_finder_debug_laser<iterator_type>& arg); //!< INTERNAL ONLY
    void on_range_finder_settings(const range_finder_settings<iterator_type>& arg); //!< INTERNAL ONLY
    void on_range_finder_program_2(const range_finder_program_2<iterator_type>& arg); //!< INTERNAL ONLY
    void on_crc32_header(const crc32_header<iterator_type> &arg); //!< INTERNAL_ONLY
    void on_crc32_check(const crc32_check<iterator_type> &arg); //!< INTERNAL_ONLY
    void on_id(const package_id& id, const basic_package<iterator_type>& pkg); //!< INTERNAL_ONLY
    virtual void on_crc32_error(uint16_t stream, uint32_t sequence_begin, uint32_t sequence_end); //!< INTERNAL_ONLY

    void start_measurement(); //!< INTERNAL ONLY

    std::ostream* out;
    bool header_written;
    bool starting_called;     //!< INTERNAL ONLY

private:
    struct implementation;
    implementation* const priv;
    friend struct implementation;

    void init(); //!< INTERNAL ONLY
    void evaluate_shot(uint32_t systime); //!< INTERNAL ONLY
    void evaluate_shot_hr(uint64_t systime); //!< INTERNAL ONLY
    void send_first_or_interior(); //!< INTERNAL ONLY
    void send_single_or_last(); //!< INTERNAL ONLY
    void prepare_notch_filter(); //!< INTERNAL ONLY

    void calc_time(uint32_t t); //!< INTERNAL ONLY
    void calc_time_hr(uint64_t t); //!< INTERNAL ONLY
    bool notched(int32_t range, uint16_t amplitude); //!< INTERNAL ONLY

    void update_pps(
        uint32_t t
        , uint32_t T
    ); //!< INTERNAL ONLY

    void update_pps_hr(
        uint64_t t
        , uint64_t T
    ); //!< INTERNAL ONLY

    void pseudo_echo(
        double r
    ); //!< INTERNAL ONLY

    // per instrument constants
protected:
    //scanmech* scanner; //! INTERNAL ONLY
    scanmech scanmech_proto; //! INTERNAL ONLY
    std::vector<scanmech*> scanners; //! INTERNAL ONLY

private:
    double range_unit; //!< INTERNAL ONLY
    double ppstime_unit; //!< INTERNAL ONLY
    double ppstime_unit_hr; //!< INTERNAL ONLY
    double systime_unit; //!< INTERNAL ONLY
    double systime_hr_unit; //!< INTERNAL ONLY
    unsigned systime_hr_bits; //!< INTERNAL ONLY
    double ampl_unit; //!< INTERNAL ONLY
    double refl_unit; //!< INTERNAL ONLY
    double direction_unit; //!< INTERNAL ONLY
    double origin_unit; //!< INTERNAL ONLY
    double exttime_unit; //!< INTERNAL ONLY
    double delta_rg; //!< INTERNAL ONLY

protected:
    float notch_range_min; //!< INTERNAL ONLY
    float notch_range_max; //!< INTERNAL ONLY
    float notch_amplitude_max; //!< INTERNAL ONLY
    std::map<size_t, std::vector<std::pair<double, double>>> calib_tables;  //!< INTERNAL ONLY
private:
    int32_t raw_notch_range_min; //!< INTERNAL ONLY
    int32_t raw_notch_range_max; //!< INTERNAL ONLY
    uint16_t raw_notch_amplitude_max; //!< INTERNAL ONLY
    bool is_override_notch; //!< INTERNAL ONLY
    bool have_notch; //!< INTERNAL ONLY
    enum { notch_legacy, notch_line, notch_frame } notch_kind; //!< INTERNAL ONLY
    std::vector<int32_t> notch_range_min_lut; //!< INTERNAL ONLY
    std::vector<int32_t> notch_range_max_lut; //!< INTERNAL ONLY
    std::vector<uint16_t> notch_amplitude_max_lut; //!< INTERNAL ONLY
protected:
    double line_angle;
    double frame_angle;

private:
    // per laser shot constants //!< INTERNAL ONLY
    uint32_t line_angle_raw; //! < INTERNAL ONLY
    uint32_t frame_angle_raw; //!< INTERNAL_ONLY

    unsigned num_segments; //!< INTERNAL_ONLY
    //unsigned segment; //!< INTERNAL_ONLY

public:
    //! segment number
    /*! The current segment, usually some partitioning of the angle range
     *
     */
    unsigned segment; // segment number

private:
    unsigned segment_raw; //! INTERNAL_ONLY

    float background_radiation; //!< INTERNAL ONLY

    // per echo
    float deviation;//!< INTERNAL ONLY
    bool is_sw;//!< INTERNAL ONLY
    bool is_high_power;//!< INTERNAL ONLY
    bool is_rising_edge;//!< INTERNAL_ONLY

    // conversion from systime to pps time domain
    bool single_or_last_initialized; //!< INTERNAL ONLY
    detail::clock_compensator shot_clock; //!< INTERNAL ONLY
    detail::clock_compensator event_clock; //!< INTERNAL_ONLY
    bool counter_sync_once; //!< INTERNAL_ONLY

    // discard echoes we get before we have seen a shot
    bool shot_seen;

protected:
    // near range and reflactance tables
    float nrange_delta; //!< INTERNAL ONLY
    std::vector<float> nrange; //!< INTERNAL ONLY

private:
    float arange_scale; //!< INTERNAL ONLY
    std::vector<int> arange; //!< INTERNAL ONLY
    
protected:
    float refltab_delta; //!< INTERNAL ONLY
    std::vector<float> refltab; //!< INTERNAL ONLY

private:
    bool is_override_mta; //!< INTERNAL ONLY
    bool have_mta_sensor; //!< INTERNAL ONLY
    unsigned mta_zone; //!< INTERNAL ONLY
    double rg_near; //!< INTERNAL ONLY
    double rg_far; //!< INTERNAL ONLY

    std::vector<unsigned> multi_mta_zones; //!< INTERNAL ONLY

protected:
    double attenuation; //!< INTERNAL ONLY in Neper/m
    double pressure;    //!< INTERNAL ONLY pressure in mbar
    double pressure_sl; //!< INTERNAL ONLY user pressure in mbar
    double rel_humidity;//!< INTERNAL ONLY rel humidity in percent
    double rng_scale;   //!< INTERNAL ONLY rng_scale
    double temperature; //!< INTERNAL ONLY (ambient?) temperature in degree Celsius

    unsigned pulse_num_mod_ampl; //!< INTERNAL ONLY num of amplitude mod
    unsigned pulse_code_phase_mode; //!< INTERNAL ONLY
    unsigned pulse_phase_step; //!< INTERNAL ONLY
    std::vector<double> pulse_pos_mod; //!< INTERNAL ONLY in sec

    bool is_dyntrig; //!< INTERNAL ONLY
    float dyntrig_offset; //!< INTERNAL ONLY
    float dyntrig_scale;  //!< INTERNAL ONLY
    std::vector<float> dyntrig_thra1; //!< INTERNAL ONLY
    std::vector<float> dyntrig_del1; //!< INTERNAL ONLY
    std::vector<float> dyntrig_dec1; //!< INTERNAL ONLY
    std::vector<float> dyntrig_thra2; //!< INTERNAL ONLY
    std::vector<float> dyntrig_del2; //!< INTERNAL ONLY
    std::vector<float> dyntrig_dec2; //!< INTERNAL ONLY

private:

    // not copyable
    pointcloud(const pointcloud&);

    int total_echo_count;
    int total_shot_count;
    
    
protected:
    double wavelength; //!< INTERNAL ONLY
    bool debug; //!< INTERNAL ONLY
};

} // namespace scanlib

#endif // POINTCLOUD_HPP