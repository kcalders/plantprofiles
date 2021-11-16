/* $Id: ctrlifc_demo1.c 468 2010-08-26 14:40:59Z rs $
 *
 * (C) 2009 RIEGL LASER MEASUREMENT SYSTEM
 *
 * ctrlifc_demo1.c - Simple 3D scan VZ-400
 *   using scanner control library ctrlifc.dll
 *
 * This example demonstrates how to get and set properties and execute methods
 *   of a RIEGL V-line instrument via TCP using the scanner control library.
 *
 *   The program configures the scanner by setting the measurement program
 *   (MEAS_SET_PROG) and the field of view (SCAN_SET_RECT_FOV) of the scanner.
 *   A scan for a theta angle between 30 and 130 degrees with an increment of
 *   0.08 degrees, and a full circle for the phi angle with identical increment
 *   as for theta is configured.
 *   Now the scan acquisition is started (MEAS_START).
 *   Then, while the scanner is busy (MEAS_BUSY) it asks the scanner for its
 *   progress (SCN_PROGRESS) to update the progress bar.
 *   When the scan has finished (scanner is no longer busy), the connection to
 *   the scanner is closed.
 *
 *   Used properties/commands:
 *   INST_IDENT, MEAS_PROG, MEAS_SET_PROG(), SCN_SET_RECT_FOV(), MEAS_START(),
 *   SCN_PROGRESS, MEAS_BUSY()
 *
 * Compile instructions:
 *   Please read the instructions in CmakeLists.txt
 *
 * Usage instructions:
 * Invoke the program as:
 *   ctrlifc_demo1 <uri>
 * where
 *   <uri> is 'ip-addr[:ip-port]' (replace ip-addr with the ip address of your scanner).
 */

#include <riegl/ctrlifc.h>

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

void wait(float seconds)
{
    /* Note: this function implements busy waiting and should be
       avoided in productive code
    */
    clock_t endwait;
    endwait = clock() + (int)(seconds * (float)CLOCKS_PER_SEC);
    while (clock() < endwait) {}
}

void fprintf_list(FILE* file, ctrlifc_cszlist comment, int comment_count)
{
    int i;
    for (i = 0; i < comment_count; i++)
    {
        fprintf(file, "\\ %s\n", comment[i]);
    }
}

int main(int argc, char* argv[])
{
    /* declare session handle for connection:
    */
    ctrlifc_session session;

    /* error message string from interface:
    */
    ctrlifc_csz message;

    /* function ctrlifc_execute_command calls a method of the instrument.
       (ctrlifc_execute_command("f", "1,2,3", ...) executes "f(1,2,3)")
       Note: string values must be enclosed in quotation marks ("...") and
       multiple arguments are separated using , (comma)
    */
    ctrlifc_csz result;            /* will receive the result of call */
    ctrlifc_cszlist comment;       /* optional messages from the instrument */
    ctrlifc_int32_t comment_count; /* number of comment lines */
    ctrlifc_csz status;            /* will receive the instrument's status */

    /* Note: simply specify 0 (zero) for values you are not interested in
       Example:
       ctrlifc_execute_command(session, "meas_set_prog", "1", 0, 0, 0, 0)
       will not return output, comments (and number of) and status
    */

    int busy;
    int last_percent = 0;
    int scan_percent;

    /* check if one argument is specified, else print usage message:
    */
    if ((argc != 2) || (!strcmp(argv[1], "--help")))
    {
        fprintf(stderr, "Usage: %s address[:service]", argv[0]);
        return 1;
    }

    /* declare session handle s and construct a new session
       Note: no connection to the instrument is opened yet
    */
    ctrlifc_construct_session(&session);

    /* now connect to the instrument specified as first argument
       the connection string is expected in format
         "<host_name_or_ip_address>[:<service_name_or_port_number>]"
       Examples:
         "192.168.0.234", "192.168.0.234:20002",
         "s9991234" or "s9991234:20002"
       if port number is not specified, 20002 is assumed as default
    */
    if ( ctrlifc_connect(session, argv[1]) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "Connected to %s\n", argv[1]);

    /* first, we abort a scan that might be in progress:
    */
    if( ctrlifc_execute_command(
        session, "MEAS_ABORT", "", &result, &comment, &comment_count, &status) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "MEAS_ABORT() returned %s", result);
    fprintf_list(stdout, comment, comment_count);
    fprintf(stdout, "Status is %s\n", status);

    /* now we wait for the scanner to abort a scan in progress:
       (MEAS_BUSY(1) waits until the scanner finished its work)
    */
    if( ctrlifc_execute_command(
        session, "MEAS_BUSY", "1",
        &result, &comment, &comment_count, &status) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "MEAS_BUSY(1) returned %s\n", result);
    fprintf_list(stdout, comment, comment_count);
    fprintf(stdout, "Status is %s\n", status);

    /* The value of a property is requested using ctrlifc_get_property()
       function.
       The value will be stored in a string variable (char*) so the address
       of the value is given (&result).
       Note: the value is strictly read-only and is valid only until the
       next library function call!
       The value is returned as string and may (depending on the property)
       consist of multiple values separated by , (comma). Please refer to
       instrument's manual for a detailed description of the property and
       its values.
    */
    if( ctrlifc_get_property(session, "INST_IDENT", &result, 0, 0, 0) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "Instrument is %s\n", result);

    /* this demonstration program requires a VZ-400 instrument:
    */
    if( strncmp(result+1, "VZ-400", 6) != 0 )
    {
        fprintf(stderr, "Failure: VZ-400 instrument required\n");
        return 1;
    }

    /* query current measurement program:
    */
    if( ctrlifc_get_property(session, "MEAS_PROG", &result, 0, 0, 0) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "MEAS_PROG is %s\n", result);

    /* set measurement program:
    */
    if( ctrlifc_execute_command(
        session, "MEAS_SET_PROG", "1", &result, &comment, &comment_count, &status) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "MEAS_SET_PROG(1) returned %s\n", result);
    fprintf_list(stdout, comment, comment_count);
    fprintf(stdout, "Status is %s\n", status);

    /* set a scan for a theta angle between 30 and 130 degrees
	   with an increment of 0.08 degrees, and a full circle for
	   the phi angle with identical increment as for theta.
       (please refer to instrument manual for details):
    */
    if( ctrlifc_execute_command(
        session, "SCN_SET_RECT_FOV", "30,130,0.08, 0,360,0.08",
        &result, &comment, &comment_count, &status) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "SCN_SET_RECT_FOV(30,130,0.08, 0,360,0.08) returned %s\n", result);
    fprintf_list(stdout, comment, comment_count);
    fprintf(stdout, "Status is %s\n", status);

    /* start measurement:
    */
    if( ctrlifc_execute_command(
        session, "MEAS_START", "", &result, &comment, &comment_count, &status) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "MEAS_START() returned %s\n", result);
    fprintf_list(stdout, comment, comment_count);
    fprintf(stdout, "Status is %s\n", status);

    /* read progress and draw progress bar (write #219 every 2%):
    */
    fprintf(stdout, "Percent:\n");
    fprintf(stdout, "1..10...20...30...40...50...60...70...80...90..100\n");

    busy = 1; /* return value from MEAS_BUSY(): 1(TRUE) or 0(FALSE) */

    while(busy) /* while (scanner is busy) */
    {
        /* here you would do something useful instead of simply polling
           the progress from the instrument
           Note: You should avoid busy waiting in productive code!
        */
        wait(1.0);

        /* now let's ask for progress of current scan from instrument:
        */
        if( ctrlifc_get_property(session, "SCN_PROGRESS", &result, 0, 0, 0) )
        {
            ctrlifc_get_last_error(session, &message);
            fprintf(stderr, "Failure: %s\n", message);
            return 1;
        }
        /* convert result value (percentage) and update progress bar:
        */
        scan_percent = atoi(result);
        while(last_percent < scan_percent)
        {
            fprintf(stdout, "*");
            last_percent += 2; /* Note: 1 '*' = 2 percent */
        }
        if( ctrlifc_execute_command(
            session, "MEAS_BUSY", "0", &result, 0, 0, 0) )
        {
            ctrlifc_get_last_error(session, &message);
            fprintf(stderr, "Failure: %s\n", message);
            return 1;
        }
        busy = atoi(result);
    }
    /* scanner is no longer busy (scan finished)
    */
    fprintf(stdout, "\n");
    fprintf(stdout, "Scan finished\n");

    /* close the connection if the instrument is no longer needed:
    */
    if( ctrlifc_disconnect(session) )
    {
        ctrlifc_get_last_error(session, &message);
        fprintf(stderr, "Failure: %s\n", message);
        return 1;
    }
    fprintf(stdout, "Connection closed\n");

    /* finally the session handle is destroyed by calling
    */
    ctrlifc_destruct_session(session);

    fprintf(stdout, "Demo program successfully finished\n");

    return 0;
}
