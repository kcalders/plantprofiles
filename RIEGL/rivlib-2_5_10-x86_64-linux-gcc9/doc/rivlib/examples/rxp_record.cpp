// (c) Riegl 2008-2016
// rxp_record1.cpp - Example for logging the scanner data stream to a rxp file.
//
// NOTE: rivilib expects a working C++ 11 setup!
// This example uses the RiVLib as a statically linked C++ library.
//
// Compile instructions:
//   Please read the instructions in CmakeLists.txt
//
// Usage instructions:
// Invoke the program as:
//   rxp_record1 <uri>
//   where uri is e.g. 'rdtp://ip-addr/current' when reading data from the scanner
//   (replace ip-addr with the ip-addr of your scanner).
// The program reads the rxp stream from the scanner and decodes it.
// Additionally it writes the read rxp stream to a file.
// It stops reading and logging the rxp stream either when pressing
// CTRL+C or when the measurement stops.

#include <riegl/scanlib.hpp>

#include <iostream>
#include <exception>
#include <cmath>
#include <limits>
#include <memory>
#include <csignal>

using namespace scanlib;
using namespace std;

typedef decoder_rxpmarker::const_iterator it;
sig_atomic_t request_shutdown = 0;

// The importer class is derived from basic_packets class but it's also
// possible to derive from pointcloud if scan data with gps time are needed.
// This basic_packets class has a huge number of overridables that give
// access to data, e.g. "alert" or "housekeeping".
class importer
    : public basic_packets
{
    ostream& o;
public:
    importer(ostream& o_)
        : basic_packets()
        , o(o_)
    {
        o.precision(10);
		
        // To prevent decoding not needed packets "select_protocol" just selects
        // the following documented packets:
        // frame_start_dn, frame_start_up, frame_stop,
        // header, header_ext,
        // line_start_up, line_start_dn, line_stop,
        // line_start_segment_1, line_start_segment_2, line_start_segment_3,
        // meas_start, meas_stop
        selector = select_protocol;
    }

protected:

    // This overridable gets called when the measurement stops.
    // We request end of the dispatch loop. The dispatch function
    // in turn will resturn with true.
    void on_meas_stop(const meas_stop<it>& arg)
    {
        basic_packets::on_meas_stop(arg);
		request_dispatch_end();
    }
};


void ctrlc_handler(int s)
{
    request_shutdown = 1;
}


int main(int argc, char* argv[])
{
    // Handle CTRL+C to close the connection between the scanner
    // and the program.
    signal(SIGINT, ctrlc_handler);
  
    try
    {

        if (argc == 2) {
            // The basic_rconnection class contains the communication
            // protocol between the scanner or file and the program.
            shared_ptr<basic_rconnection> rc;

            // The static create function analyses the argument and
            // gives back a suitable class that is derived from
            // basic_rconnection and can handle the requested protocol.
            // The argument string is modelled using the common 'uri'
            // syntax, i.e. a protocol specifier such 'rdtp:"
            // is followed by the 'resource location'.
            // E.g. 'rdtp://192.168.0.128/CURRENT' specifies a connection
            // to the measurement stream of a scanner.
            // E.g. 'rdtp://192.168.0.128/CURRENT?type=mon' specifies a connection
            // to the monitoring stream of a scanner.

            rc = basic_rconnection::create(argv[1]);

            // The following line opens a file where to write the incoming
            // rxp stream to. This allows rxp data acquisition and data decoding
            // at the same time with only one connection.
            rc->open_logfile("streamlog.rxp");

            rc->open();

            // The importer class scans off distinct packets from the
            // continuous data stream i.e. the rxp format and manages
            // the packets in a buffer.
            decoder_rxpmarker dec(rc);

            // The importer ( based on basic_packets class)
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
            bool shutdown_requested = false;
            for ( dec.get(buf); !dec.eoi(); dec.get(buf) ) {

                if (imp.dispatch(buf.begin(), buf.end())) {
                    // request the end when meas_stop is received from the instrument
                    cout << "Shutdown requested by request_dispatch_end()" << endl;
                    break;
                }

                if (request_shutdown && !shutdown_requested) {
                    // request end by ending the connection
                    // note: for file based connections this will throw
                    // an exception
                    rc->request_shutdown();
                    cout << "Shutdown requested by ctrl-c" << endl;
                    shutdown_requested = true;
                }
            }

            // Close the rxp log file.
            rc->close_logfile();
            // Close the rxp input stream.
            rc->close();
            return 0;
        }

        cerr << "Usage: " << argv[0] << "<input-uri>" << endl;
        return 1;
    }
    catch(exception& e) {
		cerr << "Exception" << endl;
        cerr << e.what() << endl;
        return 1;
    }
    catch(...) {
        cerr << "unknown exception" << endl;
        return 1;
    }

    return 0;
}
