noinst_LTLIBRARIES += libitccommon.la
libitcplatform_la_LIBADD += libitccommon.la

libitccommon_la_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

itccommon_COMMON_SOURCES 	= \
				sw/itc-common/src/itcMutex.cc \
				sw/itc-common/src/itcCWrapper.cc \
				sw/itc-common/src/itcThreadManager.cc \
				sw/itc-common/src/itcFileSystem.cc \
				sw/itc-common/src/itcTransportLocal.cc \
				sw/itc-common/src/itcTransportLSocket.cc \
				sw/itc-common/src/itcTransportSysvMsgQueue.cc

###
#
# itccommon_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitccommon_la_SOURCES = $(itccommon_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitccommon_la_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###