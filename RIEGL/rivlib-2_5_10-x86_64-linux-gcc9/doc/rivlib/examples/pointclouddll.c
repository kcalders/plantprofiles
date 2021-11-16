/* $Id: pointclouddll.c 1083 2014-09-08 06:46:10Z RS $ */
/* (c) Riegl 2008
 * pointclouddll.c - Example for reading pointcloud data
 * from Riegl's *.rxp format.
 *
 * This example uses the RiVLib as a DLL, so do not forget to copy
 * the scanifc.dll to a place where it can be found ( usually at the
 * same location as the executable when on windows).
 *
 * Compile instructions:
 *   Please read the instructions in CmakeLists.txt
 *
 * Usage instructions:
 * Invoke the program as:
 *   pointclouddll <uri>
 *   where uri is e.g. file:../scan.rxp when reading from a file or
 *   rdtp://ip-addr/current when reading real-time data from the scanner
 *   (replace ip-addr with the ip-addr of your scanner).
 * The program will read the pointcloud data and print it out to the console
 * in ASCII.
 */

#include <riegl/scanifc.h>

#include <stdio.h>

void print_last_error()
{
    char buffer[512];
    scanifc_uint32_t bs;
    scanifc_get_last_error(buffer, 512, &bs);
    printf("%s\n", buffer);
}

/* Number of points to fetch per call; adjust to your preference. */
#define N_TARG 1024

int main(int argc, char* argv[])
{
    point3dstream_handle h3ds = 0; /* This is the handle to the data stream. */
    int sync_to_pps = 0; /* Set to 1 if reading data with gps sync information */
    scanifc_uint32_t count;
    scanifc_uint32_t n;
    int end_of_frame;

    /* These are the arrays for the pointcloud data */
    scanifc_xyz32       rg_xyz32[N_TARG];     /* The x,y,z coordinates*/
    scanifc_attributes  rg_attributes[N_TARG]; /* The amplitude and quality attributes */
    scanifc_time_ns     rg_time[N_TARG];      /* The timestamp (internal or gps) */

    if (argc == 2) {

        if (scanifc_point3dstream_open_with_logging(argv[1], sync_to_pps, "log.rxp", &h3ds)) {
            fprintf(stderr, "Cannot open: %s\n", argv[1]);
            print_last_error();
            return 1;
        }
        else {

            if (scanifc_point3dstream_add_demultiplexer(
                    h3ds
                    , "hk.txt", 0, "status protocol")) {
                print_last_error();
                return 1;
            }

            do {
                /* Read N_TARG pointcloud data. It is possible */
                /* to specify a null pointer if the respective */
                /* information is not needed.                  */
                if (scanifc_point3dstream_read(
                       h3ds
                       , N_TARG
                       , rg_xyz32
                       , rg_attributes
                       , rg_time
                       , &count
                       , &end_of_frame
                )) {
                    fprintf(stderr, "Error reading %s\n", argv[1]);
                    return 1;
                }
                /* The returned count may be smaller than N_TARG */
                for (n=0; n<count; ++n) {
                    fprintf(stdout, "%f, %f, %f, %f, %u, %u, %u, %u, %llu, %f, %d\n"
                        , rg_xyz32[n].x
                        , rg_xyz32[n].y
                        , rg_xyz32[n].z
                        , rg_attributes[n].amplitude
                        , rg_attributes[n].deviation
                        , rg_attributes[n].flags & 0x03 /* select multi return bits */
                        , (rg_attributes[n].flags & 0x40)>>6 /* pps sync bit */
                        , (rg_attributes[n].flags & 0x10)>>4 /* background rad bit*/
                        , rg_time[n]
                        , rg_attributes[n].background_radiation
                        , end_of_frame
                    );
                }
            } while(count >0); /* count will be zero at the end of available data */

            /* Do not forget to close, to prevent a memory leak. */
            if (scanifc_point3dstream_close(h3ds)) {
                fprintf(stderr, "Error, closing %s\n", argv[1]);
                return 1;
            }
            return 0;
        }
    }

    fprintf(stderr, "Usage: %s <input-uri>\n", argv[0]);
    return 1;
}
