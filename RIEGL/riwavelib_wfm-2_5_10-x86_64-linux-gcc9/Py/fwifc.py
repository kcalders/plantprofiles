# (c) Riegl 2011-2016
'''Python 3.x wrapper for Riegl full wave data interface DLL's.

PLEASE NOTE: This wrapper is to be understood as an example program, it is
not a production grade piece of software. If you find an error, please contact
us at support@riegl.com and/or send us a bug fix.

Example usage:
    from fwifc import *
    fw = fw_array('Filename.wfm')
    for w in fw:
        print w.time_sorg # print start of range gate
        print w[1][0] # print channel 1 time stamps
        print w[1][1] # print channel 1 sample data

The library has two flavours of interfaces, an array like and a low level one.
To access the waveform of index 100, channel 1 use:
    fw[100][1], which will give you a list of timestamps and a list of samples.
The low level API gives access to the functions of the DLL.
Note: The dll's must reside in the same directory as this script.
'''

import ctypes, os
import ctypes.util

# helper function to locate the dll's, location in the same directory as
# the fwifc.py script is preferred

def find_lib(paths, names):
    for p in paths:
        for n in names:
            for pfx,sfx in [('','.dll'), ('lib', '.so')]:
                libpath = os.path.join(p,pfx+n+sfx)
                if os.path.exists(libpath):
                    return libpath
    for n in names:
        libpath = ctypes.util.find_library(n)
        if libpath:
            return libpath
    return None

_fwifcpath = os.path.dirname(os.path.abspath(__file__))
_wfmdll = find_lib([_fwifcpath], ["wfmifc-mt-s", "wfmifc-mt"])
_sdfdll = find_lib([_fwifcpath], ["sdfifc"])

def fw_array(name):
    ''' Create a FwFile object and open it using name as filename.'''
    fw = FwFile()
    fw.open(name)
    return fw

class FwException(Exception):
    '''This class delivers the exception messages from the DLL.'''
    def __init__(self, errno, message):
        self.errno = errno
        Exception.__init__(self, message)

class _C_sbl(ctypes.Structure):
    _fields_ = [("time_sosbl",   ctypes.c_double),
                ("channel",      ctypes.c_uint32 ),
                ("sample_count", ctypes.c_uint32 ),
                ("sample_size",  ctypes.c_int32 ),
                ("sample", ctypes.POINTER(ctypes.c_uint16))]

class FwSegment(object):
    '''A segment of sampling data.

        Member variables:

            time_sosbl ... Start of sampling block relative to time_sorg in seconds.
            data       ... The sample values.
    '''
    def __init__(self):
        self.time_sosbl = None
        self.data = []

class FwShot(object):
    '''Per laser shot information and array of segemnts of the sampling data.

        Member variables:

            time_sorg     ... Start of range gate in seconds.
            time_external ... 'sorg' in external time frame in seconds.
            origin[3]     ... Beam origin vector in meter.
            direction[3]  ... Direction origin vector, dimensionless.
            flags         ... Various flags.
            facet         ... The current facet number.
            channel[4]    ... Four channels of FwSegment's.
    '''
    def __init__(self):
        self.time_sorg = None
        self.time_external = None
        self.origin = []
        self.direction = []
        self.flags = None
        self.facet = None
        self.channel = [[],[],[],[]]
        self._sampling_time = None

    def __getitem__(self, n):
        '''Array interface, returns a list of all data samples
        concatenated and a list of time stamps.'''
        d = [[],[]]
        for s in self.channel[n]:
            d[0].extend([x*self._sampling_time+s.time_sosbl for x in range(len(s.data))])
            d[1].extend(s.data)
            if len(d[0]) == 0:
                d[0].extend([s.time_sosbl])
        return d

    def __len__(self):
        return len(self.channel)

class _Info(object):
    def __init__(self):
        self.instrument = None
        self.serial = None
        self.epoch = None
        self.v_group = None
        self.sampling_time = None
        self.flags = None
        self.num_facets = None

class FwFile(object):
    '''The waveform file object.

        Member variables:

            instrument    ... The type of the measurement instrument.
            serial        ... The serial number of the measurement instrument.
            epoch         ... The epoch of the time frame of time_external.
            v_group       ... Group velocity of light in meter/second .
            sampling_time ... The sampling time interval in seconds.
            flags         ... Various flags.
            num_facets    ... The number of mirror facets.
    '''
    def __init__(self):
        self.file = ctypes.c_void_p()
        self.lib = None
        self.instrument = None
        self.serial = None
        self.epoch = None
        self.v_group = None
        self.sampling_time = None
        self.flags = None
        self.num_facets = None
        self.name = None

    def __del__(self):
        self.close()

    def _wrap_call(self, rescode):
        if rescode !=0:
            s_err = ctypes.c_char_p()
            self.lib.fwifc_get_last_error(ctypes.byref(s_err))
            raise FwException(rescode, s_err.value.decode("utf-8"))

    def __iter__(self):
        '''Get iterator to traverse the file.'''
        f = FwFile()
        f.open(self.name)
        return f

    def __next__(self):
        '''Read next record. Raise exception on end.'''
        w = self.read()
        if w:
            return w
        else:
            raise StopIteration

    def __getitem__(self, r):
        '''List interface.
            Note: seek offsets start with 1 while list offset start with 0!
        '''
        self.seek(r+1) # want zero offset array
        return self.read()

    def __len__(self):
        '''Support for len operator. Yields the total number of waveforms.'''
        if not self.file:
            raise FwException("FwFile is not open")
        self._wrap_call(self.lib.fwifc_seek(self.file, ctypes.c_uint32(0xffffffff)))
        record = ctypes.c_uint32()
        self._wrap_call(self.lib.fwifc_tell(self.file, ctypes.byref(record)))
        return int(record.value)

    def open(self, filename):
        '''Open the waveform file. The type *.sdf or *.wfm is determined from
            the filename extension.
        '''
        self.name = filename
        if self.file:
            self.close()
        base, ext = os.path.splitext(filename)
        if ext.upper() == ".SDF":
            self.lib = ctypes.CDLL(_sdfdll)
        elif ext.upper() == ".WFM":
            self.lib = ctypes.CDLL(_wfmdll)
        self._wrap_call(self.lib.fwifc_open(ctypes.c_char_p(filename.encode("utf-8")), ctypes.byref(self.file)))
        self._wrap_call(self.lib.fwifc_set_sosbl_relative(self.file, 1))
        info = self._get_info()
        self.instrument = info.instrument
        self.serial = info.serial
        self.epoch = info.epoch
        self.v_group = info.v_group
        self.sampling_time = info.sampling_time
        self.flags = info.flags
        self.num_facets = info.num_facets

    def close(self):
        '''Close the current file.'''
        if self.file:
            self._wrap_call(self.lib.fwifc_close(self.file))
        self.file = ctypes.c_void_p()
        self.lib = None

    def _get_info(self):
        if not self.file:
            raise FwException("FwFile is not open")
        instrument    = ctypes.c_char_p()
        serial        = ctypes.c_char_p()
        epoch         = ctypes.c_char_p()
        v_group       = ctypes.c_double()
        sampling_time = ctypes.c_double()
        flags         = ctypes.c_ushort()
        num_facets    = ctypes.c_ushort()
        self._wrap_call(self.lib.fwifc_get_info(
            self.file,
            ctypes.byref(instrument),
            ctypes.byref(serial),
            ctypes.byref(epoch),
            ctypes.byref(v_group),
            ctypes.byref(sampling_time),
            ctypes.byref(flags),
            ctypes.byref(num_facets)
            )
        )
        inf = _Info()
        inf.instrument = instrument.value.decode("utf-8")
        inf.serial = serial.value.decode("utf-8")
        inf.epoch = epoch.value.decode("utf-8")
        inf.v_group = float(v_group.value)
        inf.sampling_time = sampling_time.value
        inf.flags = flags.value
        inf.num_facets = num_facets.value
        return inf

    def seek_time(self, time):
        '''Seek for particular record using internal time stamps.'''
        if not self.file:
            raise FwException("FwFile is not open")
        self._wrap_call(self.lib.fwifc_seek_time(self.file, ctypes.c_double(time)))

    def seek_time_external(self, time):
        '''Seek for particular record using external time stamps.'''
        if not self.file:
            raise FwException("FwFile is not open")
        self._wrap_call(self.lib.fwifc_seek_time_external(self.file, ctypes.c_double(time)))

    def seek(self, record):
        '''Seek to a particular record number.'''
        if not self.file:
            raise FwException("FwFile is not open")
        self._wrap_call(self.lib.fwifc_seek(self.file, ctypes.c_uint32(record)))

    def tell(self):
        '''Tell the current read position.'''
        if not self.file:
            raise FwException("FwFile is not open")
        record = ctypes.c_uint32()
        self._wrap_call(self.lib.fwifc_tell(self.file, ctypes.byref(record)))
        return record.value - 1

    def reindex(self):
        '''Create or re-create the index file needed for seeking.'''
        if not self.file:
            raise FwException("FwFile is not open")
        self._wrap_call(self.lib.fwifc_reindex(self.file))

    def read(self):
        '''Read data from the current read position. Return None on end.'''
        if not self.file:
            raise FwException("FwFile is not open")
        time_sorg     = ctypes.c_double()
        time_external = ctypes.c_double()
        origin        = (ctypes.c_double*3)()
        direction     = (ctypes.c_double*3)()
        flags         = ctypes.c_uint16()
        facet         = ctypes.c_uint16()
        sbl_count     = ctypes.c_uint32()
        sbl_size      = ctypes.c_uint32()
        p_sbl         = ctypes.POINTER(_C_sbl)()
        rescode = self.lib.fwifc_read(
            self.file,
            ctypes.byref(time_sorg),
            ctypes.byref(time_external),
            ctypes.byref(origin),
            ctypes.byref(direction),
            ctypes.byref(flags),
            ctypes.byref(facet),
            ctypes.byref(sbl_count),
            ctypes.byref(sbl_size),
            ctypes.byref(p_sbl)
        )
        if rescode >= 0:
            w = FwShot()
            self._wrap_call(rescode)
            w.time_sorg = time_sorg.value
            w.time_external = time_external.value
            w.origin = [origin[0], origin[1], origin[2]]
            w.direction = [direction[0], direction[1], direction[2]]
            w.flags = flags.value
            w.facet = facet.value
            w._sampling_time = self.sampling_time
            for i in range(sbl_count.value):
                s = FwSegment()
                s.time_sosbl = p_sbl[i].time_sosbl
                s.data = [p_sbl[i].sample[j] for j in range(p_sbl[i].sample_count)]
                channel = p_sbl[i].channel
                w.channel[channel].append(s)
            return w
        else:
            return None

    def get_calib(self, table_kind):
        count = ctypes.c_uint32()
        p_abscissa = ctypes.POINTER(ctypes.c_double)()
        p_ordinate = ctypes.POINTER(ctypes.c_double)()
        self._wrap_call(self.lib.fwifc_get_calib(
            self.file,
            ctypes.c_uint16(table_kind),
            ctypes.byref(count),
            ctypes.byref(p_abscissa),
            ctypes.byref(p_ordinate))
        )
        a = []
        o = []
        for i in range(count.value):
            a.append(p_abscissa[i])
            o.append(p_ordinate[i])
        return [a,o]
