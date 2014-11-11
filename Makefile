# Makefile for the QC package

# Cx   executables of stand-alone C   program
# CPPx executables of stand-alone C++ program

Cx    =  diskUsage.x fModTime.x  unixTime.x
CPPx  =  check_CORDEX_standard-table.x convert_CF-standard-names.x \
         getNC_att.x getStatus.x syncFiles.x testValidNC.x

UTIL_SOURCE = hdhC.cpp ReadLine.cpp Split.cpp Statistics.cpp GetOpt_hdh.cpp
UTIL_HEADER = hdhC.h readline.h split.h statistics.h getopt_hdh.h

BASE_SOURCE = Annotation.cpp Base.cpp BraceOP.cpp Date.cpp \
   InFile.cpp NcAPI.cpp TimeControl.cpp Variable.cpp
BASE_HEADER = annotation.h base.h brace_op.h data_statistics.h date.h \
   geo_meta.h in_file.h iobj.h matrix_array.h nc_api.h \
   time_control.h variable.h

QC_SOURCE = CF.cpp CellStatistics.cpp FD_interface.cpp Oper.cpp OutFile.cpp Parse.cpp \
   QC.cpp QC_data.cpp QC_time.cpp QC_PT.cpp qC_main.cpp
QC_HEADER = cell_statistics.h cf.h fd_interface.h oper.h out_file.h parse.h \
   qc.h qc_data.h qc_PT.h qc_main.h qc_time.h

#vpath %.c   $(QC_PATH)/src
#vpath %.cpp $(QC_PATH)/src
#vpath %.h   $(QC_PATH)/include

VPATH = $(QC_PATH)/src $(QC_PATH)/include
all:    c-prog cpp-prog $(PRJ_NAME)

c-prog: $(Cx)

diskUsage.x: diskUsage.c
fModTime.x: fModTime.c
unixTime.x: unixTime.c

%.x: %.c
	$(CC) $(CFLAGS) -o $@ $<

cpp-prog: $(CPPx)

getNC_att.x: $(UTIL_SOURCE) $(UTIL_HEADER) brace_op.h BraceOP.cpp nc_api.h \
     annotation.h Annotation.cpp NcAPI.cpp getNC_att.cpp
getStatus.x: $(UTIL_SOURCE) $(UTIL_HEADER) getStatus.cpp
syncFiles.x: $(UTIL_SOURCE) $(UTIL_HEADER) date.h Date.cpp \
     nc_api.h NcAPI.cpp syncFiles.cpp
testValidNC.x: $(UTIL_SOURCE) $(UTIL_HEADER) nc_api.h NcAPI.cpp testValidNC.cpp

%.x: %.cpp
	$(CXX) $(CXXFLAGS) -o $@  $< \
	   -I $(QC_PATH)/include $(INCLUDE) \
           $(LIB) $(LIBDL) -ludunits2 -lnetcdf -lhdf5_hl -lhdf5 -lz -luuid

${PRJ_NAME}: ${PRJ_NAME}.x

${PRJ_NAME}.x: ${BASE_SOURCE} ${BASE_HEADER} $(QC_SOURCE) $(QC_HEADER) $(UTIL_SOURCE) $(UTIL_HEADER)
	$(CXX) $(CXXFLAGS) -o ${PRJ_NAME}.x $(QC_PATH)/src/qC_main.cpp \
           -I $(QC_PATH)/include $(INCLUDE) \
           $(LIB) $(LIBDL) -ludunits2 -lnetcdf -lhdf5_hl -lhdf5 -lz

#          -DSVN_VERSION=$(SVN_VERSION) \

CF-checker: cf-checker.x

cf-checker.x: cF_main.cpp CF.cpp $(BASE_SOURCE) ${BASE_SOURCE} $(QC_HEADER) $(UTIL_SOURCE) $(UTIL_HEADER)
	$(CXX) $(CXXFLAGS) -o cf-checker.x $(QC_PATH)/src/cF_main.cpp \
           -I $(QC_PATH)/include $(INCLUDE) \
           $(LIB) $(LIBDL) -ludunits2 -lnetcdf -lhdf5_hl -lhdf5 -lz
