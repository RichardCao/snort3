## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libnetwork_inspectors.a

libnetwork_inspectors_a_SOURCES = \
network_inspectors.cc \
network_inspectors.h

# FIXIT LIBADD here causes undefineds
#libnetwork_inspectors_a_LIBADD = \
#arp_spoof/libarp_spoof.a \
#binder/libbinder.a \
#normalize/libnormalize.a \
#perf_monitor/libperf_monitor.a \
#port_scan/libport_scan.a

SUBDIRS = \
arp_spoof \
binder \
normalize \
perf_monitor \
port_scan

AM_CXXFLAGS = @AM_CXXFLAGS@

