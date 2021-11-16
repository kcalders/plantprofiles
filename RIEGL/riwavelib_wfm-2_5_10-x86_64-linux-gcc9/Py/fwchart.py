#!/usr/bin/env python

# This is an example demonstrating the use of the full wave interface to
# Riegl *.wfm files.
# The program is looking for input from the clipboard. RiProcess when started
# with the /copyinfo switch, will put information about the current point
# not only into RiProcess info window but also to the clipboard.
#
# This example works togehther with the fwifc.py example wrapper.
# Other requirements: python 3.x, matplotlib, numpy, pyqt4.
# 
# PLEASE NOTE: This program is to be understood as an example program, it is
# not a production grade piece of software. If you find an error, please contact
# us at support@riegl.com and/or send us a bug fix.


from fwifc import fw_array
import sys, os, json
from matplotlib.backends import qt_compat
use_pyside = qt_compat.QT_API == qt_compat.QT_API_PYSIDE
if use_pyside:
    from PySide import QtGui, QtCore
else:
    from PyQt4 import QtGui, QtCore
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import numpy as np

progname = "fwchart"
progversion = "0.1"

settings = QtCore.QSettings("Riegl_LMS", progname)

class ParameterDisplay(QtGui.QGroupBox):
    
    def __init__(self, parent):
        QtGui.QGroupBox.__init__(self, "Parameter", parent)
        self.parent = parent
        self.gl = QtGui.QGridLayout()
        self.numrows = 0
        self.project = self.addRow("Project:")
        self.wavefile = self.addRow("File:")
        self.timestamp = self.addRow("Time stamp:")
        self.PRCS_xyz = self.addRow("PRCS x,y,z:")
        self.SOCS_xyz = self.addRow("SOCS x,y,z:")
        self.SOCS_rtp = self.addRow("SOCS range,theta,phi:")
        self.gl.setRowStretch(self.numrows,1)
        self.setLayout(self.gl)

    def addRow(self, name):
        self.gl.addWidget(QtGui.QLabel(name), self.numrows, 0)
        w = QtGui.QLabel()
        self.gl.addWidget(w, self.numrows, 1)
        self.numrows += 1
        return w
        
    def update_display(self, parm):
        self.project.setText(parm["project-name"])
        fn = parm["laser-data-wfm-file"]
        self.wavefile.setText(os.path.split(fn)[1])
        self.parent.setWindowTitle("%s: %s" %(progname, os.path.split(fn)[0]))
        self.timestamp.setText("%.9f" % (parm["point-time-stamp"]))        
        x = parm["point-prcs-x"]
        y = parm["point-prcs-y"]
        z = parm["point-prcs-z"]
        self.PRCS_xyz.setText("%4.3f, %4.3f, %4.3f" %(x ,y, z))
        x = parm["point-socs-x"]
        y = parm["point-socs-y"]
        z = parm["point-socs-z"]
        self.SOCS_xyz.setText("%4.3f, %4.3f, %4.3f" %(x, y, z))
        r = parm["point-socs-range"]
        t = parm["point-socs-theta"]
        p = parm["point-socs-phi"]
        self.SOCS_rtp.setText("%4.3f, %4.3f, %4.3f" %(r, t, p))
        
        
class ChartCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""

    def __init__(self, parent=None, width=5, height=4, dpi=100):
        fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = fig.add_subplot(111)
        self.axes.hold(True)
        FigureCanvas.__init__(self, fig)
        self.setParent(parent)
        FigureCanvas.setSizePolicy(self,
                                   QtGui.QSizePolicy.Expanding,
                                   QtGui.QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)
        self.parent = parent        
        self.filename = None
        self.fw = None
        
    def __del__(self):
        del self.fw        
        
    def update_chart(self, parm):
        fn = parm["laser-data-wfm-file"]
        if fn != self.filename:
            self.parent.statusBar().showMessage("opening:"+fn, 2000)
            if self.fw:
                self.fw.close()            
                del self.fw            
            self.fw = fw_array(fn)
            self.filename = fn
            self.fw.seek(len(self.fw)-1)
            shot = self.fw.read()
            # since we cannot know for sure whether day or week seconds are used
            # we have to probe the file for the largest time stamp        
            self.time_external_max = shot.time_external
        shot_time = parm["point-time-stamp"]
        if (shot_time > self.time_external_max):
            shot_time = shot_time % (24*3600) # try day seconds
        t_point = parm["point-socs-range"]*2/self.fw.v_group
        self.fw.seek_time_external(shot_time)
        shot = self.fw.read()
        if shot:
            t_sorg = shot.time_sorg
            t_ref = shot.channel[3][0].time_sosbl + t_sorg
            t_point = t_point + t_ref
            chan_low = None
            chan_high = None
            self.axes.clear() # clear chart (see axes.hold)
            # find the sample blocks containingthe clicked target 
            # please note: sosbl is used in relative mode
            while True:
                for sbl in shot.channel[1]:
                    t_a = sbl.time_sosbl + t_sorg
                    t_b = t_a + shot._sampling_time*len(sbl.data)
                    if t_a < t_point and t_point < t_b:
                        chan_low = sbl
                        chan_low.time_sosbl = t_a
                for sbl in shot.channel[0]:
                    t_a = sbl.time_sosbl + t_sorg
                    t_b = t_a + shot._sampling_time*len(sbl.data)
                    if t_a < t_point and t_point < t_b:
                        chan_high = sbl
                        chan_high.time_sosbl = t_a
                shot = self.fw.read()
                t_sorg = shot.time_sorg
                if t_sorg > t_point: # limit the search
                    break
            if chan_low:
                t = np.array([x*shot._sampling_time + chan_low.time_sosbl for x in range(len(chan_low.data))])
                t = t - t_ref                
                y = np.array(chan_low.data)
                self.axes.plot(t*0.5*self.fw.v_group, y, 'g-')
            if chan_high:
                t = np.array([x*shot._sampling_time + chan_high.time_sosbl for x in range(len(chan_high.data))])
                t = t- t_ref                
                y = np.array(chan_high.data)
                self.axes.plot(t*0.5*self.fw.v_group, y, 'r-')
            self.axes.axvline((t_point-t_ref)*0.5*self.fw.v_group, color='b')
            self.axes.grid(True)
            self.axes.set_xlabel("range in m")
            self.draw()

class Main(QtGui.QWidget):
    """A canvas that updates from clipboard events."""

    def __init__(self, parent):
        QtGui.QWidget.__init__(self, parent)
        
        self.chart = ChartCanvas(parent, width=10, height=5, dpi=100)        
        self.parameter = ParameterDisplay(parent)
        
        layout = QtGui.QHBoxLayout()
        layout.addWidget(self.chart)
        layout.addWidget(self.parameter)
        self.setLayout(layout)        

        self.clipboard = QtGui.QApplication.clipboard()
        self.clipboard.dataChanged.connect(self.on_clipboard_changed)
        self.parent = parent
        self.basedir = None

        self.clickdata = None
        self.fwname = None        
         
    def on_clipboard_changed(self):
        if (self.clipboard.mimeData().hasText()):
         try: # if the clipboard is a valid json string
             self.clickdata = json.loads(self.clipboard.text().replace("\\","/"))
             #self.clickdata = json.loads(self.clipboard.text())  #TODO: activate instead of above when #2127 is fixed
             # try to locate the wfm file                
             if not "laser-data-wfm-file" in self.clickdata:
                 fwname = self.clickdata["laser-data-rxp-file"]
                 fwname = os.path.splitext(fwname)[0]+".wfm"
                 self.clickdata["laser-data-wfm-file"] = fwname
             self.parameter.update_display(self.clickdata)
             self.chart.update_chart(self.clickdata)
         except ValueError as e:
             pass
         

class ApplicationWindow(QtGui.QMainWindow):
    
    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        
        self.file_menu = QtGui.QMenu('&File', self)
        self.file_menu.addAction('&Quit', self.fileQuit,
                                 QtCore.Qt.CTRL + QtCore.Qt.Key_Q)
        self.menuBar().addMenu(self.file_menu)

        self.help_menu = QtGui.QMenu('&Help', self)
        self.menuBar().addSeparator()
        self.menuBar().addMenu(self.help_menu)

        self.help_menu.addAction('&About', self.about)

        self.main_widget = Main(self)
        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)
        
        self.setWindowTitle("%s: %s" %(progname, self.main_widget.basedir))
        
        self.statusBar().showMessage("fwchart started", 2000)

    def fileQuit(self):
        self.close()

    def closeEvent(self, ce):
        self.fileQuit()

    def about(self):
        QtGui.QMessageBox.about(self, "About",
"""fwchart for display of full waveforms
Copyright 2016 Roland Schwarz @ riegl lms
This program works with RiProcess."""
        )

def main():
    qApp = QtGui.QApplication(sys.argv)

    aw = ApplicationWindow()
    aw.show()
    sys.exit(qApp.exec_())

if __name__ == "__main__":
    main()