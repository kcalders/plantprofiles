// $Id$

#ifndef CLOCKCOMP_HPP
#define CLOCKCOMP_HPP

#include <riegl/config.hpp>

#include <utility>
#include <deque>

namespace scanlib { namespace detail {

class clock_compensator
{
    double sys_unit;
    double pps_unit;
    double sys_unit_hr;
    double pps_unit_hr;
    
    uint32_t sync_pps_prev;
    uint32_t sync_pps_ext;
    uint32_t sync_sys_prev;
    uint32_t sync_sys_ext;
    
    uint64_t sync_pps_hr_prev;
    uint64_t sync_pps_hr_ext;
    uint64_t sync_sys_hr_prev;
    uint64_t sync_sys_hr_ext;
    
    bool unwrap_once;
    
    struct state_t {
        double sys_time;
        double drift;
        double skew;
    };
    std::deque<state_t> stateque;

    uint32_t sys_prev;
    uint32_t sys_ext;
    uint32_t sys_max;
    uint32_t sys_mask;
    uint32_t sys_sens;
    
    uint64_t sys_hr_prev;
    uint64_t sys_hr_ext;
    uint64_t sys_hr_max;
    uint64_t sys_hr_mask;
    uint64_t sys_hr_sens;
    
    bool pre_locked;
    bool locked;
    bool once_locked;

    double pps_sync_lost_time;
    
    static unsigned id_max;
    unsigned id;
    
public:
    clock_compensator();
    
    double prr;
    
    void
    init(
        double sys_unit_
        , double pps_unit_
    );

    void
    init(
        double sys_unit_
        , double pps_unit_
        , double sys_unit_hr_
        , double pps_unit_hr_
    );

    void
    init(
        double sys_unit_
        , double pps_unit_
        , double sys_unit_hr_
        , double pps_unit_hr_
        , unsigned sys_hr_bits_
        , uint64_t sens = 0
    );

    std::pair<double,double>
    sync(
        uint32_t sys_
        , uint32_t pps_
    );

    std::pair<double,double>
    sync_hr(
        uint64_t sys_
        , uint64_t pps_
    );

    std::pair<double,double>
    unwrap(
        uint32_t sys_
    );

    std::pair<double,double>
    unwrap_hr(
        uint64_t sys_hr_
    );

    bool
    is_locked(
    ) const {
        return locked;
    }

    bool
    is_once_locked(
    ) const {
        return once_locked;
    }
};

}} // namespace detail namespace scanlib

#endif // CLOCKCOMP_HPP

