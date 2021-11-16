program rxp2txt;

(******************************************************************************)
(*                                                                            *)
(*  FILE:    rxp2txt.dpr                                                      *)
(*  PROJECT: RiVLIB                                                           *)
(*  AUTHOR:  RIEGL LMS GmbH, Austria                                          *)
(*  PURPOSE: Simple example program that shows how to use 'scanifc-mt.dll'    *)
(*           in Delphi/Pascal to read points from a RXP file (or stream).     *)
(*           It reads a RXP file and prints for each point the X, Y and Z     *)
(*           coordinates as well as timestamp, amplitude, reflectance and     *)
(*           deviation point attributes.                                      *)
(*                                                                            *)
(*  NOTE: Example code successfully tested with Delphi 7 (32 bit) and         *)
(*        Delphi XE5 (32 and 64 bit).                                         *)
(*                                                                            *)
(******************************************************************************)

{$APPTYPE CONSOLE}

uses
  Windows,
  SysUtils;

//______________________________________________________________________________
//
// BASIC TYPES (makes porting from C to Pascal easier)
//______________________________________________________________________________
//
type
  int8    = System.Shortint;
  uint8   = System.Byte;
  int16   = System.Smallint;
  uint16  = System.Word;
  int32   = System.Longint;
  uint32  = System.Cardinal;
  int64   = System.Int64;
  uint64  = System.Int64;
  float32 = System.Single;
  float64 = System.Double;

//______________________________________________________________________________
//
// LIBRARY I/O TYPES
//______________________________________________________________________________
//
type
  TLibResult = int32;
  TLibHandle = System.Pointer;
  TLibString = System.PAnsiChar;

//______________________________________________________________________________
//
// POINT RECORD TYPES
//______________________________________________________________________________
//
// NOTE: see RiVLIB manual for point attribute documentation
//
type
  TRecordXYZ = record
    x : float32; // point
    y : float32; //  coordinates
    z : float32; //    in SOCS and Meters
  end;
  TPRecordXYZ = ^TRecordXYZ;

type
  TRecordMisc = record
    amplitude            : float32; // [dB]
    reflectance          : float32; // [dB]
    deviation            : uint16;  // [1]
    flags                : uint16;  //
    background_radiation : float32; // 
  end;
  TPRecordMisc = ^TRecordMisc;

type
  TRecordTime = record
    time : uint64; // [ns]
  end;
  TPRecordTime = ^TRecordTime;

//______________________________________________________________________________
//
// LIBRARY FUNCTION PROTOTYPES
//______________________________________________________________________________
//
// NOTE: see RiVLIB manual for function and parameter documentation
//       use 'cdecl' calling convention!
//
type
  TScanifc_get_library_version = function(
    var major : uint16;
    var minor : uint16;
    var build : uint16
  ) : TLibResult; cdecl;

type
  TScanifc_get_last_error = function(
    message_buffer      : TLibString;
    message_buffer_size : uint32;
    var message_size    : uint32
  ) : TLibResult; cdecl;

type
  TScanifc_point3dstream_open = function(
    uri         : TLibString;
    sync_to_pps : int32;
    var handle  : TLibHandle
  ) : TLibResult; cdecl;
  
type
  TScanifc_point3dstream_read = function(
    handle           : TLibHandle;
    want             : uint32;
    pxyz32           : TPRecordXYZ;
    pmisc            : TPRecordMisc;
    ptime            : TPRecordTime;
    var got          : uint32;
    var end_of_frame : int32
  ) : TLibResult; cdecl;

type
  TScanifc_point3dstream_close = function(
    handle : TLibHandle
  ) : TLibResult; cdecl;

//______________________________________________________________________________
//
// MAIN FUNCTION
//______________________________________________________________________________
//
const
  BLOCK_SIZE : uint32 = 1000; // number of points read from stream at once
var
  i            : Integer;
  LibHandle    : Windows.THandle;
  LibMajor     : uint16;
  LibMinor     : uint16;
  LibBuild     : uint16;
  LibVersion   : TScanifc_get_library_version;
  LibError     : TScanifc_get_last_error;
  StreamOpen   : TScanifc_point3dstream_open;
  StreamRead   : TScanifc_point3dstream_read;
  StreamClose  : TScanifc_point3dstream_close;
  StreamHandle : TLibHandle;
  BufferXYZ    : array of TRecordXYZ;
  BufferMISC   : array of TRecordMisc;
  BufferTIME   : array of TRecordTime;
  PointCount   : uint32; // number of points read from stream
  EndOfFrame   : int32;  // 1: end of frame reached, more points may come

  function LibLoad(const ProcName : String) : Pointer; // get function address from library
  begin
    Result := Windows.GetProcAddress(LibHandle, PChar(ProcName));
    if (Result = nil) then raise Exception.CreateFmt('Function "%s" not found.', [ProcName]);
  end;

  procedure LibCheck(const Code : TLibResult); // check return code of library function
  var
    Count  : uint32;
    Error  : String;
    Buffer : System.AnsiString;
  begin
    if (Code <> 0) then // error occured
    begin
      SetLength(Buffer, 1024);
      LibError(@Buffer[1], Length(Buffer), Count);
      SetLength(Buffer, Count);
      Error := 'Library error: ' + String(Buffer);
      raise Exception.Create(Error);
    end;
  end;

begin
  try
    // Load library using Windows API functions
    LibHandle := Windows.LoadLibrary('scanifc-mt.dll');
    if (LibHandle = 0) then raise Exception.Create('Failed to load library.');
    try
      // Get library function pointers and check if they are valid
      LibVersion  := LibLoad('scanifc_get_library_version');
      LibError    := LibLoad('scanifc_get_last_error'     );
      StreamOpen  := LibLoad('scanifc_point3dstream_open' );
      StreamRead  := LibLoad('scanifc_point3dstream_read' );
      StreamClose := LibLoad('scanifc_point3dstream_close');

      // Get library version
      LibCheck(LibVersion(LibMajor, LibMinor, LibBuild));
      WriteLn(Output, Format('Library version: %d.%d.%d', [LibMajor, LibMinor, LibBuild]));

      // Open RXP file (or stream, depending on URI)
      LibCheck(StreamOpen('file:scan.rxp', 0, StreamHandle));
      try
        // Output column header
        WriteLn(Output, 'X[m];Y[m];Z[m];Time[s];Amplitude[dB];Reflectance[dB];Deviation[1]');

        // Prepare point buffers
        SetLength(BufferXYZ,  BLOCK_SIZE);
        SetLength(BufferMISC, BLOCK_SIZE);
        SetLength(BufferTIME, BLOCK_SIZE);

        // Read points from stream until end of stream reached
        repeat
          LibCheck(StreamRead(
            StreamHandle, BLOCK_SIZE,
            @BufferXYZ[0], @BufferMISC[0], @BufferTIME[0], // or 'nil' if attribute not needed
            PointCount, EndOfFrame
          ));
          if (PointCount > 0) then for i := 0 to PointCount-1 do
          begin
            WriteLn(Output, Format('%.3f;%.3f;%.3f;%.6f;%.2f;%.2f;%d', [
              BufferXYZ [i].x,
              BufferXYZ [i].y,
              BufferXYZ [i].z,
              BufferTIME[i].time * 1e-9,
              BufferMISC[i].amplitude,
              BufferMISC[i].reflectance,
              BufferMISC[i].deviation
            ]));
          end;
        until (PointCount = 0) and (EndOfFrame = 0); // = end of stream
      finally
        LibCheck(StreamClose(StreamHandle));
      end;
    finally
      Windows.FreeLibrary(LibHandle);
    end;
  except
    on E:Exception do WriteLn(ErrOutput, E.Message);
  end;
end.
