// $Id: pointcloudcpp.cpp 835 2013-06-03 07:21:40Z RS $

// (c) Riegl 2008-2015
// pointcloudcpp.cpp - Example for reading pointcloud data
// from Riegl's *.rxp format.
//
// NOTE: rivilib expects a working C++ 11 setup!
// This example uses the RiVLib as a statically linked C++ library.
//
// Compile instructions:
//   Please read the instructions in CmakeLists.txt
//
// Usage instructions:
// Invoke the program as:
//   pointcloudcpp <uri>
//   where uri is e.g. 'file:../scan.rxp' when reading from a file or
//   'rdtp://ip-addr/current' when reading real-time data from the scanner
//   (replace ip-addr with the ip-addr of your scanner).
// The program will read the pointcloud data, filter out single and first targets
// and print them out to the console in ASCII.
//

#include <riegl/scanlib.hpp>

#include <iostream>
#include <exception>
#include <cmath>
#include <limits>
#include <memory>

using namespace scanlib;
using namespace std;

// The import class is derived from pointcloud class, which assembles the
// scanner data into distinct targets and computes x,y,z pointcloud data.
// The pointcloud class and its base class have a huge number of overridables
// that give access to all of the scanners data, e.g. "gps" or "housekeeping"
// data. The pointcloud class also has the necessary logic to align the
// data to gps information (if embedded in the rxp stream) and will return
// timestamps in the domain of the gps.
class importer
    : public pointcloud
{
    ostream& o;
    unsigned long line;
    unsigned long frame;
    string type_id;
    string serial;
public:
    importer(ostream& o_)
        : pointcloud(false) // set this to true if you need gps aligned timing
        , o(o_)
        , line(0)
        , frame(0)
    {
        o.precision(10);
    }

protected:

    // Overridden from pointcloud class.
    // Gets called for every decoded echo.
    void on_echo_transformed(echo_type echo)
    {
        // here we select which target types we are interested in
        if ( pointcloud::first == echo || pointcloud::single == echo) {
            // targets is a member std::vector that contains all
            // echoes seen so far, i.e. the current echo is always
            // indexed by target_count-1.
            target& t(targets[target_count-1]);

            // transform to polar coordinates
            double range = std::sqrt(
                t.vertex[0]*t.vertex[0]
                + t.vertex[1]*t.vertex[1]
                + t.vertex[2]*t.vertex[2]
            );
            if (range > numeric_limits<double>::epsilon()) {
                double phi = atan2(t.vertex[1],t.vertex[0]);
                phi = ((phi<0.0)?(phi+2.0*pi):phi);
                double theta = std::acos(t.vertex[2]/range);
                t.vertex[0] = static_cast<float>(range);
                t.vertex[1] = static_cast<float>((360.0/(2.0*pi))*theta);
                t.vertex[2] = static_cast<float>((360.0/(2.0*pi))*phi);
            }
            else
                t.vertex[0] = t.vertex[1] = t.vertex[2] = 0.0;

            // print out the result
            o << t.vertex[0] << ", " << t.vertex[1] << ", " << t.vertex[2] << ", " << t.time << endl;
        }
    }

    
    
    // This function gets called when a new scan line starts in up direction.
    // For a VQ-880-G please see on_line_start_segment_1 and on_line_start_segment_2.
    void on_line_start_up(const line_start_up<iterator_type>& arg) {
        pointcloud::on_line_start_up(arg);
        o << "line start up: " << ++line << endl;
    }

    // This function gets called when a new scan line starts in down direction.
    // For a VQ-880-G please see on_line_start_segment_1 and on_line_start_segment_2.
    void on_line_start_dn(const line_start_dn<iterator_type>& arg) {
        pointcloud::on_line_start_dn(arg);
        o << "line start dn: " << ++line << endl;
    }
    
    // This function gets called when a new scan line starts in the first segment.
    // At the moment only the VQ-880-G allows to split a mirror wheel rotation into multiple segments (lines).
    // For other scanner types please see on_line_start_up and on_line_start_dn.
    void on_line_start_segment_1(const line_start_segment_1<iterator_type> &arg) {
        pointcloud::on_line_start_segment_1(arg);
        o << "line start segment 1: " << ++line << endl;
    }
    
    // This function gets called when a new scan line starts in the second segment.
    // At the moment only the VQ-880-G allows to split a mirror wheel rotation into multiple segments (lines).
    // For other scanner types please see on_line_start_up and on_line_start_dn.
    void on_line_start_segment_2(const line_start_segment_2<iterator_type> &arg) {
        pointcloud::on_line_start_segment_2(arg);
        o << "line start segment 2: " << ++line << endl;
    }

    // The following function gets called when a scan line has been finished.
    void on_line_stop(const line_stop<iterator_type>& arg) {
        pointcloud::on_line_stop(arg);
        o << "line stop: " << endl;
    }

    
    
    // This function gets called when a new frame starts in up direction.
    // Only 3D scanner (VZ-xxx) can be configured to perform a 3D scan (frame scan).
    void on_frame_start_up(const frame_start_up<iterator_type>& arg) {
        pointcloud::on_frame_start_up(arg);
        o << "frame start up: " << ++frame << endl;
    }
    
    // This function gets called when a new frame starts in down direction.
    // Only 3D scanner (VZ-xxx) can be configured to perform a 3D scan (frame scan).
    void on_frame_start_dn(const frame_start_dn<iterator_type>& arg) {
        pointcloud::on_frame_start_dn(arg);
        o << "frame start dn: " << ++frame << endl;
    }
    
    // This function gets called when a frame has been finished.
    // Only 3D scanner (VZ-xxx) can be configured to perform a 3D scan (frame scan).
    void on_frame_stop(const frame_stop<iterator_type>& arg) {
        pointcloud::on_frame_stop(arg);
        o << "frame stop: " << endl;
    }
    
    
    
    // This function gets called when a the scanner emits a notification
    // about an exceptional state.
    void on_unsolicited_message(const unsolicited_message<iterator_type>& arg) {
        pointcloud::on_unsolicited_message(arg);

        std::string type;
        if (arg.type == 1)
            type = "INFO: ";
        else if (arg.type == 2)
            type = "WARNING: ";
        else if (arg.type == 3)
            type = "ERROR: ";
        else if (arg.type == 4)
            type = "FATAL: ";

        o << type << arg.message << endl;
    }
    
    // Decodes the GPS synchronization status and the PPS counter.
    // This function is only called on scanners WITHOUT higher resolution (hr) system time.
    // For scanners WITH higher resolution system time the function on_hk_gps_hr is called.
    void on_hk_gps_ts_status_dop_ucs(const hk_gps_ts_status_dop_ucs<iterator_type>& arg) {
        pointcloud::on_hk_gps_ts_status_dop_ucs(arg);

        std::string sync_status = "not synchronized";
        if (arg.SYNC_STATUS == 1)
            sync_status = "lost synchronization";
        else if (arg.SYNC_STATUS == 3)
            sync_status = "correctly synchronized";

        o << "hk gps: " << "sync_status=" << sync_status << "\tPPS_count=" << arg.PPS_CNT <<  endl;
    }
    
    // Decodes the GPS synchronization status and the PPS counter.
    // This function is only called on scanners WITH higher resolution (hr) system time. 
    // For scanners WITHOUT higher resolution (hr) system time the function on_hk_gps_ts_status_dop_ucs is called.
    void on_hk_gps_hr(const hk_gps_hr<iterator_type>& arg) {
        pointcloud::on_hk_gps_hr(arg);

        std::string sync_status = "not synchronized";
        if (arg.SYNC_STATUS == 1)
            sync_status = "lost synchronization";
        else if (arg.SYNC_STATUS == 3)
            sync_status = "correctly synchronized";

        o << "hk gps hr: " << "sync_status=" << sync_status << "\tPPS_count=" << arg.PPS_CNT <<  endl;
    }
    
    // This function gets called when a new header is available.
    void on_header(const header<iterator_type> &arg) {
        pointcloud::on_header(arg);
        this->type_id = arg.type_id;
        this->serial = arg.serial;
        o << "type id: " << this->type_id << endl;
        o << "serial: " << this->serial << endl;
    }
    
    // This function gets called when new housekeeping data are available.
    void on_hk_float_param(const hk_float_param<iterator_type> &arg) {
        pointcloud::on_hk_float_param(arg);
        
        for (std::size_t i = 0; i < arg.data_size; i++)
        {
            uint16_t paramId = arg.data[i].param_id;
            float value = arg.data[i].value;
            
            switch (paramId)
            {
                // device
                case 1003:
                case 1929:
                    o << "input supply voltage: " << value << " [V]" << endl;
                    break;
                case 1004:
                case 1930:
                    o << "input supply current: " << value << " [A]" << endl;
                    break;
                case 1308:
                    o << "device temperature: " << value << " [°C]" << endl;
                    break;
                
                // laser
                case 1039:
                {
                    string status = (value == 0) ? "off" : "on";
                    o << "laser status of channel 1: " << status << endl;
                    break;
                }
                case 1398:
                {
                    string status = (value == 0) ? "off" : "on";
                    o << "laser status of channel 2: " << status << endl;
                    break;
                }
                case 1295:
                {
                    string status = (value == 0) ? "not ready" : "ready";
                    o << "laser ready of channel 1: " << status << endl;
                    break;
                }
                case 1728:
                {
                    string status = (value == 0) ? "not ready" : "ready";
                    o << "laser ready of channel 2: " << status << endl;
                    break;
                }
                case 1296:
                {
                    string status = (value == 0) ? "unlocked" : "locked";
                    o << "laser lock of channel 1: " << status << endl;
                    break;
                }
                case 1392:
                {
                    string status = (value == 0) ? "unlocked" : "locked";
                    o << "laser lock of channel 2: " << status << endl;
                    break;
                }
                
                // storage
                case 1029:
                    o << "internal memory usage: " << value << " [%]" << endl;
                    break;
                case 1031:
                    o << "USB memory usage: " << value << " [%]" << endl;
                    break;
                case 1036:
                {
                    string status = (value == 0) ? "no" : "yes";
                    o << "USB memory detected: " << status << endl;
                    break;
                }
                case 1864:
                    o << "SD card usage: " << value << " [%]" << endl;
                    break;
                case 1840:
                {
                    string status = (value == 0) ? "no" : "yes";
                    o << "SD card detected: " << status << endl;
                    break;
                }
                    
                // buffer
                case 1033:
                    o << "measurement buffer utilization of channel 1: " << value << " [%]" << endl;
                    break;
                case 1034:
                    o << "monitoring buffer utilization of channel 1: " << value << " [%]" << endl;
                    break;
                case 1410:
                    o << "measurement buffer utilization of channel 2: " << value << " [%]" << endl;
                    break;
                case 1411:
                    o << "monitoring buffer utilization of channel 2: " << value << " [%]" << endl;
                    break;
                default:
                    break;
                    
                // scan sync
                case 1504:
                {
                    string status = (value == 0) ? "no" : "yes";
                    o << "scan sync active: " << status << endl;
                    break;
                }
                case 1505:
                {
                    string status = (value == 0) ? "no" : "yes";
                    o << "scan sync locked once: " << status << endl;
                    break;
                }
                case 1506:
                {
                    string status = (value == 0) ? "no" : "yes";
                    o << "scan sync unlocked: " << status << endl;
                    break;
                }
            }
        }
    }
};


int main(int argc, char* argv[])
{ 
    try {

        if (argc == 2) {
            // The basic_rconnection class contains the communication
            // protocol between the scanner or file and the program.
            shared_ptr<basic_rconnection> rc;

            // The static create function analyses the argument and
            // gives back a suitable class that is derived from
            // basic_rconnection and can handle the requested protocol.
            // The argument string is modelled using the common 'uri'
            // syntax, i.e. a protocol specifier such as 'file:' or
            // 'rdtp:" is followed by the 'resource location'.
            // E.g. 'file:C:/scans/test.rxp' would specify a file that
            // is stored on drive C in the scans subdirectory.
            // E.g. 'rdtp://192.168.0.128/CURRENT' specifies a connection
            // to the measurement stream of a scanner.
            // E.g. 'rdtp://192.168.0.128/CURRENT?type=mon' specifies a connection
            // to the monitoring stream of a scanner.
            rc = basic_rconnection::create(argv[1]);
            rc->open();

            // The decoder class scans off distinct packets from the
            // continuous data stream i.e. the rxp format and manages
            // the packets in a buffer.
            decoder_rxpmarker dec(rc);

            // The importer ( based on pointcloud and basic_packets class)
            // recognizes the packet types and calls into a distinct
            // function for each type. The functions are overidable
            // virtual functions, so a derived class can get a callback
            // per packet type.
            importer     imp(cout);

            // The buffer, despite its name is a structure that holds
            // pointers into the decoder buffer thereby avoiding
            // unnecessary copies of the data.
            buffer       buf;

            // This is the main loop, alternately fetching data from
            // the buffer and handing it over to the packets recognizer.
            // Please note, that there is no copy overhead for packets
            // which you do not need, since they will never be touched
            // if you do not access them.
            for ( dec.get(buf); !dec.eoi(); dec.get(buf) ) {
                imp.dispatch(buf.begin(), buf.end());
            }
            
            rc->close();
            return 0;
        }

        cerr << "Usage: " << argv[0] << "<input-uri>" << endl;
        return 1;
    }
    catch(exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    catch(...) {
        cerr << "unknown exception" << endl;
        return 1;
    }

    return 0;
}
