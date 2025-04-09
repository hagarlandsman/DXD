#CXX=`root-config --cxx`
#CXXFLAGS=`root-config --cflags`
#LDFLAGS=`root-config --ldflags`
#LDLIBS=`root-config --glibs`  -lSpectrum -lz
#ROOTLIBS='-lRooFit -lHtml -lMinuit -lRooFitCore -lRooStats -lHistFactory'

LDLIBS = -lm -lpthread -lz

CC = g++
CFLAGS  = -g -Wall -DLINUX -I. -I./include/. -I../include/.
am_wavedump_OBJECTS =  ./XeDAQ.o ./WDconfig.o ./WDplot.o ./X742CorrectionRoutines.o ./fft.o ./keyb.o ./X742DecodeRoutines.o
am_wavescript_OBJECTS = ./XeScript.o ./WDconfig.o ./WDplot.o ./X742CorrectionRoutines.o ./fft.o ./keyb.o ./X742DecodeRoutines.o
wavedump_OBJECTS = $(am_wavedump_OBJECTS)
wavedump_DEPENDENCIES =
DEFAULT_INCLUDES = -I../include -I ./include
wavedump_LDADD = -lCAENComm -lCAENDigitizer -lm -lpthread
iseg_LIB =  -L/home/access/Direxdaq/VME/CAENComm-1.2/lib -L/usr/X11R6/lib64 -L/home/access/Direxdaq/isegControl/isegHAL -L/home/access/Direxdaq/isegControl/isegHAL/isegHalRemoteClient -L/home/access/Direxdaq/isegControl/WIENER_SNMP -I/home/access/Direxdaq/isegControl/isegControl-build -lQt5Widgets -L/usr/lib/x86_64-linux-gnu -lQt5Network  -lQt5Gui -lQt5Core -lGL -lpthread -l CAENComm
LIBSS = -lncurses
AM_CFLAGS = -fPIC
CCLD = $(CC)
LINK_ISEG =$(CCLD) -m64 -Wl,-O1 -o $@
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@
ISEG_C= -c -m64 -pipe -O2 -Wall -W -D_REENTRANT -fPIE -DQT_NO_CAST_FROM_ASCII -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_NETWORK_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I/usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++-64 -I. -Ichanneltable -I/usr/include/qt5 -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtNetwork -I/usr/include/qt5/QtGui -I/usr/include/qt5/QtCore -I. -Iinclude/ -Ihardware/

./%.o: ./%.cpp  $(DEPS)
	echo "hi"
	$(CC) $(CFLAGS) $(CXXFLAGS) -o $@ -c $<

influx:  ./influx.o
	$(LINK) ./influx.o -L/home/access/anaconda3/lib -lcurl $(wavedump_OBJECTS) -I/home/access/anaconda3/include $(wavedump_LDADD) $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS) -I/home/access/anaconda3/include


disco: ./XeDisco.o  ./disco.o
	$(LINK) ./disco.o ./XeDisco.o $(wavedump_OBJECTS) $(wavedump_LDADD) $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)

scaler: ./XeScaler.o ./scaler.o
	$(LINK) ./XeScaler.o ./scaler.o  $(wavedump_OBJECTS) $(wavedump_LDADD) $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)


hvset: ./HVset.o $(ISEG_O)
	$(LINK_ISEG) ./HVset.o $(iseg_LIB) $(ISEG_O)  $(wavedump_OBJECTS) $(wavedump_LDADD) $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)

moni:  ./XeDAQ.o ./WDconfig.o ./WDplot.o ./X742CorrectionRoutines.o ./fft.o ./keyb.o ./moni.o
	$(LINK) ./moni.o $(wavedump_OBJECTS) $(wavedump_LDADD) $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)

XeBinToTxt: ./XeBinToTxt.o   ./XeOffline.o ./X742CorrectionRoutines.o  ./X742DecodeRoutines.o
	$(LINK) ./XeBinToTxt.o ./XeOffline.o ./X742DecodeRoutines.o ./X742CorrectionRoutines.o  $(wavedump_LDADD)  $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)



XeBinAna: ./XeBinAna.o   ./XeOffline.o ./X742CorrectionRoutines.o  ./X742DecodeRoutines.o
	$(LINK) ./XeBinAna.o ./XeOffline.o ./X742DecodeRoutines.o ./X742CorrectionRoutines.o  $(wavedump_LDADD)  $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)

XeOfflib: ./XeOfflib.o ./X742CorrectionRoutines.o  ./X742DecodeRoutines.o
	$(LINK) ./XeOfflib.o ./X742DecodeRoutines.o ./X742CorrectionRoutines.o  $(LDLIBS) -fPIC  --enable-shared --enable-threads

XeBinToRoot: ./XeBinToRoot.o   ./XeOffline.o ./X742CorrectionRoutines.o  ./X742DecodeRoutines.o
	$(LINK) ./XeBinToRoot.o ./XeOffline.o ./X742DecodeRoutines.o ./X742CorrectionRoutines.o  $(wavedump_LDADD)  $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)

XeBinStat: ./XeBinStat.o   ./XeOffline.o ./X742CorrectionRoutines.o  ./X742DecodeRoutines.o
	$(LINK) ./XeBinStat.o ./XeOffline.o ./X742DecodeRoutines.o ./X742CorrectionRoutines.o  $(wavedump_LDADD)  $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)

xegui: ./xegui.o   ./XeOffline.o ./X742CorrectionRoutines.o  ./X742DecodeRoutines.o
	$(LINK) ./xegui.o ./XeOffline.o ./X742DecodeRoutines.o ./X742CorrectionRoutines.o  $(wavedump_LDADD)  $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS) -lGui

moniScan: ./moniScan.o ./XeDAQ.o ./WDconfig.o ./WDplot.o ./X742CorrectionRoutines.o ./fft.o ./keyb.o
	$(LINK) ./moniScan.o $(wavedump_OBJECTS) $(wavedump_LDADD) $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)

xescript:	./XeScript.o ./WDconfig.o ./WDplot.o ./X742CorrectionRoutines.o ./fft.o ./keyb.o
	$(LINK) $(wavescript_OBJECTS) $(wavedump_LDADD) $(LIBS) $(DEFAULT_INCLUDES)

xedumpv2: ./XeDumpv2.o ./WDconfig.o ./WDplot.o ./X742CorrectionRoutines.o ./fft.o ./keyb.o
	$(LINK) $(wavedump_OBJECTS) $(wavedump_LDADD) $(LIBS) $(DEFAULT_INCLUDES)


$(MAIN):	$(OBJS)
	$(LINK) $(wavedump_OBJECTS) $(wavedump_LDADD) $(LIBS)  $(DEFAULT_INCLUDES) ss

#
clean:
	$(RM) count *.o *~



DxDataExtractor: ./DigDataExt.o   ./DxDataExtractor.o  ./cnpy.o  ./X742CorrectionRoutines.o  ./X742DecodeRoutines.o
	$(LINK) ./DigDataExt.o  ./DxDataExtractor.o ./cnpy.o  ./X742DecodeRoutines.o ./X742CorrectionRoutines.o  $(wavedump_LDADD)  $(LIBS) $(CXXFLAGS) $(DEFAULT_INCLUDES) $(LDLIBS)

