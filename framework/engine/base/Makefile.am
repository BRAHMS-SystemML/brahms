# Build libbrahmsFraEngBase.so or .a
CLEANFILES = *~

INCLUDES = -I$(top_srcdir) -I$(top_builddir) -I../../public

lib_LTLIBRARIES = libbrahms_engineBase.la

libbrahms_engineBase_la_SOURCES = core.cpp ipm.cpp os.cpp text.cpp \
                                  constants.cpp brahms_error.cpp \
                                  brahms_math.cpp output.cpp thread.cpp

libbrahms_engineBase_la_LDFLAGS = -version-info 1:0:0

# Or maybe nothing:
pkginclude_HEADERS = base.h
