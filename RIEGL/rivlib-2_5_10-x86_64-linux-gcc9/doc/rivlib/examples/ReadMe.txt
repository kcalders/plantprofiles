$Id: ReadMe.txt 1039 2014-05-28 09:38:03Z RS $

This directory contains several example files for the
Riegl C++ library RiVLib. If you want to build the examples from your
favorite IDE or by Makefile you can use the CMakeLists.txt file to
generate the project files. Please read the instructions within the
CMakeLists.txt file.
Note: RiVLib expects a C++11 compliant compiler environment!

pointcloudcpp.cpp :

    This example demonstrates the use of RiVLib as a
    statically bound C++ library.

pointclouddll.c :

    This example demonstrates the use of RiVLib as a
    dynamically bound library. You can use any language
    that allows dynamic binding, but this example uses
    the C language.
    If you want to start the executable from within the
    IDE, please dont forget to add a PATH= setting to
    the lib subdirectory.This will allow the executable to find the
    scanifc-mt.dll

ctrlifc_demo1.c :

    The program configures the scanner by setting the measurement
    program (MEAS_SET_PROG) and the field of view (SCAN_SET_RECT_FOV)
    of the scanner.
    A scan for a theta angle between 30 and 130 degrees with an
    increment of 0.08 degrees, and a full circle for the phi angle
    with identical increment as for theta is configured. Now the scan
    acquisition is started (MEAS_START).
    Then, while the scanner is busy (MEAS_BUSY) it asks the scanner
    for its progress (SCN_PROGRESS) to update the progress bar.When
    the scan has finished (scanner is no longer busy), the connection
    to the scanner is closed.

ctrlifc_demo2.cpp :

    This demo works for a VZ-400 only.
    First, this example program reads the scanners identification
    and refuses to continue in case an other scanner is connected.
    Then it asks for the current measurement program set and sets
    a new measurement program 1 using MEAS_SET_PROG(1)
    Then a line scan is configured (from 30 to 130 degrees with
    10 degrees resolution, and a phi angle of 180.0 degrees) and
    the internal storage is disabled.
    Finally the scan is started and data is acquired (using the
    basic_rconnection class) as long as a predefined data limit is
    not exceeded. When enough data has been collected the scanner
    is stopped and the connection is closed.

ctrlifc_demo3.cpp :

    This demo works for a VQ-250 only.
    First, this example program reads the scanners identification
    and refuses to continue in case an other scanner is connected.
    The external GPS is configured by setting
    - GPS_MODE using the ordinal value of "EXT.GPS RS232",
    - GPS_EXT_COM_BAUDRATE using ordinal value of "9600",
    - GPS_EXT_EDGE using ordinal value of "POSITIVE EDGE",
    - GPS_EXT_FORMAT using the string value "GPGGA" and
    - GPS_EXT_SEQUENCE using the string value "PPS_FIRST".
    Then the program asks for the current measurement program set
    and sets a new measurement program 1 using MEAS_SET_PROG(1).
    Afterwards a line scan is configured and the scan is started.
    When enough data has been collected and written to file the
    scanner is stopped and the connection is closed.

rxp2txt.dpr :
    
    Simple example program that shows how to use 'scanifc-mt.dll'
    in Delphi/Pascal to read points from a RXP file (or stream).
    It reads a RXP file and prints for each point the X, Y and Z
    coordinates as well as timestamp, amplitude, reflectance and
    deviation point attributes.

rxp_record :
    
    The program reads the rxp stream from the scanner and decodes it.
    Additionally it writes the read rxp stream to a file.
    It stops reading and logging the rxp stream either when pressing
    CTRL+C or when the measurement stops.
