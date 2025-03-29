# Our target dynamic library for this module
lib_LTLIBRARIES = 
noinst_LTLIBRARIES =

# Includes header files that is interface headers which are implemented in other distributed libraries/packages
include_HEADERS = sw/itc-api/if/itc.h

lib_LTLIBRARIES += \
		libitcplatform.la

# This will be adding sub-module dynamic libraries
libitcplatform_la_LIBADD = 

# Invoke sub-Makefile.am
# include sw/itc-api/src/Makefile.am
include sw/itc-common/src/Makefile.am
# include sw/itc-server/src/Makefile.am

libitcplatform_la_SOURCES =

###
#
# # Define interfaces from other modules which is used in this module
# libitcplatform_la_LIBADD += -litc -lutils -lmodule2 -lmodulex
# 
###