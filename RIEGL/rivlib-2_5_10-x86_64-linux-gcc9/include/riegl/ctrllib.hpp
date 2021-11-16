// $Id$

//------------------------------------------------------------------------------
// Author:
//   GG
// About:
//   V-Line scanner control interface class ctrl_client_session
// Usage example:
//   ctrl_client_session session();
//   // open connection to instrument S9991234:
//   session.open("S9991234");
//   // optional log communication to file:
//   session.start_logging("c:\logfile.txt");
//   // get a property value:
//   // Note: in case of failure an exception is thrown!
//   std::string value;
//   std::vector<std::string> remarks;
//   std::string status;
//   session.get_property("MEAS_PROG", value, &status, &remarks);
//   // set properties:
//   session.set_property("HMI_SOUND", "0", &status, &remarks);
//   // don't care for status and remarks in next call
//   session.set_property("TIMEZONE", "\"Europe/Vienna\"");
//   // execute a command:
//   session.execute_command("MEAS_START", "", &value, &status, &remarks);
//   // stop logging:
//   session.stop_logging();
//   // and close:
//   session.close();
//
//------------------------------------------------------------------------------

//!\file ctrllib.hpp
//! The scanner control library

#ifndef CTRLLIB_HPP
#define CTRLLIB_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

namespace ctrllib {

    //!brief exception class thrown when an operation has been cancelled
    class operation_cancelled: public std::exception
    {
        virtual const char* what() const throw()
        {
            return "Operation cancelled";
        }
    };

    //!\brief scanner control session
    //!\details The ctrl_client_session class represents a scanner's configuration port.
    class ctrl_client_session {
        public:
            //!\brief constructor for scanner control object
            //! no communication is established yet (see open)
            ctrl_client_session();
            //!\brief destructor for scanner control object
            virtual ~ctrl_client_session();
            //!\brief open a connection to the scanner instrument
            //! The open function will locate the scanner by means of the authority
            //! information, which may be given as "host:port", where host and port
            //! either are in text or numeric forms.
            //! \param authority [in]: the address of the scanner
            //! the authority string is expected in format
            //! "<host_name_or_ip_address>[:<service_name_or_port_number>]"
            //! Examples:
            //! "192.168.0.234", "192.168.0.234:20002",
            //! "s9991234" or "s9991234:20002"
            //! if port number is not specified, 20002 is assumed as default
            void open(const std::string& authority);
            //! The close method will close the connection to the scanner.
            void close();
            //! cancel a pending operation
            void cancel();
            //! INTERNAL ONLY Start logging the communication to an ascii text file
            //! Lines starting with
            //! > ... represent lines received from the instrument
            //! < ... represent data sent to the instrument
            //! ! ... represent errors (instrument errors, communication errors)
            //! i ... informational text
            //! \param logfile [in] is the name of the logfile to be created.
            void start_logging(const std::string& logfile);
            //! INTERNAL ONLY Stop logging to logfile.
            void stop_logging();
            //!\brief Execute a command (function call).
            //! The execute_command function calls a function of the scanner interface.
            //! The value is returned as string value:
            //! - string properties are enclosed in quotation marks "..."
            //! - integer and floating point values must be converted
            //! - boolean properties are strings 1 (TRUE) and 0 (FALSE)
            //! - vectors elements are separated using , (comma)
            //! \param command [in] the name of the command/function to execute/call
            //! \param arguments [in] the arguments for the function.
            //! Specify "" for no arguments to be passed to command.
            //! \param result [out] result value as string (optional).
            //!   Vectors are returned as value list separated by , (comma).
            //! \param remarks [out] received comment lines from instrument (optional).
            //! \param status [out] control port status (optional). Format is "<...>".
            void execute_command(
                const std::string& command,
                const std::string& arguments,
                std::string* result = 0,
                std::string* status = 0,
                std::vector<std::string>* remarks = 0
            );
            //!\brief Set a property value.
            //! The set_property function sets a new value for a property.
            //! The new value is given as string value:
            //! - string properties are to be enclosed in quotation marks "..."
            //! - integer and floating point values must be converted into a string
            //! - boolean properties are 1 (TRUE) and 0 (FALSE)
            //! - vectors elements are separated using , (comma)
            //! \param name [in] the name of the property to set
            //! \param value [in] the new property value
            //! \param remarks [out] received comment lines from instrument (optional).
            //! \param status [out] control port status (optional). Format is "<...>".
            void set_property(
                const std::string& name,
                const std::string& value,
                std::string* status = 0,
                std::vector<std::string>* remarks = 0
            );
            //!\brief Get a property value.
            //! The get_property function returns the value of a property.
            //! The value is returned as string value:
            //! - string properties are enclosed in quotation marks "..."
            //! - integer and floating point values must be converted
            //! - boolean properties are strings 1 (TRUE) and 0 (FALSE)
            //! - vectors elements are separated using , (comma)
            //! \param name [in] the name of the property to retrieve
            //! \param value [out] the property value as string
            //! \param remarks [out] received comment lines from instrument (optional).
            //! \param status [out] control port status (optional). Format is "<...>".
            void get_property(
                const std::string& name,
                std::string& value,
                std::string* status = 0,
                std::vector<std::string>* remarks = 0
            );
        private:
            class impl;
            impl* pImpl;
    };

} // namespace ctrllib

#endif //CTRLLIB_HPP
