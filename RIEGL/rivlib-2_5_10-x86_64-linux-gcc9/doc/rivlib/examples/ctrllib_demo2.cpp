// $Id: ctrllib_demo2.cpp 835 2013-06-03 07:21:40Z RS $

// (C) 2009-2013 RIEGL LASER MEASUREMENT SYSTEM
//
// ctrlifc_demo2.cpp - Simple 2D scan VZ-400
//   using C++ scanner control library ctrllib
//
// NOTE: rivilib expects a working C++ 11 setup!
// This example demonstrates how to get and set properties and execute methods
//   of a RIEGL V-line instrument via TCP using the scanner control library.
//   Additionally this example logs the scan data acquired by the instrument.
//
//   This demo works for a VZ-400 only.
//   First, this example program reads the scanners identification
//   and refuses to continue in case an other scanner is connected.
//   Then it asks for the current measurement program set and sets
//   a new measurement program 1 using MEAS_SET_PROG(1)
//   Then a line scan is configured (from 30 to 130 degrees with
//   10 degrees resolution, and a phi angle of 180.0 degrees) and
//   the internal storage is disabled.
//   Finally the scan is started and data is acquired (using the
//   basic_rconnection class) as long as a predefined data limit is
//   not exceeded. When enough data has been collected the scanner
//   is stopped and the connection is closed.
//
// Compile instructions:
//   Please read the instructions in CmakeLists.txt
//
// Usage instructions:
// Invoke the program as:
//   ctrlifc_demo2 <uri> <file>
// where
//   <uri> is 'ip-addr[:ip-port]' (replace ip-addr with the ip address of your scanner).
//   <file> is the name of the scan data file (.rxp) to be created
// Example:
//  ctrlifc_demo2 s9991234 c:\scans\testscan.rxp

#include <riegl/ctrllib.hpp>
#include <riegl/scanlib.hpp>

#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <memory>

using namespace ctrllib;
using namespace scanlib;
using namespace std;

int main(int argc, char* argv[])
{
    // check if one argument is specified, else print usage message:

    if ((argc != 3) || (string(argv[1]) == "--help"))
    {
        cerr << "Usage: " << argv[0] << " address[:service] outputfile" << endl;
        return 1;
    }

    // declare session handle s and construct a new session
    // Note: no connection to the instrument is opened yet

    ctrl_client_session session;

    try
    {
        // now connect to the instrument specified as first argument
        // the connection string is expected in format
        //   "<host_name_or_ip_address>[:<service_name_or_port_number>]"
        // Examples:
        //   "192.168.0.234", "192.168.0.234:20002",
        //   "s9991234" or "s9991234:20002"
        // if port number is not specified, 20002 is assumed as default

        session.open(string(argv[1]));
        cout << "Connected to " << argv[1] << endl;

        // function execute_command calls a method of the instrument.
        // (execute_command("f", "1,2,3", ...) executes "f(1,2,3)")
        // Note: string values must be enclosed in quotation marks ("...") and
        // multiple arguments are separated using , (comma)

        std::string result;               // will receive result of the function
        std::vector<std::string> comment; // optional messages from the instrument
        std::string status;               // will receive the instrument's status

        // first, we abort a scan that might be in progress:

        session.execute_command(
            "MEAS_ABORT", "", &result, &status, &comment
        );
        cout << "MEAS_ABORT() returned " << result << endl;
        for(size_t i=0; i<comment.size(); ++i)
            cout << comment[i] << endl;
        cout << "Status is " << status << endl;

        // now we wait for the scanner to abort a scan in progress:
        // (MEAS_BUSY(1) waits until the scanner finished its work)

        session.execute_command(
            "MEAS_BUSY", "1",
            &result, &status, &comment
        );
        cout << "MEAS_BUSY(1) returned " << result << endl;
        for(size_t i=0; i<comment.size(); ++i)
            cout << comment[i] << endl;
        cout << "Status is " << status << endl;

        // The value of a property is requested using get_property() method.
        // The value is returned as string and may (depending on the property)
        // consist of multiple values separated by , (comma). Please refer to
        // instrument's manual for a detailed description of the property and
        // its values.

        std::string value;
        session.get_property("INST_IDENT", value);
        cout << "Instrument is " << value << endl;

        // this demonstration program requires a VZ-400 instrument:
        if (std::string(value).compare(1, 6, "VZ-400") != 0)
        {
            throw runtime_error("Error: VZ-400 instrument required");
        }

        // query current measurement program:

        session.get_property("MEAS_PROG", value);
        cout << "MEAS_PROG is " << value << endl;

        // set measurement program:

        session.execute_command(
            "MEAS_SET_PROG", "1", &result, &status, &comment
        );
        cout << "MEAS_SET_PROG(1) returned " << result << endl;
        for(size_t i=0; i<comment.size(); ++i)
            cout << comment[i] << endl;
        cout << "Status is " << status << endl;

        // set line scan (please refer to instrument manual for details):

        session.execute_command(
            "SCN_SET_LINE_SCAN", "30, 130, 10, 180.0", &result, &status, &comment
        );
        cout << "SCN_SET_LINE_SCAN(30, 130, 10, 180.0) returned " << result << endl;
        for(size_t i=0; i<comment.size(); ++i)
            cout << comment[i] << endl;
        cout << "Status is " << status << endl;

        // don't store scan internally:

        session.set_property("STOR_MEDIA", "0");
        cout << "STOR_MEDIA set to 0" << endl;

        // start measurement:

        session.execute_command(
            "MEAS_START", "", &result, &status, &comment
        );
        cout << "MEAS_START() returned " << result << endl;
        for(size_t i=0; i<comment.size(); ++i)
            cout << comment[i] << endl;
        cout << "Status is " << status << endl;

        // start data acquisition:

        // create output file:

        cout << "Output: " << argv[2] << endl;
        ofstream outfile(argv[2], ios::binary|ios::out);

        // open input connection to scanner:

        bool shutdown_requested = false;
        string connstr(argv[1]);
        connstr = "rdtp://"+ connstr +"/current";
        cout << "Input: " << connstr << endl;

        std::shared_ptr<scanlib::basic_rconnection> rc;
        rc = basic_rconnection::create(connstr);
        rc->open();

        basic_rconnection::size_type limit = 50*1000000;
        basic_rconnection::size_type count;
        basic_rconnection::size_type total = 0;
        char buffer[8192];
        int last_percent = 0;
        int data_percent;

        cout << "Data limit: " << limit << " bytes" << endl;
        cout << "Percent:" << endl;
        cout << "1..10...20...30...40...50...60...70...80...90..100" << endl;

        // while the instrument delivers data:

        while ( 0 != (count = rc->readsome(buffer, 8192)) )
        {
            outfile.write(buffer, count);
            total += count;
            if ((total > limit) && (!shutdown_requested))
            {
                // request shutdown of data delivery:
                rc->request_shutdown();
                shutdown_requested = true;
            }
            data_percent = min(100, int(ceil(total*100.0/limit)));

            // update progresss bar:
            while (last_percent < data_percent)
            {
                cout << "*";
                last_percent += 2; // 1 '*' = 2 percent
            }
        }
        cout << endl;

        // flush and close data output file (.rxp):

        outfile.flush();
        outfile.close();

        cout << total << " bytes received" << endl;

        // stop measurement:

        session.execute_command(
            "MEAS_STOP", "", &result, &status, &comment
        );
        cout << "MEAS_STOP() returned " << result << endl;
        for(size_t i=0; i<comment.size(); ++i)
            cout << comment[i] << endl;
        cout << "Status is " << status << endl;

        // close the connection if the instrument is no longer needed

        session.close();
        cout << "Connection closed" << endl;

        cout << "Demo program successfully finished" << endl;
    }
    catch(exception& e)
    {
        cerr << e.what() << endl;
        return 1;
    }
    catch(...)
    {
        cerr << "unknown exception" << endl;
        return 1;
    }
    return 0;
}
